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

#include "clouddisk_rdbstore.h"

#include <ctime>
#include <sys/stat.h>
#include <sstream>

#include "clouddisk_sync_helper.h"
#include "clouddisk_rdb_utils.h"
#include "clouddisk_type_const.h"
#include "file_column.h"
#include "rdb_errno.h"
#include "rdb_sql_utils.h"
#include "utils_log.h"
#include "dfs_error.h"
#include "mimetype_utils.h"

namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using namespace OHOS::NativeRdb;
using namespace CloudSync;
using namespace OHOS::Media;

enum XATTR_CODE {
    ERROR_CODE = -1,
    CLOUD_LOCATION = 1,
    CLOUD_RECYCLE,
    IS_FAVORITE
};
static constexpr int32_t LOOKUP_QUERY_LIMIT = 1;
static const uint32_t SET_STATE = 1;
static const uint32_t CANCEL_STATE = 0;

static const std::string CloudSyncTriggerFunc(const std::vector<std::string> &args)
{
    size_t size = args.size();
    if (size != ARGS_SIZE) {
        LOGE("CloudSyncTriggerFunc args size error, %{public}zu", size);
        return "";
    }
    int32_t userId = std::strtol(args[ARG_USER_ID].c_str(), nullptr, 0);
    string bundleName = args[ARG_BUNDLE_NAME];
    LOGD("begin cloud sync trigger, bundleName: %{public}s, userId: %{public}d", bundleName.c_str(), userId);
    CloudDiskSyncHelper::GetInstance().RegisterTriggerSync(bundleName, userId);
    return "";
}

CloudDiskRdbStore::CloudDiskRdbStore(const std::string &bundleName, const int32_t &userId)
    : bundleName_(bundleName), userId_(userId)
{
    RdbInit();
    MimeTypeUtils::InitMimeTypeMap();
}

CloudDiskRdbStore::~CloudDiskRdbStore()
{
    Stop();
}

int32_t CloudDiskRdbStore::RdbInit()
{
    LOGI("Init rdb store, userId_ = %{private}d, bundleName_ = %{private}s", userId_, bundleName_.c_str());
    string baseDir = DATA_SERVICE_EL1_PUBLIC_CLOUDFILE;
    string customDir = baseDir.append(to_string(userId_)).append("/").append(bundleName_);
    string name = CLOUD_DISK_DATABASE_NAME;
    int32_t errCode = 0;
    string databasePath = RdbSqlUtils::GetDefaultDatabasePath(customDir, CLOUD_DISK_DATABASE_NAME, errCode);
    if (errCode != NativeRdb::E_OK) {
        LOGE("Create Default Database Path is failed, errCode = %{public}d", errCode);
        return E_PATH;
    }
    config_.SetName(move(name));
    config_.SetPath(move(databasePath));
    config_.SetScalarFunction("cloud_sync_func", ARGS_SIZE, CloudSyncTriggerFunc);
    errCode = 0;
    CloudDiskDataCallBack rdbDataCallBack;
    rdbStore_ = RdbHelper::GetRdbStore(config_, CLOUD_DISK_RDB_VERSION, rdbDataCallBack, errCode);
    if (rdbStore_ == nullptr) {
        LOGE("GetRdbStore is failed, userId_ = %{private}d, bundleName_ = %{private}s, errCode = %{public}d",
             userId_, bundleName_.c_str(), errCode);
        return E_RDB;
    }
    return E_OK;
}

void CloudDiskRdbStore::Stop()
{
    if (rdbStore_ == nullptr) {
        return;
    }
    rdbStore_ = nullptr;
}

shared_ptr<RdbStore> CloudDiskRdbStore::GetRaw()
{
    return rdbStore_;
}

