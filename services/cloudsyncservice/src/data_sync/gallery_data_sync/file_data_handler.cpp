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

#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "dfs_error.h"
#include "dk_assets_downloader.h"
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

FileDataHandler::FileDataHandler(int32_t userId, const string &bundleName, std::shared_ptr<RdbStore> rdb)
    : RdbDataHandler(TABLE_NAME, rdb), userId_(userId), bundleName_(bundleName)
{
}

void FileDataHandler::GetFetchCondition(FetchCondition &cond)
{
    cond.limitRes = LIMIT_SIZE;
    cond.recordType = recordType_;
    cond.desiredKeys = desiredKeys_;
}

static void ThumbDownloadCallback(std::shared_ptr<DKContext> context,
                                  std::shared_ptr<const DKDatabase> database,
                                  const std::map<DKDownloadAsset, DKDownloadResult> &resultMap,
                                  const DKError &err)
{
    if (err.isLocalError || err.isServerError) {
        //retry flag
        LOGE("DKAssetsDownloader err");
    } else {
        LOGI("DKAssetsDownloader ok");
    }
}

const std::vector<std::string> PULL_QUERY_COLUMNS = {
    MEDIA_DATA_DB_FILE_PATH,
    MEDIA_DATA_DB_SIZE,
    MEDIA_DATA_DB_DATE_MODIFIED,
    MEDIA_DATA_DB_DIRTY,
    MEDIA_DATA_DB_IS_TRASH,
    MEDIA_DATA_DB_POSITION,
};

int32_t FileDataHandler::OnFetchRecords(const shared_ptr<vector<DKRecord>> &records,
                                        vector<DKDownloadAsset> &outAssetsToDownload,
                                        shared_ptr<std::function<void(std::shared_ptr<DriveKit::DKContext>,
                                        std::shared_ptr<const DriveKit::DKDatabase>,
                                        const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &,
                                        const DriveKit::DKError &)>> &downloadResultCallback)
{
    LOGI("on fetch %{public}zu records", records->size());
    int32_t ret = E_OK;
    for (const auto &record : *records) {
        NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
        predicates.SetWhereClause(MEDIA_DATA_DB_CLOUD_ID + " = ?");
        predicates.SetWhereArgs({record.GetRecordId()});
        predicates.Limit(LIMIT_SIZE);
        auto resultSet = Query(predicates, PULL_QUERY_COLUMNS);
        if (resultSet == nullptr) {
            LOGE("get nullptr created result");
            ret = E_RDB;
            break;
        }
        int32_t rowCount = 0;
        ret = resultSet->GetRowCount(rowCount);
        if (ret != 0) {
            LOGE("result set get row count err %{public}d", ret);
            ret = E_RDB;
            break;
        }

        bool needPullThumbs = false;
        if ((rowCount == 0) && !record.GetIsDelete()) {
            ret = PullRecordInsert(record, needPullThumbs);
        } else if (rowCount == 1) {
            resultSet->GoToNextRow();
            if (record.GetIsDelete()) {
                ret = PullRecordDelete(record, *resultSet);
            } else {
                ret = PullRecordUpdate(record, *resultSet, needPullThumbs);
            }
        } else {
            LOGE("recordId %s has multiple file in db!", record.GetRecordId().c_str());
        }
        if (ret == E_RDB) {
            break;
        }
        if (needPullThumbs) {
            AppendToDownload(record, "lcd", outAssetsToDownload);
            AppendToDownload(record, "thumbnail", outAssetsToDownload);
        }
    }
        downloadResultCallback = make_shared<std::function<void(std::shared_ptr<DriveKit::DKContext>,
                           std::shared_ptr<const DriveKit::DKDatabase>,
                           const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &,
                           const DriveKit::DKError &)>>(ThumbDownloadCallback);
    MetaFileMgr::GetInstance().ClearAll();
    return ret;
}

