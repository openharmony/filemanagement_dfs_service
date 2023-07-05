/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <tuple>

#include <dirent.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "thumbnail_const.h"
#include "data_sync_const.h"
#include "dfs_error.h"
#include "directory_ex.h"
#include "dk_assets_downloader.h"
#include "dk_error.h"
#include "utils_log.h"
#include "gallery_file_const.h"
#include "meta_file.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace DriveKit;
using namespace Media;
using ChangeType = OHOS::AAFwk::ChangeInfo::ChangeType;

const static std::string TOTAL_PULL_COUNT = "total_pull_count";
const static std::string DOWNLOAD_THUMB_LIMIT = "download_thumb_limit";
constexpr int DEFAULT_DOWNLOAD_THUMB_LIMIT = 500;
FileDataHandler::FileDataHandler(int32_t userId, const string &bundleName, std::shared_ptr<RdbStore> rdb)
    : RdbDataHandler(TABLE_NAME, rdb), userId_(userId), bundleName_(bundleName),
    cloudPrefImpl_(userId, bundleName, TABLE_NAME)
{
    cloudPrefImpl_.GetString(START_CURSOR, startCursor_);
    cloudPrefImpl_.GetString(NEXT_CURSOR, nextCursor_);

    cloudPrefImpl_.GetInt(TOTAL_PULL_COUNT, totalPullCount_);

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
    cond.desiredKeys = desiredKeys_;
}

