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

#include "clouddisk_rdb_utils.h"
#include "rdb_sql_utils.h"
#include "utils_log.h"
#include "dfs_error.h"
#include "dk_database.h"
#include "mimetype_utils.h"
#include "cloud_pref_impl.h"

namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using namespace OHOS::NativeRdb;
using namespace DriveKit;
using namespace CloudSync;
using namespace OHOS::Media;

CloudDiskRdbStore::CloudDiskRdbStore(const std::string &bundleName, const int32_t &userId)
    : bundleName_(bundleName), userId_(userId)
{
    RdbInit();
    MimeTypeUtils::InitMimeTypeMap();
    InitRootId();
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

void CloudDiskRdbStore::InitRootId()
{
    CloudPrefImpl cloudPrefImpl(userId_, bundleName_, FileColumn::FILES_TABLE);
    cloudPrefImpl.GetString("rootId", rootId_);
    if (!rootId_.empty()) {
        return;
    } else {
        auto driveKit = DriveKit::DriveKitNative::GetInstance(userId_);
        if (driveKit == nullptr) {
            LOGE("sdk helper get drive kit instance fail");
            return;
        }
        auto container = driveKit->GetDefaultContainer(bundleName_);
        if (container == nullptr) {
            LOGE("sdk helper get drive kit container fail");
            return;
        }
        shared_ptr<DriveKit::DKDatabase> database = container->GetPrivateDatabase();
        if (database == nullptr) {
            LOGE("sdk helper get drive kit database fail");
            return;
        }
        DKError dkErr = database->GetRootId(rootId_);
        if (dkErr.dkErrorCode != DKLocalErrorCode::NO_ERROR) {
            LOGE("get root id failed, err %{public}d", dkErr.dkErrorCode);
            return;
        }
        cloudPrefImpl.SetString("rootId", rootId_);
    }
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
    if (fileName.empty()) {
        LOGE("fileName is invalid");
        return E_INVAL_ARG;
    }
    AbsRdbPredicates lookUpPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    string tempParentCloudId = parentCloudId;
    if (tempParentCloudId.empty()) {
        if (!rootId_.empty()) {
            tempParentCloudId = rootId_;
        } else {
            LOGE("root Id is empty, cannot lookup file");
            return E_INVAL_ARG;
        }
    }
    lookUpPredicates
        .EqualTo(FileColumn::PARENT_CLOUD_ID, tempParentCloudId)->And()
        ->EqualTo(FileColumn::FILE_NAME, fileName)->And()->EqualTo(FileColumn::FILE_TIME_RECYCLED, "0")->And()
        ->NotEqualTo(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_DELETED));
    auto resultSet = rdbStore_->Query(lookUpPredicates, FileColumn::FILE_SYSTEM_QUERY_COLUMNS);
    vector<CloudDiskFileInfo> infos;
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(move(resultSet), infos);
    if (ret != E_OK || infos.size() != 1) {
        LOGE("lookup file info is failed, ret %{public}d, info count %{public}d",
            ret, static_cast<int32_t>(infos.size()));
        return E_RDB;
    }
    info = move(infos.front());
    if (info.parentCloudId == rootId_) {
        info.parentCloudId = "";
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::GetAttr(const std::string &cloudId, CloudDiskFileInfo &info)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    CLOUDID_IS_NULL(cloudId);
    AbsRdbPredicates getAttrPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    getAttrPredicates.EqualTo(FileColumn::CLOUD_ID, cloudId);
    auto resultSet = rdbStore_->Query(getAttrPredicates, FileColumn::FILE_SYSTEM_QUERY_COLUMNS);
    vector<CloudDiskFileInfo> infos;
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(move(resultSet), infos);
    if (ret != E_OK || infos.size() != 1) {
        LOGE("get file attr is failed, ret %{public}d, file count %{public}d",
            ret, static_cast<int32_t>(infos.size()));
        return E_RDB;
    }
    info = move(infos.front());
    if (info.parentCloudId == rootId_) {
        info.parentCloudId = "";
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::ReadDir(const std::string &cloudId, vector<CloudDiskFileInfo> &infos)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    AbsRdbPredicates readDirPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    string tempParentCloudId = cloudId;
    if (tempParentCloudId.empty()) {
        if (!rootId_.empty()) {
            tempParentCloudId = rootId_;
        } else {
            LOGE("root Id is empty, cannot read directory");
            return E_INVAL_ARG;
        }
    }
    readDirPredicates.EqualTo(FileColumn::PARENT_CLOUD_ID, tempParentCloudId)
        ->And()->EqualTo(FileColumn::FILE_TIME_RECYCLED, "0")->And()
        ->NotEqualTo(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_DELETED));
    auto resultSet = rdbStore_->Query(readDirPredicates, FileColumn::FILE_SYSTEM_QUERY_COLUMNS);
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(move(resultSet), infos);
    if (ret != E_OK) {
        LOGE("read directory is failed, ret %{public}d", ret);
        infos.clear();
        return E_RDB;
    }
    for (auto &info : infos) {
        if (info.parentCloudId == rootId_) {
            info.parentCloudId = "";
        }
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

static int64_t Timespec2Milliseconds(struct timespec &time)
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
    if (cloudId.empty() || fileName.empty()) {
        LOGE("create parameter is invalid");
        return E_INVAL_ARG;
    }
    fileInfo.PutString(FileColumn::CLOUD_ID, cloudId);
    fileInfo.PutString(FileColumn::FILE_NAME, fileName);
    fileInfo.PutLong(FileColumn::FILE_TIME_ADDED, UTCTimeMilliSeconds());
    string tempParentCloudId = parentCloudId;
    if (tempParentCloudId.empty()) {
        if (!rootId_.empty()) {
            tempParentCloudId = rootId_;
        } else {
            LOGE("root Id is empty, cannot create file");
            return E_INVAL_ARG;
        }
    }
    fileInfo.PutString(FileColumn::PARENT_CLOUD_ID, tempParentCloudId);
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
    if (cloudId.empty() || directoryName.empty()) {
        LOGE("make directory parameter is invalid");
        return E_INVAL_ARG;
    }
    dirInfo.PutString(FileColumn::CLOUD_ID, cloudId);
    dirInfo.PutString(FileColumn::FILE_NAME, directoryName);
    dirInfo.PutLong(FileColumn::FILE_TIME_ADDED, UTCTimeMilliSeconds());
    dirInfo.PutLong(FileColumn::FILE_TIME_EDITED, UTCTimeMilliSeconds());
    dirInfo.PutLong(FileColumn::META_TIME_EDITED, UTCTimeMilliSeconds());
    dirInfo.PutInt(FileColumn::IS_DIRECTORY, DIRECTORY);
    string tempParentCloudId = parentCloudId;
    if (tempParentCloudId.empty()) {
        if (!rootId_.empty()) {
            tempParentCloudId = rootId_;
        } else {
            LOGE("root Id is empty, cannot make directory");
            return E_INVAL_ARG;
        }
    }
    dirInfo.PutString(FileColumn::PARENT_CLOUD_ID, tempParentCloudId);
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
    CLOUDID_IS_NULL(cloudId);
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

int32_t CloudDiskRdbStore::GetXAttr(const std::string &cloudId, const std::string &position, std::string &value)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || position != FileColumn::POSITION) {
        LOGE("getxattr parameter is invalid");
        return E_INVAL_ARG;
    }
    CloudDiskFileInfo info;
    if (GetAttr(cloudId, info)) {
        LOGE("get getxattr cloudId info in DB fail");
        return E_RDB;
    }
    int32_t location = static_cast<int32_t>(info.location);
    value = to_string(location);
    return E_OK;
}

int32_t CloudDiskRdbStore::SetXAttr(const std::string &cloudId, const std::string &position, const std::string &value)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || position != FileColumn::POSITION) {
        LOGE("setxattr parameter is invalid");
        return E_INVAL_ARG;
    }
    int32_t val = -1;
    istringstream transfer(value);
    transfer >> val;
    if (val != LOCAL && val != CLOUD && val != LOCAL_AND_CLOUD) {
        LOGE("setxattr unknown value");
        return E_INVAL_ARG;
    }
    ValuesBucket setXAttr;
    setXAttr.PutInt(position, val);
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