static int GetDentryPathName(const string &fullPath, string &outPath, string &outFilename)
{
    const string sandboxPrefix = "/storage/media/local";
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
    if (prop.find(MEDIA_DATA_DB_FILE_PATH) == prop.end() || prop.find(MEDIA_DATA_DB_SIZE) == prop.end() ||
        prop.find(MEDIA_DATA_DB_DATE_MODIFIED) == prop.end()) {
        LOGE("record data cannot find some properties");
        return E_INVAL_ARG;
    }

    string fullPath, relativePath, fileName;
    if (prop[MEDIA_DATA_DB_FILE_PATH].GetString(fullPath) != DKLocalErrorCode::NO_ERROR) {
        LOGE("bad file_path in props");
        return E_INVAL_ARG;
    }
    if (GetDentryPathName(fullPath, relativePath, fileName) != E_OK) {
        LOGE("split to dentry path failed, path:%s", fullPath.c_str());
        return E_INVAL_ARG;
    }

    int64_t isize, mtime;
    if (DataConvertor::GetLongComp(prop[MEDIA_DATA_DB_SIZE], isize) != E_OK) {
        LOGE("bad size in props");
        return E_INVAL_ARG;
    }
    if (DataConvertor::GetLongComp(prop[MEDIA_DATA_DB_DATE_MODIFIED], mtime) != E_OK) {
        LOGE("bad mtime in props");
        return E_INVAL_ARG;
    }

    string rawRecordId = record.GetRecordId();
    string cloudId = MetaFileMgr::RecordIdToCloudId(rawRecordId);
    auto mFile = MetaFileMgr::GetInstance().GetMetaFile(userId, relativePath);
    MetaBase mBaseLookup(fileName);
    MetaBase mBase(fileName, cloudId);
    mBase.size = isize;
    mBase.mtime = mtime;
    if (mFile->DoLookup(mBaseLookup) == E_OK) {
        LOGE("dentry exist when insert, do update instead");
        return mFile->DoUpdate(mBase);
    }
    return mFile->DoCreate(mBase);
}

int32_t FileDataHandler::PullRecordInsert(const DKRecord &record, bool &outPullThumbs)
{
    LOGI("insert of record %s", record.GetRecordId().c_str());

    /* check local file conflict */
    int ret = DentryInsert(userId_, record);
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
    values.PutInt(Media::MEDIA_DATA_DB_POSITION, POSITION_CLOUD);
    values.PutString(Media::MEDIA_DATA_DB_CLOUD_ID, record.GetRecordId());
    ret = Insert(rowId, values);
    if (ret != E_OK) {
        LOGE("Insert pull record failed, rdb ret=%{public}d", ret);
        return E_RDB;
    }

    LOGI("Insert recordId %s success", record.GetRecordId().c_str());

    outPullThumbs = true;
    return E_OK;
}

static bool IsLocalDirty(NativeRdb::ResultSet &local)
{
    int dirty, trash;
    int ret = DataConvertor::GetInt(MEDIA_DATA_DB_DIRTY, dirty, local);
    if (ret != E_OK) {
        LOGE("Get dirty int failed");
        return false;
    }
    ret = DataConvertor::GetInt(MEDIA_DATA_DB_IS_TRASH, trash, local);
    if (ret != E_OK) {
        LOGE("Get dirty int failed");
        return false;
    }
    return (dirty == static_cast<int32_t>(DirtyType::TYPE_MDIRTY)) ||
           (dirty == static_cast<int32_t>(DirtyType::TYPE_FDIRTY)) ||
           (dirty == static_cast<int32_t>(DirtyType::TYPE_DELETED)) || (trash != 0);
}

constexpr unsigned HMDFS_IOC = 0xf2;
constexpr unsigned WRITEOPEN_CMD = 0x02;
#define HMDFS_IOC_GET_WRITEOPEN_CNT _IOR(HMDFS_IOC, WRITEOPEN_CMD, uint32_t)