int32_t FileDataHandler::GetRetryRecords(std::vector<DriveKit::DKRecordId> &records)
{
    NativeRdb::AbsRdbPredicates retryPredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    retryPredicates.SetWhereClause(PhotoColumn::PHOTO_DIRTY + " = ? AND " + PhotoColumn::MEDIA_DATE_TRASHED + " = ?");
    retryPredicates.SetWhereArgs({to_string(static_cast<int32_t>(DirtyType::TYPE_RETRY)), "0"});
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
    downloadAsset.downLoadPath = createConvertor_.GetThumbPath(filePath, suffix);
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
    auto results = Query(predicates, GALLERY_FILE_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr modified result");
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
    MediaColumn::MEDIA_ID,
};

tuple<shared_ptr<NativeRdb::ResultSet>, int> FileDataHandler::QueryLocalByCloudId(const string &recordId)
{
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.EqualTo(PhotoColumn::PHOTO_CLOUD_ID, recordId);
    auto resultSet = Query(predicates, PULL_QUERY_COLUMNS);
    if (resultSet == nullptr) {
        LOGE("get nullptr created result");
        return {nullptr, 0};
    }
    int rowCount = 0;
    int ret = resultSet->GetRowCount(rowCount);
    if (ret != 0) {
        LOGE("result set get row count err %{public}d", ret);
        return {nullptr, 0};
    }
    return {std::move(resultSet), rowCount};
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

int32_t FileDataHandler::OnFetchRecords(const shared_ptr<vector<DKRecord>> &records, OnFetchParams &params)
{
    LOGI("on fetch %{public}zu records", records->size());
    int32_t ret = E_OK;
    for (const auto &record : *records) {
        auto [resultSet, rowCount] = QueryLocalByCloudId(record.GetRecordId());
        if (resultSet == nullptr) {
            return E_RDB;
        }
        int32_t fileId = 0;
        ChangeType changeType = ChangeType::INVAILD;
        if ((rowCount == 0) && !record.GetIsDelete()) {
            ret = PullRecordInsert(record, params, fileId);
            changeType = ChangeType::INSERT;
        } else if (rowCount == 1) {
            resultSet->GoToNextRow();
            if (record.GetIsDelete()) {
                ret = PullRecordDelete(record, *resultSet);
                fileId = GetFileId(*resultSet);
                changeType = ChangeType::DELETE;
            } else {
                ret = PullRecordUpdate(record, *resultSet, params);
                fileId = GetFileId(*resultSet);
                changeType = ChangeType::UPDATE;
            }
        } else {
            LOGE("recordId %s rowCount %{public}d", record.GetRecordId().c_str(), rowCount);
        }
        if (ret != E_OK) {
            LOGE("recordId %s error %{public}d", record.GetRecordId().c_str(), ret);
            if (ret == E_RDB) {
                break;
            }
            ret = E_OK;
        }
        string notifyUri = DataSyncConst::PHOTO_URI_PREFIX + to_string(fileId);
        DataSyncNotifier::GetInstance().TryNotify(notifyUri, changeType, to_string(fileId));
    }
    DataSyncNotifier::GetInstance().FinalNotify();
    MetaFileMgr::GetInstance().ClearAll();
    return ret;
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
    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGE("record data cannot find properties");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap prop;
    data[FILE_PROPERTIES].GetRecordMap(prop);
    if (prop.find(PhotoColumn::MEDIA_FILE_PATH) == prop.end() || prop.find(PhotoColumn::MEDIA_SIZE) == prop.end() ||
        prop.find(PhotoColumn::MEDIA_DATE_MODIFIED) == prop.end()) {
        LOGE("record data cannot find some properties");
        return E_INVAL_ARG;
    }

    string fullPath, relativePath, fileName;
    if (prop[PhotoColumn::MEDIA_FILE_PATH].GetString(fullPath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        return E_INVAL_ARG;
    }
    if (GetDentryPathName(fullPath, relativePath, fileName) != E_OK) {
        LOGE("split to dentry path failed, path:%s", fullPath.c_str());
        return E_INVAL_ARG;
    }

    int64_t isize, mtime;
    if (DataConvertor::GetLongComp(prop[PhotoColumn::MEDIA_SIZE], isize) != E_OK) {
        LOGE("bad size in props");
        return E_INVAL_ARG;
    }
    if (DataConvertor::GetLongComp(prop[PhotoColumn::MEDIA_DATE_MODIFIED], mtime) != E_OK) {
        LOGE("bad mtime in props");
        return E_INVAL_ARG;
    }

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
    string relativePath, fileName;
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

int FileDataHandler::AddCloudThumbs(const DKRecord &record)
{
    LOGI("thumbs of %s add to cloud_view", record.GetRecordId().c_str());
    int64_t thumbSize = 0, lcdSize = 0;

    DKRecordData datas;
    record.GetRecordData(datas);
    if (datas.find(FILE_PROPERTIES) == datas.end()) {
        LOGE("record data cannot find properties");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap prop;
    datas[FILE_PROPERTIES].GetRecordMap(prop);

    string fullPath;
    if (prop.find(PhotoColumn::MEDIA_FILE_PATH) == prop.end() ||
        prop[PhotoColumn::MEDIA_FILE_PATH].GetString(fullPath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file path in record");
        return E_INVAL_ARG;
    }
    if (prop.find(FILE_THUMB_SIZE) == prop.end() ||
        DataConvertor::GetLongComp(prop[FILE_THUMB_SIZE], thumbSize) != E_OK) {
        LOGE("bad thumb size in record");
        return E_INVAL_ARG;
    }
    if (prop.find(FILE_LCD_SIZE) == prop.end() || DataConvertor::GetLongComp(prop[FILE_LCD_SIZE], lcdSize) != E_OK) {
        LOGE("bad lcd size in record");
        return E_INVAL_ARG;
    }
    int64_t mtime;
    if (DataConvertor::GetLongComp(prop[PhotoColumn::MEDIA_DATE_MODIFIED], mtime) != E_OK) {
        LOGE("bad mtime in props");
        return E_INVAL_ARG;
    }
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

string FileDataHandler::ConflictRenameThumb(NativeRdb::ResultSet &resultSet, string fullPath)
{
    /* thumb new path */
    string tmpPath = cleanConvertor_.GetThumbPath(fullPath, THUMB_SUFFIX);
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
    string newPath = tmpPath.substr(0, found) + "_1" + tmpPath.substr(found);
    int ret = rename(tmpPath.c_str(), newPath.c_str());
    if (ret != 0) {
        LOGE("err rename tmpPath to newPath");
        return "";
    }
    /* new name */
    size_t namefound = newPath.find_last_of('/');
    if (namefound == string::npos) {
        LOGE("err name Path found");
        return "";
    }
    string newName = newPath.substr(namefound + 1);
    return newName;
}

int32_t FileDataHandler::ConflictRename(NativeRdb::ResultSet &resultSet, string &fullPath)
{
    /* sandbox new path */
    size_t rdbfound = fullPath.find_last_of('.');
    if (rdbfound == string::npos) {
        LOGE("err rdb Path found");
        return E_INVAL_ARG;
    }
    string rdbPath = fullPath.substr(0, rdbfound) + "_1" + fullPath.substr(rdbfound);
    /* local new path */
    string localPath = cleanConvertor_.GetHmdfsLocalPath(fullPath);
    if (localPath == "") {
        LOGE("err local Path for GetHmdfsLocalPath");
        return E_INVAL_ARG;
    }
    size_t localfound = localPath.find_last_of('.');
    if (localfound == string::npos) {
        LOGE("err local Path found");
        return E_INVAL_ARG;
    }
    string newLocalPath = localPath.substr(0, localfound) + "_1" + localPath.substr(localfound);
    int ret = rename(localPath.c_str(), newLocalPath.c_str());
    if (ret != 0) {
        LOGE("err rename localPath to newLocalPath");
        return E_INVAL_ARG;
    }
    /* thumb new path and new name */
    string newName = ConflictRenameThumb(resultSet, fullPath);
    if (newName == "") {
        LOGE("err Rename Thumb path");
        return E_INVAL_ARG;
    }
    int updateRows;
    ValuesBucket values;
    values.PutString(PhotoColumn::MEDIA_FILE_PATH, rdbPath);
    values.PutString(PhotoColumn::MEDIA_NAME, newName);
    string whereClause = PhotoColumn::MEDIA_FILE_PATH + " = ?";
    int32_t ret32 = Update(updateRows, values, whereClause, {fullPath});
    if (ret32 != E_OK) {
        LOGE("update retry flag failed, ret=%{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t FileDataHandler::ConflictDataMerge(const DKRecord &record, string &fullPath)
{
    int updateRows;
    ValuesBucket values;
    values.PutString(PhotoColumn::PHOTO_CLOUD_ID, record.GetRecordId());
    values.PutInt(PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(DirtyType::TYPE_SYNCED));
    values.PutInt(PhotoColumn::PHOTO_POSITION, POSITION_BOTH);

    string whereClause = PhotoColumn::MEDIA_FILE_PATH + " = ?";
    int32_t ret = Update(updateRows, values, whereClause, {fullPath});
    if (ret != E_OK) {
        LOGE("update retry flag failed, ret=%{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t FileDataHandler::ConflictHandler(NativeRdb::ResultSet &resultSet,
                                         const DKRecord &record,
                                         string &fullPath,
                                         int64_t isize,
                                         int64_t mtime,
                                         bool &comflag)
{
    bool modifyPathflag = false;
    string localId;
    int ret = DataConvertor::GetString(PhotoColumn::PHOTO_CLOUD_ID, localId, resultSet);
    if (ret != E_OK) {
        LOGI("cloud Id is NULL");
    } else {
        modifyPathflag = true;
    }
    int64_t localIsize = 0;
    ret = DataConvertor::GetLong(PhotoColumn::MEDIA_SIZE, localIsize, resultSet);
    if (ret != E_OK) {
        LOGE("Get local isize failed");
        return E_INVAL_ARG;
    }
    int64_t localMtime = 0;
    ret = DataConvertor::GetLong(PhotoColumn::MEDIA_DATE_MODIFIED, localMtime, resultSet);
    if (ret != E_OK) {
        LOGE("Get local mtime failed");
        return E_INVAL_ARG;
    }
    if (localIsize == isize && localMtime == mtime) {
        LOGI("Possible duplicate files");
    } else {
        modifyPathflag = true;
    }
    if (modifyPathflag) {
        LOGI("Different files with the same name");
        /* Modify path */
        ret = ConflictRename(resultSet, fullPath);
        if (ret != E_OK) {
            LOGE("Conflict dataMerge rename fail");
            return ret;
        }
    } else {
        LOGI("Unification of the same document");
        /* update database */
        ret = ConflictDataMerge(record, fullPath);
        if (ret != E_OK) {
            LOGE("Conflict dataMerge fail");
            return ret;
        }
        comflag = true;
    }
    return E_OK;
}

int32_t FileDataHandler::GetConflictData(const DKRecord &record, string &fullPath, int64_t &isize, int64_t &mtime)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGE("record data cannot find properties");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap prop;
    data[FILE_PROPERTIES].GetRecordMap(prop);
    if (prop.find(PhotoColumn::MEDIA_FILE_PATH) == prop.end() || prop.find(PhotoColumn::MEDIA_SIZE) == prop.end() ||
        prop.find(PhotoColumn::MEDIA_DATE_MODIFIED) == prop.end()) {
        LOGE("record data cannot find some properties");
        return E_INVAL_ARG;
    }

    if (prop[PhotoColumn::MEDIA_FILE_PATH].GetString(fullPath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        return E_INVAL_ARG;
    }

    if (DataConvertor::GetLongComp(prop[PhotoColumn::MEDIA_SIZE], isize) != E_OK) {
        LOGE("bad size in props");
        return E_INVAL_ARG;
    }
    if (DataConvertor::GetLongComp(prop[PhotoColumn::MEDIA_DATE_MODIFIED], mtime) != E_OK) {
        LOGE("bad mtime in props");
        return E_INVAL_ARG;
    }
    return E_OK;
}

int32_t FileDataHandler::PullRecordConflict(const DKRecord &record, bool &comflag)
{
    LOGI("judgment downlode conflict");
    int32_t ret = E_OK;
    string fullPath;
    int64_t isize, mtime;
    ret = GetConflictData(record, fullPath, isize, mtime);
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
        ret = E_RDB;
    }
    int32_t rowCount = 0;
    ret = resultSet->GetRowCount(rowCount);
    if (ret != 0) {
        LOGE("result set get row count err %{public}d", ret);
        return E_RDB;
    }
    if (rowCount == 0) {
        LOGI("Normal download process");
        return E_OK;
    }
    if (rowCount == 1) {
        resultSet->GoToNextRow();
        ret = ConflictHandler(*resultSet, record, fullPath, isize, mtime, comflag);
    } else {
        LOGE("unknown error: PullRecordConflict(),same path rowCount = %{public}d", rowCount);
        ret = E_RDB;
    }
    return ret;
}

int32_t FileDataHandler::PullRecordInsert(const DKRecord &record, OnFetchParams &params, int32_t &fileId)
{
    LOGI("insert of record %s", record.GetRecordId().c_str());

    /* check local file conflict */
    bool comflag = false;
    int ret = PullRecordConflict(record, comflag);
    if (comflag) {
        LOGI("Conflict:same document no Insert");
        return E_OK;
    } else if (ret != E_OK) {
        LOGE("MateFile Conflict failed %{public}d", ret);
    } else {
        LOGI("MateFile Conflict OK");
    }
    ret = DentryInsert(userId_, record);
    if (ret != E_OK) {
        LOGE("MetaFile Create failed %{public}d", ret);
        return ret;
    }

    int64_t rowId;
    ValuesBucket values;
    ret = createConvertor_.RecordToValueBucket(record, values);
    if (ret != E_OK) {
        LOGE("record to valuebucket failed, ret=%{public}d", ret);
        return ret;
    }
    values.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    values.PutInt(Media::PhotoColumn::PHOTO_POSITION, POSITION_CLOUD);
    values.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
    values.PutString(Media::PhotoColumn::PHOTO_CLOUD_ID, record.GetRecordId());
    values.PutInt(
        Media::PhotoColumn::PHOTO_SYNC_STATUS,
        static_cast<int32_t>(params.fetchThumbs ? SyncStatusType::TYPE_DOWNLOAD : SyncStatusType::TYPE_VISIBLE));
    ret = Insert(rowId, values);
    if (ret != E_OK) {
        LOGE("Insert pull record failed, rdb ret=%{public}d", ret);
        return E_RDB;
    }
    fileId = rowId;
    LOGI("Insert recordId %s success", record.GetRecordId().c_str());
    if (params.fetchThumbs || AddCloudThumbs(record) != E_OK) {
        AppendToDownload(record, "lcd", params.assetsToDownload);
        AppendToDownload(record, "thumbnail", params.assetsToDownload);
    }
    return E_OK;
}

int32_t FileDataHandler::OnDownloadThumbSuccess(const DriveKit::DKDownloadAsset &asset)
{
    LOGI("update sync_status to visible of record %s", asset.recordId.c_str());
    int updateRows;
    ValuesBucket values;
    values.PutInt(Media::PhotoColumn::PHOTO_SYNC_STATUS, static_cast<int32_t>(SyncStatusType::TYPE_VISIBLE));

    string whereClause = Media::PhotoColumn::PHOTO_CLOUD_ID + " = ?";
    int32_t ret = Update(updateRows, values, whereClause, {asset.recordId});
    if (ret != E_OK) {
        LOGE("update retry flag failed, ret=%{public}d", ret);
        return E_RDB;
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

constexpr unsigned HMDFS_IOC = 0xf2;
constexpr unsigned WRITEOPEN_CMD = 0x02;
#define HMDFS_IOC_GET_WRITEOPEN_CNT _IOR(HMDFS_IOC, WRITEOPEN_CMD, uint32_t)

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

static bool LocalWriteOpen(const string &dfsPath)
{
    unique_ptr<char[]> absPath = make_unique<char[]>(PATH_MAX + 1);
    if (realpath(dfsPath.c_str(), absPath.get()) == nullptr) {
        return false;
    }
    string realPath = absPath.get();
    int fd = open(realPath.c_str(), O_RDONLY);
    if (fd < 0) {
        LOGE("open failed, errno:%{public}d", errno);
        return false;
    }
    uint32_t writeOpenCnt = 0;
    int ret = ioctl(fd, HMDFS_IOC_GET_WRITEOPEN_CNT, &writeOpenCnt);
    if (ret < 0) {
        LOGE("ioctl failed, errno:%{public}d", errno);
        return false;
    }

    close(fd);
    return writeOpenCnt != 0;
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

static int IsMtimeChanged(const DKRecord &record, NativeRdb::ResultSet &local, bool &changed)
{
    // get local mtime
    int64_t localMtime = 0;
    int ret = DataConvertor::GetLong(PhotoColumn::MEDIA_DATE_MODIFIED, localMtime, local);
    if (ret != E_OK) {
        LOGE("Get local mtime failed");
        return E_INVAL_ARG;
    }

    // get record mtime
    int64_t cloudMtime = 0;
    DKRecordData datas;
    record.GetRecordData(datas);
    if (datas.find(FILE_PROPERTIES) == datas.end()) {
        LOGE("record data cannot find properties");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap prop;
    datas[FILE_PROPERTIES].GetRecordMap(prop);
    if (prop.find(PhotoColumn::MEDIA_DATE_MODIFIED) == prop.end() ||
        DataConvertor::GetLongComp(prop[PhotoColumn::MEDIA_DATE_MODIFIED], cloudMtime) != E_OK) {
        LOGE("bad mtime in record");
        return E_INVAL_ARG;
    }

    LOGI("cloudMtime %{public}llu, localMtime %{public}llu",
         static_cast<unsigned long long>(cloudMtime), static_cast<unsigned long long>(localMtime));
    changed = !(cloudMtime == localMtime);
    return E_OK;
}

int32_t FileDataHandler::PullRecordUpdate(const DKRecord &record, NativeRdb::ResultSet &local, OnFetchParams &params)
{
    LOGI("update of record %s", record.GetRecordId().c_str());
    if (IsLocalDirty(local)) {
        LOGI("local record dirty, ignore cloud update");
        return E_OK;
    }
    bool mtimeChanged = false;
    if (IsMtimeChanged(record, local, mtimeChanged) != E_OK) {
        return E_INVAL_ARG;
    }

    int ret = E_OK;
    if (FileIsLocal(local)) {
        string filePath = GetFilePath(local);
        string localPath = GetLocalPath(userId_, filePath);
        if (LocalWriteOpen(localPath)) {
            return SetRetry(record.GetRecordId());
        }

        if (mtimeChanged) {
            LOGI("cloud file DATA changed, %s", record.GetRecordId().c_str());
            ret = unlink(localPath.c_str());
            if (ret != 0) {
                LOGE("unlink local failed, errno %{public}d", errno);
            }
            DentryInsert(userId_, record);
        }
    }

    LOGI("cloud file META changed, %s", record.GetRecordId().c_str());
    /* update rdb */
    ValuesBucket values;
    createConvertor_.RecordToValueBucket(record, values);
    values.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
    int32_t changedRows;
    string whereClause = PhotoColumn::PHOTO_CLOUD_ID + " = ?";
    ret = Update(changedRows, values, whereClause, {record.GetRecordId()});
    if (ret != E_OK) {
        LOGE("rdb update failed, err=%{public}d", ret);
        return E_RDB;
    }
    if (mtimeChanged && (ThumbsAtLocal(record) || (AddCloudThumbs(record) != E_OK))) {
        AppendToDownload(record, "lcd", params.assetsToDownload);
        AppendToDownload(record, "thumbnail", params.assetsToDownload);
    }

    LOGI("update of record success");
    return E_OK;
}

bool FileDataHandler::ThumbsAtLocal(const DKRecord &record)
{
    DKRecordData datas;
    record.GetRecordData(datas);
    if (datas.find(FILE_PROPERTIES) == datas.end()) {
        LOGE("record data cannot find properties");
        return false;
    }
    DriveKit::DKRecordFieldMap prop;
    datas[FILE_PROPERTIES].GetRecordMap(prop);

    string fullPath;
    if (prop.find(PhotoColumn::MEDIA_FILE_PATH) == prop.end() ||
        prop[PhotoColumn::MEDIA_FILE_PATH].GetString(fullPath) != DKLocalErrorCode::NO_ERROR) {
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

// if cloud delete but local exist, we would do recycle instead of delete
int FileDataHandler::RecycleFile(const string &recordId)
{
    LOGI("recycle of record %s", recordId.c_str());

    ValuesBucket values;
    values.PutNull(PhotoColumn::PHOTO_CLOUD_ID);
    values.PutInt(PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(DirtyType::TYPE_NEW));
    values.PutInt(PhotoColumn::PHOTO_POSITION, POSITION_LOCAL);
    values.PutLong(PhotoColumn::MEDIA_DATE_TRASHED, UTCTimeSeconds());
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

int32_t FileDataHandler::PullRecordDelete(const DKRecord &record, NativeRdb::ResultSet &local)
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
            string relativePath, fileName;
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

        // delete rdb
        int32_t deletedRows;
        int ret = Delete(deletedRows, Media::PhotoColumn::PHOTO_CLOUD_ID + " = ?", {record.GetRecordId()});
        if (ret != E_OK) {
            LOGE("delete in rdb failed, ret:%{public}d", ret);
            return E_INVAL_ARG;
        }
        return E_OK;
    }

    if (IsLocalDirty(local)) {
        LOGI("local record dirty, ignore cloud delete");
        return ClearCloudInfo(record.GetRecordId());
    }

    if (LocalWriteOpen(localPath)) {
        return SetRetry(record.GetRecordId());
    }
    return RecycleFile(record.GetRecordId());
}

int32_t FileDataHandler::OnDownloadSuccess(const DriveKit::DKDownloadAsset &asset)
{
    string filePath = localConvertor_.GetSandboxPath(asset.downLoadPath + "/" + asset.asset.assetName);

    int ret = E_OK;

    // delete dentry
    string relativePath, fileName;
    if (GetDentryPathName(filePath, relativePath, fileName) != E_OK) {
        LOGE("split to dentry path failed, path:%s", filePath.c_str());
        return E_INVAL_ARG;
    }
    auto mFile = MetaFileMgr::GetInstance().GetMetaFile(userId_, relativePath);
    MetaBase mBase(fileName);
    ret = mFile->DoRemove(mBase);
    if (ret != E_OK) {
        LOGE("remove dentry failed, ret:%{public}d", ret);
    }

    // update rdb
    ValuesBucket valuesBucket;
    valuesBucket.PutInt(Media::PhotoColumn::PHOTO_POSITION, POSITION_BOTH);

    int32_t changedRows;
    string whereClause = Media::PhotoColumn::MEDIA_FILE_PATH + " = ?";
    vector<string> whereArgs = {filePath};
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
    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGE("record data cannot find properties");
        return;
    }
    DKRecordFieldMap prop;
    data[FILE_PROPERTIES].GetRecordMap(prop);

    if (prop.find(PhotoColumn::MEDIA_FILE_PATH) == prop.end()) {
        LOGE("record prop cannot find file path");
        return;
    }
    string path;
    prop[PhotoColumn::MEDIA_FILE_PATH].GetString(path);
    if (fieldKey != "content") {
        const string &suffix = fieldKey == "lcd" ? LCD_SUFFIX : THUMB_SUFFIX;
        downloadAsset.downLoadPath = createConvertor_.GetThumbPath(path, suffix);
    } else {
        downloadAsset.downLoadPath = createConvertor_.GetLowerTmpPath(path);
    }
    downloadAsset.asset.assetName = MetaFile::GetFileName(downloadAsset.downLoadPath);
    downloadAsset.downLoadPath = MetaFile::GetParentDir(downloadAsset.downLoadPath);
    ForceCreateDirectory(downloadAsset.downLoadPath);
    assetsToDownload.push_back(downloadAsset);
}

int32_t FileDataHandler::DeleteDentryFile(void)
{
    std::string cacheDir =
        "/data/service/el2/" + std::to_string(userId_) + "/hmdfs/cache/account_cache/dentry_cache/cloud/";
    LOGD("cacheDir: %s", cacheDir.c_str());
    if (!filesystem::exists(filesystem::path(cacheDir)))
        return errno;

    for (const auto& entry : filesystem::directory_iterator(cacheDir)) {
        if (filesystem::is_directory(entry.path())) {
            OHOS::ForceRemoveDirectory(entry.path());
        } else {
            remove(entry.path());
        }
    }

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

int32_t FileDataHandler::CleanPureCloudRecord(NativeRdb::ResultSet &local, const int action,
                                              const std::string &filePath)
{
    int res = E_OK;
    string thmbFile = cleanConvertor_.GetThumbPath(filePath, THUMB_SUFFIX);
    filesystem::path thmbFilePath(thmbFile.c_str());
    filesystem::path thmbFileParentPath = thmbFilePath.parent_path();
    LOGD("filePath: %s, thmbFile: %s, thmbFileParentDir: %s", filePath.c_str(),
         thmbFile.c_str(), thmbFileParentPath.string().c_str());
    ForceRemoveDirectory(thmbFileParentPath.string());
    int32_t deleteRows = 0;
    string whereClause = PhotoColumn::PHOTO_CLOUD_ID + " = ?";
    string cloudId;
    res = DataConvertor::GetString(PhotoColumn::PHOTO_CLOUD_ID, cloudId, local);
    if (res != E_OK) {
        LOGE("Get cloud_id fail");
        return res;
    }
    res = Delete(deleteRows, whereClause, {cloudId});
    LOGD("RDB Delete result: %d, deleteRows is: %d", res, deleteRows);
    if (res != 0) {
        LOGE("Clean delete in rdb failed, res:%{public}d", res);
        return E_RDB;
    }
    return E_OK;
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

int32_t FileDataHandler::CleanNotDirtyData(const string &thmbDir, const string &assetPath, const string &cloudId)
{
    int ret = E_OK;
    ForceRemoveDirectory(thmbDir);
    ret = DeleteAsset(assetPath);
    if (ret != E_OK) {
        LOGE("Clean remove assetPath failed, errno %{public}d", ret);
        return ret;
    }
    int32_t deleteRows = 0;
    std::string whereClause = Media::PhotoColumn::PHOTO_CLOUD_ID + " = ?";
    ret = Delete(deleteRows, whereClause, {cloudId});
    LOGD("RDB Delete result: %d, deleteRows is: %d", ret, deleteRows);
    if (ret != 0) {
        LOGE("Clean delete in rdb failed, ret:%{public}d", ret);
        return ret;
    }
    return ret;
}

int32_t FileDataHandler::CleanNotPureCloudRecord(NativeRdb::ResultSet &local, const int action,
                                                 const std::string &filePath)
{
    string cloudId;
    int res = DataConvertor::GetString(PhotoColumn::PHOTO_CLOUD_ID, cloudId, local);
    if (res != E_OK) {
        LOGE("Get cloud_id fail");
        return res;
    }
    string lowerPath = cleanConvertor_.GetLowerPath(filePath);
    string thmbFile = cleanConvertor_.GetThumbPath(filePath, THUMB_SUFFIX);
    filesystem::path thmbFilePath(thmbFile.c_str());
    filesystem::path thmbFileParentPath = thmbFilePath.parent_path();
    LOGD("filePath: %s, thmbFile: %s, thmbFileParentPath: %s, lowerPath: %s", filePath.c_str(),
         thmbFile.c_str(), thmbFileParentPath.string().c_str(), lowerPath.c_str());
    int32_t ret = E_OK;
    if (action == FileDataHandler::Action::CLEAR_DATA) {
        if (IsLocalDirty(local)) {
            LOGD("data is dirty, action:clear");
            ret = ClearCloudInfo(cloudId);
            if (ret != E_OK) {
                LOGE("Clean Update in rdb failed, ret:%{public}d", ret);
                return ret;
            }
        } else {
            LOGD("data is not dirty, action:clear.");
            ret = CleanNotDirtyData(thmbFileParentPath.string(), lowerPath, cloudId);
            if (ret != 0) {
                LOGE("Clean dirty data failed.");
                return ret;
            }
        }
    } else {
        LOGD("action:retain");
        ret = ClearCloudInfo(cloudId);
        if (ret != E_OK) {
            LOGE("Clean Update in rdb failed, ret:%{public}d", ret);
            return ret;
        }
    }
    return E_OK;
}

int32_t FileDataHandler::CleanCloudRecord(NativeRdb::ResultSet &local, const int action,
                                          const std::string &filePath)
{
    int res = E_OK;
    if (!FileIsLocal(local)) {
        LOGD("File is pure cloud data");
        res = CleanPureCloudRecord(local, action, filePath);
        if (res != E_OK) {
            LOGE("Clean pure cloud record failed, res:%{public}d", res);
            return res;
        }
    } else {
        LOGD("File is not pure cloud data");
        res = CleanNotPureCloudRecord(local, action, filePath);
        if (res != E_OK) {
            LOGE("Clean no pure cloud record failed, res:%{public}d", res);
            return res;
        }
    }
    return res;
}

int32_t FileDataHandler::Clean(const int action)
{
    LOGD("Enter function FileDataHandler::Clean");
    int res = E_OK;
    NativeRdb::AbsRdbPredicates cleanPredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    cleanPredicates.IsNotNull(PhotoColumn::PHOTO_CLOUD_ID);
    cleanPredicates.Limit(LIMIT_SIZE);
    while (1) {
        auto resultSet = Query(cleanPredicates, CLEAN_QUERY_COLUMNS);
        if (resultSet == nullptr) {
            LOGE("get nullptr result");
            return E_RDB;
        }
        int count = 0;
        res = resultSet->GetRowCount(count);
        if (res != E_OK || count == 0) {
            LOGE("get row count error or row count is 0, res: %{public}d", res);
            break;
        }
        NativeRdb::ResultSet *resultSetPtr = resultSet.get();

        while (resultSet->GoToNextRow() == 0) {
            string filePath;
            res = DataConvertor::GetString(PhotoColumn::MEDIA_FILE_PATH, filePath, *resultSetPtr);
            if (res != E_OK) {
                LOGE("Get path wrong");
                return E_INVAL_ARG;
            }
            res = CleanCloudRecord(*resultSetPtr, action, filePath);
            if (res != E_OK) {
                LOGE("Clean cloud record failed, res:%{public}d", res);
                return res;
            }
        }
    }
    res = DeleteDentryFile();
    if (res != E_OK) {
        LOGE("Clean remove dentry failed, res:%{public}d", res);
        return res;
    }

    return E_OK;
}
int32_t FileDataHandler::GetCreatedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    NativeRdb::AbsRdbPredicates createPredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    createPredicates
        .EqualTo(Media::PhotoColumn::PHOTO_DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_NEW)))
        ->And()
        ->EqualTo(Media::PhotoColumn::MEDIA_DATE_TRASHED, "0")
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
    auto results = Query(createPredicates, GALLERY_FILE_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr created result");
        return E_RDB;
    }

    /* results to records */
    int ret = createConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
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
    deletePredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(deletePredicates, GALLERY_FILE_COLUMNS);
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
        .EqualTo(Media::PhotoColumn::PHOTO_DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_MDIRTY)))
        ->BeginWrap()
        ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_IMAGE))
        ->Or()
        ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_VIDEO))
        ->EndWrap();
    if (!modifyFailSet_.empty()) {
        updatePredicates.And()->NotIn(Media::PhotoColumn::PHOTO_CLOUD_ID, modifyFailSet_);
    }
    updatePredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(updatePredicates, GALLERY_FILE_COLUMNS);
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

    return E_OK;
}

int32_t FileDataHandler::GetDownloadAsset(std::string cloudId, vector<DriveKit::DKDownloadAsset> &outAssetsToDownload)
{
    vector<DKRecord> records;
    NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    predicates.SetWhereClause(Media::PhotoColumn::MEDIA_FILE_PATH + " = ?");
    predicates.SetWhereArgs({cloudId});
    predicates.Limit(LIMIT_SIZE);
    auto resultSet = Query(predicates, GALLERY_FILE_COLUMNS);
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

    ret = localConvertor_.ResultSetToRecords(move(resultSet), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }
    for (const auto &record : records) {
        AppendToDownload(record, "content", outAssetsToDownload);
    }

    return E_OK;
}

int32_t FileDataHandler::GetFileModifiedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    NativeRdb::AbsRdbPredicates updatePredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    updatePredicates
        .EqualTo(Media::PhotoColumn::PHOTO_DIRTY, to_string(static_cast<int32_t>(Media::DirtyType::TYPE_FDIRTY)))
        ->And()
        ->EqualTo(Media::PhotoColumn::MEDIA_DATE_TRASHED, "0")
        ->BeginWrap()
        ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_IMAGE))
        ->Or()
        ->EqualTo(Media::PhotoColumn::MEDIA_TYPE, to_string(Media::MEDIA_TYPE_VIDEO))
        ->EndWrap();
    if (!modifyFailSet_.empty()) {
        updatePredicates.And()->NotIn(Media::PhotoColumn::PHOTO_CLOUD_ID, modifyFailSet_);
    }
    updatePredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(updatePredicates, GALLERY_FILE_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr modified result");
        return E_RDB;
    }

    /* results to records */
    int ret = fdirtyConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

static string GetFilePathFromRecord(const DKRecord &record)
{
    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGE("record data cannot find properties");
        return "";
    }
    DriveKit::DKRecordFieldMap prop = data[FILE_PROPERTIES];
    if (prop.find(PhotoColumn::MEDIA_FILE_PATH) == prop.end()) {
        LOGE("record data cannot find file path");
        return "";
    }
    string path;
    if (prop[PhotoColumn::MEDIA_FILE_PATH].GetString(path) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        return "";
    }
    return path;
}

int32_t FileDataHandler::OnCreateRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
    GetLocalTimeMap(map, localMap, Media::PhotoColumn::MEDIA_FILE_PATH);

    int32_t ret = E_OK;
    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnCreateRecordSuccess(entry, localMap);
        } else {
            err = OnRecordFailed(entry);
        }
        if (err != E_OK) {
            string filePath = GetFilePathFromRecord(entry.second.GetDKRecord());
            if (!filePath.empty()) {
                createFailSet_.push_back(filePath);
            }
            LOGE("create record fail: file path %{private}s", filePath.c_str());
        }
        if (err == E_STOP) {
            ret = E_STOP;
        }
    }
    return ret;
}

int32_t FileDataHandler::OnDeleteRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    int32_t ret = E_OK;
    for (auto &entry : map) {
        const DKRecordOperResult &result = entry.second;
        int32_t err;
        if (result.IsSuccess()) {
            err = OnDeleteRecordSuccess(entry);
        } else {
            err = OnRecordFailed(entry);
        }
        if (err != E_OK) {
            modifyFailSet_.push_back(entry.first);
            LOGE("delete record fail: cloud id: %{private}s", entry.first.c_str());
        }
        if (err == E_STOP) {
            ret = E_STOP;
        }
    }
    return ret;
}

int32_t FileDataHandler::OnModifyMdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
    GetLocalTimeMap(map, localMap, Media::PhotoColumn::PHOTO_CLOUD_ID);

    int32_t ret = E_OK;
    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnModifyRecordSuccess(entry, localMap);
        } else {
            err = OnRecordFailed(entry);
        }
        if (err != E_OK) {
            modifyFailSet_.push_back(entry.first);
            LOGE("modify mdirty record fail: cloud id: %{private}s", entry.first.c_str());
        }
        if (err == E_STOP) {
            ret = E_STOP;
        }
    }
    return ret;
}