int32_t CloudDiskRdbStore::LookUp(const std::string &parentCloudId,
    const std::string &fileName, CloudDiskFileInfo &info)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (fileName.empty() || parentCloudId.empty()) {
        LOGE("look up parameters is invalid");
        return E_INVAL_ARG;
    }
    AbsRdbPredicates lookUpPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    lookUpPredicates
        .EqualTo(FileColumn::PARENT_CLOUD_ID, parentCloudId)->And()
        ->EqualTo(FileColumn::FILE_NAME, fileName)->And()->EqualTo(FileColumn::FILE_TIME_RECYCLED, "0")->And()
        ->NotEqualTo(FileColumn::DIRTY_TYPE, to_string(static_cast<int32_t>(DirtyType::TYPE_DELETED)));
    lookUpPredicates.Limit(LOOKUP_QUERY_LIMIT);
    auto resultSet = rdbStore_->QueryByStep(lookUpPredicates, FileColumn::FILE_SYSTEM_QUERY_COLUMNS);
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(move(resultSet), info);
    if (ret != E_OK) {
        LOGE("lookup file info is failed, ret %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::GetAttr(const std::string &cloudId, CloudDiskFileInfo &info)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || cloudId == "rootId") {
        LOGE("getAttr parameter is invalid");
        return E_INVAL_ARG;
    }
    AbsRdbPredicates getAttrPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    getAttrPredicates.EqualTo(FileColumn::CLOUD_ID, cloudId);
    auto resultSet = rdbStore_->QueryByStep(getAttrPredicates, FileColumn::FILE_SYSTEM_QUERY_COLUMNS);
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(move(resultSet), info);
    if (ret != E_OK) {
        LOGE("get file attr is failed, ret %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::ReadDir(const std::string &cloudId, vector<CloudDiskFileInfo> &infos)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    CLOUDID_IS_NULL(cloudId);
    AbsRdbPredicates readDirPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    readDirPredicates.EqualTo(FileColumn::PARENT_CLOUD_ID, cloudId)
        ->And()->EqualTo(FileColumn::FILE_TIME_RECYCLED, "0")->And()
        ->NotEqualTo(FileColumn::DIRTY_TYPE, to_string(static_cast<int32_t>(DirtyType::TYPE_DELETED)));
    auto resultSet = rdbStore_->QueryByStep(readDirPredicates, { FileColumn::FILE_NAME, FileColumn::IS_DIRECTORY });
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfos(move(resultSet), infos);
    if (ret != E_OK) {
        LOGE("read directory is failed, ret %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

static int32_t GetFileExtension(const std::string &fileName, std::string &extension)
{
    size_t dotIndex = fileName.rfind(".");
    if (dotIndex != string::npos) {
        extension = fileName.substr(dotIndex + 1);
        return E_OK;
    }
    LOGE("Failed to obtain file extension");
    return E_INVAL_ARG;
}

static int32_t GetFileTypeFromMimeType(const std::string &mimeType)
{
    size_t pos = mimeType.find_first_of("/");
    if (pos == string::npos) {
        LOGE("Invalid mime type: %{public}s", mimeType.c_str());
        return E_INVAL_ARG;
    }
    string prefix = mimeType.substr(0, pos);
    if (prefix == "audio") {
        return static_cast<int32_t>(FileType::FILE_TYPE_AUDIO);
    } else if (prefix == "image") {
        return static_cast<int32_t>(FileType::FILE_TYPE_IMAGE);
    } else if (prefix == "video") {
        return static_cast<int32_t>(FileType::FILE_TYPE_VIDEO);
    } else if (prefix == "text") {
        return static_cast<int32_t>(FileType::FILE_TYPE_TEXT);
    }
    return static_cast<int32_t>(FileType::FILE_TYPE_APPLICATION);
}

static int64_t Timespec2Milliseconds(const struct timespec &time)
{
    return time.tv_sec * SECOND_TO_MILLISECOND + time.tv_nsec / MILLISECOND_TO_NANOSECOND;
}

static void FillFileType(const std::string &fileName, ValuesBucket &fileInfo)
{
    string extension;
    if (!GetFileExtension(fileName, extension)) {
        fileInfo.PutString(FileColumn::FILE_CATEGORY, extension);
        string mimeType = MimeTypeUtils::GetMimeTypeFromExtension(extension);
        fileInfo.PutString(FileColumn::MIME_TYPE, mimeType);
        int32_t fileType = GetFileTypeFromMimeType(mimeType);
        fileInfo.PutInt(FileColumn::FILE_TYPE, fileType);
    }
}

static int64_t UTCTimeMilliSeconds()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * SECOND_TO_MILLISECOND + t.tv_nsec / MILLISECOND_TO_NANOSECOND;
}

static int32_t CreateFile(const std::string &fileName, const std::string &filePath, ValuesBucket &fileInfo)
{
    struct stat statInfo {};
    int32_t ret = stat(filePath.c_str(), &statInfo);
    if (ret) {
        LOGE("filePath %{private}s is invalid", filePath.c_str());
        return E_PATH;
    }
    fileInfo.PutInt(FileColumn::IS_DIRECTORY, FILE);
    fileInfo.PutLong(FileColumn::FILE_SIZE, statInfo.st_size);
    fileInfo.PutLong(FileColumn::FILE_TIME_EDITED, Timespec2Milliseconds(statInfo.st_mtim));
    fileInfo.PutLong(FileColumn::META_TIME_EDITED, Timespec2Milliseconds(statInfo.st_mtim));
    FillFileType(fileName, fileInfo);
    return E_OK;
}

int32_t CloudDiskRdbStore::Create(const std::string &cloudId, const std::string &parentCloudId,
    const std::string &fileName)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    ValuesBucket fileInfo;
    if (cloudId.empty() || parentCloudId.empty() || fileName.empty()) {
        LOGE("create parameter is invalid");
        return E_INVAL_ARG;
    }
    fileInfo.PutString(FileColumn::CLOUD_ID, cloudId);
    fileInfo.PutString(FileColumn::FILE_NAME, fileName);
    fileInfo.PutLong(FileColumn::FILE_TIME_ADDED, UTCTimeMilliSeconds());
    fileInfo.PutString(FileColumn::PARENT_CLOUD_ID, parentCloudId);
    fileInfo.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_NO_NEED_UPLOAD));
    fileInfo.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::NEW));
    string filePath = CloudFileUtils::GetLocalFilePath(cloudId, bundleName_, userId_);
    if (CreateFile(fileName, filePath, fileInfo)) {
        LOGE("file path is invalid, cannot create file record");
        return E_PATH;
    }
    int64_t outRowId = 0;
    int32_t ret = rdbStore_->Insert(outRowId, FileColumn::FILES_TABLE, fileInfo);
    if (ret != E_OK) {
        LOGE("insert new file record in DB is failed, ret = %{public}d", ret);
        return ret;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::MkDir(const std::string &cloudId, const std::string &parentCloudId,
    const std::string &directoryName)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    ValuesBucket dirInfo;
    if (cloudId.empty() || parentCloudId.empty() || directoryName.empty()) {
        LOGE("make directory parameter is invalid");
        return E_INVAL_ARG;
    }
    dirInfo.PutString(FileColumn::CLOUD_ID, cloudId);
    dirInfo.PutString(FileColumn::FILE_NAME, directoryName);
    dirInfo.PutLong(FileColumn::FILE_TIME_ADDED, UTCTimeMilliSeconds());
    dirInfo.PutLong(FileColumn::FILE_TIME_EDITED, UTCTimeMilliSeconds());
    dirInfo.PutLong(FileColumn::META_TIME_EDITED, UTCTimeMilliSeconds());
    dirInfo.PutInt(FileColumn::IS_DIRECTORY, DIRECTORY);
    dirInfo.PutString(FileColumn::PARENT_CLOUD_ID, parentCloudId);
    dirInfo.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::NEW));
    int64_t outRowId = 0;
    int32_t ret = rdbStore_->Insert(outRowId, FileColumn::FILES_TABLE, dirInfo);
    if (ret != E_OK) {
        LOGE("insert new directory record in DB is failed, ret = %{public}d", ret);
        return ret;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::Write(const std::string &cloudId)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || cloudId == "rootId") {
        LOGE("write parameter is invalid");
        return E_INVAL_ARG;
    }
    string filePath = CloudFileUtils::GetLocalFilePath(cloudId, bundleName_, userId_);
    struct stat statInfo {};
    int32_t ret = stat(filePath.c_str(), &statInfo);
    if (ret) {
        LOGE("filePath %{private}s is invalid", filePath.c_str());
        return E_PATH;
    }
    CloudDiskFileInfo info;
    if (GetAttr(cloudId, info)) {
        LOGE("get write cloudId info in DB fail");
        return E_RDB;
    }
    int32_t position = static_cast<int32_t>(info.location);
    ValuesBucket write;
    write.PutLong(FileColumn::FILE_SIZE, statInfo.st_size);
    write.PutLong(FileColumn::FILE_TIME_EDITED, Timespec2Milliseconds(statInfo.st_mtim));
    write.PutLong(FileColumn::META_TIME_EDITED, Timespec2Milliseconds(statInfo.st_mtim));
    write.PutLong(FileColumn::FILE_TIME_VISIT, Timespec2Milliseconds(statInfo.st_atim));
    if (position != LOCAL) {
        write.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_FDIRTY));
        write.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::UPDATE));
    } else {
        write.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_NEW));
    }
    int32_t changedRows = -1;
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(cloudId);
    ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, write,
        FileColumn::CLOUD_ID + " = ?", bindArgs);
    if (ret != E_OK) {
        LOGE("write file record in DB fail, ret %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::LocationSetXattr(const std::string &cloudId, const std::string &value)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    int32_t val = -1;
    istringstream transfer(value);
    transfer >> val;
    if (val != LOCAL && val != CLOUD && val != LOCAL_AND_CLOUD) {
        LOGE("setxattr unknown value");
        return E_INVAL_ARG;
    }
    ValuesBucket setXAttr;
    setXAttr.PutInt(FileColumn::POSITION, val);
    int32_t changedRows = -1;
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(cloudId);
    int32_t ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, setXAttr,
        FileColumn::CLOUD_ID + " = ?", bindArgs);
    if (ret != E_OK) {
        LOGE("set xAttr location fail, ret %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::RecycleSetXattr(const std::string &cloudId, const std::string &value)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    int32_t val = std::stoi(value);
    LOGE("recycleSetXattr, val %{public}d", val);
    ValuesBucket setXAttr;
    if (val == 0) {
        setXAttr.PutLong(FileColumn::FILE_TIME_RECYCLED, CANCEL_STATE);
        setXAttr.PutInt(FileColumn::DIRECTLY_RECYCLED, CANCEL_STATE);
    } else if (val == 1) {
        setXAttr.PutLong(FileColumn::FILE_TIME_RECYCLED, UTCTimeMilliSeconds());
        setXAttr.PutInt(FileColumn::DIRECTLY_RECYCLED, SET_STATE);
    } else {
        return E_RDB;
    }
    int32_t changedRows = -1;
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(cloudId);
    int32_t ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, setXAttr,
        FileColumn::CLOUD_ID + " = ?", bindArgs);
    if (ret != E_OK) {
        LOGE("set xAttr location fail, ret %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::FavoriteSetXattr(const std::string &cloudId, const std::string &value)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    int32_t val = std::stoi(value);
    LOGE("favoriteSetXattr, val %{public}d", val);
    ValuesBucket setXAttr;
    if (val == 0) {
        setXAttr.PutInt(FileColumn::IS_FAVORITE, CANCEL_STATE);
    } else if (val == 1) {
        setXAttr.PutInt(FileColumn::IS_FAVORITE, SET_STATE);
    } else {
        return E_RDB;
    }
    int32_t changedRows = -1;
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(cloudId);
    int32_t ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, setXAttr,
        FileColumn::CLOUD_ID + " = ?", bindArgs);
    if (ret != E_OK) {
        LOGE("set xAttr location fail, ret %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t CheckXattr(const std::string &key)
{
    if (key == CLOUD_FILE_LOCATION) {
        return CLOUD_LOCATION;
    } else if (key == CLOUD_CLOUD_RECYCLE_XATTR) {
        return CLOUD_RECYCLE;
    } else if (key == IS_FAVORITE_XATTR) {
        return IS_FAVORITE;
    } else {
        return ERROR_CODE;
    }
}

int32_t CloudDiskRdbStore::LocationGetXattr(const std::string &cloudId, const std::string &key, std::string &value)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || cloudId == "rootId" || key != CLOUD_FILE_LOCATION) {
        LOGE("getxattr parameter is invalid");
        return E_INVAL_ARG;
    }
    AbsRdbPredicates getXAttrPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    getXAttrPredicates.EqualTo(FileColumn::CLOUD_ID, cloudId);
    auto resultSet = rdbStore_->QueryByStep(getXAttrPredicates, { FileColumn::POSITION });
    if (resultSet == nullptr) {
        LOGE("get nullptr getxattr result");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        LOGE("getxattr result set go to next row failed");
        return E_RDB;
    }
    int32_t position;
    CloudDiskRdbUtils::GetInt(FileColumn::POSITION, position, resultSet);
    value = to_string(position);
    return E_OK;
}

int32_t CloudDiskRdbStore::FavoriteGetXattr(const std::string &cloudId, const std::string &key, std::string &value)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || cloudId == "rootId" || key != IS_FAVORITE_XATTR) {
        LOGE("getxattr parameter is invalid");
        return E_INVAL_ARG;
    }
    AbsRdbPredicates getXAttrPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    getXAttrPredicates.EqualTo(FileColumn::CLOUD_ID, cloudId);
    auto resultSet = rdbStore_->QueryByStep(getXAttrPredicates, { FileColumn::IS_FAVORITE });
    if (resultSet == nullptr) {
        LOGE("get nullptr getxattr result");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        LOGE("getxattr result set go to next row failed");
        return E_RDB;
    }
    int32_t isFavorite;
    CloudDiskRdbUtils::GetInt(FileColumn::IS_FAVORITE, isFavorite, resultSet);
    value = to_string(isFavorite);
    return E_OK;
}

int32_t CloudDiskRdbStore::GetXAttr(const std::string &cloudId, const std::string &key, std::string &value)
{
    int32_t num = CheckXattr(key);
    switch (num) {
        case CLOUD_LOCATION:
            return LocationGetXattr(cloudId, key, value);
            break;
        case IS_FAVORITE:
            return FavoriteGetXattr(cloudId, key, value);
            break;
    }
    if (cloudId.empty() || cloudId == "rootId") {
        LOGE("getxattr parameter is invalid");
        return E_INVAL_ARG;
    } else {
        return E_OK;
    }
}

int32_t CloudDiskRdbStore::SetXAttr(const std::string &cloudId, const std::string &key, const std::string &value)
{
    int32_t num = CheckXattr(key);
    switch (num) {
        case CLOUD_LOCATION:
            return LocationSetXattr(cloudId, value);
            break;
        case CLOUD_RECYCLE:
            return RecycleSetXattr(cloudId, value);
            break;
        case IS_FAVORITE:
            return FavoriteSetXattr(cloudId, value);
    }
    if (cloudId.empty() || cloudId == "rootId") {
        LOGE("setxattr parameter is invalid");
        return E_INVAL_ARG;
    } else {
        return E_OK;
    }
}

static void FileRename(ValuesBucket &values, const int32_t &position, const std::string &newFileName)
{
    values.PutString(FileColumn::FILE_NAME, newFileName);
    FillFileType(newFileName, values);
    if (position != LOCAL) {
        values.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_MDIRTY));
        values.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::RENAME));
    }
}