static string GetFilePath(NativeRdb::ResultSet &local)
{
    string filePath;
    int ret = DataConvertor::GetString(MEDIA_DATA_DB_FILE_PATH, filePath, local);
    if (ret != E_OK) {
        LOGE("Get file path failed");
        return "";
    }
    return filePath;
}

static string GetLocalPath(int userId, const string &filePath)
{
    const string sandboxPrefix = "/storage/media/local";
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
    int fd = open(dfsPath.c_str(), O_RDONLY);
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
    values.PutInt(MEDIA_DATA_DB_DIRTY, static_cast<int32_t>(DirtyType::TYPE_RETRY));

    string whereClause = MEDIA_DATA_DB_CLOUD_ID + " = ?";
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
    int ret = DataConvertor::GetInt(MEDIA_DATA_DB_POSITION, position, local);
    if (ret != E_OK) {
        LOGE("Get local position failed");
        return E_INVAL_ARG;
    }

    return !!(position & 1);
}

static int IsMtimeChanged(const DKRecord &record, NativeRdb::ResultSet &local, bool &changed)
{
    // get local mtime
    int64_t localMtime = 0;
    int ret = DataConvertor::GetLong(MEDIA_DATA_DB_DATE_MODIFIED, localMtime, local);
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
    if (prop.find(MEDIA_DATA_DB_DATE_MODIFIED) == prop.end() ||
        DataConvertor::GetLongComp(prop[MEDIA_DATA_DB_DATE_MODIFIED], cloudMtime) != E_OK) {
        LOGE("bad mtime in record");
        return E_INVAL_ARG;
    }

    LOGI("cloudMtime %{public}llu, localMtime %{public}llu", (unsigned long long)cloudMtime, (unsigned long long)localMtime);
    changed = !(cloudMtime == localMtime);
    return E_OK;
}

int32_t FileDataHandler::PullRecordUpdate(const DKRecord &record, NativeRdb::ResultSet &local, bool &outPullThumbs)
{
    LOGI("update of record %s", record.GetRecordId().c_str());
    if (IsLocalDirty(local)) {
        LOGI("local record dirty, ignore cloud update");
        return E_OK;
    }

    int ret = E_OK;
    if (FileIsLocal(local)) {
        string filePath = GetFilePath(local);
        string localPath = GetLocalPath(userId_, filePath);
        if (LocalWriteOpen(localPath)) {
            return SetRetry(record.GetRecordId());
        }

        bool mtimeChanged = false;
        if (IsMtimeChanged(record, local, mtimeChanged) != E_OK) {
            return E_INVAL_ARG;
        }
        if (mtimeChanged) {
            LOGI("cloud file DATA changed, %s", record.GetRecordId().c_str());
            ret = unlink(localPath.c_str());
            if (ret != 0) {
                LOGE("unlink of %s failed ,errno %{public}d", localPath.c_str(), errno);
            }
            DentryInsert(userId_, record);

            // delete thumbnail
            outPullThumbs = true;
        } else {
            LOGI("cloud file META changed, %s", record.GetRecordId().c_str());
        }
    }

    /* update rdb */
    ValuesBucket values;
    createConvertor_.RecordToValueBucket(record, values);
    int32_t changedRows;
    string whereClause = MEDIA_DATA_DB_CLOUD_ID + " = ?";
    ret = Update(changedRows, values, whereClause, {record.GetRecordId()});
    if (ret != E_OK) {
        LOGE("rdb update failed, err=%{public}d", ret);
        return E_RDB;
    }

    LOGI("update of record success");
    return E_OK;
}