int32_t FileDataHandler::OnModifyFdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
    GetLocalTimeMap(map, localMap, Media::PhotoColumn::PHOTO_CLOUD_ID);

    int32_t ret = E_OK;
    for (auto &entry : map) {
        int32_t err;
        const DKRecordOperResult &result = entry.second;
        if (result.IsSuccess()) {
            err = OnModifyRecordSuccess(entry, localMap);
        } else {
            err = OnRecordFailed(entry);
        }
        if (err != E_OK) {
            modifyFailSet_.push_back(entry.first);
            LOGE("modify fdirty record fail: cloud id: %{public}s", entry.first.c_str());
        }
        if (err == E_STOP) {
            ret = E_STOP;
        }
    }
    return ret;
}

void FileDataHandler::Reset()
{
    modifyFailSet_.clear();
    createFailSet_.clear();
}

int32_t FileDataHandler::OnCreateRecordSuccess(
    const std::pair<DKRecordId, DKRecordOperResult> &entry,
    const std::map<std::string, std::pair<std::int64_t, std::int64_t>> &localMap)
{
    auto record = entry.second.GetDKRecord();

    DKRecordData data;
    record.GetRecordData(data);
    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGE("record data cannot find properties");
        return E_INVAL_ARG;
    }
    DriveKit::DKRecordFieldMap prop = data[FILE_PROPERTIES];
    if (prop.find(PhotoColumn::MEDIA_FILE_PATH) == prop.end()) {
        LOGE("record data cannot find file path");
        return E_INVAL_ARG;
    }

    string path;
    if (prop[PhotoColumn::MEDIA_FILE_PATH].GetString(path) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        return E_INVAL_ARG;
    }

    ValuesBucket valuesBucket;
    valuesBucket.PutString(Media::PhotoColumn::PHOTO_CLOUD_ID, entry.first);
    valuesBucket.PutInt(Media::PhotoColumn::PHOTO_POSITION, POSITION_BOTH);
    valuesBucket.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
    int32_t changedRows;
    string whereClause = Media::PhotoColumn::MEDIA_FILE_PATH + " = ?";
    vector<string> whereArgs = {path};

    /* compare mtime and metatime */
    if (OnCreateIsTimeChanged(data, localMap, path, Media::PhotoColumn::MEDIA_DATE_MODIFIED)) {
        valuesBucket.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_FDIRTY));
    } else if (OnCreateIsTimeChanged(data, localMap, path, PhotoColumn::PHOTO_META_DATE_MODIFIED)) {
        valuesBucket.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_MDIRTY));
    } else {
        valuesBucket.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));
    }

    int32_t ret = Update(changedRows, valuesBucket, whereClause, whereArgs);
    if (ret != 0) {
        LOGE("on create records update synced err %{public}d", ret);
        return ret;
    }
    return E_OK;
}

