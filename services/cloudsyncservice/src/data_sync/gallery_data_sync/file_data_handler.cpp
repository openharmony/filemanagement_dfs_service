/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "file_data_handler.h"

#include <cstring>
#include <filesystem>
#include <set>
#include <string>
#include <tuple>
#include <map>
#include <dirent.h>
#include <sys/statvfs.h>
#include <utime.h>

#include "cloud_file_utils.h"
#include "data_sync_const.h"
#include "dfs_error.h"
#include "dfsu_timer.h"
#include "directory_ex.h"
#include "dk_assets_downloader.h"
#include "dk_error.h"
#include "gallery_album_const.h"
#include "gallery_file_const.h"
#include "gallery_sysevent.h"
#include "media_column.h"
#include "medialibrary_errno.h"
#include "medialibrary_rdb_utils.h"
#include "medialibrary_type_const.h"
#include "meta_file.h"
#include "shooting_mode_column.h"
#include "thumbnail_const.h"
#include "task_state_manager.h"
#include "vision_column.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace DriveKit;
using namespace Media;
using ChangeType = OHOS::AAFwk::ChangeInfo::ChangeType;
using PAC = Media::PhotoAlbumColumns;
using PM = Media::PhotoMap;
using PC = Media::PhotoColumn;

constexpr uint32_t DAY_TO_SECONDS = 24 * 60 * 60;
constexpr uint32_t TO_MILLISECONDS = 1000;
static const string HMDFS_PATH_PREFIX = "/mnt/hmdfs/";
static const string CLOUD_MERGE_VIEW_PATH_SUFFIX = "/account/cloud_merge_view";
static const string TMP_SUFFIX = ".temp.download";
static const double START_AGING_SIZE = 0.5;
static const double STOP_AGING_SIZE = 0.6;
static const double LOWEST_START_AGING_SIZE = 0.15;
static const double LOWEST_STOP_AGING_SIZE = 0.2;
static const int64_t UNIT = 1000;
static const int64_t STD_UNIT = 1024;
static const int64_t THRESHOLD = 512;

static string GetCloudMergeViewPath(int32_t userId, const string &relativePath)
{
    return HMDFS_PATH_PREFIX + to_string(userId) + CLOUD_MERGE_VIEW_PATH_SUFFIX + relativePath;
}

static int64_t GetTotalSize()
{
    struct statvfs diskInfo;
    int ret = statvfs("/data", &diskInfo);
    if (ret != 0) {
        LOGE("get totalsize failed, errno:%{public}d", errno);
        return E_GET_SIZE_ERROR;
    }
    int64_t totalSize =
        static_cast<long long>(diskInfo.f_bsize) * static_cast<long long>(diskInfo.f_blocks);
    return totalSize;
}

static int64_t GetFreeSize()
{
    struct statvfs diskInfo;
    int ret = statvfs("/data", &diskInfo);
    if (ret != 0) {
        LOGE("get feeesize failed, errno:%{public}d", errno);
        return E_GET_SIZE_ERROR;
    }
    int64_t freeSize =
        static_cast<long long>(diskInfo.f_bsize) * static_cast<long long>(diskInfo.f_bfree);
    return freeSize;
}

static int64_t GetRoundSize(int64_t size)
{
    uint64_t val = 1;
    int64_t multple = UNIT;
    int64_t stdMultiple = STD_UNIT;
    while (static_cast<int64_t>(val) * stdMultiple < size) {
        val <<= 1;
        if (val > THRESHOLD) {
            val = 1;
            multple *= UNIT;
            stdMultiple *= STD_UNIT;
        }
    }
    return static_cast<int64_t>(val) * multple;
}

constexpr int DEFAULT_DOWNLOAD_THUMB_LIMIT = 500;
FileDataHandler::FileDataHandler(int32_t userId, const string &bundleName,
                                 std::shared_ptr<RdbStore> rdb, shared_ptr<bool> stopFlag)
    : RdbDataHandler(userId, bundleName, TABLE_NAME, rdb, stopFlag), userId_(userId), bundleName_(bundleName)
{
    cloudPrefImpl_.GetInt(DOWNLOAD_THUMB_LIMIT, downloadThumbLimit_);
    if (downloadThumbLimit_ <= 0) {
        downloadThumbLimit_ = DEFAULT_DOWNLOAD_THUMB_LIMIT;
        cloudPrefImpl_.SetInt(DOWNLOAD_THUMB_LIMIT, downloadThumbLimit_);
    }
}

void FileDataHandler::GetFetchCondition(FetchCondition &cond)
{
    cond.limitRes = LIMIT_SIZE;
    cond.recordType = recordType_;
    cond.fullKeys = desiredKeys_;
    if (GetCheckFlag()) {
        cond.desiredKeys = checkedKeys_;
    } else {
        cond.desiredKeys = desiredKeys_;
    }
}

int32_t FileDataHandler::GetRetryRecords(std::vector<DriveKit::DKRecordId> &records)
{
    NativeRdb::AbsRdbPredicates retryPredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    retryPredicates.EqualTo(PC::PHOTO_DIRTY, to_string(static_cast<int32_t>(DirtyType::TYPE_RETRY)));
    retryPredicates.EqualTo(PC::PHOTO_CLEAN_FLAG, to_string(static_cast<int32_t>(NOT_NEED_CLEAN)));
    retryPredicates.Limit(LIMIT_SIZE);

    auto results = Query(retryPredicates, {PhotoColumn::PHOTO_CLOUD_ID});
    if (results == nullptr) {
        LOGE("get nullptr modified result");
        return E_RDB;
    }

    while (results->GoToNextRow() == 0) {
        string record;
        int ret = DataConvertor::GetString(PhotoColumn::PHOTO_CLOUD_ID, record, *results);
        if (ret == E_OK) {
            records.emplace_back(record);
        }
    }

    return E_OK;
}

void FileDataHandler::AppendToDownload(NativeRdb::ResultSet &local,
                                       const std::string &fieldKey,
                                       std::vector<DKDownloadAsset> &assetsToDownload)
{
    DKDownloadAsset downloadAsset;
    downloadAsset.recordType = recordType_;
    downloadAsset.fieldKey = fieldKey;
    string filePath;
    int ret = DataConvertor::GetString(PhotoColumn::MEDIA_FILE_PATH, filePath, local);
    if (ret != E_OK) {
        LOGE("Get file path failed");
        return;
    }
    string recordId;
    ret = DataConvertor::GetString(PhotoColumn::PHOTO_CLOUD_ID, recordId, local);
    if (ret != E_OK) {
        LOGE("cannot get cloud_id fron result set");
        return;
    }
    downloadAsset.recordId = recordId;

    const string &suffix = (fieldKey == "lcd") ? LCD_SUFFIX : THUMB_SUFFIX;
    downloadAsset.downLoadPath = createConvertor_.GetThumbPath(filePath, suffix) + TMP_SUFFIX;
    downloadAsset.asset.assetName = MetaFile::GetFileName(downloadAsset.downLoadPath);
    downloadAsset.downLoadPath = MetaFile::GetParentDir(downloadAsset.downLoadPath);
    ForceCreateDirectory(downloadAsset.downLoadPath);
    assetsToDownload.push_back(downloadAsset);
}

int32_t FileDataHandler::GetAssetsToDownload(vector<DriveKit::DKDownloadAsset> &outAssetsToDownload)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.EqualTo(Media::PhotoColumn::PHOTO_SYNC_STATUS,
                       to_string(static_cast<int32_t>(SyncStatusType::TYPE_DOWNLOAD)));
    predicates.EqualTo(PC::PHOTO_CLEAN_FLAG, to_string(static_cast<int32_t>(NOT_NEED_CLEAN)));
    auto results = Query(predicates, MEDIA_CLOUD_SYNC_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr TYPE_DOWNLOAD result");
        return E_RDB;
    }

    while (results->GoToNextRow() == 0) {
        AppendToDownload(*results, "lcd", outAssetsToDownload);
        AppendToDownload(*results, "thumbnail", outAssetsToDownload);
    }
    return E_OK;
}

const std::vector<std::string> PULL_QUERY_COLUMNS = {
    PhotoColumn::MEDIA_FILE_PATH,
    PhotoColumn::MEDIA_SIZE,
    PhotoColumn::MEDIA_DATE_MODIFIED,
    PhotoColumn::PHOTO_DIRTY,
    PhotoColumn::MEDIA_DATE_TRASHED,
    PhotoColumn::PHOTO_POSITION,
    PhotoColumn::PHOTO_CLOUD_ID,
    PhotoColumn::PHOTO_CLOUD_VERSION,
    MediaColumn::MEDIA_ID,
    PhotoColumn::MEDIA_RELATIVE_PATH,
    PhotoColumn::MEDIA_DATE_ADDED,
    PhotoColumn::PHOTO_META_DATE_MODIFIED,
    PhotoColumn::MEDIA_FILE_PATH,
    PhotoColumn::PHOTO_SYNC_STATUS,
    PhotoColumn::PHOTO_THUMB_STATUS,
    PhotoColumn::MEDIA_NAME,
};

tuple<shared_ptr<NativeRdb::ResultSet>, map<string, int>> FileDataHandler::QueryLocalByCloudId(
    const vector<string> &recordIds)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.In(PhotoColumn::PHOTO_CLOUD_ID, recordIds);
    auto resultSet = Query(predicates, PULL_QUERY_COLUMNS);
    auto recordIdRowIdMap = map<string, int>();
    if (resultSet == nullptr) {
        LOGE("get nullptr created result");
        return {nullptr, std::move(recordIdRowIdMap)};
    }
    int rowCount = 0;
    int ret = resultSet->GetRowCount(rowCount);
    if (ret != 0 || rowCount < 0) {
        LOGE("result set get row count err %{public}d, rowCount %{public}d", ret, rowCount);
        return {nullptr, std::move(recordIdRowIdMap)};
    }
    int columnIndex = 0;
    resultSet->GetColumnIndex(PhotoColumn::PHOTO_CLOUD_ID, columnIndex);
    for (int rowId = 0; rowId < rowCount; ++rowId) {
        resultSet->GoToNextRow();
        string recordId = "";
        resultSet->GetString(columnIndex, recordId);
        recordIdRowIdMap.insert(make_pair(recordId, rowId));
    }
    return {std::move(resultSet), std::move(recordIdRowIdMap)};
}

const std::vector<std::string> CLEAN_QUERY_COLUMNS = {
    PhotoColumn::MEDIA_FILE_PATH,
    PhotoColumn::PHOTO_DIRTY,
    PhotoColumn::MEDIA_DATE_TRASHED,
    PhotoColumn::PHOTO_POSITION,
    PhotoColumn::PHOTO_CLOUD_ID,
};


static int32_t GetFileId(NativeRdb::ResultSet &local)
{
    int32_t fileId;
    int ret = DataConvertor::GetInt(MediaColumn::MEDIA_ID, fileId, local);
    if (ret != E_OK) {
        LOGE("Get file id failed");
        return 0; // 0:invalid file id, not exits in db
    }
    return fileId;
}

int32_t FileDataHandler::HandleRecord(shared_ptr<vector<DKRecord>> &records, OnFetchParams &params,
    vector<string> &recordIds, const std::shared_ptr<NativeRdb::ResultSet> &resultSet,
    const std::map<std::string, int> &recordIdRowIdMap)
{
    int32_t ret = E_OK;
    uint64_t success = 0;
    uint64_t rdbFail = 0;
    uint64_t dataFail = 0;
    vector<DKRecord> insertRecords;

    for (auto &record : *records) {
        int32_t fileId = 0;
        ChangeType changeType = ChangeType::INVAILD;
        if ((recordIdRowIdMap.find(record.GetRecordId()) == recordIdRowIdMap.end()) && !record.GetIsDelete()) {
            CompensateFilePath(record);
            insertRecords.emplace_back(record);
        } else if (recordIdRowIdMap.find(record.GetRecordId()) != recordIdRowIdMap.end()) {
            resultSet->GoToRow(recordIdRowIdMap.at(record.GetRecordId()));
            if (record.GetIsDelete()) {
                ret = PullRecordDelete(record, *resultSet);
                fileId = GetFileId(*resultSet);
                changeType = ChangeType::DELETE;
            } else {
                ret = PullRecordUpdate(record, *resultSet, params);
                fileId = GetFileId(*resultSet);
                changeType = ChangeType::UPDATE;
                UpdateAssetInPhotoMap(record, fileId);
            }
        }
        if (ret != E_OK) {
            LOGE("recordId %s error %{public}d", record.GetRecordId().c_str(), ret);
            if (ret == E_RDB) {
                rdbFail++;
                continue;
            }
            /* might need to specifiy which type error */
            dataFail++;
            ret = E_OK;
        } else {
            /* not include insert, update insert successful stat after batch insert */
            if (changeType != ChangeType::INSERT && changeType != ChangeType::INVAILD) {
                success++;
            }
        }
        if (changeType != ChangeType::INVAILD) {
            string notifyUri = PHOTO_URI_PREFIX + to_string(fileId);
            DataSyncNotifier::GetInstance().TryNotify(notifyUri, changeType, to_string(fileId));
        }
    }

    UpdateMetaStat(INDEX_DL_META_SUCCESS, success);
    UpdateMetaStat(INDEX_DL_META_ERROR_RDB, rdbFail);
    UpdateMetaStat(INDEX_DL_META_ERROR_DATA, dataFail);

    /* batch processing insertion */
    PullRecordsInsert(insertRecords, params);
    return ret;
}

int32_t FileDataHandler::OnFetchRecords(shared_ptr<vector<DKRecord>> &records, OnFetchParams &params)
{
    LOGI("on fetch %{public}zu records", records->size());
    int32_t ret = E_OK;
    auto recordIds = vector<string>();
    for (auto &record : *records) {
        recordIds.push_back(record.GetRecordId());
    }
    auto [resultSet, recordIdRowIdMap] = QueryLocalByCloudId(recordIds);
    if (resultSet == nullptr) {
        UpdateMetaStat(INDEX_DL_META_ERROR_RDB, recordIds.size());
        return E_RDB;
    }
    ret = FileDataHandler::HandleRecord(records, params, recordIds, resultSet, recordIdRowIdMap);
    resultSet->Close();
    LOGI("before BatchInsert size len %{public}zu, map size %{public}zu", params.insertFiles.size(),
        params.recordAlbumMaps.size());
    if (!params.insertFiles.empty() || !params.recordAlbumMaps.empty() || !params.recordAnalysisAlbumMaps.empty()) {
        int64_t rowId;
        ret = BatchInsert(rowId, TABLE_NAME, params.insertFiles);
        if (ret != E_OK) {
            LOGE("batch insert failed return %{public}d", ret);
            UpdateMetaStat(INDEX_DL_META_ERROR_RDB, params.insertFiles.size());
            ret = E_RDB;
            params.assetsToDownload.clear();
            return ret;
        } else {
            UpdateMetaStat(INDEX_DL_META_SUCCESS, params.insertFiles.size());
            BatchInsertAssetMaps(params);
            BatchInsertAssetAnalysisMaps(params);
        }
    }
    UpdateAlbumInternal();
    LOGI("after BatchInsert ret %{public}d", ret);
    DataSyncNotifier::GetInstance().TryNotify(PHOTO_URI_PREFIX, ChangeType::INSERT,
                                              INVALID_ASSET_ID);
    DataSyncNotifier::GetInstance().FinalNotify();
    MetaFileMgr::GetInstance().ClearAll();
    return ret;
}

int32_t FileDataHandler::CompensateFilePath(DriveKit::DKRecord &record)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end() || data.find(FILE_SIZE) == data.end()) {
        LOGE("record data cannot find attributes or size");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap attributes;
    data[FILE_ATTRIBUTES].GetRecordMap(attributes);
    if (attributes.find(PhotoColumn::MEDIA_FILE_PATH) == attributes.end()) {
        LOGE("record data cannot find some attributes, may came from old devices");
        string fullPath;
        int ret = CalculateFilePath(record, fullPath);
        if (ret != E_OK) {
            return ret;
        }
        attributes[PhotoColumn::MEDIA_FILE_PATH] = DriveKit::DKRecordField(fullPath);
        data[FILE_ATTRIBUTES] = DriveKit::DKRecordField(attributes);
        record.SetRecordData(data);
    }
    return E_OK;
}

static int GetDentryPathName(const string &fullPath, string &outPath, string &outFilename)
{
    const string sandboxPrefix = "/storage/cloud";
    size_t pos = fullPath.find_first_of(sandboxPrefix);
    size_t lpos = fullPath.find_last_of("/");
    if (pos != 0 || pos == string::npos || lpos == string::npos) {
        LOGE("invalid path %{private}s", fullPath.c_str());
        return E_INVAL_ARG;
    }

    outPath = fullPath.substr(sandboxPrefix.length(), lpos - sandboxPrefix.length());
    outPath = (outPath == "") ? "/" : outPath;
    outFilename = fullPath.substr(lpos + 1);
    return E_OK;
}