int32_t CloudDiskRdbStore::Rename(const std::string &cloudId,
    const std::string &newParentCloudId, const std::string &newFileName)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    CLOUDID_IS_NULL(cloudId);
    if (newFileName.empty()) {
        LOGE("new file name is null, cannot be renamed");
        return E_INVAL_ARG;
    }
    CloudDiskFileInfo info;
    if (GetAttr(cloudId, info)) {
        LOGE("get rename cloudId info in DB fail");
        return E_RDB;
    }
    string oldFileName = info.fileName;
    string oldParentCloudId = info.parentCloudId;
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
    string tempNewParentCloudId = newParentCloudId;
    if (tempNewParentCloudId.empty()) {
        if (!rootId_.empty()) {
            tempNewParentCloudId = rootId_;
        } else {
            LOGE("root Id is empty, cannot rename file to root directory");
            return E_INVAL_ARG;
        }
    }
    if (oldFileName == newFileName && oldParentCloudId != newParentCloudId) {
        FileMove(rename, position, tempNewParentCloudId);
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

int32_t CloudDiskRdbStore::UnlinkSyncedFile(const std::string &cloudId,
    const int32_t &isDirectory, const int32_t &position)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    CLOUDID_IS_NULL(cloudId);
    if (isDirectory == DIRECTORY && position != LOCAL) {
        AbsRdbPredicates deleteSyncedPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
        deleteSyncedPredicates.EqualTo(FileColumn::PARENT_CLOUD_ID, cloudId);
        auto resultSet = rdbStore_->Query(deleteSyncedPredicates, FileColumn::FILE_SYSTEM_QUERY_COLUMNS);
        int32_t rowCount = 0;
        int32_t ret = E_OK;
        if (resultSet) {
            ret = resultSet->GetRowCount(rowCount);
        }
        if (resultSet == nullptr || ret != E_OK || rowCount < 0) {
            LOGE("result set is nullptr or get result set rowCount is failed, ret %{public}d, rowCount %{public}d",
                ret, rowCount);
            return E_RDB;
        }
        while (resultSet->GoToNextRow() == E_OK) {
            int32_t childIsDirectory;
            string childCloudId;
            int32_t childPosition = -1;
            if (CloudDiskRdbUtils::GetInt(FileColumn::IS_DIRECTORY, childIsDirectory, resultSet) ||
                CloudDiskRdbUtils::GetString(FileColumn::CLOUD_ID, childCloudId, resultSet) ||
                CloudDiskRdbUtils::GetInt(FileColumn::POSITION, childPosition, resultSet)) {
                LOGE("handle synced result set query fail");
                return E_RDB;
            }
            RETURN_ON_ERR(UnlinkLocalFile(childCloudId, childIsDirectory, childPosition));
            RETURN_ON_ERR(UnlinkSyncedFile(childCloudId, childIsDirectory, childPosition));
        }
    }
    if (position != LOCAL) {
        int32_t changedRows = -1;
        ValuesBucket deleteValue;
        deleteValue.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_DELETED));
        vector<ValueObject> bindArgs;
        bindArgs.emplace_back(cloudId);
        int32_t ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, deleteValue,
            FileColumn::CLOUD_ID + " = ?", bindArgs);
        if (ret != E_OK) {
            LOGE("unlink synced file itself fail, ret %{public}d", ret);
            return E_RDB;
        }
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::UnlinkLocalFile(const std::string &cloudId, const int32_t &isDirectory,
    const int32_t &position = LOCAL)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    CLOUDID_IS_NULL(cloudId);
    int32_t changedRows = -1;
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(cloudId);
    if (isDirectory == DIRECTORY && position == LOCAL) {
        AbsRdbPredicates deleteLocalPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
        deleteLocalPredicates.EqualTo(FileColumn::PARENT_CLOUD_ID, cloudId)
            ->And()->EqualTo(FileColumn::IS_DIRECTORY, DIRECTORY);
        auto resultSet = rdbStore_->Query(deleteLocalPredicates, FileColumn::FILE_SYSTEM_QUERY_COLUMNS);
        int32_t rowCount = 0;
        int32_t ret = E_OK;
        if (resultSet) {
            ret = resultSet->GetRowCount(rowCount);
        }
        if (resultSet == nullptr || ret != E_OK || rowCount < 0) {
            LOGE("result set is nullptr or get result set rowCount is failed, ret %{public}d, rowCount %{public}d",
                ret, rowCount);
            return E_RDB;
        }
        while (resultSet->GoToNextRow() == E_OK) {
            string childCloudId;
            if (CloudDiskRdbUtils::GetString(FileColumn::CLOUD_ID, childCloudId, resultSet)) {
                LOGE("unlink local directory query cloudId fail");
                return E_RDB;
            }
            RETURN_ON_ERR(UnlinkLocalFile(childCloudId, DIRECTORY));
        }
        ret = rdbStore_->Delete(changedRows, FileColumn::FILES_TABLE,
            FileColumn::PARENT_CLOUD_ID + " = ?", bindArgs);
        if (ret != E_OK) {
            LOGE("unlink local directory fail, ret %{public}d", ret);
            return E_RDB;
        }
    }
    if (position == LOCAL) {
        int32_t ret = rdbStore_->Delete(changedRows, FileColumn::FILES_TABLE,
            FileColumn::CLOUD_ID + " = ?", bindArgs);
        if (ret != E_OK) {
            LOGE("unlink local file itself fail, ret %{public}d", ret);
            return E_RDB;
        }
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::Unlink(const std::string &cloudId)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    CLOUDID_IS_NULL(cloudId);
    CloudDiskFileInfo info;
    if (GetAttr(cloudId, info)) {
        LOGE("get deleted cloudId info in DB fail");
        return E_RDB;
    }
    int32_t isDirectory = info.IsDirectory ? DIRECTORY : FILE;
    int32_t position = static_cast<int32_t>(info.location);
    RETURN_ON_ERR(UnlinkLocalFile(cloudId, isDirectory, position));
    RETURN_ON_ERR(UnlinkSyncedFile(cloudId, isDirectory, position));
    return E_OK;
}

static int32_t ExecuteSql(RdbStore &store)
{
    static const vector<string> executeSqlStrs = {
        FileColumn::CREATE_FILE_TABLE,
    };

    for (const string& sqlStr : executeSqlStrs) {
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

int32_t CloudDiskDataCallBack::OnUpgrade(RdbStore &store, int32_t oldVersion, int32_t newVersion)
{
    return NativeRdb::E_OK;
}
}