int32_t FileDataHandler::OnDeleteRecordSuccess(const std::pair<DKRecordId, DKRecordOperResult> &entry)
{
    string cloudId = entry.first;
    /* delete local */
    int32_t deletedRows;
    string whereClause = Media::PhotoColumn::PHOTO_CLOUD_ID + " = ?";
    int32_t ret = Delete(deletedRows, whereClause, {cloudId});
    if (ret != 0) {
        LOGE("on delete records update err %{public}d", ret);
        return ret;
    }
    return E_OK;
}

int32_t FileDataHandler::OnModifyRecordSuccess(
    const std::pair<DKRecordId, DKRecordOperResult> &entry,
    const std::map<std::string, std::pair<std::int64_t, std::int64_t>> &localMap)
{
    auto record = entry.second.GetDKRecord();
    DKRecordData data;
    record.GetRecordData(data);
    string cloudId = entry.first;

    /* compare mtime */
    if (OnModifyIsTimeChanged(data, localMap, cloudId, Media::PhotoColumn::MEDIA_DATE_MODIFIED)) {
        LOGI("mtime changed, need to update fdirty");
        return E_OK;
    }
    /* record to value bucket */
    ValuesBucket valuesBucket;
    valuesBucket.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));

    int32_t changedRows;
    string whereClause = Media::PhotoColumn::PHOTO_CLOUD_ID + " = ?";
    valuesBucket.PutLong(Media::PhotoColumn::PHOTO_CLOUD_VERSION, record.GetVersion());
    int32_t ret = Update(changedRows, valuesBucket, whereClause, {cloudId});
    if (ret != 0) {
        LOGE("on modify records update synced err %{public}d", ret);
        return ret;
    }

    /* compare metatime */
    if (OnModifyIsTimeChanged(data, localMap, cloudId, PhotoColumn::PHOTO_META_DATE_MODIFIED)) {
        LOGI("metatime changed, need to update mdirty");
        valuesBucket.PutInt(Media::PhotoColumn::PHOTO_DIRTY, static_cast<int32_t>(Media::DirtyType::TYPE_MDIRTY));
        ret = Update(changedRows, valuesBucket, whereClause, {cloudId});
        if (ret != 0) {
            LOGE("on modify records update mdirty err %{public}d", ret);
            return ret;
        }
    }
    return E_OK;
}