static int32_t DentryInsert(int userId, const DKRecord &record)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end() || data.find(FILE_SIZE) == data.end()) {
        LOGE("record data cannot find attributes or size");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap attibutes;
    data[FILE_ATTRIBUTES].GetRecordMap(attibutes);
    if (attibutes.find(PhotoColumn::MEDIA_FILE_PATH) == attibutes.end()) {
        LOGE("record data cannot find some attributes");
        return E_OK;
    }

    string fullPath;
    string relativePath;
    string fileName;
    if (attibutes[PhotoColumn::MEDIA_FILE_PATH].GetString(fullPath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        return E_INVAL_ARG;
    }
    if (GetDentryPathName(fullPath, relativePath, fileName) != E_OK) {
        LOGE("split to dentry path failed, path:%s", fullPath.c_str());
        return E_INVAL_ARG;
    }

    int64_t isize;
    if (DataConvertor::GetLongComp(data[PhotoColumn::MEDIA_SIZE], isize) != E_OK) {
        LOGE("bad size in props");
        return E_INVAL_ARG;
    }
    int64_t mtime = static_cast<int64_t>(record.GetEditedTime()) / MILLISECOND_TO_SECOND;
    string rawRecordId = record.GetRecordId();
    string cloudId = MetaFileMgr::RecordIdToCloudId(rawRecordId);
    auto mFile = MetaFileMgr::GetInstance().GetMetaFile(userId, relativePath);
    MetaBase mBaseLookup(fileName);
    MetaBase mBase(fileName, cloudId);
    mBase.size = static_cast<uint64_t>(isize);
    mBase.mtime = static_cast<uint64_t>(mtime);
    if (mFile->DoLookup(mBaseLookup) == E_OK) {
        LOGE("dentry exist when insert, do update instead");
        return mFile->DoUpdate(mBase);
    }
    return mFile->DoCreate(mBase);
}

int FileDataHandler::DentryInsertThumb(const string &fullPath,
                                       const string &recordId,
                                       uint64_t size,
                                       uint64_t mtime,
                                       const string &type)
{
    string thumbnailPath = createConvertor_.GetThumbPathInCloud(fullPath, type);
    string relativePath;
    string fileName;
    if (GetDentryPathName(thumbnailPath, relativePath, fileName) != E_OK) {
        LOGE("split to dentry path failed, path:%s", thumbnailPath.c_str());
        return E_INVAL_ARG;
    }

    string cloudId = MetaFileMgr::RecordIdToCloudId(recordId);
    auto mFile = MetaFileMgr::GetInstance().GetMetaFile(userId_, relativePath);
    MetaBase mBaseLookup(fileName);
    MetaBase mBase(fileName, cloudId);
    mBase.size = size;
    mBase.mtime = mtime;
    mBase.fileType = (type == THUMB_SUFFIX) ? FILE_TYPE_THUMBNAIL : FILE_TYPE_LCD;
    if (mFile->DoLookup(mBaseLookup) == E_OK) {
        LOGI("dentry exist when insert, do update instead");
        return mFile->DoUpdate(mBase);
    }
    return mFile->DoCreate(mBase);
}

int FileDataHandler::DentryRemoveThumb(const string &downloadPath)
{
    string thumbnailPath = createConvertor_.GetLocalPathToCloud(downloadPath);
    string relativePath;
    string fileName;
    if (GetDentryPathName(thumbnailPath, relativePath, fileName) != E_OK) {
        LOGE("split to dentry path failed, path:%s", thumbnailPath.c_str());
        return E_INVAL_ARG;
    }
    auto mFile = MetaFileMgr::GetInstance().GetMetaFile(userId_, relativePath);
    MetaBase mBase(fileName, "");
    int ret = mFile->DoRemove(mBase);
    if (ret != E_OK) {
        LOGE("remove dentry failed, ret:%{public}d", ret);
    }

    string cloudMergeViewPath = GetCloudMergeViewPath(userId_, relativePath + "/" + mBase.name);
    if (remove(cloudMergeViewPath.c_str()) != 0) {
        LOGD("update kernel dentry cache fail, errno: %{public}d, cloudMergeViewPath: %{public}s",
             errno, cloudMergeViewPath.c_str());
    }
    return E_OK;
}

int FileDataHandler::AddCloudThumbs(const DKRecord &record)
{
    LOGD("thumbs of %s add to cloud_view", record.GetRecordId().c_str());
    constexpr uint64_t THUMB_SIZE = 2 * 1024 * 1024; // thumbnail and lcd size show as 2MB
    uint64_t thumbSize = THUMB_SIZE;
    uint64_t lcdSize = THUMB_SIZE;

    DKRecordData data;
    DKAsset val;
    record.GetRecordData(data);
    if ((data.find(FILE_THUMBNAIL) != data.end()) &&
        (data[FILE_THUMBNAIL].GetAsset(val) == DKLocalErrorCode::NO_ERROR) && (val.size != 0)) {
        thumbSize = val.size;
    } else {
        LOGE("record data cannot get FILE_THUMBNAIL");
    }
    if ((data.find(FILE_LCD) != data.end()) &&
        (data[FILE_LCD].GetAsset(val) == DKLocalErrorCode::NO_ERROR) && (val.size != 0)) {
        lcdSize = val.size;
    } else {
        LOGE("record data cannot get FILE_LCD");
    }
    DriveKit::DKRecordFieldMap attributes;
    string fullPath;
    if (data.find(FILE_ATTRIBUTES) != data.end()) {
        data[FILE_ATTRIBUTES].GetRecordMap(attributes);
    }
    if (attributes.find(PhotoColumn::MEDIA_FILE_PATH) == attributes.end() ||
        attributes[PhotoColumn::MEDIA_FILE_PATH].GetString(fullPath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file path in record");
        return E_INVAL_ARG;
    }

    int64_t mtime = static_cast<int64_t>(record.GetEditedTime()) / MILLISECOND_TO_SECOND;
    int ret = DentryInsertThumb(fullPath, record.GetRecordId(), thumbSize, mtime, THUMB_SUFFIX);
    if (ret != E_OK) {
        LOGE("dentry insert of thumb failed ret %{public}d", ret);
        return ret;
    }
    ret = DentryInsertThumb(fullPath, record.GetRecordId(), lcdSize, mtime, LCD_SUFFIX);
    if (ret != E_OK) {
        LOGE("dentry insert of lcd failed ret %{public}d", ret);
        return ret;
    }
    return E_OK;
}

string FileDataHandler::ConflictRenameThumb(string fullPath, string &tmpPath, string &newPath)
{
    /* thumb new path */
    tmpPath = cleanConvertor_.GetThumbPath(fullPath, THUMB_SUFFIX);
    if (tmpPath == "") {
        LOGE("err tmp Path for GetThumbPath");
        return "";
    }
    size_t tmpfound = tmpPath.find_last_of('/');
    if (tmpfound == string::npos) {
        LOGE("err tmp Path found");
        return "";
    }
    tmpPath.resize(tmpfound);
    size_t found = tmpPath.find_last_of('.');
    if (found == string::npos) {
        LOGE("err thumb Path found");
        return "";
    }
    newPath = tmpPath.substr(0, found) + CON_SUFFIX + tmpPath.substr(found);
    /* new name */
    size_t namefound = newPath.find_last_of('/');
    if (namefound == string::npos) {
        LOGE("err name Path found");
        return "";
    }
    string newName = newPath.substr(namefound + 1);
    return newName;
}

int32_t FileDataHandler::ConflictRenamePath(string &fullPath, string &rdbPath, string &localPath, string &newLocalPath)
{
    /* sandbox new path */
    size_t rdbfound = fullPath.find_last_of('.');
    if (rdbfound == string::npos) {
        LOGE("err rdb Path found");
        return E_INVAL_ARG;
    }
    rdbPath = fullPath.substr(0, rdbfound) + CON_SUFFIX + fullPath.substr(rdbfound);
    /* local new path */
    localPath = cleanConvertor_.GetHmdfsLocalPath(fullPath);
    if (localPath == "") {
        LOGE("err local Path for GetHmdfsLocalPath");
        return E_INVAL_ARG;
    }
    size_t localfound = localPath.find_last_of('.');
    if (localfound == string::npos) {
        LOGE("err local Path found");
        return E_INVAL_ARG;
    }
    newLocalPath = localPath.substr(0, localfound) + CON_SUFFIX + localPath.substr(localfound);
    return E_OK;
}

static int RenameFilePath(const std::string &oldPath, const std::string &newPath)
{
    int ret = rename(oldPath.c_str(), newPath.c_str());
    if (ret != 0) {
        LOGE("err rename localPath to newPath, ret=%{public}d, errno=%{public}d, path=%{public}s",
             ret, errno, oldPath.c_str());
        return E_INVAL_ARG;
    }
    return ret;
}

int32_t FileDataHandler::ConflictRename(string &fullPath, string &relativePath)
{
    string rdbPath;
    string tmpPath;
    string newPath;
    string localPath;
    string newLocalPath;
    int ret = ConflictRenamePath(fullPath, rdbPath, tmpPath, newPath);
    if (ret != E_OK) {
        LOGE("ConflictRenamePath failed, ret=%{public}d", ret);
        return E_INVAL_ARG;
    }

    string newName = ConflictRenameThumb(fullPath, localPath, newLocalPath);
    if (newName == "") {
        LOGE("err Rename Thumb path");
        return E_INVAL_ARG;
    }

    /* thumb new path and new name */
    ret = RenameFilePath(tmpPath, newPath);
    if (ret != E_OK) {
        return ret;
    }
    ret = RenameFilePath(localPath, newLocalPath);
    if (ret != 0) {
        (void)RenameFilePath(newPath, tmpPath);
    }

    ValuesBucket values;
    values.PutString(PhotoColumn::MEDIA_FILE_PATH, rdbPath);
    if (!relativePath.empty()) {
        string newvirPath = relativePath + newName;
        values.PutString(PhotoColumn::MEDIA_VIRTURL_PATH, newvirPath);
    }
    string whereClause = PhotoColumn::MEDIA_FILE_PATH + " = ?";
    int updateRows = -1;
    ret = Update(updateRows, values, whereClause, {fullPath});
    if (updateRows <= 0  || ret != E_OK) {
        (void)RenameFilePath(newPath, tmpPath);
        (void)RenameFilePath(newLocalPath, localPath);
        LOGE("update retry flag failed, updateRows=%{public}d, ret=%{public}d", updateRows, ret);
        return E_RDB;
    }
    return E_OK;
}

void FileDataHandler::HandleShootingMode(const DriveKit::DKRecord &record, const NativeRdb::ValuesBucket &valuebucket,
    OnFetchParams &params)
{
    string shootingMode = "";
    ValueObject valueObject;
    if (valuebucket.GetObject(Media::PhotoColumn::PHOTO_SHOOTING_MODE, valueObject)) {
        valueObject.GetString(shootingMode);
    }
    if (!shootingMode.empty()) {
        params.recordAnalysisAlbumMaps[record.GetRecordId()] = stoi(shootingMode);
    }
}

int32_t FileDataHandler::ConflictDataMerge(DKRecord &record, string &fullPath, bool upflag)
{
    int updateRows;
    ValuesBucket values;
    OnFetchParams params;
    values.PutString(PhotoColumn::PHOTO_CLOUD_ID, record.GetRecordId());
    values.PutInt(PhotoColumn::PHOTO_POSITION, POSITION_BOTH);
    values.PutInt(PC::PHOTO_CLEAN_FLAG, NOT_NEED_CLEAN);
    if (upflag) {
        values.PutInt(PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(DirtyType::TYPE_MDIRTY));
    } else {
        createConvertor_.RecordToValueBucket(record, values);
        values.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
        values.PutInt(PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(DirtyType::TYPE_SYNCED));
    }
    HandleShootingMode(record, values, params);
    string whereClause = PhotoColumn::MEDIA_FILE_PATH + " = ?";
    int32_t ret = Update(updateRows, values, whereClause, {fullPath});
    if (ret != E_OK) {
        LOGE("update retry flag failed, ret=%{public}d", ret);
        return E_RDB;
    }
    BatchInsertAssetAnalysisMaps(params);
    return E_OK;
}

int32_t FileDataHandler::ConflictHandler(NativeRdb::ResultSet &resultSet,
                                         const DKRecord &record,
                                         int64_t isize,
                                         bool &modifyPathflag)
{
    string localId;
    int ret = DataConvertor::GetString(PhotoColumn::PHOTO_CLOUD_ID, localId, resultSet);
    if (ret != E_OK) {
        LOGE("Get cloud id failed");
        return E_INVAL_ARG;
    }
    if (ret == E_OK && !localId.empty() && localId != record.GetRecordId()) {
        LOGI("clould id not NULL %{public}s", localId.c_str());
        modifyPathflag = true;
    }
    int64_t localIsize = 0;
    ret = DataConvertor::GetLong(PhotoColumn::MEDIA_SIZE, localIsize, resultSet);
    if (ret != E_OK) {
        LOGE("Get local isize failed");
        return E_INVAL_ARG;
    }
    int64_t localCrtime = 0;
    ret = DataConvertor::GetLong(PhotoColumn::MEDIA_DATE_ADDED, localCrtime, resultSet);
    if (ret != E_OK) {
        LOGE("Get local ctime failed");
        return E_INVAL_ARG;
    }
    int64_t crTime = static_cast<int64_t>(record.GetCreateTime());
    if (localIsize == isize && localCrtime == crTime) {
        LOGD("Possible duplicate files");
    } else {
        modifyPathflag = true;
    }
    return E_OK;
}

int32_t FileDataHandler::ConflictDifferent(NativeRdb::ResultSet &resultSet,
                                           const DKRecord &record,
                                           string &fullPath,
                                           string &relativePath)
{
    LOGD("Different files with the same name");
    /* Modify path */
    int ret = ConflictRename(fullPath, relativePath);
    if (ret != E_OK) {
        LOGE("Conflict dataMerge rename fail, path :%{public}s", fullPath.c_str());
        return ret;
    }
    return E_OK;
}

int32_t FileDataHandler::ConflictMerge(NativeRdb::ResultSet &resultSet,
                                       DKRecord &record,
                                       string &fullPath,
                                       bool &comflag,
                                       int64_t &imetaModified)
{
    LOGI("Unification of the same document");
    /* flag for update local data */
    bool upflag = false;
    int64_t localMeta = 0;
    int ret = DataConvertor::GetLong(PhotoColumn::PHOTO_META_DATE_MODIFIED, localMeta, resultSet);
    if (ret != E_OK) {
        LOGE("Get local meta data modified failed");
        return E_INVAL_ARG;
    }
    if (imetaModified < localMeta) {
        upflag = true;
    }
    /* update database */
    ret = ConflictDataMerge(record, fullPath, upflag);
    if (ret != E_OK) {
        LOGE("Conflict dataMerge fail");
        return ret;
    }
    comflag = true;
    return E_OK;
}

int32_t FileDataHandler::GetConflictData(const DKRecord &record,
                                         string &fullPath,
                                         int64_t &isize,
                                         int64_t &imetaModified,
                                         string &relativePath)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end() || data.find(FILE_SIZE) == data.end()) {
        LOGE("record data cannot find attributes or size");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap attributes;
    data[FILE_ATTRIBUTES].GetRecordMap(attributes);
    if (attributes.find(PhotoColumn::MEDIA_FILE_PATH) == attributes.end()) {
        LOGE("record data cannot find some attributes, may came from old devices");
        return E_INVAL_ARG;
    }
    if (attributes[PhotoColumn::MEDIA_FILE_PATH].GetString(fullPath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in attributes");
        return E_INVAL_ARG;
    }
    if (attributes[PhotoColumn::MEDIA_RELATIVE_PATH].GetString(relativePath) != DKLocalErrorCode::NO_ERROR) {
        LOGD("bad virtual_Path in attributes");
        return E_INVAL_ARG;
    }
    if (DataConvertor::GetLongComp(data[PhotoColumn::MEDIA_SIZE], isize) != E_OK) {
        LOGE("bad size in data");
        return E_INVAL_ARG;
    }
    if (attributes[PhotoColumn::PHOTO_META_DATE_MODIFIED].GetLong(imetaModified) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad meta data modified in attributes");
        return E_INVAL_ARG;
    }
    return E_OK;
}

int32_t FileDataHandler::CalculateFilePath(DKRecord &record, string &filePath)
{
    // method reference from medialibrary
    int32_t mediaType;
    int32_t ret = GetMediaType(record, mediaType);
    if (ret != E_OK) {
        return E_INVAL_ARG;
    }
    int32_t uniqueId = GetAssetUniqueId(mediaType);
    if (uniqueId < 0) {
        return E_RDB;
    }
    int32_t errCode = CreateAssetPathById(record, uniqueId, mediaType, filePath);
    return errCode;
}

int32_t FileDataHandler::GetMediaType(DKRecord &record, int32_t &mediaType)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_FILETYPE) == data.end()) {
        LOGE("record data cannot find properties");
        return E_INVAL_ARG;
    }
    int32_t fileType;
    if (data[FILE_FILETYPE].GetInt(fileType) != DKLocalErrorCode::NO_ERROR) {
        LOGE("record data cannot find fileType");
        return E_INVAL_ARG;
    }
    mediaType = fileType == FILE_TYPE_VIDEO ? MediaType::MEDIA_TYPE_VIDEO : MediaType::MEDIA_TYPE_IMAGE;
    return E_OK;
}

int32_t FileDataHandler::GetAssetUniqueId(int32_t &type)
{
    string typeString;
    switch (type) {
        case MediaType::MEDIA_TYPE_IMAGE:
            typeString += IMAGE_ASSET_TYPE;
            break;
        case MediaType::MEDIA_TYPE_VIDEO:
            typeString += VIDEO_ASSET_TYPE;
            break;
        default:
            LOGE("This type %{public}d can not get unique id", type);
            return MediaType::MEDIA_TYPE_FILE;
    }
    std::lock_guard<std::mutex> lock(rdbMutex_);
    string sql = " UPDATE UniqueNumber SET unique_number = unique_number + 1  WHERE media_type = ?";
    std::vector<NativeRdb::ValueObject> bingArgs;
    bingArgs.emplace_back(typeString);
    int32_t ret = ExecuteSql(sql, bingArgs);
    if (ret != E_OK) {
        LOGI("update unique number fail");
        return -1;
    }
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(ASSET_UNIQUE_NUMBER_TABLE);
    predicates.SetWhereClause(ASSET_MEDIA_TYPE + " = ?");
    predicates.SetWhereArgs({typeString});
    auto resultSet = Query(predicates, {UNIQUE_NUMBER});
    int32_t uniqueId = 0;
    if (resultSet->GoToNextRow() == 0) {
        if (DataConvertor::GetInt(UNIQUE_NUMBER, uniqueId, *resultSet) != E_OK) {
            LOGI("query unique number fail");
            return -1;
        }
    }
    return uniqueId;
}

int32_t FileDataHandler::CreateAssetPathById(DKRecord &record, int32_t &uniqueId,
    int32_t &mediaType, string &filePath)
{
    string mediaDirPath = "Photo/";
    int32_t bucketNum = 0;
    int32_t errCode = CreateAssetBucket(uniqueId, bucketNum);
    if (errCode != E_OK) {
        return errCode;
    }

    string realName;
    errCode = CreateAssetRealName(uniqueId, mediaType, GetFileExtension(record), realName);
    if (errCode != E_OK) {
        return errCode;
    }
    string dirPath = ROOT_MEDIA_DIR + mediaDirPath + to_string(bucketNum);
    filePath = dirPath + "/" + realName;
    return E_OK;
}

int32_t FileDataHandler::CreateAssetBucket(int32_t &uniqueId, int32_t &bucketNum)
{
    if (uniqueId < 0) {
        LOGE("input uniqueId [%{private}d] is invalid", uniqueId);
        return E_INVAL_ARG;
    }
    int start = ASSET_DIR_START_NUM;
    int divider = ASSET_DIR_START_NUM;
    while (uniqueId > start * ASSET_IN_BUCKET_NUM_MAX) {
        divider = start;
        start <<= 1;
    }

    int fileIdRemainder = uniqueId % divider;
    if (fileIdRemainder == 0) {
        bucketNum = start + fileIdRemainder;
    } else {
        bucketNum = (start - divider) + fileIdRemainder;
    }
    return E_OK;
}

int32_t FileDataHandler::CreateAssetRealName(int32_t &fileId, int32_t &mediaType,
    const string &extension, string &name)
{
    string fileNumStr = to_string(fileId);
    if (fileId <= ASSET_MAX_COMPLEMENT_ID) {
        size_t fileIdLen = fileNumStr.length();
        fileNumStr = ("00" + fileNumStr).substr(fileIdLen - 1);
    }

    string mediaTypeStr;
    switch (mediaType) {
        case MediaType::MEDIA_TYPE_IMAGE:
            mediaTypeStr = DEFAULT_IMAGE_NAME;
            break;
        case MediaType::MEDIA_TYPE_VIDEO:
            mediaTypeStr = DEFAULT_VIDEO_NAME;
            break;
        default:
            LOGE("This mediatype %{public}d can not get real name", mediaType);
            return E_INVAL_ARG;
    }

    name = mediaTypeStr + to_string(UTCTimeSeconds()) + "_" + fileNumStr + "." + extension;
    return E_OK;
}

int32_t FileDataHandler::PullRecordConflict(DKRecord &record, bool &comflag)
{
    LOGD("judgment downlode conflict");
    string fullPath, relativePath;
    int64_t isize, imetaModified;
    int32_t ret = GetConflictData(record, fullPath, isize, imetaModified, relativePath);
    if (ret != E_OK) {
        LOGE("Getdata fail");
        return ret;
    }
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.SetWhereClause(PhotoColumn::MEDIA_FILE_PATH + " = ?");
    predicates.SetWhereArgs({fullPath});
    predicates.Limit(LIMIT_SIZE);
    auto resultSet = Query(predicates, PULL_QUERY_COLUMNS);
    if (resultSet == nullptr) {
        LOGE("get nullptr created result");
        return E_RDB;
    }
    int32_t rowCount = 0;
    ret = resultSet->GetRowCount(rowCount);
    if (ret != 0) {
        LOGE("result set get row count err %{public}d", ret);
        return E_RDB;
    }
    if (rowCount == 0) {
        LOGD("Normal download process");
        return E_OK;
    }
    if (rowCount == 1) {
        resultSet->GoToNextRow();
        bool modifyPathflag = false;
        ret = ConflictHandler(*resultSet, record, isize, modifyPathflag);
        if (ret != E_OK) {
            return ret;
        }
        if (modifyPathflag) {
            ret = ConflictDifferent(*resultSet, record, fullPath, relativePath);
        } else {
            ret = ConflictMerge(*resultSet, record, fullPath, comflag, imetaModified);
        }
        if (comflag) {
            UpdateAssetInPhotoMap(record, GetFileId(*resultSet));
        }
    } else {
        LOGE("unknown error: PullRecordConflict(),same path rowCount = %{public}d", rowCount);
        ret = E_RDB;
    }
    return ret;
}

string FileDataHandler::GetFileExtension(DKRecord &record)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_FILE_NAME) == data.end()) {
        LOGE("record data cannot find properties");
        return "";
    }
    string displayName;
    if (data[FILE_FILE_NAME].GetString(displayName) != DKLocalErrorCode::NO_ERROR) {
        LOGE("record data cannot find fileType");
        return "";
    }
    string extension;
    if (!displayName.empty()) {
        string::size_type pos = displayName.find_last_of('.');
        if (pos == string::npos) {
            return "";
        }
        extension = displayName.substr(pos + 1);
    }
    return extension;
}

static bool IfContainsFullField(const DriveKit::DKRecord &record)
{
    DriveKit::DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) != data.end()) {
        DriveKit::DKRecordFieldMap attributes;
        data[FILE_ATTRIBUTES].GetRecordMap(attributes);
        return !(attributes.find(PhotoColumn::MEDIA_TITLE) == attributes.end());
    }
    return false;
}

static string GetDisplayName(const DKRecord &record)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_FILE_NAME) == data.end()) {
        LOGE("record data cannot find filename");
        return "";
    }
    string displayName;
    if (data[FILE_FILE_NAME].GetString(displayName) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract displayname failed");
        return "";
    }
    return displayName;
}

tuple<shared_ptr<ResultSet>, int32_t> FileDataHandler::BatchQueryLocal(vector<DKRecord> &records)
{
    vector<string> displayNames;
    for (auto &record : records) {
        string displayName = GetDisplayName(record);
        if (displayName.empty()) {
            continue;
        }
        displayNames.emplace_back(displayName);
    }

    auto vecSize = displayNames.size();
    LOGD("batch query num %{public}zu", vecSize);
    if (vecSize == 0) {
        LOGE("record not include displayname");
        return {nullptr, 0};
    }

    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.EqualTo(PhotoColumn::PHOTO_POSITION, POSITION_LOCAL);
    predicates.In(PhotoColumn::MEDIA_NAME, displayNames);
    auto resultSet = Query(predicates, PULL_QUERY_COLUMNS);
    if (resultSet == nullptr) {
        LOGE("get nullptr query result");
        return {nullptr, 0};
    }
    int32_t rowCount = 0;
    auto ret = resultSet->GetRowCount(rowCount);
    if (ret != 0) {
        LOGE("result set get row count err %{public}d", ret);
        return {nullptr, 0};
    }
    return {move(resultSet), rowCount};
}

int32_t FileDataHandler::GetLocalKeyData(ResultSet &resultSet, KeyData &keyData)
{
    int32_t ret = DataConvertor::GetString(Media::PhotoColumn::MEDIA_NAME, keyData.displayName, resultSet);
    if (ret != E_OK) {
        LOGE("Get local displayname failed");
        return E_INVAL_ARG;
    }

    ret = DataConvertor::GetLong(Media::PhotoColumn::MEDIA_SIZE, keyData.isize, resultSet);
    if (ret != E_OK) {
        LOGE("Get local isize failed");
        return E_INVAL_ARG;
    }

    ret = DataConvertor::GetLong(Media::PhotoColumn::MEDIA_DATE_ADDED, keyData.createTime, resultSet);
    if (ret != E_OK) {
        LOGE("Get local ctime failed");
        return E_INVAL_ARG;
    }

    ret = DataConvertor::GetString(Media::PhotoColumn::MEDIA_FILE_PATH, keyData.filePath, resultSet);
    if (ret != E_OK) {
        LOGE("get file path failed");
        return E_INVAL_ARG;
    }
    return E_OK;
}