int FileDataHandler::RecycleFile(const string &recordId)
{
    LOGI("recycle of record %s", recordId.c_str());
    ValuesBucket values;
    values.PutInt(MEDIA_DATA_DB_IS_TRASH, 1);
    int32_t changedRows;
    string whereClause = MEDIA_DATA_DB_CLOUD_ID + " = ?";
    int ret = Update(changedRows, values, whereClause, {recordId});
    if (ret != E_OK) {
        LOGE("rdb update failed, err=%{public}d", ret);
        return E_RDB;
    }

    // do force delete instead, before medialibrary API10
    int32_t deletedRows;
    ret = Delete(deletedRows, whereClause, {recordId});
    if (ret != 0) {
        LOGE("delete in rdb failed, ret:%{public}d", ret);
        return E_RDB;
    }
    LOGI("force delete instead");

    return E_OK;
}

int32_t FileDataHandler::PullRecordDelete(const DKRecord &record, NativeRdb::ResultSet &local)
{
    LOGI("delete of record %s", record.GetRecordId().c_str());

    if (IsLocalDirty(local)) {
        LOGI("local record dirty, ignore cloud delete");
        return E_OK;
    }
    string filePath = GetFilePath(local);
    string localPath = GetLocalPath(userId_, filePath);

    int ret = E_OK;
    if (FileIsLocal(local)) {
        if (LocalWriteOpen(localPath)) {
            return SetRetry(record.GetRecordId());
        }

        ret = RecycleFile(record.GetRecordId());
    } else {
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

        // delete rdb
        int32_t deletedRows;
        string whereClause = Media::MEDIA_DATA_DB_CLOUD_ID + " = ?";
        ret = Delete(deletedRows, whereClause, {record.GetRecordId()});
        if (ret != 0) {
            LOGE("delete in rdb failed, ret:%{public}d", ret);
        }

        // delete thumbnail
        LOGI("force delete success");
    }

    return ret;
}

static std::string GetParentDir(const std::string &path)
{
    if ((path == "/") || (path == "")) {
        return "";
    }

    auto pos = path.find_last_of('/');
    if ((pos == std::string::npos) || (pos == 0)) {
        return "/";
    }

    return path.substr(0, pos);
}

static std::string GetFileName(const std::string &path)
{
    if ((path == "/") || (path == "")) {
        return "";
    }

    auto pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return "";
    }

    return path.substr(pos + 1);
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
    if (prop.find(MEDIA_DATA_DB_FILE_PATH) == prop.end()) {
        LOGE("record prop cannot find file path");
        return;
    }
    string path;
    prop[MEDIA_DATA_DB_FILE_PATH].GetString(path);
    if (prop.find(fieldKey) == prop.end()) {
        LOGE("record prop cannot find fieldKey");
        return;
    }
    string key;
    prop[fieldKey].GetString(key);
    downloadAsset.downLoadPath = createConvertor_.GetThumbPathDownloadLCD(path, key);
    downloadAsset.asset.assetName = GetFileName(downloadAsset.downLoadPath);
    downloadAsset.downLoadPath = GetParentDir(downloadAsset.downLoadPath);
    assetsToDownload.push_back(downloadAsset);
}