bool FileDataHandler::OnCreateIsTimeChanged(
    const DKRecordData &data,
    const std::map<std::string, std::pair<std::int64_t, std::int64_t>> &localMap,
    const std::string &path,
    const std::string &type)
{
    int64_t cloudtime;
    int64_t localtime;

    /* any error will return true，do not update to synced */
    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGE("record data cannot find properties");
        return true;
    }
    DKRecordFieldMap prop = const_cast<DKRecordData&>(data)[FILE_PROPERTIES];
    if (prop.find(type) == prop.end()) {
        LOGE("record data cannot find some properties");
        return true;
    }

    if (prop[type].GetLong(cloudtime) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        return true;
    }
    auto it = localMap.find(path);
    if (it == localMap.end()) {
        return true;
    }
    /* get mtime or metatime */
    if (type == Media::PhotoColumn::MEDIA_DATE_MODIFIED) {
        localtime = it->second.first;
    } else {
        localtime = it->second.second;
    }

    /* compare mtime metatime */
    if (localtime == cloudtime) {
        return false;
    }
    return true;
}

bool FileDataHandler::OnModifyIsTimeChanged(
    const DKRecordData &data,
    const std::map<std::string, std::pair<std::int64_t, std::int64_t>> &localMap,
    const std::string &cloudId,
    const std::string &type)
{
    int64_t cloudtime;
    int64_t localtime;

    /* any error will return true，do not update to synced */
    if (data.find(FILE_PROPERTIES) == data.end()) {
        LOGE("record data cannot find properties");
        return true;
    }
    DKRecordFieldMap prop = const_cast<DKRecordData&>(data)[FILE_PROPERTIES];
    if (prop.find(type) == prop.end()) {
        LOGE("record data cannot find some properties");
        return true;
    }

    if (prop[type].GetLong(cloudtime) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        return true;
    }
    auto it = localMap.find(cloudId);
    if (it == localMap.end()) {
        return true;
    }
    /* get mtime or metatime */
    if (type == Media::PhotoColumn::MEDIA_DATE_MODIFIED) {
        localtime = it->second.first;
    } else {
        localtime = it->second.second;
    }

    /* compare mtime or metatime */
    if (localtime == cloudtime) {
        return false;
    }
    return true;
}