int32_t FileDataHandler::GetCloudKeyData(const DKRecord &record, KeyData &keyData)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end() || data.find(FILE_FILE_NAME) == data.end() ||
        data.find(FILE_SIZE) == data.end()) {
        LOGE("record data cannot find attributes or filename or size");
        return E_INVAL_ARG;
    }
    if (data[FILE_FILE_NAME].GetString(keyData.displayName) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract displayName failed");
        return E_INVAL_ARG;
    }
    DKRecordFieldMap attributes;
    data[FILE_ATTRIBUTES].GetRecordMap(attributes);
    if (attributes.find(PhotoColumn::MEDIA_FILE_PATH) == attributes.end()) {
        LOGE("record data cannot find some attributes, may came from old devices");
        return E_INVAL_ARG;
    }
    if (attributes[PhotoColumn::MEDIA_FILE_PATH].GetString(keyData.filePath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in attributes");
        return E_INVAL_ARG;
    }
    if (data[FILE_SIZE].GetLong(keyData.isize) != DKLocalErrorCode::NO_ERROR) {
        LOGE("extract size error");
        return E_INVAL_ARG;
    }
    keyData.createTime = static_cast<int64_t>(record.GetCreateTime());
    return E_OK;
}

std::tuple<bool, bool> FileDataHandler::JudgeConflict(const KeyData &localKeyData, const KeyData &cloudKeyData)
{
    bool isMatchConflict = false;
    bool isSamePath = false;
    if ((localKeyData.displayName == cloudKeyData.displayName) && (localKeyData.isize == cloudKeyData.isize) &&
        (localKeyData.createTime == cloudKeyData.createTime)) {
        LOGI("Possible duplicate files");
        isMatchConflict = true;
    }

    if (localKeyData.filePath == cloudKeyData.filePath) {
        LOGI("same path");
        isSamePath = true;
    }

    return {isMatchConflict, isSamePath};
}

int32_t FileDataHandler::MergeRenamePath(const std::string &localFilePath, const std::string &cloudFilePath)
{
    string localOriginPath = cleanConvertor_.GetHmdfsLocalPath(localFilePath);
    if (localOriginPath == "") {
        LOGE("err local path");
        return E_INVAL_ARG;
    }

    string cloudOriginPath = cleanConvertor_.GetHmdfsLocalPath(cloudFilePath);
    if (localOriginPath == "") {
        LOGE("err cloud path");
        return E_INVAL_ARG;
    }

    ForceCreateDirectory(MetaFile::GetParentDir(cloudOriginPath));
    LOGD("rename localOriginPath:%{public}s to cloudOriginPath:%{public}s", localOriginPath.c_str(),
         cloudOriginPath.c_str());
    auto ret = rename(localOriginPath.c_str(), cloudOriginPath.c_str());
    if (ret != E_OK) {
        LOGE("err rename localOriginPath:%{public}s to cloudOriginPath:%{public}s, errno:%{public}d",
             localOriginPath.c_str(), cloudOriginPath.c_str(), errno);
        return E_INVAL_ARG;
    }
    return E_OK;
}

std::string FileDataHandler::GetThumbDir(const std::string &filePath)
{
    string thumbPath = cleanConvertor_.GetThumbPath(filePath, THUMB_SUFFIX);
    if (thumbPath == "") {
        LOGE("err tmp Path for GetThumbPath");
        return "";
    }
    size_t tmpfound = thumbPath.find_last_of('/');
    if (tmpfound == string::npos) {
        LOGE("err tmp Path found");
        return "";
    }
    return thumbPath.substr(0, tmpfound);
}

int32_t FileDataHandler::MergeRenameThumb(const std::string &localFilePath, const std::string &cloudFilePath)
{
    string localThumbPath = GetThumbDir(localFilePath);
    if (localThumbPath == "") {
        return E_INVAL_ARG;
    }
    string cloudThumbPath = GetThumbDir(cloudFilePath);
    if (cloudThumbPath == "") {
        return E_INVAL_ARG;
    }

    ForceCreateDirectory(MetaFile::GetParentDir(cloudThumbPath));
    LOGD("rename localThumbPath:%{public}s to cloudThumbPath:%{public}s", localThumbPath.c_str(),
         cloudThumbPath.c_str());
    auto ret = rename(localThumbPath.c_str(), cloudThumbPath.c_str());
    if (ret != E_OK) {
        LOGE("err rename localThumbPath:%{public}s to cloudThumbPath:%{public}s, errno:%{public}d",
             localThumbPath.c_str(), cloudThumbPath.c_str(), errno);
        return E_INVAL_ARG;
    }
    return E_OK;
}

int32_t FileDataHandler::MergeRename(const std::string &localFilePath, const std::string &cloudFilePath)
{
    auto ret = MergeRenameThumb(localFilePath, cloudFilePath);
    if (ret != E_OK) {
        return ret;
    }
    return MergeRenamePath(localFilePath, cloudFilePath);
}

int32_t FileDataHandler::DoDataMerge(DKRecord &record,
                                     std::string &localFilePath,
                                     const std::string &cloudFilePath,
                                     bool isSamePath)
{
    if (!isSamePath) {
        LOGI("diff path, local:%{public}s, cloud:%{public}s", localFilePath.c_str(), cloudFilePath.c_str());
        auto ret = MergeRename(localFilePath, cloudFilePath);
        if (ret != E_OK) {
            return ret;
        }
    }
    /* update database */
    auto ret = ConflictDataMerge(record, localFilePath, false);
    if (ret != E_OK) {
        LOGE("Conflict dataMerge fail");
        return ret;
    }
    return E_OK;
}

int32_t FileDataHandler::PullRecordsConflictProc(std::vector<DriveKit::DKRecord> &records)
{
    RETURN_ON_ERR(IsStop());
    auto [resultSet, rowCount] = BatchQueryLocal(records);
    if (!resultSet) {
        LOGE("Batch query failed");
        return E_RDB;
    }
    if (rowCount == 0) {
        LOGD("Normal download process");
        return E_OK;
    }

    LOGI("same displayName files num: %{public}d", rowCount);
    KeyData localKeyData;
    while (resultSet->GoToNextRow() == 0) {
        auto ret = GetLocalKeyData(*resultSet, localKeyData);
        if (ret != E_OK) {
            LOGE("GetLocalKey Data failed");
            continue;
        }
        KeyData cloudKeyData;
        for (auto iter = records.begin(); iter != records.end();) {
            ret = GetCloudKeyData(*iter, cloudKeyData);
            if (ret != E_OK) {
                LOGE("GetCloudKey data failed");
                ++iter;
                continue;
            }
            auto [isMatchConflict, isSamePath] = JudgeConflict(localKeyData, cloudKeyData);
            if (isMatchConflict) {
                LOGD("merge record, recordId:%{public}s", (*iter).GetRecordId().c_str());
                DoDataMerge(*iter, localKeyData.filePath, cloudKeyData.filePath, isSamePath);
                UpdateAssetInPhotoMap((*iter), GetFileId(*resultSet));
                records.erase(iter);
            } else {
                ++iter;
            }
        }
    }
    return E_OK;
}

int32_t FileDataHandler::PathConflictProc(const DriveKit::DKRecord &record)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end()) {
        LOGE("record data cannot find attributes");
        return E_INVAL_ARG;
    }

    DKRecordFieldMap attributes;
    data[FILE_ATTRIBUTES].GetRecordMap(attributes);
    if (attributes.find(PhotoColumn::MEDIA_FILE_PATH) == attributes.end()) {
        LOGE("record data cannot find some attributes");
        return E_INVAL_ARG;
    }
    string filePath;
    if (attributes[PhotoColumn::MEDIA_FILE_PATH].GetString(filePath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in attributes");
        return E_INVAL_ARG;
    }

    /* local new path */
    string localPath = cleanConvertor_.GetHmdfsLocalPath(filePath);
    if (localPath == "") {
        LOGE("err local Path for GetHmdfsLocalPath");
        return E_INVAL_ARG;
    }

    bool local = access(localPath.c_str(), F_OK) == 0;
    if (!local && errno != ENOENT) {
        LOGE("fail to access %{public}d", errno);
        return E_SYSCALL;
    }

    if (!local && errno == ENOENT) {
        LOGD("fail to access path %{public}s", localPath.c_str());
        return E_OK;
    }

    LOGE("Conflict filePath:%{public}s, errno:%{public}d", filePath.c_str(), errno);
    string relativePath("");
    return ConflictRename(filePath, relativePath);
}

void FileDataHandler::PullRecordsInsert(std::vector<DriveKit::DKRecord> &records, OnFetchParams &params)
{
    if (records.empty()) {
        return;
    }
    PullRecordsConflictProc(records);
    uint64_t dataFail = 0;
    /* insert records after filtering conflicts */
    for (auto &record : records) {
        if (IsStop()) {
            return;
        }
        LOGI("insert of record %{public}s", record.GetRecordId().c_str());
        /* different records have same path */
        auto ret = PathConflictProc(record);
        if (ret != E_OK) {
            LOGE("path conflict proc failed");
            dataFail++;
            continue;
        }

        ret = DentryInsert(userId_, record);
        if (ret != E_OK) {
            LOGE("MetaFile Create failed %{public}d", ret);
            dataFail++;
            continue;
        }

        ret = GetInsertParams(record, params);
        if (ret != E_OK) {
            LOGE("Get insert params failed %{public}d", ret);
            dataFail++;
            continue;
        }

        if (AddCloudThumbs(record) != E_OK || !IsPullRecords()) {
            AppendToDownload(record, "lcd", params.assetsToDownload);
            AppendToDownload(record, "thumbnail", params.assetsToDownload);
        } else if ((++params.totalPullCount <= downloadThumbLimit_)) {
            // the first 500 thms when pull records.
            AppendToDownload(record, "thumbnail", params.assetsToDownload);
        }

        InsertAssetToPhotoMap(record, params);
        UpdateMetaStat(INDEX_DL_META_ERROR_DATA, dataFail);
    }
    return;
}

int32_t FileDataHandler::GetInsertParams(DKRecord &record, OnFetchParams &params)
{
    ValuesBucket values;
    auto ret = createConvertor_.RecordToValueBucket(record, values);
    if (ret != E_OK) {
        LOGE("record to valuebucket failed, ret=%{public}d", ret);
        return ret;
    }
    if (!IfContainsFullField(record)) {
        values.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SDIRTY));
    } else {
        values.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    }
    values.PutInt(PC::PHOTO_CLEAN_FLAG, static_cast<int32_t>(NOT_NEED_CLEAN));
    values.PutInt(Media::PhotoColumn::PHOTO_POSITION, POSITION_CLOUD);
    values.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
    values.PutInt(Media::PhotoColumn::PHOTO_THUMB_STATUS, static_cast<int32_t>(ThumbState::TO_DOWNLOAD));
    values.PutString(Media::PhotoColumn::PHOTO_CLOUD_ID, record.GetRecordId());
    if (IsPullRecords()) {
        values.PutInt(Media::PhotoColumn::PHOTO_SYNC_STATUS, static_cast<int32_t>(SyncStatusType::TYPE_VISIBLE));
    } else {
        values.PutInt(Media::PhotoColumn::PHOTO_SYNC_STATUS, static_cast<int32_t>(SyncStatusType::TYPE_DOWNLOAD));
    }
    HandleShootingMode(record, values, params);
    params.insertFiles.push_back(values);
    return E_OK;
}

int32_t FileDataHandler::PullRecordInsert(DKRecord &record, OnFetchParams &params)
{
    RETURN_ON_ERR(IsStop());
    LOGI("insert of record %{public}s", record.GetRecordId().c_str());
    /* check local file conflict */
    bool comflag = false;
    int ret = PullRecordConflict(record, comflag);
    if (comflag) {
        LOGI("Conflict:same document no Insert");
        return E_OK;
    } else if (ret != E_OK) {
        LOGE("MetaFile Conflict failed %{public}d", ret);
    }
    ret = DentryInsert(userId_, record);
    if (ret != E_OK) {
        LOGE("MetaFile Create failed %{public}d", ret);
        return ret;
    }

    ValuesBucket values;
    ret = createConvertor_.RecordToValueBucket(record, values);
    if (ret != E_OK) {
        LOGE("record to valuebucket failed, ret=%{public}d", ret);
        return ret;
    }
    if (!IfContainsFullField(record)) {
        values.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SDIRTY));
    } else {
        values.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    }
    values.PutInt(PC::PHOTO_CLEAN_FLAG, static_cast<int32_t>(NOT_NEED_CLEAN));
    values.PutInt(Media::PhotoColumn::PHOTO_POSITION, POSITION_CLOUD);
    values.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
    values.PutInt(Media::PhotoColumn::PHOTO_THUMB_STATUS, static_cast<int32_t>(ThumbState::TO_DOWNLOAD));
    values.PutString(Media::PhotoColumn::PHOTO_CLOUD_ID, record.GetRecordId());
    if (IsPullRecords()) {
        values.PutInt(Media::PhotoColumn::PHOTO_SYNC_STATUS, static_cast<int32_t>(SyncStatusType::TYPE_VISIBLE));
    } else {
        values.PutInt(Media::PhotoColumn::PHOTO_SYNC_STATUS, static_cast<int32_t>(SyncStatusType::TYPE_DOWNLOAD));
    }
    HandleShootingMode(record, values, params);
    params.insertFiles.push_back(values);
    if (AddCloudThumbs(record) != E_OK || !IsPullRecords()) {
        AppendToDownload(record, "lcd", params.assetsToDownload);
        AppendToDownload(record, "thumbnail", params.assetsToDownload);
    } else if ((++params.totalPullCount <= downloadThumbLimit_)) {
        // the first 500 thms when pull records.
        AppendToDownload(record, "thumbnail", params.assetsToDownload);
    }
    return E_OK;
}

int32_t FileDataHandler::OnDownloadAssets(const map<DKDownloadAsset, DKDownloadResult> &resultMap)
{
    RETURN_ON_ERR(IsStop());
    uint64_t thumbError = 0;
    uint64_t lcdError = 0;

    UpdateThmVec();
    UpdateLcdVec();
    for (const auto &it : resultMap) {
        auto asset = it.first;
        if (it.second.IsSuccess()) {
            LOGI("%{public}s %{public}s %{public}s download succeed", asset.recordId.c_str(),
                asset.asset.assetName.c_str(), asset.fieldKey.c_str());
            continue;
        }

        if (it.first.fieldKey == FILE_THUMBNAIL) {
            thumbError++;
            if (IsPullRecords()) {
                continue;
            }
            // set visible when download failed
            LOGI("update sync_status to visible of record %s", it.first.recordId.c_str());
            int updateRows;
            ValuesBucket values;
            values.PutInt(Media::PhotoColumn::PHOTO_SYNC_STATUS, static_cast<int32_t>(SyncStatusType::TYPE_VISIBLE));

            string whereClause = Media::PhotoColumn::PHOTO_CLOUD_ID + " = ?";
            int32_t ret = Update(updateRows, values, whereClause, {it.first.recordId});
            if (ret != E_OK) {
                LOGE("update retry flag failed, ret=%{public}d", ret);
            }
            DataSyncNotifier::GetInstance().TryNotify(PHOTO_URI_PREFIX, ChangeType::INSERT,
                                                      to_string(updateRows));
        } else if (it.first.fieldKey == FILE_LCD) {
            lcdError++;
        }
        LOGE("%{public}s %{public}s %{public}s download fail, localErr: %{public}d, serverErr: %{public}d",
            asset.recordId.c_str(), asset.asset.assetName.c_str(), asset.fieldKey.c_str(),
            static_cast<int>(it.second.GetDKError().dkErrorCode), it.second.GetDKError().serverErrorCode);
    }

    if (thumbError > 0) {
        UpdateAttachmentStat(INDEX_THUMB_ERROR_SDK, thumbError);
    }

    if (lcdError > 0) {
        UpdateAttachmentStat(INDEX_LCD_ERROR_SDK, lcdError);
    }

    return E_OK;
}

int32_t FileDataHandler::OnDownloadAssets(const DKDownloadAsset &asset)
{
    RETURN_ON_ERR(IsStop());
    if (asset.fieldKey == "thumbnail") {
        std::lock_guard<std::mutex> lock(thmMutex_);
        thmVec_.emplace_back(asset.recordId);
    }
    if (asset.fieldKey == "lcd") {
        std::lock_guard<std::mutex> lock(lcdMutex_);
        lcdVec_.emplace_back(asset.recordId);
    }
    if (thmVec_.size() >= UPDATE_VEC_SIZE) {
        UpdateThmVec();
    }
    if (lcdVec_.size() >= UPDATE_VEC_SIZE) {
        UpdateLcdVec();
    }
    string tempPath = asset.downLoadPath + "/" + asset.asset.assetName;
    string localPath = CloudDisk::CloudFileUtils::GetPathWithoutTmp(tempPath);
    DentryRemoveThumb(localPath);
    if (rename(tempPath.c_str(), localPath.c_str()) != 0) {
        LOGE("err rename, errno: %{public}d, tmpLocalPath: %s, localPath: %s",
             errno, tempPath.c_str(), localPath.c_str());
    }
    MetaFileMgr::GetInstance().ClearAll();
    return E_OK;
}

int32_t FileDataHandler::OnDownloadAssetsFailure(const std::vector<DriveKit::DKDownloadAsset> &assets)
{
    uint64_t thumbError = 0;
    uint64_t lcdError = 0;

    /* account first */
    for (auto &asset : assets) {
        if (asset.fieldKey == FILE_THUMBNAIL) {
            thumbError++;
        } else if (asset.fieldKey == FILE_LCD) {
            lcdError++;
        }
    }

    /* update atomic value at once */
    if (thumbError > 0) {
        UpdateAttachmentStat(INDEX_THUMB_ERROR_SDK, thumbError);
    }

    if (lcdError > 0) {
        UpdateAttachmentStat(INDEX_LCD_ERROR_SDK, lcdError);
    }

    return E_OK;
}

static bool IsLocalDirty(NativeRdb::ResultSet &local)
{
    int dirty;
    int ret = DataConvertor::GetInt(PhotoColumn::PHOTO_DIRTY, dirty, local);
    if (ret != E_OK) {
        LOGE("Get dirty int failed");
        return false;
    }
    return (dirty == static_cast<int32_t>(DirtyType::TYPE_MDIRTY)) ||
           (dirty == static_cast<int32_t>(DirtyType::TYPE_FDIRTY)) ||
           (dirty == static_cast<int32_t>(DirtyType::TYPE_DELETED));
}

static string GetFilePath(NativeRdb::ResultSet &local)
{
    string filePath;
    int ret = DataConvertor::GetString(PhotoColumn::MEDIA_FILE_PATH, filePath, local);
    if (ret != E_OK) {
        LOGE("Get file path failed");
        return "";
    }
    return filePath;
}

static string GetLocalPath(int userId, const string &filePath)
{
    const string sandboxPrefix = "/storage/cloud";
    const string dfsPrefix = "/mnt/hmdfs/";
    const string dfsSuffix = "/account/cloud_merge_view";
    size_t pos = filePath.find_first_of(sandboxPrefix);
    if (pos != 0 || pos == string::npos) {
        LOGE("invalid path %{private}s", filePath.c_str());
        return "";
    }

    string dfsPath = dfsPrefix + to_string(userId) + dfsSuffix + filePath.substr(sandboxPrefix.length());
    return dfsPath;
}
int32_t FileDataHandler::SetRetry(vector<NativeRdb::ValueObject> &retryList)
{
    if (retryList.empty()) {
        return E_OK;
    }
    LOGE("abnormal data set retry!");
    std::stringstream ss;
    for (unsigned int i = 0; i < retryList.size(); i++) {
        if (ss.tellp() != 0) {
            ss << ",";
        }
        ss <<" ? ";
    }
    string sql = "UPDATE " + PC::PHOTOS_TABLE + " SET " + PC::PHOTO_DIRTY + " = " +
        to_string(static_cast<int32_t>(Media::DirtyType::TYPE_RETRY)) + " WHERE " + PC::PHOTO_CLOUD_ID +
        " IN ( " + ss.str() + " )";
    int32_t ret = ExecuteSql(sql, retryList);
    if (ret != E_OK) {
        LOGE("update retry flag failed, ret=%{public}d", ret);
        return E_RDB;
    }
    retryList.clear();
    return E_OK;
}

/**
 * Add locks to prevent multiple threads from updating albums at the same time.
 * Only set a flag in media library while using this function.
 * When data number is large, use this function.
 */
void FileDataHandler::UpdateAlbumInternal()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    MediaLibraryRdbUtils::UpdateAllAlbumsCountForCloud(GetRaw());
}

/**
 * Add locks to prevent multiple threads from updating albums at the same time.
 * Check Photos table to update all photo albums (system and user) in a synchronous operation.
 * When clean user data, use this function
 */
void FileDataHandler::UpdateAllAlbums()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    MediaLibraryRdbUtils::UpdateAllAlbumsForCloud(GetRaw());
}