int32_t FileDataHandler::GetCreatedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    NativeRdb::AbsRdbPredicates createPredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    createPredicates.SetWhereClause(Media::MEDIA_DATA_DB_DIRTY + " = ? AND " +
        Media::MEDIA_DATA_DB_IS_TRASH + " = ? AND (" + Media::MEDIA_DATA_DB_MEDIA_TYPE +
        " = ? OR " + Media::MEDIA_DATA_DB_MEDIA_TYPE + " = ?)");
    createPredicates.SetWhereArgs({to_string(static_cast<int32_t>(Media::DirtyType::TYPE_NEW)),
        "0", to_string(Media::MEDIA_TYPE_IMAGE), to_string(Media::MEDIA_TYPE_VIDEO)});
    /* small-size first */
    createPredicates.OrderByAsc(Media::MEDIA_DATA_DB_SIZE);
    createPredicates.Offset(createOffset_);
    createPredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(createPredicates, GALLERY_FILE_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr created result");
        return E_RDB;
    }
    /* update offset */
    createOffset_ += LIMIT_SIZE;

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
    deletePredicates.SetWhereClause(Media::MEDIA_DATA_DB_DIRTY + " = ? AND (" +
        Media::MEDIA_DATA_DB_MEDIA_TYPE + " = ? OR " + Media::MEDIA_DATA_DB_MEDIA_TYPE + " = ?)");
    deletePredicates.SetWhereArgs({to_string(static_cast<int32_t>(Media::DirtyType::TYPE_DELETED)),
        to_string(Media::MEDIA_TYPE_IMAGE), to_string(Media::MEDIA_TYPE_VIDEO)});
    deletePredicates.Offset(deleteOffset_);
    deletePredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(deletePredicates, GALLERY_FILE_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr deleted result");
        return E_RDB;
    }
    /* update offset */
    deleteOffset_ += LIMIT_SIZE;

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
    updatePredicates.SetWhereClause(Media::MEDIA_DATA_DB_DIRTY + " = ? AND " +
        Media::MEDIA_DATA_DB_IS_TRASH + " = ? AND (" + Media::MEDIA_DATA_DB_MEDIA_TYPE +
        " = ? OR " + Media::MEDIA_DATA_DB_MEDIA_TYPE + " = ?)");
    updatePredicates.SetWhereArgs({to_string(static_cast<int32_t>(Media::DirtyType::TYPE_MDIRTY)),
        "0", to_string(Media::MEDIA_TYPE_IMAGE), to_string(Media::MEDIA_TYPE_VIDEO)});
    updatePredicates.Offset(mdirtyOffset_);
    updatePredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(updatePredicates, GALLERY_FILE_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr modified result");
        return E_RDB;
    }
    /* update offset */
    mdirtyOffset_ += LIMIT_SIZE;

    /* results to records */
    int ret = mdirtyConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::GetFileModifiedRecords(vector<DKRecord> &records)
{
    /* build predicates */
    NativeRdb::AbsRdbPredicates updatePredicates = NativeRdb::AbsRdbPredicates(TABLE_NAME);
    updatePredicates.SetWhereClause(Media::MEDIA_DATA_DB_DIRTY + " = ? AND " +
        Media::MEDIA_DATA_DB_IS_TRASH + " = ? AND (" + Media::MEDIA_DATA_DB_MEDIA_TYPE +
        " = ? OR " + Media::MEDIA_DATA_DB_MEDIA_TYPE + " = ?)");
    updatePredicates.SetWhereArgs({to_string(static_cast<int32_t>(Media::DirtyType::TYPE_FDIRTY)),
        "0", to_string(Media::MEDIA_TYPE_IMAGE), to_string(Media::MEDIA_TYPE_VIDEO)});
    updatePredicates.Offset(fdirtyOffset_);
    updatePredicates.Limit(LIMIT_SIZE);

    /* query */
    auto results = Query(updatePredicates, GALLERY_FILE_COLUMNS);
    if (results == nullptr) {
        LOGE("get nullptr modified result");
        return E_RDB;
    }
    /* update offset */
    fdirtyOffset_ += LIMIT_SIZE;

    /* results to records */
    int ret = fdirtyConvertor_.ResultSetToRecords(move(results), records);
    if (ret != 0) {
        LOGE("result set to records err %{public}d", ret);
        return ret;
    }

    return E_OK;
}