void FileDataHandler::GetLocalTimeMap(const std::map<DKRecordId, DKRecordOperResult> &map,
                                      std::map<std::string, std::pair<std::int64_t, std::int64_t>> &cloudMap,
                                      const std::string &type)
{
    std::vector<std::string> path;
    for (auto &entry : map) {
        if (type == Media::PhotoColumn::PHOTO_CLOUD_ID) {
            path.push_back(entry.first);
        } else {
            auto record = entry.second.GetDKRecord();
            DKRecordData data;
            record.GetRecordData(data);
            if (data.find(FILE_PROPERTIES) == data.end()) {
                LOGE("record data cannot find properties");
                return;
            }
            DriveKit::DKRecordFieldMap prop = data[FILE_PROPERTIES];
            if (prop.find(PhotoColumn::MEDIA_FILE_PATH) == prop.end()) {
                LOGE("record data cannot find some properties");
                return;
            }
            string curPath;
            if (prop[PhotoColumn::MEDIA_FILE_PATH].GetString(curPath) != DKLocalErrorCode::NO_ERROR) {
                LOGE("bad file_path in props");
                return;
            }
            path.push_back(curPath);
        }
    }
    NativeRdb::AbsRdbPredicates createPredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    createPredicates.And()->In(type, path);
    auto resultSet = Query(createPredicates, GALLERY_FILE_COLUMNS);
    if (resultSet == nullptr) {
        return;
    }

    OnResultSetConvertToMap(move(resultSet), cloudMap, type);
}