int FileDataHandler::SetRetry(const string &recordId)
{
    LOGI("set retry of record %s", recordId.c_str());
    int updateRows;
    ValuesBucket values;
    values.PutInt(PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(DirtyType::TYPE_RETRY));
    string whereClause = PhotoColumn::PHOTO_CLOUD_ID + " = ?";
    int32_t ret = Update(updateRows, values, whereClause, {recordId});
    if (ret != E_OK) {
        LOGE("update retry flag failed, ret=%{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

static bool FileIsLocal(NativeRdb::ResultSet &local)
{
    int position = 0;
    int ret = DataConvertor::GetInt(PhotoColumn::PHOTO_POSITION, position, local);
    if (ret != E_OK) {
        LOGE("Get local position failed");
        return false;
    }

    return !!(static_cast<uint32_t>(position) & 1);
}

static inline int GetCloudMtime(const DKRecord &record, int64_t &dateModified)
{
    DriveKit::DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end()) {
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap attributes;
    data[FILE_ATTRIBUTES].GetRecordMap(attributes);
    if (attributes.find(FILE_EDITED_TIME_MS) == data.end()) {
        return E_INVAL_ARG;
    }
    if (attributes[FILE_EDITED_TIME_MS].GetLong(dateModified) !=  DKLocalErrorCode::NO_ERROR) {
        return E_INVAL_ARG;
    }
    return E_OK;
}

static int IsMtimeChanged(const DKRecord &record, NativeRdb::ResultSet &local, bool &changed)
{
    int64_t localMtime = 0;
    int64_t dateModified = 0;
    int ret = DataConvertor::GetLong(PhotoColumn::MEDIA_DATE_MODIFIED, localMtime, local);
    if ((ret == E_OK) && (GetCloudMtime(record, dateModified) == E_OK)) {
        LOGI("dateModified %{public}llu, localMtime %{public}llu",
            static_cast<unsigned long long>(dateModified), static_cast<unsigned long long>(localMtime));
        changed = !(dateModified == localMtime);
        return E_OK;
    }

    // get local mtime
    int64_t localCrtime = 0;
    ret = DataConvertor::GetLong(PhotoColumn::MEDIA_DATE_ADDED, localCrtime, local);
    if (ret != E_OK) {
        LOGE("Get local ctime failed");
        return E_INVAL_ARG;
    }

    // get record mtime
    int64_t crTime = static_cast<int64_t>(record.GetCreateTime());
    LOGI("cloudMtime %{public}llu, localMtime %{public}llu",
         static_cast<unsigned long long>(crTime), static_cast<unsigned long long>(localCrtime));
    changed = !(crTime == localCrtime);
    return E_OK;
}

int32_t FileDataHandler::PullRecordUpdate(DKRecord &record, NativeRdb::ResultSet &local, OnFetchParams &params)
{
    RETURN_ON_ERR(IsStop());
    LOGD("update of record %s", record.GetRecordId().c_str());
    if (IsLocalDirty(local)) {
        LOGI("local record dirty, ignore cloud update");
        return E_OK;
    }
    bool mtimeChanged = false;
    if (IsMtimeChanged(record, local, mtimeChanged) != E_OK) {
        return E_INVAL_ARG;
    }
    bool isLocal = FileIsLocal(local);
    if (isLocal) {
        if (CloudDisk::CloudFileUtils::LocalWriteOpen(GetLocalPath(userId_, GetFilePath(local)))) {
            return SetRetry(record.GetRecordId());
        }
    }
    int32_t changedRows = 0;
    RETURN_ON_ERR(UpdateRecordToDatabase(record, local, changedRows, params));
    if (isLocal && mtimeChanged && changedRows != 0) {
        LOGI("cloud file DATA changed, %s", record.GetRecordId().c_str());
        int ret = unlink(GetLocalPath(userId_, GetFilePath(local)).c_str());
        if (ret != 0) {
            LOGE("unlink local failed, errno %{public}d", errno);
        }
        DentryInsert(userId_, record);
        int32_t changePositionRows = -1;
        ValuesBucket values;
        values.PutInt(PC::PHOTO_POSITION, static_cast<int32_t>(POSITION_CLOUD));
        string whereClause = PC::PHOTO_CLOUD_ID + " = ?";
        vector<string> whereArgs = {record.GetRecordId()};
        ret = Update(changePositionRows, values, whereClause, whereArgs);
        if (ret != E_OK || changePositionRows != 1) {
            LOGE("RDB error, update fail ret is %{public}d, changeRows is %{public}d", ret, changePositionRows);
            AppendToDownload(record, "content", params.assetsToDownload);
        }
    }
    if (mtimeChanged && (ThumbsAtLocal(record) || (AddCloudThumbs(record) != E_OK)) && changedRows != 0) {
        AppendToDownload(record, "lcd", params.assetsToDownload);
        AppendToDownload(record, "thumbnail", params.assetsToDownload);
    }
    return E_OK;
}

int32_t FileDataHandler::UpdateRecordToDatabase(DriveKit::DKRecord &record,
                                                NativeRdb::ResultSet &local,
                                                int32_t &changeRows, OnFetchParams &params)
{
    int64_t localMtime = 0;
    ValuesBucket values;
    createConvertor_.RecordToValueBucket(record, values);
    int32_t ret = UpdateMediaFilePath(record, local);
    if (ret != E_OK) {
        LOGE("update media file path fail, ret = %{public}d", ret);
        values.Delete(PC::MEDIA_FILE_PATH);
    }
    ret = DataConvertor::GetLong(PhotoColumn::MEDIA_DATE_MODIFIED, localMtime, local);
    if (ret != E_OK) {
        return ret;
    }
    values.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
    values.PutInt(PhotoMap::DIRTY, static_cast<int32_t>(DirtyTypes::TYPE_SYNCED));
    values.PutInt(PC::PHOTO_CLEAN_FLAG, static_cast<int32_t>(NOT_NEED_CLEAN));
    string whereClause = PhotoColumn::PHOTO_CLOUD_ID + " = ? AND " + PC::PHOTO_DIRTY + " = ?";
    vector<std::string> whereArgs = {record.GetRecordId(),
        to_string(static_cast<int32_t>(DirtyType::TYPE_SYNCED))};
    HandleShootingMode(record, values, params);
    ret = Update(changeRows, values, whereClause, whereArgs);
    if (ret != E_OK) {
        LOGE("RDB error, update fail ret is %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t FileDataHandler::CheckContentConsistency(NativeRdb::ResultSet &resultSet)
{
    string filePath = GetFilePath(resultSet);
    string localPath = createConvertor_.GetLowerPath(filePath);

    /* local */
    bool local = access(localPath.c_str(), F_OK) == 0;
    if (!local && errno != ENOENT) {
        LOGE("fail to access %{public}d", errno);
        return E_SYSCALL;
    }

    /* cloud */
    string relativePath;
    string fileName;
    int ret = GetDentryPathName(filePath, relativePath, fileName);
    if (ret != E_OK) {
        LOGE("split to dentry path failed, path:%s", filePath.c_str());
        return ret;
    }
    auto dir = MetaFileMgr::GetInstance().GetMetaFile(userId_, relativePath);
    if (dir == nullptr) {
        return E_DATA;
    }
    MetaBase file(fileName);
    bool cloud = dir->DoLookup(file) == E_OK;

    /* check */
    if (local && cloud) {
        LOGE("[CHECK AND FIX] try to delete cloud dentry %{public}s", filePath.c_str());
        UpdateCheckAttachment(INDEX_CHECK_FOUND, 1);

        struct stat st;
        ret = stat(localPath.c_str(), &st);
        if (ret != 0) {
            LOGE("stat local file error %{public}d", errno);
            return E_SYSCALL;
        }

        if (st.st_size != static_cast<long>(file.size)) {
            ret = unlink(localPath.c_str());
            if (ret != 0) {
                LOGE("unlink local file error %{public}d", errno);
                return E_SYSCALL;
            }
        } else {
            ret = dir->DoRemove(file);
            if (ret != E_OK) {
                LOGE("remove cloud dentry fail %{public}d", ret);
                return ret;
            }
        }

        UpdateCheckAttachment(INDEX_CHECK_FIXED, 1);
    } else if (!local && !cloud) {
        LOGE("[CHECK AND FIX] try to fill in cloud dentry %{public}s", filePath.c_str());
        UpdateCheckAttachment(INDEX_CHECK_FOUND, 1);

        string cloudId;
        ret = DataConvertor::GetString(PhotoColumn::PHOTO_CLOUD_ID, cloudId, resultSet);
        if (ret != E_OK) {
            return ret;
        }
        file.cloudId = MetaFileMgr::RecordIdToCloudId(cloudId);

        int64_t val;
        ret = DataConvertor::GetLong(PhotoColumn::MEDIA_SIZE, val, resultSet);
        if (ret != E_OK) {
            return ret;
        }
        file.size = val;

        ret = DataConvertor::GetLong(PhotoColumn::MEDIA_DATE_MODIFIED, val, resultSet);
        if (ret != E_OK) {
            return ret;
        }
        file.mtime = val;

        ret = dir->DoCreate(file);
        if (ret != E_OK) {
            LOGE("create cloud dentry fail %{public}d", ret);
            return ret;
        }

        UpdateCheckAttachment(INDEX_CHECK_FIXED, 1);
    }

    return E_OK;
}

int32_t FileDataHandler::CheckThumbConsistency(NativeRdb::ResultSet &resultSet, const string &suffix)
{
    string filePath;
    int32_t ret = DataConvertor::GetString(PhotoColumn::MEDIA_FILE_PATH, filePath, resultSet);
    if (ret != E_OK) {
        LOGE("Get file path failed %{public}d", ret);
        return ret;
    }
    string thumb = createConvertor_.GetThumbPath(filePath, suffix);

    /* local */
    bool local = access(thumb.c_str(), F_OK) == 0;
    if (!local && errno != ENOENT) {
        LOGE("fail to access %{public}d", errno);
        return E_SYSCALL;
    }

    /* cloud */
    string relativePath;
    string fileName;
    string cloudPath = createConvertor_.GetThumbPathInCloud(filePath, suffix);
    if (GetDentryPathName(cloudPath, relativePath, fileName) != E_OK) {
        LOGE("split to dentry path failed, path: %s", cloudPath.c_str());
        return E_INVAL_ARG;
    }

    auto dir = MetaFileMgr::GetInstance().GetMetaFile(userId_, relativePath);
    if (dir == nullptr) {
        return E_DATA;
    }
    MetaBase file(fileName);
    bool cloud = dir->DoLookup(file) == E_OK;
    if (local && cloud) {
        LOGE("[CHECK AND FIX] try to delete cloud dentry %{public}s", cloudPath.c_str());
        UpdateCheckAttachment(INDEX_CHECK_FOUND, 1);

        struct stat st;
        ret = stat(thumb.c_str(), &st);
        if (ret != 0) {
            LOGE("stat local file error %{public}d", errno);
            return E_SYSCALL;
        }

        if (st.st_size != static_cast<long>(file.size)) {
            ret = unlink(thumb.c_str());
            if (ret != 0) {
                LOGE("unlink local file error %{public}d", errno);
                return E_SYSCALL;
            }
        } else {
            ret = dir->DoRemove(file);
            if (ret != E_OK) {
                LOGE("remove cloud dentry fail %{public}d", ret);
                return ret;
            }
        }

        UpdateCheckAttachment(INDEX_CHECK_FIXED, 1);
    } else if (!local && !cloud) {
        /* FIX ME: hardcode thumbnail size show as 2MB */
        constexpr uint64_t THUMB_SIZE = 2 * 1024 * 1024;
        file.size = THUMB_SIZE;

        LOGE("[CHECK AND FIX] try to fill in cloud dentry %{public}s", thumb.c_str());
        UpdateCheckAttachment(INDEX_CHECK_FOUND, 1);

        string cloudId;
        ret = DataConvertor::GetString(PhotoColumn::PHOTO_CLOUD_ID, cloudId, resultSet);
        if (ret != E_OK) {
            return ret;
        }
        file.cloudId = MetaFileMgr::RecordIdToCloudId(cloudId);

        int64_t val;
        ret = DataConvertor::GetLong(PhotoColumn::MEDIA_DATE_MODIFIED, val, resultSet);
        if (ret != E_OK) {
            return ret;
        }
        file.mtime = val;

        file.fileType = (suffix == THUMB_SUFFIX) ? FILE_TYPE_THUMBNAIL : FILE_TYPE_LCD;

        ret = dir->DoCreate(file);
        if (ret != E_OK) {
            LOGE("dir create file error %{public}d", ret);
            return ret;
        }

        UpdateCheckAttachment(INDEX_CHECK_FIXED, 1);
    }

    return E_OK;
}

int32_t FileDataHandler::CheckAssetConsistency(NativeRdb::ResultSet &resultSet)
{
    /* content */
    int32_t ret = CheckContentConsistency(resultSet);
    if (ret != E_OK) {
        return ret;
    }

    /* thumb */
    ret = CheckThumbConsistency(resultSet, THUMB_SUFFIX);
    if (ret != E_OK) {
        return ret;
    }

    /* lcd */
    ret = CheckThumbConsistency(resultSet, LCD_SUFFIX);
    if (ret != E_OK) {
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::CheckDirtyConsistency(NativeRdb::ResultSet &resultSet)
{
    int32_t dirty;
    int32_t ret = DataConvertor::GetInt(PhotoColumn::PHOTO_DIRTY, dirty, resultSet);
    if (ret != E_OK) {
        return ret;
    }

    if (dirty == static_cast<int32_t>(DirtyType::TYPE_FDIRTY)) {
        string filePath = GetFilePath(resultSet);
        string localPath = createConvertor_.GetLowerPath(filePath);

        bool local = access(localPath.c_str(), F_OK) == 0;
        if (!local && errno != ENOENT) {
            LOGE("fail to access %{public}d", errno);
            return E_SYSCALL;
        }
        if (!local) {
            LOGE("[CHECK AND FIX] try to set dirty as mdirty %{public}s", filePath.c_str());
            UpdateCheckFile(INDEX_CHECK_FOUND, 1);

            ValuesBucket values;
            values.PutInt(PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(DirtyTypes::TYPE_MDIRTY));
            int32_t changedRows;
            string whereClause = PhotoColumn::MEDIA_FILE_PATH + " = ?";
            ret = Update(changedRows, values, whereClause, { filePath });
            if (ret != E_OK) {
                LOGE("rdb update failed, err = %{public}d", ret);
                return E_RDB;
            }

            UpdateCheckFile(INDEX_CHECK_FIXED, 1);
        }
    }

    return E_OK;
}

int32_t FileDataHandler::CheckPositionConsistency(NativeRdb::ResultSet &resultSet)
{
    int32_t pos;
    int32_t ret = DataConvertor::GetInt(PhotoColumn::PHOTO_POSITION, pos, resultSet);
    if (ret != E_OK) {
        return ret;
    }

    string filePath = GetFilePath(resultSet);
    string localPath = createConvertor_.GetLowerPath(filePath);
    bool local = access(localPath.c_str(), F_OK) == 0;
    if (!local && errno != ENOENT) {
        LOGE("fail to access %{public}d", errno);
        return E_SYSCALL;
    }

    ValuesBucket values;
    if (pos == POSITION_CLOUD && local) {
        values.PutInt(PhotoColumn::PHOTO_POSITION, POSITION_BOTH);
    } else if (pos == POSITION_BOTH && !local) {
        values.PutInt(PhotoColumn::PHOTO_POSITION, POSITION_CLOUD);
    } else if (pos == POSITION_LOCAL && !local) {
        LOGE("position is local but local file not exist!");
        return E_OK;
    } else {
        return E_OK;
    }

    LOGE("[CHECK AND FIX] try to change position %{public}s", filePath.c_str());
    UpdateCheckFile(INDEX_CHECK_FOUND, 1);

    int32_t changedRows;
    string whereClause = PhotoColumn::MEDIA_FILE_PATH + " = ?";
    ret = Update(changedRows, values, whereClause, { filePath });
    if (ret != E_OK) {
        LOGE("rdb update failed, err = %{public}d", ret);
        return E_RDB;
    }

    UpdateCheckFile(INDEX_CHECK_FIXED, 1);

    return E_OK;
}

int32_t FileDataHandler::SetSyncStatusConsistency(string &filePath, bool thumbLocal, bool lcdLocal,
    int32_t thumbStatus, int32_t syncStatus)
{
    uint32_t expectedThumbStatus = static_cast<uint32_t>(ThumbState::DOWNLOADED);
    if (!thumbLocal) {
        expectedThumbStatus |= static_cast<uint32_t>(ThumbState::THM_TO_DOWNLOAD);
    }
    if (!lcdLocal) {
        expectedThumbStatus |= static_cast<uint32_t>(ThumbState::LCD_TO_DOWNLOAD);
    }

    ValuesBucket values;
    bool modFlag = false;
    if (thumbLocal && syncStatus != static_cast<int32_t>(SyncStatusType::TYPE_VISIBLE)) {
        modFlag = true;
        values.PutInt(PhotoColumn::PHOTO_SYNC_STATUS, static_cast<int32_t>(SyncStatusType::TYPE_VISIBLE));
    }

    if (static_cast<int32_t>(expectedThumbStatus) != thumbStatus) {
        modFlag = true;
        values.PutInt(PhotoColumn::PHOTO_THUMB_STATUS, static_cast<int32_t>(expectedThumbStatus));
    }

    if (modFlag) {
        LOGE("[CHECK AND FIX] try to change sync status %{public}s", filePath.c_str());
        UpdateCheckFile(INDEX_CHECK_FOUND, 1);
        int32_t changedRows;
        string whereClause = PhotoColumn::MEDIA_FILE_PATH + " = ?";
        int32_t ret = Update(changedRows, values, whereClause, { filePath });
        if (ret != E_OK) {
            LOGE("rdb update failed, err = %{public}d", ret);
            return E_RDB;
        }
        UpdateCheckFile(INDEX_CHECK_FIXED, 1);
    }

    return E_OK;
}

int32_t FileDataHandler::CheckSyncStatusConsistency(NativeRdb::ResultSet &resultSet)
{
    int32_t syncStatus;
    int32_t ret = DataConvertor::GetInt(PhotoColumn::PHOTO_SYNC_STATUS, syncStatus, resultSet);
    if (ret != E_OK) {
        return ret;
    }

    int32_t thumbStatus;
    ret = DataConvertor::GetInt(PhotoColumn::PHOTO_THUMB_STATUS, thumbStatus, resultSet);
    if (ret != E_OK) {
        return ret;
    }

    string filePath = GetFilePath(resultSet);
    string thumbLocalPath = createConvertor_.GetThumbPath(filePath, THUMB_SUFFIX);
    bool thumbLocal = access(thumbLocalPath.c_str(), F_OK) == 0;
    if (!thumbLocal && errno != ENOENT) {
        LOGE("fail to access thumb %{public}d", errno);
        return E_SYSCALL;
    }
    string lcdLocalPath = createConvertor_.GetThumbPath(filePath, LCD_SUFFIX);
    bool lcdLocal = access(lcdLocalPath.c_str(), F_OK) == 0;
    if (!lcdLocal && errno != ENOENT) {
        LOGE("fail to access lcd %{public}d", errno);
        return E_SYSCALL;
    }

    return SetSyncStatusConsistency(filePath, thumbLocal, lcdLocal, thumbStatus, syncStatus);
}

int32_t FileDataHandler::CheckStatusConsistency(NativeRdb::ResultSet &resultSet)
{
    /* dirty */
    int32_t ret = CheckDirtyConsistency(resultSet);
    if (ret != E_OK) {
        return ret;
    }

    /* postion */
    ret = CheckPositionConsistency(resultSet);
    if (ret != E_OK) {
        return ret;
    }

    /* sync status */
    ret = CheckSyncStatusConsistency(resultSet);
    if (ret != E_OK) {
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::CheckDataConsistency(NativeRdb::ResultSet &resultSet)
{
    /* asset */
    int32_t ret = CheckAssetConsistency(resultSet);
    if (ret == E_STOP) {
        return ret;
    }

    /* status */
    ret = CheckStatusConsistency(resultSet);
    if (ret == E_STOP) {
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::GetCheckRecords(vector<DriveKit::DKRecordId> &checkRecords,
    const shared_ptr<std::vector<DriveKit::DKRecord>> &records)
{
    auto recordIds = vector<string>();
    for (const auto &record : *records) {
        recordIds.emplace_back(record.GetRecordId());
    }
    auto [resultSet, recordIdRowIdMap] = QueryLocalByCloudId(recordIds);
    if (resultSet == nullptr) {
        return E_RDB;
    }

    for (const auto &record : *records) {
        if ((recordIdRowIdMap.find(record.GetRecordId()) == recordIdRowIdMap.end()) && !record.GetIsDelete()) {
            checkRecords.emplace_back(record.GetRecordId());
        } else if (recordIdRowIdMap.find(record.GetRecordId()) != recordIdRowIdMap.end()) {
            int32_t ret = resultSet->GoToRow(recordIdRowIdMap.at(record.GetRecordId()));
            if (ret != NativeRdb::E_OK) {
                LOGE("got to row error %{public}d", ret);
                continue;
            }

            ret = CheckDataConsistency(*resultSet);
            if (ret != E_OK) {
                LOGE("data check error");
                /* implies continue if returning non-ok */
                continue;
            }

            int64_t version = 0;
            DataConvertor::GetLong(PhotoColumn::PHOTO_CLOUD_VERSION, version, *resultSet);
            if (record.GetVersion() != static_cast<unsigned long>(version) &&
                (!IsLocalDirty(*resultSet) || record.GetIsDelete())) {
                checkRecords.emplace_back(record.GetRecordId());
            }
        } else {
            LOGE("recordId %s has multiple file in db!", record.GetRecordId().c_str());
        }
    }
    MetaFileMgr::GetInstance().ClearAll();
    return E_OK;
}

bool FileDataHandler::ThumbsAtLocal(const DKRecord &record)
{
    DKRecordData datas;
    record.GetRecordData(datas);
    if (datas.find(FILE_ATTRIBUTES) == datas.end()) {
        LOGE("record data cannot find attributes");
        return false;
    }
    DriveKit::DKRecordFieldMap attributes;
    datas[FILE_ATTRIBUTES].GetRecordMap(attributes);

    string fullPath;
    if (attributes.find(PhotoColumn::MEDIA_FILE_PATH) == attributes.end() ||
        attributes[PhotoColumn::MEDIA_FILE_PATH].GetString(fullPath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file path in record");
        return false;
    }

    string thumbLocalPath = createConvertor_.GetThumbPath(fullPath, THUMB_SUFFIX);
    return access(thumbLocalPath.c_str(), F_OK) == 0;
}

static bool FileIsRecycled(NativeRdb::ResultSet &local)
{
    int64_t localDateTrashed = 0;
    int ret = DataConvertor::GetLong(PhotoColumn::MEDIA_DATE_TRASHED, localDateTrashed, local);
    if (ret != E_OK) {
        LOGE("Get local recycled failed");
        return false;
    }

    return localDateTrashed > 0;
}

static int64_t UTCTimeMilliSeconds()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * SECOND_TO_MILLISECOND + t.tv_nsec / SECOND_TO_MILLISECOND;
}

// if cloud delete but local exist, we would do recycle instead of delete
int FileDataHandler::RecycleFile(const string &recordId)
{
    LOGI("recycle of record %s", recordId.c_str());

    ValuesBucket values;
    values.PutNull(PhotoColumn::PHOTO_CLOUD_ID);
    values.PutInt(PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(DirtyType::TYPE_NEW));
    values.PutInt(PhotoColumn::PHOTO_POSITION, POSITION_LOCAL);
    values.PutLong(PhotoColumn::MEDIA_DATE_TRASHED, UTCTimeMilliSeconds());
    values.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, 0);
    int32_t changedRows;
    string whereClause = PhotoColumn::PHOTO_CLOUD_ID + " = ?";
    int ret = Update(changedRows, values, whereClause, {recordId});
    if (ret != E_OK) {
        LOGE("rdb update failed, err=%{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

void FileDataHandler::RemoveThmParentPath(const string &filePath)
{
    string thmbFile = cleanConvertor_.GetThumbPath(filePath, THUMB_SUFFIX);
    filesystem::path thmbFilePath(thmbFile.c_str());
    filesystem::path thmbFileParentPath = thmbFilePath.parent_path();
    LOGD("filePath: %s, thmbFile: %s, thmbFileParentDir: %s", filePath.c_str(), thmbFile.c_str(),
         thmbFileParentPath.string().c_str());
    ForceRemoveDirectory(thmbFileParentPath.string());
}

int32_t FileDataHandler::PullRecordDelete(DKRecord &record, NativeRdb::ResultSet &local)
{
    LOGI("delete of record %s", record.GetRecordId().c_str());
    string filePath = GetFilePath(local);
    string localPath = GetLocalPath(userId_, filePath);

    bool isLocal = FileIsLocal(local);
    if (FileIsRecycled(local) || !isLocal) {
        if (isLocal) {
            LOGI("force delete instead");
            int ret = unlink(localPath.c_str());
            if (ret != 0) {
                LOGE("unlink local failed, errno %{public}d", errno);
            }
        } else { // delete dentry
            string relativePath;
            string fileName;
            if (GetDentryPathName(filePath, relativePath, fileName) != E_OK) {
                LOGE("split to dentry path failed, path:%s", filePath.c_str());
                return E_INVAL_ARG;
            }
            auto mFile = MetaFileMgr::GetInstance().GetMetaFile(userId_, relativePath);
            MetaBase mBase(fileName);
            int ret = mFile->DoRemove(mBase);
            if (ret != E_OK) {
                LOGE("remove dentry failed, ret:%{public}d", ret);
            }
        }
        // delete THM
        RemoveThmParentPath(filePath);
        // delete rdb
        int32_t deletedRows;
        int ret = Delete(deletedRows, Media::PhotoColumn::PHOTO_CLOUD_ID + " = ?", {record.GetRecordId()});
        if (ret != E_OK) {
            LOGE("delete in rdb failed, ret:%{public}d", ret);
            return E_INVAL_ARG;
        }
        DeleteAssetInPhotoMap(GetFileId(local));
        return E_OK;
    }

    if (IsLocalDirty(local)) {
        LOGI("local record dirty, ignore cloud delete");
        return ClearCloudInfo(record.GetRecordId());
    }

    if (CloudDisk::CloudFileUtils::LocalWriteOpen(localPath)) {
        return SetRetry(record.GetRecordId());
    }
    return RecycleFile(record.GetRecordId());
}

static int32_t DeleteMetaFile(const string &sboxPath, const int32_t &userId)
{
    // delete dentry
    string fileName;
    string relativePath;
    if (GetDentryPathName(sboxPath, relativePath, fileName) != E_OK) {
        LOGE("split to dentry path failed, path:%s", sboxPath.c_str());
        return E_INVAL_ARG;
    }

    auto mFile = MetaFileMgr::GetInstance().GetMetaFile(userId, relativePath);
    MetaBase mBase(fileName);
    int ret = mFile->DoRemove(mBase);
    if (ret != E_OK) {
        LOGE("remove dentry failed, ret:%{public}d", ret);
    }

    /*
     * after removing file item from dentryfile, we should delete file
     * of cloud merge view to update kernel dentry cache.
     */
    string cloudMergeViewPath = GetCloudMergeViewPath(userId, relativePath + "/" + mBase.name);
    if (remove(cloudMergeViewPath.c_str()) != 0) {
        LOGE("update kernel dentry cache fail, errno: %{public}d, cloudMergeViewPath: %{public}s",
             errno, cloudMergeViewPath.c_str());
    }

    return E_OK;
}

int32_t FileDataHandler::OnDownloadSuccess(const DriveKit::DKDownloadAsset &asset,
    std::shared_ptr<DriveKit::DKContext> context)
{
    string tmpLocalPath = asset.downLoadPath + "/" + asset.asset.assetName;
    string localPath = CloudDisk::CloudFileUtils::GetPathWithoutTmp(tmpLocalPath);
    string tmpSboxPath = localConvertor_.GetSandboxPath(tmpLocalPath);
    string sboxPath = CloudDisk::CloudFileUtils::GetPathWithoutTmp(tmpSboxPath);

    // delete dentry
    int ret = DeleteMetaFile(sboxPath, userId_);
    if (ret != E_OK) {
        return ret;
    }

    if (rename(tmpLocalPath.c_str(), localPath.c_str()) != 0) {
        LOGE("err rename, errno: %{public}d, tmpLocalPath: %s, localPath: %s",
             errno, tmpLocalPath.c_str(), localPath.c_str());
    }

    auto recordIds = vector<string>();
    recordIds.push_back(asset.recordId);
    auto [resultSet, recordIdRowIdMap] = QueryLocalByCloudId(recordIds);
    if (resultSet == nullptr || recordIdRowIdMap.size() != 1) {
        LOGE("QueryLocalByCloudId failed rowCount %{public}zu", recordIdRowIdMap.size());
        return E_INVAL_ARG;
    }
    resultSet->GoToNextRow();
    int64_t localMtime = 0;
    ret = DataConvertor::GetLong(PhotoColumn::MEDIA_DATE_MODIFIED, localMtime, *resultSet);
    if (ret == E_OK) {
        struct utimbuf ubuf {
            .actime = localMtime / MILLISECOND_TO_SECOND, .modtime = localMtime / MILLISECOND_TO_SECOND
        };
        LOGI("update downloaded file mtime %{public}llu", static_cast<unsigned long long>(localMtime));
        if (utime(localPath.c_str(), &ubuf) < 0) {
            LOGE("utime failed return %{public}d, localPath: %{public}s", errno, localPath.c_str());
        }
    }

    // update rdb
    ValuesBucket valuesBucket;
    valuesBucket.PutInt(Media::PhotoColumn::PHOTO_POSITION, POSITION_BOTH);

    int32_t changedRows;
    string whereClause = Media::PhotoColumn::MEDIA_FILE_PATH + " = ?";
    vector<string> whereArgs = {sboxPath};
    ret = Update(changedRows, valuesBucket, whereClause, whereArgs);
    if (ret != 0) {
        LOGE("on download file from cloud err %{public}d", ret);
    }
    return ret;
}

void FileDataHandler::AppendToDownload(const DKRecord &record,
                                       const std::string &fieldKey,
                                       std::vector<DKDownloadAsset> &assetsToDownload)
{
    DKDownloadAsset downloadAsset;
    downloadAsset.recordType = record.GetRecordType();
    downloadAsset.recordId = record.GetRecordId();
    downloadAsset.fieldKey = fieldKey;
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end()) {
        LOGE("record data cannot find attributes");
        return;
    }
    DKRecordFieldMap attributes;
    data[FILE_ATTRIBUTES].GetRecordMap(attributes);

    if (attributes.find(PhotoColumn::MEDIA_FILE_PATH) == attributes.end()) {
        LOGE("record prop cannot find file path");
        return;
    }
    string path;
    attributes[PhotoColumn::MEDIA_FILE_PATH].GetString(path);
    if (fieldKey != "content") {
        const string &suffix = fieldKey == "lcd" ? LCD_SUFFIX : THUMB_SUFFIX;
        downloadAsset.downLoadPath = createConvertor_.GetThumbPath(path, suffix) + TMP_SUFFIX;
    } else {
        downloadAsset.downLoadPath = createConvertor_.GetLowerTmpPath(path);
    }
    downloadAsset.asset.assetName = MetaFile::GetFileName(downloadAsset.downLoadPath);
    downloadAsset.downLoadPath = MetaFile::GetParentDir(downloadAsset.downLoadPath);
    ForceCreateDirectory(downloadAsset.downLoadPath);
    assetsToDownload.push_back(downloadAsset);
}

int32_t FileDataHandler::UpdateAssetInPhotoMap(const DKRecord &record, int32_t fileId)
{
    NativeRdb::AbsRdbPredicates updatePredicates = NativeRdb::AbsRdbPredicates(PhotoMap::TABLE);
    updatePredicates.EqualTo(PhotoMap::ASSET_ID, to_string(fileId));
    auto resultSet = Query(updatePredicates, CLEAN_QUERY_COLUMNS);

    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_LOGIC_ALBUM_IDS) == data.end()) {
        LOGE("record data cannot find properties");
        return E_INVAL_ARG;
    }
    DKRecordFieldList list;
    if (data[FILE_LOGIC_ALBUM_IDS].GetRecordList(list) != DKLocalErrorCode::NO_ERROR) {
        LOGE("cannot get album ids from record");
        return E_INVAL_ARG;
    }

    set<int> cloudMapIds;
    for (const auto &it : list) {
        DKReference ref;
        if (it.GetReference(ref) != DKLocalErrorCode::NO_ERROR) {
            LOGE("id list member not a reference");
            continue;
        }
        int albumId = GetAlbumIdFromCloudId(ref.recordId);
        if (albumId < 0) {
            LOGD("cannot get album id from album name %{public}s, ignore", ref.recordId.c_str());
            continue;
        }
        LOGI("record get albumId %{public}d", albumId);
        cloudMapIds.insert(albumId);
        QueryAndInsertMap(albumId, fileId);
    }

    QueryAndDeleteMap(fileId, cloudMapIds);
    return E_OK;
}

int32_t FileDataHandler::InsertAssetToPhotoMap(const DKRecord &record, OnFetchParams &params)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_LOGIC_ALBUM_IDS) == data.end()) {
        LOGE("record data cannot find properties");
        return E_INVAL_ARG;
    }
    DKRecordFieldList list;
    if (data[FILE_LOGIC_ALBUM_IDS].GetRecordList(list) != DKLocalErrorCode::NO_ERROR) {
        LOGE("cannot get album ids from record");
        return E_INVAL_ARG;
    }

    set<int> cloudMapIds;
    for (const auto &it : list) {
        DKReference ref;
        if (it.GetReference(ref) != DKLocalErrorCode::NO_ERROR) {
            LOGE("id list member not a reference");
            continue;
        }
        int albumId = GetAlbumIdFromCloudId(ref.recordId);
        if (albumId < 0) {
            LOGD("cannot get album id from album name %{public}s, ignore", ref.recordId.c_str());
            continue;
        }
        LOGI("record get albumId %{public}d", albumId);
        cloudMapIds.insert(albumId);
    }

    params.recordAlbumMaps[record.GetRecordId()] = cloudMapIds;
    return E_OK;
}

int32_t FileDataHandler::BatchInsertAssetMaps(OnFetchParams &params)
{
    auto recordIds = vector<string>();
    for (const auto &it : params.recordAlbumMaps) {
        recordIds.push_back(it.first);
    }
    auto [resultSet, recordIdRowIdMap] = QueryLocalByCloudId(recordIds);
    if (resultSet == nullptr) {return E_RDB;}
    vector<ValuesBucket> valuesList;
    int ret = E_OK;
    for (const auto &it : params.recordAlbumMaps) {
        if (recordIdRowIdMap.find(it.first) == recordIdRowIdMap.end()) {
            continue;
        }
        resultSet->GoToRow(recordIdRowIdMap.at(it.first));
        int fileId = 0;
        ret = DataConvertor::GetInt(MediaColumn::MEDIA_ID, fileId, *resultSet);
        if (ret != E_OK) {
            LOGE("Get media id failed");
            continue;
        }
        for (auto albumId : it.second) {
            ValuesBucket values;
            values.PutInt(PhotoMap::ALBUM_ID, albumId);
            values.PutInt(PhotoMap::ASSET_ID, fileId);
            values.PutInt(PhotoMap::DIRTY, static_cast<int32_t>(DirtyTypes::TYPE_SYNCED));
            valuesList.emplace_back(values);
        }
    }
    int64_t rowId;
    ret = BatchInsert(rowId, PhotoMap::TABLE, valuesList);
    if (ret != E_OK) {
        LOGE("fail to insert album mapping, ret %{public}d", ret);
        return ret;
    }
    LOGI("add mapping success");
    return E_OK;
}

int32_t FileDataHandler::BatchInsertAssetAnalysisMaps(OnFetchParams &params)
{
    auto recordIds = vector<string>();
    for (const auto &it : params.recordAnalysisAlbumMaps) {
        recordIds.push_back(it.first);
    }
    auto [resultSet, recordIdRowIdMap] = QueryLocalByCloudId(recordIds);
    if (resultSet == nullptr) {return E_RDB;}
    for (const auto &it : params.recordAnalysisAlbumMaps) {
        if (recordIdRowIdMap.find(it.first) == recordIdRowIdMap.end()) {
            continue;
        }
        resultSet->GoToRow(recordIdRowIdMap.at(it.first));
        int fileId = 0;
        int ret = DataConvertor::GetInt(MediaColumn::MEDIA_ID, fileId, *resultSet);
        if (ret != E_OK) {
            LOGE("Get media id failed");
            continue;
        }

        int64_t rowId;
        ValuesBucket values;
        values.PutInt(PhotoMap::ALBUM_ID, it.second);
        values.PutInt(PhotoMap::ASSET_ID, fileId);
        ret = Insert(rowId, ANALYSIS_PHOTO_MAP_TABLE, values);
        if (ret != E_OK) {
            LOGE("fail to insert albumId %{public}d - fileId %{public}d mapping, ret %{public}d",
                it.second, fileId, ret);
            continue;
        }
        LOGI("albumId %{public}d - fileId %{public}d add mapping success", it.second, fileId);
    }
    return E_OK;
}

int32_t FileDataHandler::GetAlbumIdFromCloudId(const std::string &cloudId)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(PhotoAlbumColumns::TABLE);
    predicates.EqualTo(PhotoAlbumColumns::ALBUM_CLOUD_ID, cloudId);
    auto resultSet = Query(predicates, {PhotoAlbumColumns::ALBUM_ID});
    int rowCount = 0;
    int ret = -1;
    if (resultSet) {
        ret = resultSet->GetRowCount(rowCount);
    }
    if (resultSet == nullptr || ret != E_OK || rowCount < 0) {
        LOGE("get nullptr result or rowcount %{public}d", rowCount);
        return -1;
    }
    if (resultSet->GoToNextRow() == 0) {
        int albumId;
        ret = DataConvertor::GetInt(PhotoAlbumColumns::ALBUM_ID, albumId, *resultSet);
        if (ret == E_OK) {
            return albumId;
        }
    }
    LOGD("fail to get ALBUM_ID value");
    return -1;
}

int32_t FileDataHandler::BatchGetFileIdFromCloudId(const std::vector<NativeRdb::ValueObject> &recordIds,
    std::vector<int> &fileIds)
{
    if (recordIds.size() == 0) {
        return E_OK;
    }
    uint32_t size = 0;
    if (recordIds.size() <= BATCH_LIMIT_SIZE) {
        size = recordIds.size();
    } else {
        uint32_t remain = recordIds.size() % BATCH_LIMIT_SIZE;
        if (remain == 0) {
            size = recordIds.size() - BATCH_LIMIT_SIZE;
        } else {
            size = recordIds.size() - remain;
        }
    }

    std::vector<std::string> thmStrVec;
    thmStrVec.reserve(size);
    for (uint32_t recordIndex = 0; recordIndex < size; recordIndex++) {
        std::string cloudId;
        int valueErr = recordIds[recordIndex].GetString(cloudId);
        if (valueErr == E_OK) {
            thmStrVec.push_back(cloudId);
        }
    }

    return QueryWithBatchCloudId(fileIds, thmStrVec);
}

int32_t FileDataHandler::QueryWithBatchCloudId(std::vector<int> &fileIds, std::vector<std::string> &thmStrVec)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(PhotoColumn::PHOTOS_TABLE);
    predicates.In(PhotoColumn::PHOTO_CLOUD_ID, thmStrVec);
    auto resultSet = Query(predicates, {MediaColumn::MEDIA_ID});
    int rowCount = 0;
    int ret = -1;
    if (resultSet) {
        ret = resultSet->GetRowCount(rowCount);
    }
    if (resultSet == nullptr || ret != E_OK || rowCount < 0) {
        LOGE("get nullptr result or rowcount %{public}d", rowCount);
        return E_RDB;
    }

    int err = 0;
    err = resultSet->GoToFirstRow();
    if (err != E_OK) {
        LOGE("get first result of file id fail %{public}d", err);
        return E_RDB;
    }

    do {
        int fileId;
        ret = DataConvertor::GetInt(MediaColumn::MEDIA_ID, fileId, *resultSet);
        if (ret == E_OK) {
            fileIds.push_back(fileId);
        }
    } while (resultSet->GoToNextRow() == E_OK);

    return E_OK;
}

void FileDataHandler::QueryAndInsertMap(int32_t albumId, int32_t fileId)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(PhotoMap::TABLE);
    predicates.EqualTo(PhotoMap::ALBUM_ID, to_string(albumId))->And()->EqualTo(PhotoMap::ASSET_ID, to_string(fileId));
    auto resultSet = Query(predicates, {});
    int rowCount = 0;
    int ret = -1;
    if (resultSet) {
        ret = resultSet->GetRowCount(rowCount);
    }
    if (resultSet == nullptr || ret != E_OK || rowCount < 0) {
        LOGE("get nullptr result or rowcount %{public}d", rowCount);
        return;
    }
    if (rowCount > 0) {
        LOGI("albumId %{public}d - fileId %{public}d already mapped", albumId, fileId);
        return;
    }

    int64_t rowId;
    ValuesBucket values;
    values.PutInt(PhotoMap::ALBUM_ID, albumId);
    values.PutInt(PhotoMap::ASSET_ID, fileId);
    values.PutInt(PhotoMap::DIRTY, static_cast<int32_t>(DirtyTypes::TYPE_SYNCED));
    ret = Insert(rowId, PhotoMap::TABLE, values);
    if (ret != E_OK) {
        LOGE("fail to insert albumId %{public}d - fileId %{public}d mapping, ret %{public}d", albumId, fileId, ret);
        return;
    }
    LOGI("albumId %{public}d - fileId %{public}d add mapping success", albumId, fileId);
}

void FileDataHandler::QueryAndDeleteMap(int32_t fileId, const set<int> &cloudMapIds)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(PhotoMap::TABLE);
    predicates.EqualTo(PhotoMap::ASSET_ID, to_string(fileId));

    auto resultSet = Query(predicates, {});
    int rowCount = 0;
    int ret = -1;
    if (resultSet) {
        ret = resultSet->GetRowCount(rowCount);
    }
    if (resultSet == nullptr || ret != E_OK || rowCount < 0) {
        LOGE("get nullptr result or rowcount %{public}d", rowCount);
        return;
    }
    while (resultSet->GoToNextRow() == 0) {
        int albumId;
        ret = DataConvertor::GetInt(PhotoMap::ALBUM_ID, albumId, *resultSet);
        if (ret != E_OK) {
            LOGE("fail to get ALBUM_ID value");
            continue;
        }
        int localDirty  = 0;
        ret = DataConvertor::GetInt(PhotoMap::DIRTY, localDirty, *resultSet);
        if (ret != E_OK) {
            LOGE("fail to get Dirty value");
            continue;
        }

        if (localDirty != static_cast<int32_t>(DirtyTypes::TYPE_SYNCED)) {
            LOGI("mapping albumId %{public}d - fileId %{public}d local dirty %{public}d, skip",
                albumId, fileId, localDirty);
            continue;
        }
        if (cloudMapIds.find(albumId) == cloudMapIds.end()) {
            LOGE("delete mapping albumId %{public}d - fileId %{public}d", albumId, fileId);
            int deletedRows;
            ret = Delete(deletedRows, PhotoMap::TABLE, PhotoMap::ASSET_ID + " = ? AND " + PhotoMap::ALBUM_ID + " = ?",
                         {to_string(fileId), to_string(albumId)});
            if (ret != E_OK) {
                LOGE("delete mapping failed %{public}d", ret);
                continue;
            }
        }
    }
}

int32_t FileDataHandler::DeleteAssetInPhotoMap(int32_t fileId)
{
    if (fileId <= 0) {
        return E_INVAL_ARG;
    }

    int32_t deletedRows;
    int ret = Delete(deletedRows, PhotoMap::TABLE, PhotoMap::ASSET_ID + " = ?", {to_string(fileId)});
    if (ret != E_OK) {
        LOGE("delete in rdb failed, ret: %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t FileDataHandler::DeleteDentryFile(void)
{
    std::string cacheDir =
        "/data/service/el2/" + std::to_string(userId_) + "/hmdfs/cache/account_cache/dentry_cache/cloud/";
    LOGD("cacheDir: %s", cacheDir.c_str());
    ForceRemoveDirectory(cacheDir);
    MetaFileMgr::GetInstance().ClearAll();
    return E_OK;
}

int32_t FileDataHandler::ClearCloudInfo(const string &cloudId)
{
    ValuesBucket values;
    values.PutNull(PhotoColumn::PHOTO_CLOUD_ID);
    values.PutInt(PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(DirtyType::TYPE_NEW));
    values.PutInt(PhotoColumn::PHOTO_POSITION, POSITION_LOCAL);
    values.PutLong(PhotoColumn::PHOTO_CLOUD_VERSION, 0);
    int32_t updateRows = 0;
    std::string whereClause = PhotoColumn::PHOTO_CLOUD_ID + " = ?";
    int ret = Update(updateRows, values, whereClause, {cloudId});
    if (ret != E_OK) {
        LOGE("Update in rdb failed, ret:%{public}d", ret);
    }
    return ret;
}

static int32_t DeleteAsset(const string &assetPath)
{
    int ret = E_OK;
    if (access(assetPath.c_str(), F_OK) == 0) {
        LOGD("assetPath exist");
        ret = remove(assetPath.c_str());
        if (ret != 0) {
            LOGE("Clean remove assetPath failed, errno %{public}d", errno);
            return errno;
        }
    }
    return ret;
}

int32_t FileDataHandler::Clean(const int action)
{
    std::lock_guard<std::mutex> lck(cleanMutex_);
    RETURN_ON_ERR(CleanPureCloudRecord());
    int32_t ret = DeleteCloudPhotoDir();
    if (ret != E_OK) {
        LOGE("clean cloud photo dir err: %{public}d", ret);
    }
    UpdateAllAlbums();
    DataSyncNotifier::GetInstance().TryNotify(PHOTO_URI_PREFIX, ChangeType::INSERT,
                                              INVALID_ASSET_ID);
    DataSyncNotifier::GetInstance().FinalNotify();

    return E_OK;
}

int32_t FileDataHandler::CleanPureCloudRecord(bool isReamin)
{
    LOGI("begin clean pure cloud record");
    int32_t ret = E_OK;
    NativeRdb::AbsRdbPredicates cleanPredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    cleanPredicates.EqualTo(PhotoColumn::PHOTO_POSITION, POSITION_CLOUD);
    cleanPredicates.EqualTo(PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(DirtyType::TYPE_SYNCED));
    if (isReamin) {
        cleanPredicates.EqualTo(PhotoColumn::PHOTO_CLEAN_FLAG, NEED_CLEAN);
    }
    cleanPredicates.Limit(BATCH_LIMIT_SIZE);
    vector<ValueObject> deleteFileId;
    shared_ptr<vector<string>> filePaths = make_shared<vector<string>>();
    int32_t removeCount = 0;
    do {
        deleteFileId.clear();
        filePaths->clear();
        RETURN_ON_ERR(GetFilePathAndId(cleanPredicates, deleteFileId, *filePaths));
        ret = BatchDetete(PhotoMap::TABLE, PhotoMap::ASSET_ID, deleteFileId);
        if (ret != E_OK) {
            LOGW("BatchDetete fail, remove count is %{public}d", removeCount);
            return ret;
        }
        ret = BatchDetete(PC::PHOTOS_TABLE, MediaColumn::MEDIA_ID, deleteFileId);
        if (ret != E_OK) {
            LOGW("BatchDetete fail, remove count is %{public}d", removeCount);
            return ret;
        }
        RemoveCloudRecord(filePaths);
        removeCount += deleteFileId.size();
    } while (deleteFileId.size() != 0);
    LOGI("end clean pure cloud record, remove count is %{public}d", removeCount);
    return ret;
}

int32_t FileDataHandler::CleanNotPureCloudRecord(const int32_t action)
{
    int32_t ret = E_OK;
    LOGD("Clean not Pure CloudRecord");
    if (action == CleanAction::CLEAR_DATA) {
        ret = CleanNotDirtyData();
    }
    ret = CleanAllCloudInfo();
    return ret;
}

int32_t FileDataHandler::CleanRemainRecord()
{
    if (!IsPullRecords()) {
        std::lock_guard<std::mutex> lck(cleanMutex_);
        int32_t ret = CleanPureCloudRecord(true);
        if (ret != E_OK) {
            LOGW("clean pure cloud record fail, try next time");
        }
        ret = CleanNotDirtyData(true);
        if (ret != E_OK) {
            LOGW("Clean not dirty data fail, try next time");
        }
    }
    return E_OK;
}

int32_t FileDataHandler::DeleteCloudPhotoDir()
{
    std::string photoDir = "/mnt/hmdfs/" + to_string(userId_) + "/account/device_view/cloud/files/Photo";
    std::string thumbsDir = "/mnt/hmdfs/" + to_string(userId_) + "/account/device_view/cloud/files/.thumbs/Photo";
    if (!ForceRemoveDirectory(photoDir)) {
        LOGE("remove photo dir fail, err: %{public}d", errno);
    }
    if (!ForceRemoveDirectory(thumbsDir)) {
        LOGE("remove thumbs dir fail, err: %{public}d", errno);
    }
    return E_OK;
}

int32_t FileDataHandler::CleanNotDirtyData(bool isReamin)
{
    LOGI("begin clean not dirty data");
    int32_t ret = E_OK;
    NativeRdb::AbsRdbPredicates cleanPredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    cleanPredicates.EqualTo(PhotoColumn::PHOTO_POSITION, POSITION_BOTH);
    cleanPredicates.EqualTo(PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(DirtyType::TYPE_SYNCED));
    if (isReamin) {
        cleanPredicates.EqualTo(PhotoColumn::PHOTO_CLEAN_FLAG, NEED_CLEAN);
    }
    cleanPredicates.Limit(BATCH_LIMIT_SIZE);
    vector<ValueObject> deleteFileId;
    shared_ptr<vector<string>> filePaths = make_shared<vector<string>>();
    int32_t removeCount = 0;
    do {
        deleteFileId.clear();
        filePaths->clear();
        RETURN_ON_ERR(GetFilePathAndId(cleanPredicates, deleteFileId, *filePaths));
        ret = BatchDetete(PhotoMap::TABLE, PhotoMap::ASSET_ID, deleteFileId);
        if (ret != E_OK) {
            LOGW("BatchDetete fail, remove count is %{public}d", removeCount);
            return ret;
        }
        ret = BatchDetete(PC::PHOTOS_TABLE, MediaColumn::MEDIA_ID, deleteFileId);
        if (ret != E_OK) {
            LOGW("BatchDetete fail, remove count is %{public}d", removeCount);
            return ret;
        }
        RemoveBothRecord(filePaths);
        removeCount += deleteFileId.size();
    } while (deleteFileId.size() != 0);
    LOGI("end clean not dirty data, remove count is %{public}d", removeCount);
    return ret;
}

int32_t FileDataHandler::CleanAllCloudInfo()
{
    LOGD("Clean Pure CloudRecord");
    ValuesBucket values;
    values.PutNull(PhotoColumn::PHOTO_CLOUD_ID);
    values.PutInt(PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(DirtyType::TYPE_NEW));
    values.PutInt(PhotoColumn::PHOTO_POSITION, POSITION_LOCAL);
    values.PutLong(PhotoColumn::PHOTO_CLOUD_VERSION, 0);
    int32_t updateRows = 0;
    std::string whereClause = PhotoColumn::PHOTO_POSITION + " = ?";
    vector<string> whereArgs = {to_string(POSITION_BOTH)};
    int32_t ret = Update(updateRows, values, whereClause, whereArgs);
    if (ret != E_OK) {
        LOGE("Update in rdb failed, ret:%{public}d", ret);
    }
    return ret;
}

int32_t FileDataHandler::GetFilePathAndId(NativeRdb::AbsRdbPredicates cleanPredicates,
                                          vector<ValueObject> &deleteFileId,
                                          vector<string> &filePaths)
{
    auto result = Query(cleanPredicates, {MediaColumn::MEDIA_ID, PC::MEDIA_FILE_PATH});
    if (result == nullptr) {
        LOGE("get result fail");
        return E_RDB;
    }
    int32_t count = 0;
    int32_t ret = result->GetRowCount(count);
    if (ret != E_OK || count < 0) {
        LOGE("get row count error , ret: %{public}d", ret);
        return E_RDB;
    }
    while (result->GoToNextRow() == 0) {
        string fileId;
        ret = DataConvertor::GetString(PC::MEDIA_ID, fileId, *result);
        string filePath;
        if (ret != E_OK) {
            LOGE("Get file_id wrong");
            return E_INVAL_ARG;
        }
        ret = DataConvertor::GetString(PC::MEDIA_FILE_PATH, filePath, *result);
        if (ret != E_OK) {
            LOGE("Get path wrong");
            return E_INVAL_ARG;
        }
        filePaths.emplace_back(filePath);
        deleteFileId.emplace_back(fileId);
    }
    return E_OK;
}

void FileDataHandler::RemoveCloudRecord(shared_ptr<vector<string>> filePaths)
{
    if (filePaths == nullptr) {
        LOGE("filePaths is nullptr");
        return;
    }
    for (auto &filePath : *filePaths) {
        RemoveThmParentPath(filePath);
    }
    LOGI("remove count is %{public}zu", filePaths->size());
}

void FileDataHandler::RemoveBothRecord(shared_ptr<vector<string>> filePaths)
{
    if (filePaths == nullptr) {
        LOGE("filePaths is nullptr");
        return;
    }
    for (auto &filePath : *filePaths) {
        RemoveThmParentPath(filePath);
        string lowerPath = cleanConvertor_.GetLowerPath(filePath);
        string thmbFileParentPath = cleanConvertor_.GetThumbParentPath(filePath);
        ForceRemoveDirectory(thmbFileParentPath);
        DeleteAsset(lowerPath);
    }
    LOGI("remove count is %{public}zu", filePaths->size());
}

int32_t FileDataHandler::MarkClean(const int32_t action)
{
    cloudPrefImpl_.SetInt("cleanAction", action);
    int32_t changedRows;
    NativeRdb::ValuesBucket values;
    values.PutInt(PC::PHOTO_CLEAN_FLAG, NEED_CLEAN);
    values.PutInt(PC::PHOTO_DIRTY, static_cast<int32_t>(DirtyType::TYPE_SYNCED));
    string whereClause = PC::PHOTO_POSITION + " = ?";
    vector<string> whereArgs = {to_string(static_cast<int32_t>(POSITION_CLOUD))};
    if (action == CleanAction::CLEAR_DATA) {
        whereClause += " OR ( " + PC::PHOTO_POSITION + " = ? AND " + PC::PHOTO_DIRTY + " = ? )";
        whereArgs.push_back(to_string(static_cast<int32_t>(POSITION_BOTH)));
        whereArgs.push_back(to_string(static_cast<int32_t>(DirtyType::TYPE_SYNCED)));
    }
    int32_t ret =
        Update(changedRows, PC::PHOTOS_TABLE, values, whereClause, whereArgs);
    if (ret != E_OK) {
        LOGW("mark clean error %{public}d", ret);
    }
    UpdateAllAlbums();
    DataSyncNotifier::GetInstance().TryNotify(PHOTO_URI_PREFIX, ChangeType::INSERT,
                                              INVALID_ASSET_ID);
    DataSyncNotifier::GetInstance().FinalNotify();
    RETURN_ON_ERR(CleanNotPureCloudRecord(action));
    return ret;
}

void FileDataHandler::HandleCreateConvertErr(int32_t err, NativeRdb::ResultSet &resultSet)
{
    string path;
    int32_t ret = createConvertor_.GetString(PC::MEDIA_FILE_PATH, path, resultSet);
    if (ret != E_OK) {
        LOGE("get path err");
        return;
    }
    createFailSet_.push_back(path);
}

void FileDataHandler::HandleFdirtyConvertErr(int32_t err, NativeRdb::ResultSet &resultSet)
{
    string cloudId;
    int32_t ret = createConvertor_.GetString(PC::PHOTO_CLOUD_ID, cloudId, resultSet);
    if (ret != E_OK) {
        LOGE("get cloud id err");
        return;
    }
    if (err == E_PATH) {
        retrySet_.push_back(cloudId);
    }
    modifyFailSet_.push_back(cloudId);
}

int32_t FileDataHandler::GetCreatedRecords(vector<DKRecord> &records)
{
    while (records.size() == 0) {
        /* build predicates */
        NativeRdb::AbsRdbPredicates createPredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
        createPredicates
            .EqualTo(Media::PhotoColumn::PHOTO_DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_NEW)))
            ->And()->EqualTo(Media::PhotoColumn::MEDIA_DATE_TRASHED, "0")
            ->BeginWrap()
            ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_IMAGE))
            ->Or()
            ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_VIDEO))
            ->EndWrap();
        if (!createFailSet_.empty()) {
            createPredicates.And()->NotIn(Media::PhotoColumn::MEDIA_FILE_PATH, createFailSet_);
        }
        /* small-size first */
        createPredicates.OrderByAsc(Media::PhotoColumn::MEDIA_SIZE);
        createPredicates.Limit(LIMIT_SIZE);
        /* query */
        auto results = Query(createPredicates, MEDIA_CLOUD_SYNC_COLUMNS);
        if (results == nullptr) {
            LOGE("get nullptr created result");
            return E_RDB;
        }
        /* results to records */
        int32_t ret = createConvertor_.ResultSetToRecords(move(results), records);
        if (ret != E_OK) {
            if (ret == E_STOP) {
                return E_OK;
            }
            LOGE("result set to records err %{public}d", ret);
            return ret;
        }
    }

    /* bind album */
    int32_t ret = BindAlbums(records);
    if (ret != E_OK) {
        LOGE("bind albums err %{public}d", ret);
        return ret;
    }

    /* erase local info */
    ret = EraseLocalInfo(records);
    if (ret != 0) {
        LOGE("erase local info err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::GetDeletedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    NativeRdb::AbsRdbPredicates deletePredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    deletePredicates
        .EqualTo(Media::PhotoColumn::PHOTO_DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_DELETED)))
        ->BeginWrap()
        ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_IMAGE))
        ->Or()
        ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_VIDEO))
        ->EndWrap();
    if (!modifyFailSet_.empty()) {
        deletePredicates.And()->NotIn(Media::PhotoColumn::PHOTO_CLOUD_ID, modifyFailSet_);
    }
    deletePredicates.Limit(DELETE_BATCH_NUM);

    /* query */
    auto results = Query(deletePredicates, MEDIA_CLOUD_SYNC_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr deleted result");
        return E_RDB;
    }

    /* results to records */
    int ret = deleteConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::GetMetaModifiedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    NativeRdb::AbsRdbPredicates updatePredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    updatePredicates
        .BeginWrap()
        ->EqualTo(Media::PhotoColumn::PHOTO_DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_MDIRTY)))
        ->Or()
        ->EqualTo(Media::PhotoColumn::PHOTO_DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_SDIRTY)))
        ->EndWrap()
        ->And()
        ->BeginWrap()
        ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_IMAGE))
        ->Or()
        ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_VIDEO))
        ->EndWrap();
    if (!modifyFailSet_.empty()) {
        updatePredicates.And()->NotIn(Media::PhotoColumn::PHOTO_CLOUD_ID, modifyFailSet_);
    }
    updatePredicates.Limit(MODIFY_BATCH_NUM);

    /* query */
    auto results = Query(updatePredicates, MEDIA_CLOUD_SYNC_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr modified result");
        return E_RDB;
    }

    /* results to records */
    int ret = mdirtyConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    /* album map change */
    ret = BindAlbumChanges(records);
    if (ret != E_OK) {
        LOGE("update album map change err %{public}d", ret);
        return ret;
    }

    /* erase local info */
    ret = EraseLocalInfo(records);
    if (ret != 0) {
        LOGE("erase local info err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

static inline int32_t GetFileIdFromRecord(DKRecord &record, int32_t &fileId)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end()) {
        LOGE("file attributes is no exit");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap attributes = data[FILE_ATTRIBUTES];
    if (attributes.find(Media::MediaColumn::MEDIA_ID) == attributes.end()) {
        LOGE("media id is no exit");
        return E_INVAL_ARG;
    }
    fileId = attributes[Media::MediaColumn::MEDIA_ID];
    return E_OK;
}

static inline void InsertAlbumIds(DKRecord &record, vector<string> &cloudIds)
{
    DKRecordData data;
    DKRecordFieldList list;
    record.StealRecordData(data);
    for (auto &id : cloudIds) {
        list.push_back(DKRecordField(DKReference{ id, "album" }));
    }
    data[FILE_LOGIC_ALBUM_IDS] = DKRecordField(list);
    record.SetRecordData(data);
}

static inline void InsertAlbumIdChanges(DKRecord &record, vector<string> &add, vector<string> &rm)
{
    DKRecordData data;
    record.StealRecordData(data);
    /* add */
    DKRecordFieldList addList;
    for (auto &id : add) {
        addList.push_back(DKRecordField(DKReference{ id, "album" }));
    }
    data[FILE_ADD_LOGIC_ALBUM_IDS] = DKRecordField(addList);
    /* remove */
    DKRecordFieldList rmList;
    for (auto &id : rm) {
        rmList.push_back(DKRecordField(DKReference{ id, "album" }));
    }
    data[FILE_RM_LOGIC_ALBUM_IDS] = DKRecordField(rmList);
    record.SetRecordData(data);
}

/* need optimization: IN or JOIN */
int32_t FileDataHandler::GetAlbumCloudIds(vector<int32_t> &localIds, vector<string> &cloudIds)
{
    for (auto localId : localIds) {
        NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(PAC::TABLE);
        predicates.EqualTo(PAC::ALBUM_ID, to_string(localId));
        /* query map */
        auto results = Query(predicates, {});
        if (results == nullptr) {
            LOGE("get nullptr result");
            return E_RDB;
        }
        if (results->GoToNextRow() != NativeRdb::E_OK) {
            LOGE("failed to go to next row");
            return E_RDB;
        }
        string cloudId;
        int32_t ret = createConvertor_.GetString(PAC::ALBUM_CLOUD_ID, cloudId, *results);
        if (ret != E_OK) {
            return ret;
        }
        if (!cloudId.empty()) {
            cloudIds.push_back(cloudId);
        }
    }
    return E_OK;
}

int32_t FileDataHandler::GetAlbumIdsFromResultSet(const shared_ptr<NativeRdb::ResultSet> resultSet,
    vector<int32_t> &ids)
{
    while (resultSet->GoToNextRow() == 0) {
        int32_t id;
        int32_t ret = createConvertor_.GetInt(PM::ALBUM_ID, id, *resultSet);
        if (ret != E_OK) {
            return ret;
        }
        ids.push_back(id);
    }
    return E_OK;
}

int32_t FileDataHandler::BindAlbums(std::vector<DriveKit::DKRecord> &records)
{
    for (auto &record : records) {
        NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(PM::TABLE);
        /* map exceed limit? */
        int32_t fileId;
        int32_t ret = GetFileIdFromRecord(record, fileId);
        if (ret != E_OK) {
            continue;
        }
        predicates.EqualTo(PM::ASSET_ID, to_string(fileId));
        /* query map */
        auto results = Query(predicates, {});
        if (results == nullptr) {
            LOGE("get nullptr result");
            return E_RDB;
        }
        vector<int32_t> albumIds;
        ret = GetAlbumIdsFromResultSet(results, albumIds);
        if (ret != E_OK) {
            LOGE("get album ids from result set err %{public}d", ret);
            return ret;
        }
        /* query album */
        vector<string> cloudIds;
        ret = GetAlbumCloudIds(albumIds, cloudIds);
        if (ret != E_OK) {
            LOGE("get album cloud id err %{public}d", ret);
            return ret;
        }
        /* insert album ids */
        InsertAlbumIds(record, cloudIds);
    }
    return E_OK;
}

int32_t FileDataHandler::GetAlbumIdsFromResultSet(const shared_ptr<NativeRdb::ResultSet> resultSet,
    vector<int32_t> &add, vector<int32_t> &rm)
{
    while (resultSet->GoToNextRow() == 0) {
        int32_t id;
        int32_t ret = createConvertor_.GetInt(PM::ALBUM_ID, id, *resultSet);
        if (ret != E_OK) {
            return ret;
        }
        int32_t state;
        ret = createConvertor_.GetInt(PM::DIRTY, state, *resultSet);
        if (ret != E_OK) {
            return ret;
        }
        if (state == static_cast<int32_t>(Media::DirtyType::TYPE_NEW)) {
            add.push_back(id);
        } else if (state == static_cast<int32_t>(Media::DirtyType::TYPE_DELETED)) {
            rm.push_back(id);
        }
    }
    return E_OK;
}

int32_t FileDataHandler::BindAlbumChanges(std::vector<DriveKit::DKRecord> &records)
{
    for (auto &record : records) {
        NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(PM::TABLE);
        /* map exceed limit? */
        int32_t fileId;
        int32_t ret = GetFileIdFromRecord(record, fileId);
        if (ret != E_OK) {
            continue;
        }
        predicates.EqualTo(PM::ASSET_ID, to_string(fileId))
            ->And()
            ->NotEqualTo(PM::DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED)));
        /* query map */
        auto results = Query(predicates, {});
        if (results == nullptr) {
            LOGE("get nullptr result");
            return E_RDB;
        }
        /* new or delete */
        vector<int32_t> add, remove;
        ret = GetAlbumIdsFromResultSet(results, add, remove);
        if (ret != E_OK) {
            LOGE("get album ids from result set err %{public}d", ret);
            return ret;
        }
        /* query album */
        vector<string> addCloud, removeCloud;
        ret = GetAlbumCloudIds(add, addCloud);
        if (ret != E_OK) {
            LOGE("get album cloud id err %{public}d", ret);
            return ret;
        }
        ret = GetAlbumCloudIds(remove, removeCloud);
        if (ret != E_OK) {
            LOGE("get album cloud id err %{public}d", ret);
            return ret;
        }
        /* update */
        InsertAlbumIdChanges(record, addCloud, removeCloud);
    }
    return E_OK;
}

