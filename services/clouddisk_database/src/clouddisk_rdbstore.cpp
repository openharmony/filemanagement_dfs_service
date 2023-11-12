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

constexpr int64_t SECOND_TO_MILLISECOND = 1e3;
constexpr int64_t MILLISECOND_TO_NANOSECOND = 1e6;

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
        ->EqualTo(FileColumn::FILE_NAME, fileName)->And()->EqualTo(FileColumn::FILE_TIME_RECYCLED, "0");
    auto resultSet = rdbStore_->Query(lookUpPredicates, FileColumn::FILE_SYSTEM_QUERY_COLUMNS);
    vector<CloudDiskFileInfo> infos;
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(move(resultSet), infos);
    if (ret != E_OK || infos.size() != 1) {
        LOGE("lookup file info is failed, ret %{public}d, info count %{public}d",
            ret, static_cast<int32_t>(infos.size()));
        return E_RDB;
    }
    info = move(infos.front());
    return E_OK;
}

int32_t CloudDiskRdbStore::GetAttr(const std::string &cloudId, CloudDiskFileInfo &info)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
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
        ->And()->EqualTo(FileColumn::FILE_TIME_RECYCLED, "0");
    auto resultSet = rdbStore_->Query(readDirPredicates, FileColumn::FILE_SYSTEM_QUERY_COLUMNS);
    int32_t ret = CloudDiskRdbUtils::ResultSetToFileInfo(move(resultSet), infos);
    if (ret != E_OK) {
        LOGE("read directory is failed, ret %{public}d", ret);
        infos.clear();
        return E_RDB;
    }
    return E_OK;
}

static int32_t GetFileExtension(const std::string &fileName, std::string &extension)
{
    if (!fileName.empty()) {
        size_t dotIndex = fileName.rfind(".");
        if (dotIndex != string::npos) {
            extension = fileName.substr(dotIndex + 1);
            return E_OK;
        }
    }
    LOGE("Failed to obtain file extension");
    return E_INVAL_ARG;
}

static int32_t GetFileTypeFromMimeType(const std::string &mimeType)
{
    size_t pos = mimeType.find_first_of("/");
    if (pos == string::npos) {
        LOGE("Invalid mime type: %{private}s", mimeType.c_str());
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

int32_t CloudDiskRdbStore::FillFileType(const std::string &fileName, ValuesBucket &fileInfo)
{
    if (!fileName.empty()) {
        string extension;
        int32_t ret = GetFileExtension(fileName, extension);
        if (ret == E_OK) {
            fileInfo.PutString(FileColumn::FILE_CATEGORY, extension);
            string mimeType = MimeTypeUtils::GetMimeTypeFromExtension(extension);
            fileInfo.PutString(FileColumn::MIME_TYPE, mimeType);
            int32_t fileType = GetFileTypeFromMimeType(mimeType);
            fileInfo.PutInt(FileColumn::FILE_TYPE, fileType);
            return E_OK;
        } else {
            LOGE("fill file type fail, ret %{public}d", ret);
            return ret;
        }
    } else {
        LOGE("fill file type fail, the file name is invalid");
        return E_INVAL_ARG;
    }
}

static int64_t UTCTimeMilliSeconds()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * SECOND_TO_MILLISECOND + t.tv_nsec / MILLISECOND_TO_NANOSECOND;
}

int32_t CloudDiskRdbStore::UpdateParentFolder(const std::string &parentCloudId, const std::string &filePath)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    struct stat statInfo {};
    int32_t ret = stat(filePath.c_str(), &statInfo);
    if (ret) {
        LOGE("filePath %{private}s is invalid", filePath.c_str());
        return E_PATH;
    }
    if (!parentCloudId.empty()) {
        ValuesBucket dirInfo;
        dirInfo.PutLong(FileColumn::FILE_TIME_EDITED, Timespec2Milliseconds(statInfo.st_mtim));
        dirInfo.PutLong(FileColumn::META_TIME_EDITED, Timespec2Milliseconds(statInfo.st_mtim));
        int32_t changedRows = 0;
        vector<ValueObject> bindArgs;
        bindArgs.emplace_back(parentCloudId);
        ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, dirInfo,
            FileColumn::CLOUD_ID + " = ?", bindArgs);
        if (ret != E_OK) {
            LOGE("update file parent folder fail, ret %{public}d", ret);
            return ret;
        }
        return E_OK;
    } else {
        LOGI("The parent folder is root folder, no update is required");
        return E_OK;
    }
}

int32_t CloudDiskRdbStore::Create(const std::string &cloudId, const std::string &parentCloudId,
    const std::string &fileName)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    string filePath = CloudFileUtils::GetLocalFilePath(cloudId, bundleName_, userId_);
    ValuesBucket fileInfo;
    fileInfo.PutString(FileColumn::CLOUD_ID, cloudId);
    fileInfo.PutString(FileColumn::FILE_NAME, fileName);
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
    struct stat statInfo {};
    int32_t ret = stat(filePath.c_str(), &statInfo);
    if (ret) {
        LOGE("filePath %{private}s is invalid", filePath.c_str());
        return E_PATH;
    }
    fileInfo.PutLong(FileColumn::FILE_SIZE, statInfo.st_size);
    fileInfo.PutLong(FileColumn::FILE_TIME_ADDED, UTCTimeMilliSeconds());
    fileInfo.PutLong(FileColumn::FILE_TIME_EDITED, Timespec2Milliseconds(statInfo.st_mtim));
    fileInfo.PutLong(FileColumn::META_TIME_EDITED, Timespec2Milliseconds(statInfo.st_mtim));
    fileInfo.PutString(FileColumn::IS_DIRECTORY, S_ISDIR(statInfo.st_mode) ? "directory" : "file");
    if (!S_ISDIR(statInfo.st_mode)) {
        if (FillFileType(fileName, fileInfo)) {
            LOGE("fille file type fail");
            return E_INVAL_ARG;
        }
    }
    int64_t outRowId = 0;
    ret = rdbStore_->Insert(outRowId, FileColumn::FILES_TABLE, fileInfo);
    if (ret != E_OK) {
        LOGE("insert new file record in DB is failed, ret = %{public}d", ret);
        return ret;
    }
    if (UpdateParentFolder(parentCloudId, filePath)) {
        LOGE("update parent folder edited time fail");
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::GetXAttr(const std::string &cloudId, const std::string &position, std::string &value)
{
    return E_OK;
}

int32_t CloudDiskRdbStore::SetXAttr(const std::string &cloudId, const std::string &position, const std::string &value)
{
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