int32_t FileDataHandler::OnCreateRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    for (auto &entry : map) {
        DKRecordOperResult &result = const_cast<DKRecordOperResult &>(entry.second);
        if (!result.IsSuccess()) {
            LOGE("on create record err %{public}d", result.GetDKError().dkErrorCode);
            continue;
        }

        auto record = result.GetDKRecord();
        /* record to value bucket */
        ValuesBucket valuesBucket;
        valuesBucket.PutString(Media::MEDIA_DATA_DB_CLOUD_ID, entry.first);
        valuesBucket.PutInt(Media::MEDIA_DATA_DB_DIRTY,
            static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));

        DKRecordData data;
        record.GetRecordData(data);
        auto iter = data.find(FILE_FILE_NAME);
        if (iter == data.end()) {
            LOGE("no id in record data");
            continue;
        }
        string fileName = iter->second;

        /* update local */
        int32_t changedRows;
        string whereClause = Media::MEDIA_DATA_DB_NAME + " = ?";
        int32_t ret = Update(changedRows, valuesBucket, whereClause, { fileName });
        if (ret != 0) {
            LOGE("on create records update err %{public}d, file name %{private}s",
                ret, fileName.c_str());
            continue;
        }
    }

    return E_OK;
}

int32_t FileDataHandler::OnDeleteRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    for (auto &entry : map) {
        DKRecordOperResult &result = const_cast<DKRecordOperResult &>(entry.second);
        string cloudId = entry.first;
        if (!result.IsSuccess()) {
            LOGE("on delete record %{private}s err %{public}d", cloudId.c_str(),
                result.GetDKError().dkErrorCode);
            continue;
        }

        /* delete local */
        int32_t deletedRows;
        string whereClause = Media::MEDIA_DATA_DB_CLOUD_ID + " = ?";
        int32_t ret = Delete(deletedRows, whereClause, { cloudId });
        if (ret != 0) {
            LOGE("on delete records update err %{public}d, cloudId %{private}s",
                ret, cloudId.c_str());
            continue;
        }
    }

    return E_OK;
}

int32_t FileDataHandler::OnModifyMdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    for (auto &entry : map) {
        DKRecordOperResult &result = const_cast<DKRecordOperResult &>(entry.second);
        string cloudId = entry.first;
        if (!result.IsSuccess()) {
            LOGE("on modify mdirty record %{private}s err %{public}d", cloudId.c_str(),
                result.GetDKError().dkErrorCode);
            continue;
        }

        /* record to value bucket */
        ValuesBucket valuesBucket;
        valuesBucket.PutInt(Media::MEDIA_DATA_DB_DIRTY,
            static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));

        /* update local */
        int32_t changedRows;
        string whereClause = Media::MEDIA_DATA_DB_CLOUD_ID + " = ?";
        int32_t ret = Update(changedRows, valuesBucket, whereClause, { cloudId });
        if (ret != 0) {
            LOGE("on modify mdirty records update err %{public}d, cloudId %{private}s",
                ret, cloudId.c_str());
            continue;
        }
    }

    return E_OK;
}

int32_t FileDataHandler::OnModifyFdirtyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    for (auto &entry : map) {
        DKRecordOperResult &result = const_cast<DKRecordOperResult &>(entry.second);
        string cloudId = entry.first;
        if (!result.IsSuccess()) {
            LOGE("on modify fdirty record %{private}s err %{public}d", cloudId.c_str(),
                result.GetDKError().dkErrorCode);
            continue;
        }

        DKRecord record = result.GetDKRecord();
        /* record to value bucket */
        ValuesBucket valuesBucket;
        int32_t ret = fdirtyConvertor_.RecordToValueBucket(record, valuesBucket);
        if (ret != E_OK) {
            LOGE("record to value bucket err %{public}d", ret);
            continue;
        }
        valuesBucket.PutInt(Media::MEDIA_DATA_DB_DIRTY,
            static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED));

        /* update local */
        int32_t changedRows;
        string whereClause = Media::MEDIA_DATA_DB_CLOUD_ID + " = ?";
        ret = Update(changedRows, valuesBucket, whereClause, { cloudId });
        if (ret != 0) {
            LOGE("on modify fdirty records update err %{public}d, cloudId %{private}s",
                ret, cloudId.c_str());
            continue;
        }
    }

    return E_OK;
}

void FileDataHandler::Reset()
{
    createOffset_ = 0;
    deleteOffset_ = 0;
    mdirtyOffset_ = 0;
    fdirtyOffset_ = 0;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