int32_t FileDataHandler::GetDownloadAsset(std::string cloudId, vector<DriveKit::DKDownloadAsset> &outAssetsToDownload,
    std::shared_ptr<DentryContext> dentryContext)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.SetWhereClause(Media::PhotoColumn::MEDIA_FILE_PATH + " = ?");
    predicates.SetWhereArgs({cloudId});
    predicates.Limit(LIMIT_SIZE);
    auto resultSet = Query(predicates, {PC::PHOTO_CLOUD_ID, PC::MEDIA_FILE_PATH});
    if (resultSet == nullptr) {
        LOGE("get nullptr created result");
        return E_RDB;
    }
    int32_t rowCount = 0;
    int32_t ret = resultSet->GetRowCount(rowCount);
    if (ret != 0) {
        LOGE("result set get row count err %{public}d", ret);
        return E_RDB;
    }
    while (resultSet->GoToNextRow() == 0) {
        DKRecord record;
        localConvertor_.ConvertAsset(record, *resultSet);
        if (record.GetRecordId().empty()) {
            continue;
        }
        AppendToDownload(record, "content", outAssetsToDownload);
    }
    return E_OK;
}
int32_t FileDataHandler::GetFileModifiedRecords(vector<DKRecord> &records)
{
    while (records.size() == 0) {
        /* build predicates */
        NativeRdb::AbsRdbPredicates updatePredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
        updatePredicates
            .EqualTo(Media::PhotoColumn::PHOTO_DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_FDIRTY)))
            ->And()
            ->BeginWrap()
            ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_IMAGE))
            ->Or()
            ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_VIDEO))
            ->EndWrap();
        if (!modifyFailSet_.empty()) {
            updatePredicates.And()->NotIn(Media::PhotoColumn::PHOTO_CLOUD_ID, modifyFailSet_);
        }
        updatePredicates.OrderByAsc(Media::PhotoColumn::MEDIA_SIZE);
        updatePredicates.Limit(LIMIT_SIZE);
        /* query */
        auto results = Query(updatePredicates, MEDIA_CLOUD_SYNC_COLUMNS);
        if (results == nullptr) {
            LOGE("get nullptr modified result");
            return E_RDB;
        }
        /* results to records */
        int32_t ret = fdirtyConvertor_.ResultSetToRecords(move(results), records);
        if (ret != E_OK) {
            if (ret == E_STOP) {
                return E_OK;
            }
            LOGE("result set to records err %{public}d", ret);
            return ret;
        }
        SetRetry(retrySet_);
    }

    /* album map change */
    int32_t ret = BindAlbumChanges(records);
    if (ret != E_OK) {
        LOGE("update album map change err %{public}d", ret);
        return ret;
    }

    /* erase local info */
    ret = EraseLocalInfo(records);
    if (ret != E_OK) {
        LOGE("erase local info err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::EraseLocalInfo(vector<DriveKit::DKRecord> &records)
{
    for (auto &record : records) {
        DKRecordData data;
        record.GetRecordData(data);
        DKRecordFieldMap attributes;
        data[FILE_ATTRIBUTES].GetRecordMap(attributes);
        attributes.erase(Media::MediaColumn::MEDIA_ID);
        attributes.erase(Media::PhotoColumn::PHOTO_CLOUD_ID);
        data[FILE_ATTRIBUTES] = DKRecordField(attributes);
        record.SetRecordData(data);
    }
    return E_OK;
}

static string GetFilePathFromRecord(const DKRecord &record)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end()) {
        LOGE("record data cannot find attributes");
        return "";
    }
    DriveKit::DKRecordFieldMap attributes = data[FILE_ATTRIBUTES];
    if (attributes.find(PhotoColumn::MEDIA_FILE_PATH) == attributes.end()) {
        LOGE("record data cannot find file path");
        return "";
    }
    string path;
    if (attributes[PhotoColumn::MEDIA_FILE_PATH].GetString(path) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        return "";
    }
    return path;
}