void FileDataHandler::OnResultSetConvertToMap(const shared_ptr<NativeRdb::ResultSet> resultSet,
                                              std::map<std::string, std::pair<std::int64_t, std::int64_t>> &cloudMap,
                                              const std::string &type)
{
    int32_t idIndex = -1;
    int32_t mtimeIndex = -1;
    int32_t metatimeIndex = -1;
    resultSet->GetColumnIndex(type, idIndex);
    resultSet->GetColumnIndex(Media::PhotoColumn::MEDIA_DATE_MODIFIED, mtimeIndex);
    resultSet->GetColumnIndex(PhotoColumn::PHOTO_META_DATE_MODIFIED, metatimeIndex);

    /* iterate all rows compare mtime metatime */
    while (resultSet->GoToNextRow() == 0) {
        string idValue;
        int64_t mtime;
        int64_t metatime;
        if (resultSet->GetString(idIndex, idValue) == 0 && resultSet->GetLong(mtimeIndex, mtime) == 0 &&
            resultSet->GetLong(metatimeIndex, metatime) == 0) {
            cloudMap.insert(std::make_pair(idValue, std::make_pair(mtime, metatime)));
        }
    }
}

int32_t FileDataHandler::OnRecordFailed(const std::pair<DKRecordId, DKRecordOperResult> &entry)
{
    const DKRecordOperResult &result = entry.second;
    int32_t serverErrorCode = INT32_MAX;
    serverErrorCode = result.GetDKError().serverErrorCode;
    LOGE("serverErrorCode %{public}d", serverErrorCode);
    if (result.GetDKError().errorDetails.size() == 0) {
        LOGE("errorDetails is empty");
        return E_INVAL_ARG;
    }
    string errorDetailcode = "";
    errorDetailcode = result.GetDKError().errorDetails[0].errorCode;
    if (static_cast<DKServerErrorCode>(serverErrorCode) == DKServerErrorCode::ACCESS_DENIED &&
        errorDetailcode == SPACE_NOT_ENOUGH) {
        return HandleCloudSpaceNotEnough();
    } else if (static_cast<DKServerErrorCode>(serverErrorCode) == DKServerErrorCode::AUTHENTICATION_FAILED &&
               errorDetailcode == AT_FAILED) {
        return HandleATFailed();
    } else if (static_cast<DKServerErrorCode>(serverErrorCode) == DKServerErrorCode::ACCESS_DENIED &&
               errorDetailcode == NAME_CONFLICT) {
        return HandleNameConflict();
    } else if (static_cast<DKServerErrorCode>(serverErrorCode) == DKServerErrorCode::ATOMIC_ERROR &&
               errorDetailcode == INVALID_FILE) {
        return HandleNameInvalid();
    } else {
        LOGE(" unknown error code record failed, serverErrorCode = %{public}d, errorDetailcode = %{public}s",
             serverErrorCode, errorDetailcode.c_str());
        return E_STOP;
    }
    return E_OK;
}