static void FileMove(ValuesBucket &values, const int32_t &position, const std::string &newParentCloudId)
{
    values.PutString(FileColumn::PARENT_CLOUD_ID, newParentCloudId);
    if (position != LOCAL) {
        values.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_MDIRTY));
        values.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::MOVE));
    }
}

int32_t CloudDiskRdbStore::Rename(const std::string &oldParentCloudId, const std::string &oldFileName,
    const std::string &newParentCloudId, const std::string &newFileName)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (oldParentCloudId.empty() || oldFileName.empty() || newParentCloudId.empty() || newFileName.empty()) {
        LOGE("rename parameters is invalid");
        return E_INVAL_ARG;
    }
    CloudDiskFileInfo info;
    if (LookUp(oldParentCloudId, oldFileName, info)) {
        LOGE("get rename cloudId info in DB fail");
        return E_RDB;
    }
    string cloudId = info.cloudId;
    int32_t position = static_cast<int32_t>(info.location);
    if (oldFileName == newFileName && oldParentCloudId == newParentCloudId) {
        LOGI("the rename parameter is same as old");
        return E_OK;
    }
    ValuesBucket rename;
    rename.PutLong(FileColumn::META_TIME_EDITED, UTCTimeMilliSeconds());
    if (oldFileName != newFileName && oldParentCloudId == newParentCloudId) {
        FileRename(rename, position, newFileName);
    }
    if (oldFileName == newFileName && oldParentCloudId != newParentCloudId) {
        FileMove(rename, position, newParentCloudId);
    }
    int32_t changedRows = -1;
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(cloudId);
    int32_t ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, rename,
        FileColumn::CLOUD_ID + " = ?", bindArgs);
    if (ret != E_OK) {
        LOGE("rename file fail, ret %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::UnlinkSynced(const std::string &cloudId)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    CLOUDID_IS_NULL(cloudId);
    int32_t changedRows = -1;
    ValuesBucket updateValue;
    vector<string> whereArgs = {cloudId};
    updateValue.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_DELETED));
    int32_t ret = rdbStore_
        ->Update(changedRows, FileColumn::FILES_TABLE, updateValue, FileColumn::CLOUD_ID + " = ?", whereArgs);
    if (ret != E_OK) {
        LOGE("unlink synced directory fail, ret %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::UnlinkLocal(const std::string &cloudId)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    CLOUDID_IS_NULL(cloudId);
    int32_t changedRows = -1;
    vector<string> whereArgs = {cloudId};
    int32_t ret = rdbStore_
        ->Delete(changedRows, FileColumn::FILES_TABLE, FileColumn::CLOUD_ID + " = ?", whereArgs);
    if (ret != E_OK) {
        LOGE("unlink local directory fail, ret %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::Unlink(const std::string &parentCloudId, const std::string &fileName, string &unlinkCloudId)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (parentCloudId.empty() || fileName.empty()) {
        LOGE("Unlink parameters is invalid");
        return E_INVAL_ARG;
    }
    AbsRdbPredicates unlinkPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    unlinkPredicates
        .EqualTo(FileColumn::PARENT_CLOUD_ID, parentCloudId)->And()
        ->EqualTo(FileColumn::FILE_NAME, fileName)->And()->EqualTo(FileColumn::FILE_TIME_RECYCLED, "0")->And()
        ->NotEqualTo(FileColumn::DIRTY_TYPE, to_string(static_cast<int32_t>(DirtyType::TYPE_DELETED)));
    auto resultSet =
        rdbStore_->QueryByStep(unlinkPredicates,
            {FileColumn::CLOUD_ID, FileColumn::IS_DIRECTORY, FileColumn::POSITION});
    if (resultSet == nullptr) {
        LOGE("unlink result set is nullptr");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        LOGW("not need to unlink");
        return E_OK;
    }
    string cloudId;
    CloudDiskRdbUtils::GetString(FileColumn::CLOUD_ID, cloudId, resultSet);
    int32_t isDirectory;
    CloudDiskRdbUtils::GetInt(FileColumn::IS_DIRECTORY, isDirectory, resultSet);
    int32_t position;
    CloudDiskRdbUtils::GetInt(FileColumn::POSITION, position, resultSet);
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(cloudId);
    if (position == CLOUD) {
        RETURN_ON_ERR(UnlinkSynced(cloudId));
        return E_OK;
    } else if (position == LOCAL) {
        RETURN_ON_ERR(UnlinkLocal(cloudId));
    } else {
        RETURN_ON_ERR(UnlinkSynced(cloudId));
    }
    if (isDirectory == FILE) {
        unlinkCloudId = cloudId;
    }
    return E_OK;
}

static void GenCloudSyncTriggerFuncParams(RdbStore &store, std::string &userId, std::string &bundleName)
{
    string databasePath = store.GetPath();
    string str = "cloudfile/";
    size_t startPos = databasePath.find(str);
    size_t endPos = databasePath.find("/rdb");
    if (startPos != std::string::npos && endPos != std::string::npos) {
        startPos += str.size();
        string tempStr = databasePath.substr(startPos, endPos - startPos);
        size_t pos = tempStr.find('/');
        if (pos != std::string::npos) {
            userId = tempStr.substr(0, pos);
            bundleName = tempStr.substr(pos + 1);
            LOGI("generate CloudSyncTriggerFunc parameters success, userId: %{public}s, bundleName: %{public}s",
                userId.c_str(), bundleName.c_str());
            return;
        }
    }
    LOGE("generate CloudSyncTriggerFunc parameters fail");
    return;
}

static const std::string &CreateFolderTriggerSync(RdbStore &store)
{
    string userId;
    string bundleName;
    GenCloudSyncTriggerFuncParams(store, userId, bundleName);
    static const string CREATE_FOLDERS_NEW_CLOUD_SYNC =
        "CREATE TRIGGER folders_new_cloud_sync_trigger AFTER INSERT ON " + FileColumn::FILES_TABLE +
        " FOR EACH ROW WHEN new.isDirectory == 1 AND new.dirty_type == " +
        std::to_string(static_cast<int32_t>(DirtyType::TYPE_NEW)) +
        " BEGIN SELECT cloud_sync_func(" + "'" + userId + "', " + "'" + bundleName + "'); END;";
    return CREATE_FOLDERS_NEW_CLOUD_SYNC;
}

static const std::string &UpdateFileTriggerSync(RdbStore &store)
{
    string userId;
    string bundleName;
    GenCloudSyncTriggerFuncParams(store, userId, bundleName);
    static const string CREATE_FILES_UPDATE_CLOUD_SYNC =
        "CREATE TRIGGER files_update_cloud_sync_trigger AFTER UPDATE ON " + FileColumn::FILES_TABLE +
        " FOR EACH ROW WHEN OLD.dirty_type IN (0,1,2,3) AND new.dirty_type IN (2,3)" +
        " BEGIN SELECT cloud_sync_func(" + "'" + userId + "', " + "'" + bundleName + "'); END;";
    return CREATE_FILES_UPDATE_CLOUD_SYNC;
}

static const std::string &DeleteFileTriggerSync(RdbStore &store)
{
    string userId;
    string bundleName;
    GenCloudSyncTriggerFuncParams(store, userId, bundleName);
    static const string CREATE_FILES_DELETE_CLOUD_SYNC =
        "CREATE TRIGGER files_delete_cloud_sync_trigger AFTER UPDATE ON " + FileColumn::FILES_TABLE +
        " FOR EACH ROW WHEN OLD.dirty_type IN (0,2,3) AND new.dirty_type == " +
        std::to_string(static_cast<int32_t>(DirtyType::TYPE_DELETED)) +
        " BEGIN SELECT cloud_sync_func(" + "'" + userId + "', " + "'" + bundleName + "'); END;";
    return CREATE_FILES_DELETE_CLOUD_SYNC;
}

static const std::string &LocalFileTriggerSync(RdbStore &store)
{
    string userId;
    string bundleName;
    GenCloudSyncTriggerFuncParams(store, userId, bundleName);
    static const string CREATE_FILES_LOCAL_CLOUD_SYNC =
        "CREATE TRIGGER files_local_cloud_sync_trigger AFTER UPDATE ON " + FileColumn::FILES_TABLE +
        " FOR EACH ROW WHEN OLD.dirty_type IN (1,6) AND new.dirty_type == " +
        std::to_string(static_cast<int32_t>(DirtyType::TYPE_NEW)) +
        " BEGIN SELECT cloud_sync_func(" + "'" + userId + "', " + "'" + bundleName + "'); END;";
    return CREATE_FILES_LOCAL_CLOUD_SYNC;
}

static int32_t ExecuteSql(RdbStore &store)
{
    static const vector<string> onCreateSqlStrs = {
        FileColumn::CREATE_FILE_TABLE,
        CreateFolderTriggerSync(store),
        UpdateFileTriggerSync(store),
        FileColumn::CREATE_PARENT_CLOUD_ID_INDEX,
        DeleteFileTriggerSync(store),
        LocalFileTriggerSync(store),
    };
    for (const string& sqlStr : onCreateSqlStrs) {
        if (store.ExecuteSql(sqlStr) != NativeRdb::E_OK) {
            return NativeRdb::E_ERROR;
        }
    }
    return NativeRdb::E_OK;
}

int32_t CloudDiskDataCallBack::OnCreate(RdbStore &store)
{
    if (ExecuteSql(store) != NativeRdb::E_OK) {
        return NativeRdb::E_ERROR;
    }
    return NativeRdb::E_OK;
}

static void VersionAddParentCloudIdIndex(RdbStore &store)
{
    const string executeSqlStr = FileColumn::CREATE_PARENT_CLOUD_ID_INDEX;
    int32_t ret = store.ExecuteSql(executeSqlStr);
    if (ret != NativeRdb::E_OK) {
        LOGE("add parent cloud id index fail, err %{public}d", ret);
    }
}

static void VersionFixFileTrigger(RdbStore &store)
{
    const string dropFilesUpdateTrigger = "DROP TRIGGER IF EXISTS files_update_cloud_sync_trigger";
    if (store.ExecuteSql(dropFilesUpdateTrigger) != NativeRdb::E_OK) {
        LOGE("drop files_update_cloud_sync_trigger fail");
    }
    const string addUpdateFileTrigger = UpdateFileTriggerSync(store);
    int32_t ret = store.ExecuteSql(addUpdateFileTrigger);
    if (ret != NativeRdb::E_OK) {
        LOGE("add update file trigger fail, err %{public}d", ret);
    }
    const string addDeleteFileTrigger = DeleteFileTriggerSync(store);
    ret = store.ExecuteSql(addDeleteFileTrigger);
    if (ret != NativeRdb::E_OK) {
        LOGE("add delete file trigger fail, err %{public}d", ret);
    }
    const string addLocalFileTrigger = LocalFileTriggerSync(store);
    ret = store.ExecuteSql(addLocalFileTrigger);
    if (ret != NativeRdb::E_OK) {
        LOGE("add local file trigger fail, err %{public}d", ret);
    }
}

static void VersionFixCreateAndLocalTrigger(RdbStore &store)
{
    const string dropFilesCreateTrigger = "DROP TRIGGER IF EXISTS files_new_cloud_sync_trigger";
    if (store.ExecuteSql(dropFilesCreateTrigger) != NativeRdb::E_OK) {
        LOGE("drop files_new_cloud_sync_trigger fail");
    }
    const string dropFilesLocalTrigger = "DROP TRIGGER IF EXISTS files_local_cloud_sync_trigger";
    if (store.ExecuteSql(dropFilesLocalTrigger) != NativeRdb::E_OK) {
        LOGE("drop files_local_cloud_sync_trigger fail");
    }
    const string addLocalFileTrigger = LocalFileTriggerSync(store);
    int32_t ret = store.ExecuteSql(addLocalFileTrigger);
    if (ret != NativeRdb::E_OK) {
        LOGE("add local file trigger fail, err %{public}d", ret);
    }
    const string addCreateFolderTrigger = CreateFolderTriggerSync(store);
    ret = store.ExecuteSql(addCreateFolderTrigger);
    if (ret != NativeRdb::E_OK) {
        LOGE("add create folder trigger fail, err %{public}d", ret);
    }
}

static void VersionAddFileStatusAndErrorCode(RdbStore &store)
{
    const string addIsFavorite = FileColumn::ADD_IS_FAVORITE;
    int32_t ret = store.ExecuteSql(addIsFavorite);
    if (ret != NativeRdb::E_OK) {
        LOGE("add is_favorite fail, err %{public}d", ret);
    }
}

int32_t CloudDiskDataCallBack::OnUpgrade(RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    LOGD("OnUpgrade old:%d, new:%d", oldVersion, newVersion);
    if (oldVersion < VERSION_ADD_PARENT_CLOUD_ID_INDEX) {
        VersionAddParentCloudIdIndex(store);
    }
    if (oldVersion < VERSION_FIX_FILE_TRIGGER) {
        VersionFixFileTrigger(store);
    }
    if (oldVersion < VERSION_FIX_CREATE_AND_LOCAL_TRIGGER) {
        VersionFixCreateAndLocalTrigger(store);
    }
    if (oldVersion < VERSION_ADD_STATUS_ERROR_FAVORITE) {
        VersionAddFileStatusAndErrorCode(store);
    }
    return NativeRdb::E_OK;
}
}