int32_t FileDataHandler::UpdateMediaFilePath(DriveKit::DKRecord &record, NativeRdb::ResultSet &local)
{
    string newFileName = "";
    string relativePath = "";
    DKRecordData data;
    record.GetRecordData(data);
    string filePath = GetFilePathFromRecord(record);
    if (filePath.empty()) {
        return E_INVAL_ARG;
    }
    if (GetDentryPathName(filePath, relativePath, newFileName) != E_OK) {
        LOGE("split filePath to dentry path failed, path:%s", filePath.c_str());
        return E_INVAL_ARG;
    }
    string newThumbParentPath = createConvertor_.GetThumbParentPath(filePath);
    bool isDataChanged = access(newThumbParentPath.c_str(), F_OK) == 0;
    if (!isDataChanged) {
        LOGI("media file path change");
        string oldFilePath = "";
        string oldFileName = "";
        int32_t ret = DataConvertor::GetString(PC::MEDIA_FILE_PATH, oldFilePath, local);
        if (ret != E_OK) {
            return ret;
        }
        if (GetDentryPathName(oldFilePath, relativePath, oldFileName) != E_OK) {
            LOGE("split old filePath to dentry path failed, path:%s", oldFilePath.c_str());
            return E_INVAL_ARG;
        }
        LOGI("update media file path, old path is %{public}s, new path is %{public}s",
            oldFilePath.c_str(), filePath.c_str());
        auto mFile = MetaFileMgr::GetInstance().GetMetaFile(userId_, relativePath);
        MetaBase mBase(oldFileName);
        ret = mFile->DoRename(mBase, newFileName);
        if (ret != E_OK) {
            LOGE("rename dentry failed, ret:%{public}d", ret);
        }
        string oldThumbParentPath = createConvertor_.GetThumbParentPath(oldFilePath);
        ret = rename(oldThumbParentPath.c_str(), newThumbParentPath.c_str());
        if (ret != E_OK) {
            MetaBase mBase(newFileName);
            ret = mFile->DoRename(mBase, oldFileName);
            LOGE("rename thmbPath faill, errno is %{public}d, restore dentry file ret is %{public}d", errno, ret);
            return E_RENAME_FAIL;
        }
    }
    return E_OK;
}