int32_t FileDataHandler::HandleCloudSpaceNotEnough()
{
    LOGE("Cloud Space Not Enough");
    /* Stop sync */
    return E_STOP;
}

int32_t FileDataHandler::HandleATFailed()
{
    LOGE("AT Failed");
    return E_OK;
}

int32_t FileDataHandler::HandleNameConflict()
{
    LOGE("Name Conflict");
    return E_OK;
}

int32_t FileDataHandler::HandleNameInvalid()
{
    LOGE("Name Invalid");
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

void FileDataHandler::SetNextCursor(const DriveKit::DKQueryCursor &cursor)
{
    nextCursor_ = cursor;
    cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
}

void FileDataHandler::SetTempStartCursor(const DriveKit::DKQueryCursor &cursor)
{
    tempStartCursor_ = cursor;
    cloudPrefImpl_.SetString(TEMP_START_CURSOR, nextCursor_);
}

void FileDataHandler::ClearCursor()
{
    startCursor_.clear();
    nextCursor_.clear();
    cloudPrefImpl_.SetString(START_CURSOR, startCursor_);
    cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
    if (!tempStartCursor_.empty()) {
        tempStartCursor_.clear();
        cloudPrefImpl_.Delete(TEMP_START_CURSOR);
    }
}

void FileDataHandler::SetTotalPullCount(const int32_t totalPullCount)
{
    totalPullCount_ = totalPullCount;
    cloudPrefImpl_.SetInt(TOTAL_PULL_COUNT, totalPullCount_);
}

void FileDataHandler::FinishPull(const DriveKit::DKQueryCursor &nextCursor)
{
    if (IsPullRecords()) {
        startCursor_ = tempStartCursor_;
        nextCursor_.clear();
        tempStartCursor_.clear();
        totalPullCount_ = 0;
        cloudPrefImpl_.SetString(START_CURSOR, startCursor_);
        cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
        cloudPrefImpl_.Delete(TEMP_START_CURSOR);
        cloudPrefImpl_.Delete(TOTAL_PULL_COUNT);
    } else {
        startCursor_ = nextCursor;
        nextCursor_.clear();
        cloudPrefImpl_.SetString(START_CURSOR, startCursor_);
        cloudPrefImpl_.SetString(NEXT_CURSOR, nextCursor_);
    }
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