int32_t FileDataHandler::OnCreateRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    unordered_map<string, LocalInfo> localMap;
    uint64_t success = 0;
    uint64_t failure = 0;

    int32_t ret = GetLocalInfo(map, localMap, Media::PhotoColumn::MEDIA_FILE_PATH);
    if (ret != E_OK) {
        LOGE("get local match info err %{public}d", ret);
        UpdateMetaStat(INDEX_UL_META_ERROR_DATA, map.size());
        return ret;
    }

    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnCreateRecordSuccess(entry, localMap);
        } else {
            err = OnRecordFailed(entry);
            failure++;
        }
        if (err != E_OK) {
            string filePath = GetFilePathFromRecord(entry.second.GetDKRecord());
            if (!filePath.empty()) {
                createFailSet_.push_back(filePath);
            }
            LOGE("create record fail: file path %{private}s", filePath.c_str());
        } else {
            success++;
        }
        GetReturn(err, ret);
    }
    (void)DataSyncNotifier::GetInstance().FinalNotify();

    UpdateMetaStat(INDEX_UL_META_ERROR_SDK, failure);
    UpdateMetaStat(INDEX_UL_META_SUCCESS, success);

    return ret;
}

int32_t FileDataHandler::OnDeleteRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    uint64_t success = 0;
    uint64_t failure = 0;
    int32_t ret = E_OK;

    for (auto &entry : map) {
        const DKRecordOperResult &result = entry.second;
        int32_t err;
        if (result.IsSuccess()) {
            err = OnDeleteRecordSuccess(entry);
        } else {
            err = OnRecordFailed(entry);
            failure++;
        }
        if (err != E_OK) {
            modifyFailSet_.push_back(entry.first);
            LOGE("delete record fail: cloud id: %{private}s", entry.first.c_str());
        } else {
            success++;
        }
        GetReturn(err, ret);
    }

    UpdateMetaStat(INDEX_UL_META_ERROR_SDK, failure);
    UpdateMetaStat(INDEX_UL_META_SUCCESS, success);

    return ret;
}

int32_t FileDataHandler::OnModifyMdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    unordered_map<string, LocalInfo> localMap;
    uint64_t success = 0;
    uint64_t failure = 0;

    int32_t ret = GetLocalInfo(map, localMap, Media::PhotoColumn::PHOTO_CLOUD_ID);
    if (ret != E_OK) {
        LOGE("get local match info err %{public}d", ret);
        UpdateMetaStat(INDEX_UL_META_ERROR_DATA, map.size());
        return ret;
    }

    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnMdirtyRecordSuccess(entry, localMap);
        } else {
            err = OnRecordFailed(entry);
            failure++;
        }
        if (err != E_OK) {
            modifyFailSet_.push_back(entry.first);
            LOGE("modify mdirty record fail: cloud id: %{private}s", entry.first.c_str());
        } else {
            success++;
        }
        GetReturn(err, ret);
    }

    UpdateMetaStat(INDEX_UL_META_ERROR_SDK, failure);
    UpdateMetaStat(INDEX_UL_META_SUCCESS, success);

    return ret;
}

int32_t FileDataHandler::OnModifyFdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    unordered_map<string, LocalInfo> localMap;
    uint64_t success = 0;
    uint64_t failure = 0;

    int32_t ret = GetLocalInfo(map, localMap, Media::PhotoColumn::PHOTO_CLOUD_ID);
    if (ret != E_OK) {
        LOGE("get local match info err %{public}d", ret);
        UpdateMetaStat(INDEX_UL_META_ERROR_DATA, map.size());
        return ret;
    }

    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnFdirtyRecordSuccess(entry, localMap);
        } else {
            err = OnRecordFailed(entry);
            failure++;
        }
        if (err != E_OK) {
            modifyFailSet_.push_back(entry.first);
            LOGE("modify fdirty record fail: cloud id: %{public}s", entry.first.c_str());
        } else {
            success++;
        }
        GetReturn(err, ret);
    }

    UpdateMetaStat(INDEX_UL_META_ERROR_SDK, failure);
    UpdateMetaStat(INDEX_UL_META_SUCCESS, success);

    return ret;
}

void FileDataHandler::Reset()
{
    modifyFailSet_.clear();
    createFailSet_.clear();
}

int32_t FileDataHandler::CheckRecordData(DKRecordData &data, string &path)
{
    if (data.find(FILE_ATTRIBUTES) == data.end()) {
        LOGE("record data cannot find attributes");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap attributes = data[FILE_ATTRIBUTES];
    if (attributes.find(PhotoColumn::MEDIA_FILE_PATH) == attributes.end()) {
        LOGE("record data cannot find file path");
        return E_INVAL_ARG;
    }
    if (attributes[PhotoColumn::MEDIA_FILE_PATH].GetString(path) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        return E_INVAL_ARG;
    }

    return E_OK;
}

int32_t FileDataHandler::OnCreateRecordSuccess(const pair<DKRecordId, DKRecordOperResult> &entry,
    const unordered_map<string, LocalInfo> &localMap)
{
    auto record = entry.second.GetDKRecord();
    DKRecordData data;
    record.GetRecordData(data);
    string path;
    int32_t checkResult = FileDataHandler::CheckRecordData(data, path);
    if (checkResult != E_OK) {
        UpdateMetaStat(INDEX_UL_META_ERROR_DATA, 1);
        return checkResult;
    }

    /* local file deleted */
    if (localMap.find(path) == localMap.end()) {
        return E_OK;
    }

    ValuesBucket valuesBucket;
    valuesBucket.PutString(Media::PhotoColumn::PHOTO_CLOUD_ID, entry.first);
    valuesBucket.PutInt(Media::PhotoColumn::PHOTO_POSITION, POSITION_BOTH);
    valuesBucket.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
    /**
     * Fix me: Non-atomic compare
     * A file can be modify after the local match query.
     * Possible Solution: if-else in SQL update
     */
    if (IsTimeChanged(record, localMap, path, Media::PhotoColumn::MEDIA_DATE_MODIFIED)) {
        valuesBucket.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_FDIRTY));
    } else if (IsTimeChanged(record, localMap, path, PhotoColumn::PHOTO_META_DATE_MODIFIED)) {
        valuesBucket.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_MDIRTY));
    } else {
        valuesBucket.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    }

    /* Fix me: might need a transaction to do an atomic update for files and their album maps */
    int32_t ret = UpdateLocalAlbumMap(entry.first);
    if (ret != E_OK) {
        LOGE("update local album map err %{public}d", ret);
        UpdateMetaStat(INDEX_UL_META_ERROR_RDB, 1);
        return ret;
    }

    /* update file */
    int32_t changedRows;
    string whereClause = PC::MEDIA_FILE_PATH + " = ? AND " + PC::PHOTO_DIRTY + " = ?";
    vector<string> whereArgs = { path, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_NEW)) };
    ret = Update(changedRows, valuesBucket, whereClause, whereArgs);
    if (ret != 0) {
        LOGE("on create records update synced err %{public}d", ret);
        UpdateMetaStat(INDEX_UL_META_ERROR_RDB, 1);
        return ret;
    }

    /* notify */
    int32_t fileId;
    if (data[FILE_LOCAL_ID].GetInt(fileId) == DKLocalErrorCode::NO_ERROR) {
        (void)DataSyncNotifier::GetInstance().TryNotify(PHOTO_URI_PREFIX + to_string(fileId),
            ChangeType::UPDATE, to_string(fileId));
    }

    return E_OK;
}

int32_t FileDataHandler::OnDeleteRecordSuccess(const std::pair<DKRecordId, DKRecordOperResult> &entry)
{
    string cloudId = entry.first;
    /* delete local */
    int32_t deletedRows;
    string whereClause = Media::PhotoColumn::PHOTO_CLOUD_ID + " = ?";
    int32_t ret = Delete(deletedRows, whereClause, { cloudId });
    if (ret != 0) {
        LOGE("on delete records update err %{public}d", ret);
        UpdateMetaStat(INDEX_UL_META_ERROR_RDB, 1);
        return ret;
    }

    /* no error handler! */
    DeleteAssetInPhotoMap(deletedRows);
    return E_OK;
}

int32_t FileDataHandler::OnMdirtyRecordSuccess(
    const pair<DKRecordId, DKRecordOperResult> &entry,
    const unordered_map<string, LocalInfo> &localMap)
{
    auto record = entry.second.GetDKRecord();
    DKRecordData data;
    record.GetRecordData(data);
    string cloudId = entry.first;

    /* Fix me: might need a transaction to do an atomic update for files and their album maps */
    int32_t ret = UpdateLocalAlbumMap(entry.first);
    if (ret != E_OK) {
        LOGE("update local album map err %{public}d", ret);
        UpdateMetaStat(INDEX_UL_META_ERROR_RDB, 1);
        return ret;
    }

    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end()) {
        LOGE("record data cannot find attributes");
        UpdateMetaStat(INDEX_UL_META_ERROR_DATA, 1);
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap attributes = data[FILE_ATTRIBUTES];
    if (attributes.find(PhotoColumn::PHOTO_META_DATE_MODIFIED) == attributes.end()) {
        LOGE("record data cannot find file path");
        UpdateMetaStat(INDEX_UL_META_ERROR_DATA, 1);
        return E_INVAL_ARG;
    }
    int64_t meta_date_modified;
    if (attributes[PhotoColumn::PHOTO_META_DATE_MODIFIED].GetLong(meta_date_modified) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        UpdateMetaStat(INDEX_UL_META_ERROR_DATA, 1);
        return E_INVAL_ARG;
    }

    int32_t changedRows;
    ValuesBucket valuesBucket;
    valuesBucket.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
    valuesBucket.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    /* mdirty -> synced: only if no change in meta_date_modified */
    ret = Update(changedRows, valuesBucket, PC::PHOTO_CLOUD_ID + " = ? AND " +
        PC::PHOTO_META_DATE_MODIFIED + " = ?", { cloudId, to_string(meta_date_modified) });
    if (ret != E_OK) {
        LOGE("on modify records update synced err %{public}d", ret);
        /* update record version anyway */
        valuesBucket.Clear();
        valuesBucket.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
        ret = Update(changedRows, valuesBucket, PC::PHOTO_CLOUD_ID + " = ?", { cloudId });
        if (ret != E_OK) {
            LOGE("update record version err %{public}d", ret);
            UpdateMetaStat(INDEX_UL_META_ERROR_RDB, 1);
            return ret;
        }
    }

    return E_OK;
}

int32_t FileDataHandler::OnFdirtyRecordSuccess(
    const pair<DKRecordId, DKRecordOperResult> &entry,
    const unordered_map<string, LocalInfo> &localMap)
{
    auto record = entry.second.GetDKRecord();
    DKRecordData data;
    record.GetRecordData(data);
    string cloudId = entry.first;

    /* Fix me: might need a transaction to do an atomic update for files and their album maps */
    int32_t ret = UpdateLocalAlbumMap(entry.first);
    if (ret != E_OK) {
        LOGE("update local album map err %{public}d", ret);
        UpdateMetaStat(INDEX_UL_META_ERROR_RDB, 1);
        return ret;
    }

    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end()) {
        LOGE("record data cannot find attributes");
        UpdateMetaStat(INDEX_UL_META_ERROR_DATA, 1);
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap attributes = data[FILE_ATTRIBUTES];
    if (attributes.find(PhotoColumn::PHOTO_META_DATE_MODIFIED) == attributes.end()) {
        LOGE("record data cannot find file path");
        UpdateMetaStat(INDEX_UL_META_ERROR_DATA, 1);
        return E_INVAL_ARG;
    }
    int64_t meta_date_modified;
    if (attributes[PhotoColumn::PHOTO_META_DATE_MODIFIED].GetLong(meta_date_modified) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        UpdateMetaStat(INDEX_UL_META_ERROR_DATA, 1);
        return E_INVAL_ARG;
    }

    int32_t changedRows;
    ValuesBucket valuesBucket;
    valuesBucket.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
    valuesBucket.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    /**
     * fdirty -> synced: only if no change in meta_date_modified.
     * Fix me: if date_modified unchanged, update fdirty -> mdirty
     */
    ret = Update(changedRows, valuesBucket, PC::PHOTO_CLOUD_ID + " = ? AND " +
        PC::PHOTO_META_DATE_MODIFIED + " = ?", { cloudId, to_string(meta_date_modified) });
    if (ret != E_OK) {
        LOGE("on modify records update synced err %{public}d", ret);
        /* update record version anyway */
        valuesBucket.Clear();
        valuesBucket.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
        ret = Update(changedRows, valuesBucket, PC::PHOTO_CLOUD_ID + " = ?", { cloudId });
        if (ret != E_OK) {
            LOGE("update record version err %{public}d", ret);
            UpdateMetaStat(INDEX_UL_META_ERROR_RDB, 1);
            return ret;
        }
    }

    return E_OK;
}

int32_t FileDataHandler::UpdateLocalAlbumMap(const string &cloudId)
{
    /* update deleted */
    string deleteSql = "DELETE FROM " + PM::TABLE + " WHERE " + PM::DIRTY + " = " +
        to_string(static_cast<int32_t>(Media::DirtyType::TYPE_DELETED)) + " AND " + PM::ASSET_ID +
        " IN (SELECT " + PC::MEDIA_ID + " FROM " + PC::PHOTOS_TABLE + " WHERE " +
        PC::PHOTO_CLOUD_ID + " = '" + cloudId + "')";
    int32_t ret = ExecuteSql(deleteSql);
    if (ret != NativeRdb::E_OK) {
        LOGE("delete local album map err %{public}d", ret);
        return ret;
    }

    /* update new */
    string newSql = "UPDATE " + PM::TABLE + " SET " + PM::DIRTY + " = " +
        to_string(static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED)) + " WHERE " + PM::ASSET_ID +
        " IN (SELECT " + PC::MEDIA_ID + " FROM " + PC::PHOTOS_TABLE + " WHERE " +
        PC::PHOTO_CLOUD_ID + " = '" + cloudId + "')";
    ret = ExecuteSql(newSql);
    if (ret != NativeRdb::E_OK) {
        LOGE("Update local album map err %{public}d", ret);
        return ret;
    }
    return ret;
}

bool FileDataHandler::IsTimeChanged(const DriveKit::DKRecord &record,
    const unordered_map<string, LocalInfo> &localMap, const string &path, const string &type)
{
    int64_t cloudtime = 0;
    int64_t localtime = 0;
    auto it = localMap.find(path);
    if (it == localMap.end()) {
        return true;
    }
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_ATTRIBUTES) == data.end()) {
        LOGE("record data cannot find attributes");
        return false;
    }
    DriveKit::DKRecordFieldMap attributes;
    data[FILE_ATTRIBUTES].GetRecordMap(attributes);
    /* get mtime or metatime */
    if (type == Media::PhotoColumn::MEDIA_DATE_MODIFIED) {
        localtime = it->second.fdirtyTime;
        if (attributes[FILE_EDITED_TIME_MS].GetLong(cloudtime) != DKLocalErrorCode::NO_ERROR) {
            LOGE("obtain MEDIA_DATE_MODIFIED error");
            return false;
        }
    } else {
        localtime = it->second.mdirtyTime;
        if (attributes[PhotoColumn::PHOTO_META_DATE_MODIFIED].GetLong(cloudtime) != DKLocalErrorCode::NO_ERROR) {
            LOGE("obtain PHOTO_META_DATE_MODIFIED error");
            return false;
        }
    }
    if (localtime == cloudtime) {
        return false;
    }
    return true;
}

int32_t FileDataHandler::GetLocalInfo(const map<DKRecordId, DKRecordOperResult> &map,
    unordered_map<string, LocalInfo> &infoMap, const string &type)
{
    vector<string> path;
    for (auto &entry : map) {
        if (type == Media::PhotoColumn::PHOTO_CLOUD_ID) {
            path.push_back(entry.first);
        } else {
            auto record = entry.second.GetDKRecord();
            DKRecordData data;
            record.GetRecordData(data);
            if (data.find(FILE_ATTRIBUTES) == data.end()) {
                LOGE("record data cannot find attributes");
                return E_DATA;
            }
            DriveKit::DKRecordFieldMap attributes = data[FILE_ATTRIBUTES];
            if (attributes.find(PhotoColumn::MEDIA_FILE_PATH) == attributes.end()) {
                LOGE("record data cannot find some attributes");
                return E_DATA;
            }
            string curPath;
            if (attributes[PhotoColumn::MEDIA_FILE_PATH].GetString(curPath) != DKLocalErrorCode::NO_ERROR) {
                LOGE("bad file_path in props");
                return E_DATA;
            }
            path.push_back(curPath);
        }
    }

    NativeRdb::AbsRdbPredicates createPredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    createPredicates.And()->In(type, path);
    auto resultSet = Query(createPredicates, ON_UPLOAD_COLUMNS);
    if (resultSet == nullptr) {
        LOGE("query rdb err");
        return E_RDB;
    }

    return BuildInfoMap(move(resultSet), infoMap, type);
}

int32_t FileDataHandler::BuildInfoMap(const shared_ptr<NativeRdb::ResultSet> resultSet,
    unordered_map<string, LocalInfo> &infoMap, const string &type)
{
    int32_t idIndex = -1;
    int32_t mtimeIndex = -1;
    int32_t metatimeIndex = -1;
    RETURN_ON_ERR(resultSet->GetColumnIndex(type, idIndex));
    RETURN_ON_ERR(resultSet->GetColumnIndex(PC::MEDIA_DATE_MODIFIED, mtimeIndex));
    RETURN_ON_ERR(resultSet->GetColumnIndex(PC::PHOTO_META_DATE_MODIFIED, metatimeIndex));

    /* iterate all rows compare mtime metatime */
    while (resultSet->GoToNextRow() == 0) {
        string idValue;
        int64_t mtime;
        int64_t metatime;
        if (resultSet->GetString(idIndex, idValue) == 0 && resultSet->GetLong(mtimeIndex, mtime) == 0 &&
            resultSet->GetLong(metatimeIndex, metatime) == 0) {
            infoMap.insert({ idValue, { metatime, mtime } });
        }
    }
    return E_OK;
}

int64_t FileDataHandler::UTCTimeSeconds()
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &ts);
    return static_cast<int64_t>(ts.tv_sec);
}


int32_t FileDataHandler::GetFilePosStat(std::vector<uint64_t> &filePosStat)
{
    if (filePosStat.size() != FILE_POSITION_LEN) {
        LOGE("file position stat size is wrong with %{public}zu", filePosStat.size());
        return E_DATA;
    }

    /* get local file status */
    int num = 0;
    QueryFilePosStat(POSITION_LOCAL, num);
    filePosStat[FilePositionIndex::LOCAL] = static_cast<uint64_t>(num);

    /* get cloud file status */
    QueryFilePosStat(POSITION_CLOUD, num);
    filePosStat[FilePositionIndex::CLOUD] = num;

    /* get both file status */
    QueryFilePosStat(POSITION_BOTH, num);
    filePosStat[FilePositionIndex::BOTH] = num;

    return E_OK;
}

int32_t FileDataHandler::GetCloudThmStat(std::vector<uint64_t> &cloudThmStat)
{
    if (cloudThmStat.size() != CLOUD_THM_STAT_LEN) {
        LOGE("cloud thm stat size is wrong with %{public}zu", cloudThmStat.size());
        return E_DATA;
    }

    /* get DOWNLOADED thm status */
    int num = 0;
    QueryCloudThmStat(static_cast<int32_t>(ThumbState::DOWNLOADED), num);
    cloudThmStat[CloudThmStatIndex::DOWNLOADED] = static_cast<uint64_t>(num);

    /* get LCD_TO_DOWNLOAD thm status */
    QueryCloudThmStat(static_cast<int32_t>(ThumbState::LCD_TO_DOWNLOAD), num);
    cloudThmStat[CloudThmStatIndex::LCD_TO_DOWNLOAD] = num;

    /* get THM_TO_DOWNLOAD thm status */
    QueryCloudThmStat(static_cast<int32_t>(ThumbState::THM_TO_DOWNLOAD), num);
    cloudThmStat[CloudThmStatIndex::THM_TO_DOWNLOAD] = num;

    /* get TO_DOWNLOAD thm status */
    QueryCloudThmStat(static_cast<int32_t>(ThumbState::TO_DOWNLOAD), num);
    cloudThmStat[CloudThmStatIndex::TO_DOWNLOAD] = num;

    return E_OK;
}

int32_t FileDataHandler::GetDirtyTypeStat(std::vector<uint64_t> &dirtyTypeStat)
{
    if (dirtyTypeStat.size() != DIRTY_TYPE_LEN) {
        LOGE("dirty type size is wrong with %{public}zu", dirtyTypeStat.size());
        return E_DATA;
    }

    /* get synced dirty type status */
    int num = 0;
    QueryDirtyTypeStat(static_cast<int32_t>(DirtyType::TYPE_SYNCED), num);
    dirtyTypeStat[DirtyTypeIndex::SYNCED] = static_cast<uint64_t>(num);

    /* get new dirty type status */
    QueryDirtyTypeStat(static_cast<int32_t>(DirtyType::TYPE_NEW), num);
    dirtyTypeStat[DirtyTypeIndex::NEW] = static_cast<uint64_t>(num);

    /* get mdirty dirty type status */
    QueryDirtyTypeStat(static_cast<int32_t>(DirtyType::TYPE_MDIRTY), num);
    dirtyTypeStat[DirtyTypeIndex::MDIRTY] = static_cast<uint64_t>(num);

    /* get fdirty dirty type status */
    QueryDirtyTypeStat(static_cast<int32_t>(DirtyType::TYPE_FDIRTY), num);
    dirtyTypeStat[DirtyTypeIndex::FDIRTY] = static_cast<uint64_t>(num);

    /* get deleted dirty type status */
    QueryDirtyTypeStat(static_cast<int32_t>(DirtyType::TYPE_DELETED), num);
    dirtyTypeStat[DirtyTypeIndex::DELETED] = static_cast<uint64_t>(num);

    return E_OK;
}

int32_t FileDataHandler::QueryDataStat(const NativeRdb::AbsRdbPredicates &predicates,
    const std::string &queryData, const int32_t &value, int &num)
{
    auto results = Query(predicates, { queryData });
    if (results == nullptr) {
        LOGE("get nullptr, query %{public}s value:%{public}d failed!", queryData.c_str(), value);
        return E_RDB;
    }

    int32_t ret = results->GetRowCount(num);
    if (ret != 0) {
        LOGE("get row count failed ret is %{public}d", ret);
        return E_RDB;
    }

    LOGI("query data status success type:%{public}s value:%{public}d num:%{public}d",
        queryData.c_str(), value, num);
    return E_OK;
}

int32_t FileDataHandler::QueryFilePosStat(const int32_t &position, int &num)
{
    num = 0;
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.EqualTo(PhotoColumn::PHOTO_POSITION, position);
    return QueryDataStat(predicates, PhotoColumn::PHOTO_POSITION, position, num);
}

int32_t FileDataHandler::QueryCloudThmStat(const int32_t &cloudThmStat, int &num)
{
    num = 0;
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.EqualTo(PhotoColumn::PHOTO_THUMB_STATUS, cloudThmStat)
              ->And()
              ->BeginWrap()
              ->EqualTo(PhotoColumn::PHOTO_POSITION, POSITION_CLOUD)
              ->Or()
              ->EqualTo(PhotoColumn::PHOTO_POSITION, POSITION_BOTH)
              ->EndWrap();
    return QueryDataStat(predicates, PhotoColumn::PHOTO_THUMB_STATUS, cloudThmStat, num);
}

int32_t FileDataHandler::QueryDirtyTypeStat(const int32_t &dirtyType, int &num)
{
    num = 0;
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.EqualTo(PhotoColumn::PHOTO_DIRTY, dirtyType);
    return QueryDataStat(predicates, PhotoColumn::PHOTO_DIRTY, dirtyType, num);
}

void FileDataHandler::ReportFilePosStat()
{
    std::vector<uint64_t> filePosStat(FILE_POSITION_LEN);
    GetFilePosStat(filePosStat);
    UpdateCheckFilePos(FilePositionIndex::LOCAL, filePosStat[FilePositionIndex::LOCAL]);
    UpdateCheckFilePos(FilePositionIndex::CLOUD, filePosStat[FilePositionIndex::CLOUD]);
    UpdateCheckFilePos(FilePositionIndex::BOTH, filePosStat[FilePositionIndex::BOTH]);
}

void FileDataHandler::ReportCloudThmStat()
{
    std::vector<uint64_t> dirtyTypeStat(CLOUD_THM_STAT_LEN);
    GetCloudThmStat(dirtyTypeStat);
    UpdateCheckCloudThmStat(CloudThmStatIndex::DOWNLOADED,
        dirtyTypeStat[CloudThmStatIndex::DOWNLOADED]);
    UpdateCheckCloudThmStat(CloudThmStatIndex::LCD_TO_DOWNLOAD,
        dirtyTypeStat[CloudThmStatIndex::LCD_TO_DOWNLOAD]);
    UpdateCheckCloudThmStat(CloudThmStatIndex::THM_TO_DOWNLOAD,
        dirtyTypeStat[CloudThmStatIndex::THM_TO_DOWNLOAD]);
    UpdateCheckCloudThmStat(CloudThmStatIndex::TO_DOWNLOAD,
        dirtyTypeStat[CloudThmStatIndex::TO_DOWNLOAD]);
}

void FileDataHandler::ReportDirtyTypeStat()
{
    std::vector<uint64_t> dirtyTypeStat(DIRTY_TYPE_LEN);
    GetDirtyTypeStat(dirtyTypeStat);
    UpdateCheckDirtyType(DirtyTypeIndex::SYNCED, dirtyTypeStat[DirtyTypeIndex::SYNCED]);
    UpdateCheckDirtyType(DirtyTypeIndex::NEW, dirtyTypeStat[DirtyTypeIndex::NEW]);
    UpdateCheckDirtyType(DirtyTypeIndex::MDIRTY, dirtyTypeStat[DirtyTypeIndex::MDIRTY]);
    UpdateCheckDirtyType(DirtyTypeIndex::FDIRTY, dirtyTypeStat[DirtyTypeIndex::FDIRTY]);
    UpdateCheckDirtyType(DirtyTypeIndex::DELETED, dirtyTypeStat[DirtyTypeIndex::DELETED]);
}

void FileDataHandler::SetCheckReportStatus()
{
    /* set report file position status */
    ReportFilePosStat();

    /* set report dirty type status */
    ReportDirtyTypeStat();

    /* set report cloud thm status */
    ReportCloudThmStat();
}

int32_t FileDataHandler::OptimizeStorage(const int32_t agingDays)
{
    int64_t totalSize = GetTotalSize();
    totalSize = GetRoundSize(totalSize);
    int64_t freeSize = GetFreeSize();
    int64_t agingTime = 0;
    int64_t deleteSize = 0;
    int ret = 0;
    if (totalSize * START_AGING_SIZE > static_cast<double>(freeSize)) {
        agingTime = agingDays * DAY_TO_SECONDS;
        deleteSize = int64_t(totalSize * STOP_AGING_SIZE - static_cast<double>(freeSize));
        ret = FileAgingDelete(agingTime, deleteSize);
        if (ret != E_OK) {
            LOGE("OptimizeStorage Error");
            return ret;
        }
    }

    freeSize = GetFreeSize();
    if (totalSize * LOWEST_START_AGING_SIZE > static_cast<double>(freeSize)) {
        agingTime = 0;
        deleteSize = int64_t(totalSize * LOWEST_STOP_AGING_SIZE - static_cast<double>(freeSize));
        ret = FileAgingDelete(agingTime, deleteSize);
        if (ret != E_OK) {
            LOGE("OptimizeStorage Error");
            return ret;
        }
    }
    return E_OK;
}

std::shared_ptr<NativeRdb::ResultSet> FileDataHandler::GetAgingFile(const int64_t agingTime,
                                                                    int32_t &rowCount)
{
    auto time = UTCTimeSeconds();
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.LessThanOrEqualTo(PC::PHOTO_LAST_VISIT_TIME, to_string(TO_MILLISECONDS * (time - agingTime)));
    predicates.And()->EqualTo(PC::PHOTO_POSITION, POSITION_BOTH);
    predicates.OrderByAsc(PC::PHOTO_LAST_VISIT_TIME);
    auto results = Query(predicates, MEDIA_CLOUD_SYNC_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr result");
        return results;
    }
    int ret = results->GetRowCount(rowCount);
    LOGE("rowCount = %{public}d", rowCount);
    if (ret != 0) {
        LOGE("get rowCount error");
        return nullptr;
    }
    return results;
}

int32_t FileDataHandler::UpdateAgingFile(const string cloudId)
{
    ValuesBucket values;
    values.PutInt(PhotoColumn::PHOTO_POSITION, POSITION_CLOUD);
    int32_t changedRows;
    string whereClause = PhotoColumn::PHOTO_CLOUD_ID + " = ?";
    int ret = Update(changedRows, values, whereClause, {cloudId});
    if (ret != E_OK) {
        LOGE("rdb update failed, err=%{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t FileDataHandler::FileAgingDelete(const int64_t agingTime, const int64_t deleteSize)
{
    int rowCount = 0;
    int64_t totalSize = 0;
    auto results = GetAgingFile(agingTime, rowCount);
    if (results == nullptr) {
        LOGE("get nullptr result");
        return E_RDB;
    }
    if (rowCount == 0) {
        LOGE("get null file");
        return E_OK;
    }
    while (results->GoToNextRow() == 0) {
        string path;
        int64_t size;
        string cloudId;
        int ret = DataConvertor::GetString(PhotoColumn::MEDIA_FILE_PATH, path, *results);
        if (ret != E_OK) {
            LOGE("get path error");
            continue;
        }
        ret = DataConvertor::GetString(PhotoColumn::PHOTO_CLOUD_ID, cloudId, *results);
        if (ret != E_OK) {
            LOGE("get cloudId error");
            continue;
        }
        ret = DataConvertor::GetLong(PhotoColumn::MEDIA_SIZE, size, *results);
        if (ret != E_OK) {
            LOGE("get size error");
            continue;
        }
        string filePath = GetLocalPath(userId_, path);
        ret = unlink(filePath.c_str());
        if (ret != 0) {
            LOGE("fail to delete");
            continue;
        }
        ret = UpdateAgingFile(cloudId);
        if (ret != E_OK) {
            LOGE("update failed");
            continue;
        }
        totalSize += size;
        if (totalSize > deleteSize) {
            break;
        }
    }
    return E_OK;
}

int32_t FileDataHandler::GetThumbToDownload(std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.EqualTo(PC::PHOTO_SYNC_STATUS, to_string(static_cast<int32_t>(SyncStatusType::TYPE_VISIBLE)));
    predicates.EqualTo(PC::PHOTO_CLEAN_FLAG, to_string(static_cast<int32_t>(NOT_NEED_CLEAN)))
        ->And()->BeginWrap()
        ->EqualTo(PC::PHOTO_THUMB_STATUS, to_string(static_cast<int32_t>(ThumbState::TO_DOWNLOAD)))
        ->Or()->EqualTo(PC::PHOTO_THUMB_STATUS, to_string(static_cast<int32_t>(ThumbState::THM_TO_DOWNLOAD)))
        ->Or()->EqualTo(PC::PHOTO_THUMB_STATUS, to_string(static_cast<int32_t>(ThumbState::LCD_TO_DOWNLOAD)))
        ->EndWrap();
    predicates.Limit(DOWNLOAD_LIMIT_SIZE);
    auto results = Query(predicates, { PC::MEDIA_FILE_PATH, PC::PHOTO_CLOUD_ID, PC::PHOTO_THUMB_STATUS });
    if (results == nullptr) {
        LOGE("get nullptr TYPE_DOWNLOAD result");
        return E_RDB;
    }

    while (results->GoToNextRow() == 0) {
        int32_t thmState = 0;
        int32_t ret = DataConvertor::GetInt(PC::PHOTO_THUMB_STATUS, thmState, *results);
        if (ret != E_OK) {
            LOGE("Get file path failed");
            continue;
        }
        if (thmState == static_cast<int32_t>(ThumbState::TO_DOWNLOAD)) {
            AppendToDownload(*results, "thumbnail", outAssetsToDownload);
            AppendToDownload(*results, "lcd", outAssetsToDownload);
        } else if (thmState == static_cast<int32_t>(ThumbState::LCD_TO_DOWNLOAD)) {
            AppendToDownload(*results, "lcd", outAssetsToDownload);
        } else if (thmState == static_cast<int32_t>(ThumbState::THM_TO_DOWNLOAD)) {
            AppendToDownload(*results, "thumbnail", outAssetsToDownload);
        }
    }
    return E_OK;
}

void FileDataHandler::UpdateThmVec()
{
    if (!thmVec_.empty()) {
        LOGI("thmVec_ size is %{public}zu", thmVec_.size());
        string sql = "UPDATE " + PC::PHOTOS_TABLE + " SET " + PC::PHOTO_SYNC_STATUS + " = " +
            to_string(static_cast<int32_t>(SyncStatusType::TYPE_VISIBLE)) + " , " + PC::PHOTO_THUMB_STATUS +
            " = CASE WHEN " + PC::PHOTO_THUMB_STATUS + " - " +
            to_string(static_cast<int32_t>(ThumbState::THM_TO_DOWNLOAD)) + " < 0 THEN 0 ELSE " + PC::PHOTO_THUMB_STATUS
            + " - " + to_string(static_cast<int32_t>(ThumbState::THM_TO_DOWNLOAD)) + " END ";
        int32_t ret = E_OK;
        uint64_t count = 0;
        uint32_t size = 0;
        vector<ValueObject> tmp;
        {
            std::lock_guard<std::mutex> lock(thmMutex_);
            size = thmVec_.size();
            tmp.assign(thmVec_.begin(), thmVec_.begin() + size);
            thmVec_.erase(thmVec_.begin(), thmVec_.begin() + size);
        }
        vector<int> fileIds;
        BatchGetFileIdFromCloudId(tmp, fileIds);
        ret = BatchUpdate(sql, PC::PHOTO_CLOUD_ID, tmp, count);
        LOGI("update size is %{public}u, success count is %{public}" PRIu64 ", fail count is %{public}zu",
            size, count, tmp.size());
        if (ret != E_OK) {
            LOGW("update thm fail");
            std::lock_guard<std::mutex> lock(thmMutex_);
            thmVec_.insert(thmVec_.end(), tmp.begin(), tmp.end());
            fileIds.clear();
        }
        if (count != 0) {
            UpdateAttachmentStat(INDEX_THUMB_SUCCESS, count);
        }
        if (!IsPullRecords()) {
            UpdateAlbumInternal();
        }
        for (auto &fileId : fileIds) {
            DataSyncNotifier::GetInstance().TryNotify(PHOTO_URI_PREFIX + to_string(fileId), ChangeType::INSERT, "");
        }
        DataSyncNotifier::GetInstance().TryNotify(PHOTO_URI_PREFIX, ChangeType::INSERT, "");
        DataSyncNotifier::GetInstance().FinalNotify();
    }
}

void FileDataHandler::UpdateLcdVec()
{
    if (!lcdVec_.empty()) {
        LOGI("lcdVec_ size is %{public}zu", lcdVec_.size());
        string sql = "UPDATE " + PC::PHOTOS_TABLE + " SET " + PC::PHOTO_THUMB_STATUS + " = CASE WHEN " +
            PC::PHOTO_THUMB_STATUS + " - " + to_string(static_cast<int32_t>(ThumbState::LCD_TO_DOWNLOAD)) +
            " < 0 THEN 0 ELSE " + PC::PHOTO_THUMB_STATUS + " - " +
            to_string(static_cast<int32_t>(ThumbState::LCD_TO_DOWNLOAD)) + " END ";
        int32_t ret = E_OK;
        uint64_t count = 0;
        uint32_t size = 0;
        vector<ValueObject> tmp;
        {
            std::lock_guard<std::mutex> lock(lcdMutex_);
            size = lcdVec_.size();
            tmp.assign(lcdVec_.begin(), lcdVec_.begin() + size);
            lcdVec_.erase(lcdVec_.begin(), lcdVec_.begin() + size);
        }
        ret = BatchUpdate(sql, PC::PHOTO_CLOUD_ID, tmp, count);
        LOGI("update size is %{public}u, success count is %{public}" PRIu64 ", fail count is %{public}zu",
            size, count, tmp.size());
        if (ret != E_OK) {
            LOGW("update thm fail");
            std::lock_guard<std::mutex> lock(lcdMutex_);
            lcdVec_.insert(lcdVec_.end(), tmp.begin(), tmp.end());
        }
        if (count != 0) {
            UpdateAttachmentStat(INDEX_LCD_SUCCESS, count);
        }
        if (!IsPullRecords()) {
            UpdateAlbumInternal();
        }
    }
}


void FileDataHandler::StopUpdataFiles()
{
    const uint32_t MAX_TRY_TIMES = 5;
    uint32_t tryCount = 1;
    while (tryCount <= MAX_TRY_TIMES && (!lcdVec_.empty() || !thmVec_.empty())) {
        UpdateThmVec();
        UpdateLcdVec();
        tryCount++;
    }
    if (tryCount > MAX_TRY_TIMES && (!lcdVec_.empty() || !thmVec_.empty())) {
        LOGE("StopUpdataFiles update failed!");
    }
    UpdateAttachmentStat(INDEX_THUMB_ERROR_RDB, thmVec_.size());
    UpdateAttachmentStat(INDEX_LCD_ERROR_RDB, lcdVec_.size());
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
