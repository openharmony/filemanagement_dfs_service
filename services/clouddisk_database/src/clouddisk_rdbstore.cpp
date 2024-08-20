/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include <ctime>
#include <sys/stat.h>
#include <sstream>
#include <functional>

#include "cloud_pref_impl.h"
#include "clouddisk_db_const.h"
#include "clouddisk_notify.h"
#include "clouddisk_notify_utils.h"
#include "clouddisk_rdb_transaction.h"
#include "clouddisk_rdb_utils.h"
#include "clouddisk_sync_helper.h"
#include "clouddisk_type_const.h"
#include "dfs_error.h"
#include "file_column.h"
#include "ffrt_inner.h"
#include "nlohmann/json.hpp"
#include "parameters.h"
#include "rdb_errno.h"
#include "rdb_sql_utils.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using namespace OHOS::NativeRdb;
using namespace CloudSync;

enum XATTR_CODE {
    ERROR_CODE = -1,
    CLOUD_LOCATION = 1,
    CLOUD_RECYCLE,
    IS_FAVORITE,
    FILE_SYNC_STATUS,
    IS_EXT_ATTR
};
static constexpr int32_t LOOKUP_QUERY_LIMIT = 1;
static constexpr int32_t CHECK_QUERY_LIMIT = 2000;
static const uint32_t SET_STATE = 1;
static const uint32_t CANCEL_STATE = 0;
static const uint32_t MAX_FILE_NAME_SIZE = 246;
static const uint32_t MAX_QUERY_TIMES = 1024;
static const uint32_t STAT_MODE_DIR = 0771;
static const uint32_t STAT_MODE_REG = 0660;
const string BUNDLENAME_FLAG = "<BundleName>";
const string CLOUDDISK_URI_PREFIX = "file://<BundleName>/data/storage/el2/cloud";
const string BACKFLASH = "/";
static const string RECYCLE_FILE_NAME = ".trash";
static const string ROOT_CLOUD_ID = "rootId";
static const std::string FILEMANAGER_KEY = "persist.kernel.bundle_name.filemanager";

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
    return "";
}

CloudDiskRdbStore::CloudDiskRdbStore(const std::string &bundleName, const int32_t &userId)
    : bundleName_(bundleName), userId_(userId)
{
    RdbInit();
}

CloudDiskRdbStore::~CloudDiskRdbStore()
{
    Stop();
}

int32_t CloudDiskRdbStore::ReBuildDatabase(const string &databasePath)
{
    LOGI("database need to be rebuilded");
    int32_t errCode = RdbHelper::DeleteRdbStore(databasePath);
    if (errCode != NativeRdb::E_OK) {
        LOGE("Delete CloudDisk Database is failed, err = %{public}d", errCode);
        return errCode;
    }
    errCode = 0;
    CloudDiskDataCallBack rdbDataCallBack;
    rdbStore_ = RdbHelper::GetRdbStore(config_, CLOUD_DISK_RDB_VERSION, rdbDataCallBack, errCode);
    if (rdbStore_ == nullptr) {
        LOGE("ReGetRdbStore is failed, userId_ = %{public}d, bundleName_ = %{public}s, errCode = %{public}d",
             userId_, bundleName_.c_str(), errCode);
        return errCode;
    }
    DatabaseRestore();
    return E_OK;
}

int32_t CloudDiskRdbStore::RdbInit()
{
    LOGI("Init rdb store, userId_ = %{public}d, bundleName_ = %{public}s", userId_, bundleName_.c_str());
    string baseDir = DATA_SERVICE_EL1_PUBLIC_CLOUDFILE;
    string customDir = baseDir.append(to_string(userId_))
        .append("/").append(system::GetParameter(FILEMANAGER_KEY, ""));
    string name = CLOUD_DISK_DATABASE_NAME;
    int32_t errCode = 0;
    string databasePath = RdbSqlUtils::GetDefaultDatabasePath(customDir, CLOUD_DISK_DATABASE_NAME, errCode);
    if (errCode != NativeRdb::E_OK) {
        LOGE("Create Default Database Path is failed, errCode = %{public}d", errCode);
        return E_PATH;
    }
    config_.SetName(name);
    config_.SetPath(databasePath);
    config_.SetReadConSize(CONNECT_SIZE);
    config_.SetScalarFunction("cloud_sync_func", ARGS_SIZE, CloudSyncTriggerFunc);
    errCode = 0;
    CloudDiskDataCallBack rdbDataCallBack;
    rdbStore_ = RdbHelper::GetRdbStore(config_, CLOUD_DISK_RDB_VERSION, rdbDataCallBack, errCode);
    if (rdbStore_ == nullptr) {
        LOGE("GetRdbStore is failed, userId_ = %{public}d, bundleName_ = %{public}s, errCode = %{public}d",
             userId_, bundleName_.c_str(), errCode);
        if (errCode == NativeRdb::E_SQLITE_CORRUPT) {
            if (ReBuildDatabase(databasePath)) {
                LOGE("clouddisk db image is malformed, ReBuild failed");
            }
        }
        return errCode;
    } else if (errCode == NativeRdb::E_SQLITE_CORRUPT) { DatabaseRestore(); }
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

void CloudDiskRdbStore::DatabaseRestore()
{
    if (rdbStore_ == nullptr) {
        LOGE("rdbStore_ is nullptr");
        return;
    }
    LOGI("clouddisk db image is malformed, need to restore");
    auto fileName = "/data/service/el1/public/cloudfile/" +
        to_string(userId_) + "/" + system::GetParameter(FILEMANAGER_KEY, "") + "/backup/clouddisk_backup.db";
    int32_t ret = -1;
    if (access(fileName.c_str(), F_OK) == 0) {
        {
            lock_guard<mutex> lock(backupMutex_);
            ret = rdbStore_->Restore(fileName);
        }
        if (ret != 0) {
            LOGE("cloudisk restore failed, ret %{public}d", ret);
        }
    } else {
        LOGE("clouddisk backup db is not exist");
    }
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
        ->EqualTo(FileColumn::ROOT_DIRECTORY, bundleName_)->And()
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
    if (cloudId.empty() || cloudId == ROOT_CLOUD_ID) {
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

int32_t CloudDiskRdbStore::SetAttr(const std::string &fileName, const std::string &parentCloudId,
    const std::string &cloudId, const unsigned long long &size)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty()) {
        LOGE("cloudId is empty");
        return E_INVAL_ARG;
    }
    if (cloudId == ROOT_CLOUD_ID) {
        LOGE("cloudId is rootId");
        return E_INVAL_ARG;
    }

    ValuesBucket setAttr;
    setAttr.PutLong(FileColumn::FILE_SIZE, static_cast<int64_t>(size));
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(cloudId);
    TransactionOperations rdbTransaction(rdbStore_);
    int32_t ret = rdbTransaction.Start();
    if (ret != E_OK) {
        LOGE("rdbstore begin transaction failed, ret = %{public}d", ret);
        return ret;
    }
    int32_t changedRows = -1;
    ret =
        rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, setAttr, FileColumn::CLOUD_ID + " = ?", bindArgs);
    if (ret != E_OK) {
        LOGE("setAttr size fail, ret: %{public}d", ret);
        return E_RDB;
    }

    MetaBase metaBase(fileName, cloudId);
    metaBase.size = size;
    auto callback = [&metaBase] (MetaBase &m) {
        m.size = metaBase.size;
    };
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId_, bundleName_, parentCloudId);
    ret = metaFile->DoLookupAndUpdate(fileName, callback);
    if (ret != E_OK) {
        LOGE("update new dentry failed, ret = %{public}d", ret);
        return ret;
    }
    rdbTransaction.Finish();
    return E_OK;
}

int32_t CloudDiskRdbStore::ReadDir(const std::string &cloudId, vector<CloudDiskFileInfo> &infos)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    CLOUDID_IS_NULL(cloudId);
    AbsRdbPredicates readDirPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    readDirPredicates.EqualTo(FileColumn::PARENT_CLOUD_ID, cloudId)
        ->And()->EqualTo(FileColumn::FILE_TIME_RECYCLED, "0")->And()
        ->EqualTo(FileColumn::ROOT_DIRECTORY, bundleName_)->And()
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

static void FillFileType(const std::string &fileName, ValuesBucket &fileInfo)
{
    string extension;
    if (!GetFileExtension(fileName, extension)) {
        fileInfo.PutString(FileColumn::FILE_CATEGORY, extension);
    }
}

static int64_t UTCTimeMilliSeconds()
{
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    return t.tv_sec * SECOND_TO_MILLISECOND + t.tv_nsec / MILLISECOND_TO_NANOSECOND;
}

static int32_t CheckNameForSpace(const std::string& fileName, const int32_t isDir)
{
    if (fileName.empty()) {
        return EINVAL;
    }
    if (fileName[0] == ' ') {
        LOGI("Illegal name");
        return EINVAL;
    }
    if (isDir == DIRECTORY) {
        if ((fileName.length() >= 1 && fileName[fileName.length() - 1] == ' ') || fileName == RECYCLE_FILE_NAME) {
            LOGI("Illegal name");
            return EINVAL;
        }
    }
    return E_OK;
}

static int32_t CheckName(const std::string &fileName)
{
    if (fileName.empty() ||
        fileName == "." ||
        fileName == ".." ||
        fileName.length() > MAX_FILE_NAME_SIZE) {
        return EINVAL;
    }
    std::map<char, bool> illegalCharacter = {
        {'<', true},
        {'>', true},
        {'|', true},
        {':', true},
        {'?', true},
        {'/', true},
        {'\\', true},
        {'"', true},
        {'*', true},
    };
    for (char c : fileName) {
        if (illegalCharacter.find(c) != illegalCharacter.end()) {
            LOGI("Illegal name");
            return EINVAL;
        }
    }
    return E_OK;
}

static int32_t CreateFile(const std::string &fileName, const std::string &filePath, ValuesBucket &fileInfo,
    struct stat *statInfo)
{
    int32_t ret = stat(filePath.c_str(), statInfo);
    if (ret) {
        LOGE("filePath %{private}s is invalid", GetAnonyString(filePath).c_str());
        return E_PATH;
    }
    fileInfo.PutInt(FileColumn::IS_DIRECTORY, FILE);
    fileInfo.PutLong(FileColumn::FILE_SIZE, statInfo->st_size);
    fileInfo.PutLong(FileColumn::FILE_TIME_EDITED, CloudFileUtils::Timespec2Milliseconds(statInfo->st_mtim));
    fileInfo.PutLong(FileColumn::META_TIME_EDITED, CloudFileUtils::Timespec2Milliseconds(statInfo->st_mtim));
    FillFileType(fileName, fileInfo);
    return E_OK;
}

static int32_t CreateDentry(MetaBase &metaBase, uint32_t userId, const std::string &bundleName,
    const std::string &fileName, const std::string &parentCloudId)
{
    auto callback = [&metaBase] (MetaBase &m) {
        m.cloudId = metaBase.cloudId;
        m.atime = metaBase.atime;
        m.mtime = metaBase.mtime;
        m.size = metaBase.size;
        m.mode = metaBase.mode;
        m.position = metaBase.position;
        m.fileType = metaBase.fileType;
        m.noUpload = metaBase.noUpload;
    };
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, bundleName, parentCloudId);
    int32_t ret = metaFile->DoLookupAndUpdate(fileName, callback);
    if (ret != E_OK) {
        LOGE("update new dentry failed, ret = %{public}d", ret);
        return ret;
    }
    return E_OK;
}

static void UpdateMetabase(MetaBase &metaBase, int64_t fileTimeAdded, struct stat *statInfo)
{
    metaBase.atime = static_cast<uint64_t>(fileTimeAdded);
    metaBase.mtime = static_cast<uint64_t>(CloudFileUtils::Timespec2Milliseconds(statInfo->st_mtim));
    metaBase.mode = statInfo->st_mode;
    metaBase.size = static_cast<uint64_t>(statInfo->st_size);
    metaBase.position = LOCAL;
    metaBase.fileType = FILE_TYPE_CONTENT;
}

static void HandleCreateValue(ValuesBucket &fileInfo, const std::string &cloudId, const std::string &parentCloudId,
                              const std::string &fileName, const std::string &bundleName)
{
    fileInfo.PutString(FileColumn::CLOUD_ID, cloudId);
    fileInfo.PutString(FileColumn::FILE_NAME, fileName);
    fileInfo.PutString(FileColumn::PARENT_CLOUD_ID, parentCloudId);
    fileInfo.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_NO_NEED_UPLOAD));
    fileInfo.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::NEW));
    fileInfo.PutString(FileColumn::ROOT_DIRECTORY, bundleName);
}

int32_t CloudDiskRdbStore::Create(const std::string &cloudId, const std::string &parentCloudId,
    const std::string &fileName, bool noNeedUpload)
{
    int32_t ret = CheckName(fileName);
    if (ret != E_OK) {
        return ret;
    }
    ret = CheckNameForSpace(fileName, FILE);
    if (ret != E_OK) {
        return ret;
    }
    RDBPTR_IS_NULLPTR(rdbStore_);
    ValuesBucket fileInfo;
    if (cloudId.empty() || parentCloudId.empty() || fileName.empty()) {
        LOGE("create parameter is invalid");
        return E_INVAL_ARG;
    }

    MetaBase metaBase(fileName, cloudId);
    if (noNeedUpload) {
        fileInfo.PutInt(FileColumn::NO_NEED_UPLOAD, NO_UPLOAD);
        metaBase.noUpload = NO_UPLOAD;
    }

    int64_t fileTimeAdded = UTCTimeMilliSeconds();
    fileInfo.PutLong(FileColumn::FILE_TIME_ADDED, fileTimeAdded);
    HandleCreateValue(fileInfo, cloudId, parentCloudId, fileName, bundleName_);
    struct stat statInfo {};
    string filePath = CloudFileUtils::GetLocalFilePath(cloudId, bundleName_, userId_);
    if (CreateFile(fileName, filePath, fileInfo, &statInfo)) {
        LOGE("file path is invalid, cannot create file record");
        return E_PATH;
    }
    TransactionOperations rdbTransaction(rdbStore_);
    ret = rdbTransaction.Start();
    if (ret != E_OK) {
        LOGE("rdbstore begin transaction failed, ret = %{public}d", ret);
        return ret;
    }
    int64_t outRowId = 0;
    ret = rdbStore_->Insert(outRowId, FileColumn::FILES_TABLE, fileInfo);
    if (ret != E_OK) {
        LOGE("insert new file record in DB is failed, ret = %{public}d", ret);
        return ret;
    }
    
    UpdateMetabase(metaBase, fileTimeAdded, &statInfo);
    ret = CreateDentry(metaBase, userId_, bundleName_, fileName, parentCloudId);
    if (ret != E_OK) {
        LOGE("create new dentry failed, ret = %{public}d", ret);
        return ret;
    }
    rdbTransaction.Finish();
    return E_OK;
}

int32_t CloudDiskRdbStore::MkDir(const std::string &cloudId, const std::string &parentCloudId,
    const std::string &directoryName, bool noNeedUpload)
{
    int32_t ret = CheckName(directoryName);
    if (ret != E_OK) {
        return ret;
    }
    ret = CheckNameForSpace(directoryName, DIRECTORY);
    if (ret != E_OK) {
        return ret;
    }
    RDBPTR_IS_NULLPTR(rdbStore_);
    ValuesBucket dirInfo;
    if (cloudId.empty() || parentCloudId.empty() || directoryName.empty()) {
        LOGE("make directory parameter is invalid");
        return E_INVAL_ARG;
    }

    MetaBase metaBase(directoryName, cloudId);
    if (noNeedUpload) {
        dirInfo.PutInt(FileColumn::NO_NEED_UPLOAD, NO_UPLOAD);
        metaBase.noUpload = NO_UPLOAD;
    }

    dirInfo.PutString(FileColumn::CLOUD_ID, cloudId);
    dirInfo.PutString(FileColumn::FILE_NAME, directoryName);
    int64_t fileTimeAdded = UTCTimeMilliSeconds();
    dirInfo.PutLong(FileColumn::FILE_TIME_ADDED, fileTimeAdded);
    int64_t fileTimeEdited = UTCTimeMilliSeconds();
    dirInfo.PutLong(FileColumn::FILE_TIME_EDITED, fileTimeEdited);
    int64_t metaTimeEdited = UTCTimeMilliSeconds();
    dirInfo.PutLong(FileColumn::META_TIME_EDITED, metaTimeEdited);
    dirInfo.PutInt(FileColumn::IS_DIRECTORY, DIRECTORY);
    dirInfo.PutString(FileColumn::PARENT_CLOUD_ID, parentCloudId);
    dirInfo.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::NEW));
    dirInfo.PutInt(FileColumn::FILE_STATUS, FileStatus::TO_BE_UPLOADED);
    dirInfo.PutString(FileColumn::ROOT_DIRECTORY, bundleName_);
    TransactionOperations rdbTransaction(rdbStore_);
    ret = rdbTransaction.Start();
    if (ret != E_OK) {
        LOGE("rdbstore begin transaction failed, ret = %{public}d", ret);
        return ret;
    }
    int64_t outRowId = 0;
    ret = rdbStore_->Insert(outRowId, FileColumn::FILES_TABLE, dirInfo);
    if (ret != E_OK) {
        LOGE("insert new directory record in DB is failed, ret = %{public}d", ret);
        return ret;
    }

    metaBase.atime = static_cast<uint64_t>(fileTimeAdded);
    metaBase.mtime = static_cast<uint64_t>(fileTimeEdited);
    metaBase.mode = S_IFDIR | STAT_MODE_DIR;
    metaBase.position = LOCAL;
    metaBase.fileType = FILE_TYPE_CONTENT;
    ret = CreateDentry(metaBase, userId_, bundleName_, directoryName, parentCloudId);
    if (ret != E_OK) {
        LOGE("create new dentry failed, ret = %{public}d", ret);
        return ret;
    }
    rdbTransaction.Finish();
    CloudDiskSyncHelper::GetInstance().RegisterTriggerSync(bundleName_, userId_);
    return E_OK;
}

static void HandleWriteValue(ValuesBucket &write, int32_t position, struct stat &statInfo)
{
    write.PutLong(FileColumn::FILE_SIZE, statInfo.st_size);
    write.PutLong(FileColumn::FILE_TIME_EDITED, CloudFileUtils::Timespec2Milliseconds(statInfo.st_mtim));
    write.PutLong(FileColumn::META_TIME_EDITED, CloudFileUtils::Timespec2Milliseconds(statInfo.st_mtim));
    write.PutLong(FileColumn::FILE_TIME_VISIT, CloudFileUtils::Timespec2Milliseconds(statInfo.st_atim));
    write.PutInt(FileColumn::FILE_STATUS, FileStatus::TO_BE_UPLOADED);
    if (position != LOCAL) {
        write.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_FDIRTY));
        write.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::UPDATE));
    } else {
        write.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_NEW));
    }
}

static int32_t WriteUpdateDentry(MetaBase &metaBase, uint32_t userId, const std::string &bundleName,
    const std::string &fileName, const std::string &parentCloudId)
{
    auto callback = [&metaBase] (MetaBase &m) {
        m.mtime = metaBase.mtime;
        m.size = metaBase.size;
    };
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, bundleName, parentCloudId);
    LOGD("write update dentry start");
    int32_t ret = metaFile->DoChildUpdate(fileName, callback);
    if (ret != E_OK) {
        LOGE("update new dentry failed, ret = %{public}d", ret);
        return ret;
    }
    return ret;
}

int32_t CloudDiskRdbStore::Write(const std::string &fileName, const std::string &parentCloudId,
    const std::string &cloudId)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || cloudId == ROOT_CLOUD_ID) {
        LOGE("write parameter is invalid");
        return E_INVAL_ARG;
    }
    string filePath = CloudFileUtils::GetLocalFilePath(cloudId, bundleName_, userId_);
    struct stat statInfo {};
    int32_t ret = stat(filePath.c_str(), &statInfo);
    if (ret) {
        LOGE("filePath %{private}s is invalid", GetAnonyString(filePath).c_str());
        return E_PATH;
    }
    CloudDiskFileInfo info;
    if (GetAttr(cloudId, info)) {
        LOGE("get write cloudId info in DB fail");
        return E_RDB;
    }
    int32_t position = static_cast<int32_t>(info.location);
    ValuesBucket write;
    HandleWriteValue(write, position, statInfo);
    int32_t changedRows = -1;
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(cloudId);
    TransactionOperations rdbTransaction(rdbStore_);
    ret = rdbTransaction.Start();
    if (ret != E_OK) {
        LOGE("rdbstore begin transaction failed, ret = %{public}d", ret);
        return ret;
    }
    ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, write,
        FileColumn::CLOUD_ID + " = ?", bindArgs);
    if (ret != E_OK) {
        LOGE("write file record in DB fail, ret %{public}d", ret);
        return E_RDB;
    }
    MetaBase metaBase(fileName, cloudId);
    metaBase.mtime = static_cast<uint64_t>(CloudFileUtils::Timespec2Milliseconds(statInfo.st_mtim));
    metaBase.size = static_cast<uint64_t>(statInfo.st_size);
    ret = WriteUpdateDentry(metaBase, userId_, bundleName_, fileName, parentCloudId);
    if (ret != E_OK) {
        LOGE("write update dentry failed, ret %{public}d", ret);
        return E_RDB;
    }
    rdbTransaction.Finish();
    CloudDiskSyncHelper::GetInstance().RegisterTriggerSync(bundleName_, userId_);
    return E_OK;
}

int32_t CloudDiskRdbStore::LocationSetXattr(const std::string &name, const std::string &parentCloudId,
    const std::string &cloudId, const std::string &value)
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
    TransactionOperations rdbTransaction(rdbStore_);
    int32_t ret = rdbTransaction.Start();
    if (ret != E_OK) {
        LOGE("rdbstore begin transaction failed, ret = %{public}d", ret);
        return ret;
    }
    ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, setXAttr,
        FileColumn::CLOUD_ID + " = ?", bindArgs);
    if (ret != E_OK) {
        LOGE("set xAttr location fail, ret %{public}d", ret);
        return E_RDB;
    }
    MetaBase metaBase(name, cloudId);
    auto callback = [&val] (MetaBase &m) {
        m.position = val;
    };
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId_, bundleName_, parentCloudId);
    ret = metaFile->DoLookupAndUpdate(name, callback);
    if (ret != E_OK) {
        LOGE("update new dentry failed, ret = %{public}d", ret);
        return ret;
    }
    rdbTransaction.Finish();
    return E_OK;
}

int32_t CloudDiskRdbStore::GetRowId(const std::string &cloudId, int64_t &rowId)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    CLOUDID_IS_NULL(cloudId);
    AbsRdbPredicates getRowIdPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    getRowIdPredicates.EqualTo(FileColumn::CLOUD_ID, cloudId);
    auto resultSet = rdbStore_->QueryByStep(getRowIdPredicates, {FileColumn::ROW_ID});
    if (resultSet == nullptr) {
        LOGE("get nullptr result set");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        LOGE("getRowId result set go to next row failed");
        return E_RDB;
    }
    CloudDiskRdbUtils::GetLong(FileColumn::ROW_ID, rowId, resultSet);
    return E_OK;
}

static int32_t RecycleSetValue(int32_t val, ValuesBucket &setXAttr)
{
    setXAttr.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_MDIRTY));
    if (val == 0) {
        setXAttr.PutInt(FileColumn::OPERATE_TYPE, static_cast<int32_t>(OperationType::RESTORE));
        setXAttr.PutLong(FileColumn::FILE_TIME_RECYCLED, CANCEL_STATE);
        setXAttr.PutInt(FileColumn::DIRECTLY_RECYCLED, CANCEL_STATE);
        setXAttr.PutLong(FileColumn::META_TIME_EDITED, UTCTimeMilliSeconds());
    } else if (val == 1) {
        int64_t recycledTime = UTCTimeMilliSeconds();
        setXAttr.PutInt(FileColumn::OPERATE_TYPE, static_cast<int32_t>(OperationType::DELETE));
        setXAttr.PutLong(FileColumn::FILE_TIME_RECYCLED, recycledTime);
        setXAttr.PutInt(FileColumn::DIRECTLY_RECYCLED, SET_STATE);
        setXAttr.PutLong(FileColumn::META_TIME_EDITED, recycledTime);
    } else {
        LOGE("invalid value");
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::GetParentCloudId(const std::string &cloudId, std::string &parentCloudId)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    AbsRdbPredicates getParentCloudIdPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    getParentCloudIdPredicates.EqualTo(FileColumn::CLOUD_ID, cloudId);
    auto resultSet = rdbStore_->QueryByStep(getParentCloudIdPredicates, { FileColumn::PARENT_CLOUD_ID });
    if (resultSet == nullptr) {
        LOGE("get nullptr parentCloudId resultSet");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        LOGE("get parentCloudId go to next row failed");
        return E_RDB;
    }
    int32_t ret = CloudDiskRdbUtils::GetString(FileColumn::PARENT_CLOUD_ID, parentCloudId, resultSet);
    if (ret != E_OK) {
        LOGE("get parent cloudId failed");
        return ret;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::RecycleSetXattr(const std::string &name, const std::string &parentCloudId,
    const std::string &cloudId, const std::string &value)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    bool isNum = std::all_of(value.begin(), value.end(), ::isdigit);
    if (!isNum) {
        return EINVAL;
    }
    int32_t val = std::stoi(value);
    ValuesBucket setXAttr;
    int32_t ret = RecycleSetValue(val, setXAttr);
    if (ret != E_OK) {
        return ret;
    }
    int32_t changedRows = -1;
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(cloudId);
    TransactionOperations rdbTransaction(rdbStore_);
    ret = rdbTransaction.Start();
    if (ret != E_OK) {
        LOGE("rdbstore begin transaction failed, ret = %{public}d", ret);
        return ret;
    }
    ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, setXAttr,
        FileColumn::CLOUD_ID + " = ?", bindArgs);
    if (ret != E_OK) {
        LOGE("set xAttr location fail, ret %{public}d", ret);
        return E_RDB;
    }
    int64_t rowId = 0;
    ret = GetRowId(cloudId, rowId);
    if (ret != E_OK) {
        LOGE("get rowId fail, ret %{public}d", ret);
        return ret;
    }
    if (val == 0) {
        ret = MetaFileMgr::GetInstance().RemoveFromRecycleDentryfile(userId_, bundleName_, name,
            parentCloudId, rowId);
    } else {
        ret = MetaFileMgr::GetInstance().MoveIntoRecycleDentryfile(userId_, bundleName_, name,
            parentCloudId, rowId);
    }
    if (ret != E_OK) {
        LOGE("recycle set dentryfile failed, ret = %{public}d", ret);
        return ret;
    }
    rdbTransaction.Finish();
    CloudDiskSyncHelper::GetInstance().RegisterTriggerSync(bundleName_, userId_);
    return E_OK;
}

int32_t CloudDiskRdbStore::FavoriteSetXattr(const std::string &cloudId, const std::string &value)
{
    LOGD("favoriteSetXattr, value %{public}s", value.c_str());
    RDBPTR_IS_NULLPTR(rdbStore_);
    bool isNum = std::all_of(value.begin(), value.end(), ::isdigit);
    if (!isNum) {
        return EINVAL;
    }
    int32_t val = std::stoi(value);
    ValuesBucket setXAttr;
    if (val == 0) {
        setXAttr.PutInt(FileColumn::IS_FAVORITE, CANCEL_STATE);
        setXAttr.PutLong(FileColumn::META_TIME_EDITED, UTCTimeMilliSeconds());
    } else if (val == 1) {
        setXAttr.PutInt(FileColumn::IS_FAVORITE, SET_STATE);
        setXAttr.PutLong(FileColumn::META_TIME_EDITED, UTCTimeMilliSeconds());
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
    } else if (key == IS_FILE_STATUS_XATTR) {
        return FILE_SYNC_STATUS;
    } else if (key == CLOUD_EXT_ATTR) {
        return IS_EXT_ATTR;
    } else {
        return ERROR_CODE;
    }
}

int32_t CloudDiskRdbStore::LocationGetXattr(const std::string &name, const std::string &key, std::string &value,
    const std::string &parentCloudId)
{
    if (key != CLOUD_FILE_LOCATION) {
        LOGE("getxattr parameter is invalid");
        return E_INVAL_ARG;
    }
    MetaBase metaBase(name);
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId_, bundleName_, parentCloudId);
    int32_t ret = metaFile->DoLookup(metaBase);
    if (ret != E_OK) {
        LOGE("lookup dentry failed, ret = %{public}d", ret);
        return ENOENT;
    }
    value = std::to_string(metaBase.position);
    return E_OK;
}

int32_t CloudDiskRdbStore::FavoriteGetXattr(const std::string &cloudId, const std::string &key, std::string &value)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || cloudId == ROOT_CLOUD_ID || key != IS_FAVORITE_XATTR) {
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

int32_t CloudDiskRdbStore::FileStatusGetXattr(const std::string &cloudId, const std::string &key, std::string &value)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || cloudId == ROOT_CLOUD_ID || key != IS_FILE_STATUS_XATTR) {
        LOGE("getxattr parameter is invalid");
        return E_INVAL_ARG;
    }
    AbsRdbPredicates getXAttrPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    getXAttrPredicates.EqualTo(FileColumn::CLOUD_ID, cloudId);
    auto resultSet = rdbStore_->QueryByStep(getXAttrPredicates, { FileColumn::FILE_STATUS });
    if (resultSet == nullptr) {
        LOGE("get nullptr getxattr result");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        LOGE("getxattr result set go to next row failed");
        return E_RDB;
    }
    int32_t fileStatus;
    int32_t ret = CloudDiskRdbUtils::GetInt(FileColumn::FILE_STATUS, fileStatus, resultSet);
    if (ret != E_OK) {
        LOGE("get file status failed");
        return ret;
    }
    value = to_string(fileStatus);
    return E_OK;
}

int32_t CloudDiskRdbStore::GetExtAttrValue(const std::string &cloudId, const std::string &key, std::string &value)
{
    if (cloudId.empty() || cloudId == ROOT_CLOUD_ID || key.empty()) {
        LOGE("get ext attr value parameter is invalid");
        return E_INVAL_ARG;
    }

    std::string res;
    int32_t pos = 0;
    int32_t ret = GetExtAttr(cloudId, res, pos);
    if (ret != E_OK || res.empty()) {
        LOGE("get ext attr value res is empty");
        return E_RDB;
    }

    nlohmann::json jsonObj = nlohmann::json::parse(res, nullptr, false);
    if (jsonObj.is_discarded()) {
        LOGE("get ext jsonObj parse failed");
        return E_RDB;
    }

    LOGD("GetExtAttrValue, name %{public}s", key.c_str());
    if (!jsonObj.contains(key) || !jsonObj[key].is_string()) {
        LOGE("get ext not a string");
        return E_RDB;
    }

    value = jsonObj[key].get<std::string>();
    return E_OK;
}

int32_t CloudDiskRdbStore::GetExtAttr(const std::string &cloudId, std::string &value, int32_t &position)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || cloudId == ROOT_CLOUD_ID) {
        LOGE("get ext attr parameter is invalid");
        return E_INVAL_ARG;
    }
    AbsRdbPredicates getAttrPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    getAttrPredicates.EqualTo(FileColumn::CLOUD_ID, cloudId);
    auto resultSet = rdbStore_->QueryByStep(getAttrPredicates, FileColumn::EXT_ATTR_QUERY_COLUMNS);
    if (resultSet == nullptr) {
        LOGE("get nullptr get ext attr result");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        LOGE("get ext attr result set go to next row failed");
        return E_RDB;
    }

    int32_t ret = CloudDiskRdbUtils::GetString(FileColumn::ATTRIBUTE, value, resultSet);
    if (ret != E_OK) {
        LOGE("get ext attr value failed");
        return ret;
    }

    ret = CloudDiskRdbUtils::GetInt(FileColumn::POSITION, position, resultSet);
    if (ret != E_OK) {
        LOGE("get location value failed");
        return ret;
    }

    return E_OK;
}

int32_t CloudDiskRdbStore::GetXAttr(const std::string &cloudId, const std::string &key, std::string &value,
    const CacheNode &node, const std::string &extAttrKey)
{
    int32_t num = CheckXattr(key);
    switch (num) {
        case CLOUD_LOCATION:
            return LocationGetXattr(node.fileName, key, value, node.parentCloudId);
            break;
        case IS_FAVORITE:
            return FavoriteGetXattr(cloudId, key, value);
            break;
        case FILE_SYNC_STATUS:
            return FileStatusGetXattr(cloudId, key, value);
            break;
        case IS_EXT_ATTR:
            return GetExtAttrValue(cloudId, extAttrKey, value);
    }
    
    return E_INVAL_ARG;
}

static int32_t ExtAttributeSetValue(const std::string &key, const std::string &value,
    ValuesBucket &setXAttr, std::string &xattrList, int32_t pos)
{
    nlohmann::json jsonObj;
    if (xattrList.empty()) {
        jsonObj = nlohmann::json({{key, value}});
    } else {
        jsonObj = nlohmann::json::parse(xattrList, nullptr, false);
        if (jsonObj.is_discarded()) {
            LOGE("ext jsonObj parse failed");
            return E_RDB;
        }

        jsonObj[key] = value;
    }
    std::string jsonValue = jsonObj.dump();
    setXAttr.PutString(FileColumn::ATTRIBUTE, jsonValue);
    if (pos != LOCAL) {
        setXAttr.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_MDIRTY));
        setXAttr.PutLong(FileColumn::OPERATE_TYPE, static_cast<int32_t>(OperationType::UNKNOWN_TYPE));
    }
    setXAttr.PutLong(FileColumn::META_TIME_EDITED, UTCTimeMilliSeconds());
    return E_OK;
}

int32_t CloudDiskRdbStore::ExtAttributeSetXattr(const std::string &cloudId, const std::string &value,
    const std::string &key)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    ValuesBucket setAttr;
    int32_t changedRows = -1;
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(cloudId);

    TransactionOperations rdbTransaction(rdbStore_);
    int32_t ret = rdbTransaction.Start();
    if (ret != E_OK) {
        LOGE("Ext rdbstore begin transaction failed, ret = %{public}d", ret);
        return ret;
    }
    std::string xattrList;
    int32_t pos = 0;
    RETURN_ON_ERR(GetExtAttr(cloudId, xattrList, pos));
    RETURN_ON_ERR(ExtAttributeSetValue(key, value, setAttr, xattrList, pos));
    ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, setAttr,
        FileColumn::CLOUD_ID + " = ?", bindArgs);
    if (ret != E_OK) {
        LOGE("ext attr location fail, ret %{public}d", ret);
        return E_RDB;
    }
    rdbTransaction.Finish();
    CloudDiskSyncHelper::GetInstance().RegisterTriggerSync(bundleName_, userId_);
    return E_OK;
}

int32_t CloudDiskRdbStore::SetXAttr(const std::string &cloudId, const std::string &key, const std::string &value,
    const std::string &name, const std::string &parentCloudId)
{
    int32_t num = CheckXattr(key);
    switch (num) {
        case CLOUD_LOCATION:
            return LocationSetXattr(name, parentCloudId, cloudId, value);
            break;
        case CLOUD_RECYCLE:
            return RecycleSetXattr(name, parentCloudId, cloudId, value);
            break;
        case IS_FAVORITE:
            return FavoriteSetXattr(cloudId, value);
            break;
        case IS_EXT_ATTR:
            return ExtAttributeSetXattr(cloudId, value, name);
    }
    
    return E_INVAL_ARG;
}

static void FileRename(ValuesBucket &values, const int32_t &position, const std::string &newFileName)
{
    values.PutString(FileColumn::FILE_NAME, newFileName);
    values.PutInt(FileColumn::FILE_STATUS, FileStatus::TO_BE_UPLOADED);
    FillFileType(newFileName, values);
    if (position != LOCAL) {
        values.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_MDIRTY));
        values.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::RENAME));
    }
}

static void FileMove(ValuesBucket &values, const int32_t &position, const std::string &newParentCloudId)
{
    values.PutString(FileColumn::PARENT_CLOUD_ID, newParentCloudId);
    values.PutInt(FileColumn::FILE_STATUS, FileStatus::TO_BE_UPLOADED);
    if (position != LOCAL) {
        values.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_MDIRTY));
        values.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::MOVE));
    }
}

static void FileMoveAndRename(ValuesBucket &values, const int32_t &position, const std::string &newParentCloudId,
    const std::string &newFileName)
{
    values.PutString(FileColumn::PARENT_CLOUD_ID, newParentCloudId);
    values.PutString(FileColumn::FILE_NAME, newFileName);
    values.PutInt(FileColumn::FILE_STATUS, FileStatus::TO_BE_UPLOADED);
    FillFileType(newFileName, values);
    if (position != LOCAL) {
        values.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_MDIRTY));
        values.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::MOVE));
    }
}

static void HandleRenameValue(ValuesBucket &rename, int32_t position, const CacheNode &oldNode,
    const CacheNode &newNode)
{
    string oldParentCloudId = oldNode.parentCloudId;
    string oldFileName = oldNode.fileName;
    string newParentCloudId = newNode.parentCloudId;
    string newFileName = newNode.fileName;
    rename.PutLong(FileColumn::META_TIME_EDITED, UTCTimeMilliSeconds());
    if (oldFileName != newFileName && oldParentCloudId == newParentCloudId) {
        FileRename(rename, position, newFileName);
    }
    if (oldFileName == newFileName && oldParentCloudId != newParentCloudId) {
        FileMove(rename, position, newParentCloudId);
    }
    if (oldFileName != newFileName && oldParentCloudId != newParentCloudId) {
        FileMoveAndRename(rename, position, newParentCloudId, newFileName);
    }
}

int32_t CloudDiskRdbStore::Rename(const std::string &oldParentCloudId, const std::string &oldFileName,
    const std::string &newParentCloudId, const std::string &newFileName)
{
    int32_t ret = CheckName(newFileName);
    if (ret != E_OK) {
        return ret;
    }
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (oldParentCloudId.empty() || oldFileName.empty() || newParentCloudId.empty() || newFileName.empty()) {
        LOGE("rename parameters is invalid");
        return E_INVAL_ARG;
    }
    MetaBase metaBase(oldFileName);
    auto oldMetaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId_, bundleName_, oldParentCloudId);
    ret = oldMetaFile->DoLookup(metaBase);
    if (ret != E_OK) {
        LOGE("lookup dentry failed, ret = %{public}d", ret);
        return EINVAL;
    }
    ret = CheckNameForSpace(newFileName, S_ISDIR(metaBase.mode));
    if (ret != E_OK) {
        return ret;
    }
    ValuesBucket rename;
    CacheNode newNode = {.parentCloudId = newParentCloudId, .fileName = newFileName};
    CacheNode oldNode = {.parentCloudId = oldParentCloudId, .fileName = oldFileName};
    HandleRenameValue(rename, metaBase.position, oldNode, newNode);
    vector<ValueObject> bindArgs;
    bindArgs.emplace_back(metaBase.cloudId);
    auto newMetaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId_, bundleName_, newParentCloudId);
    ret = oldMetaFile->DoRename(metaBase, newFileName, newMetaFile);
    if (ret != E_OK) {
        LOGE("rename dentry failed, ret = %{public}d", ret);
        return EINVAL;
    }
    function<void()> rdbUpdate = [this, rename, bindArgs] {
        int32_t changedRows = -1;
        int32_t ret = rdbStore_ ->Update(changedRows, FileColumn::FILES_TABLE, rename,
                                         FileColumn::CLOUD_ID + " = ?", bindArgs);
        if (ret != E_OK) {
            LOGE("rename file fail, ret %{public}d", ret);
        }
        CloudDiskSyncHelper::GetInstance().RegisterTriggerSync(bundleName_, userId_);
    };
    ffrt::thread(rdbUpdate).detach();
    return E_OK;
}

int32_t CloudDiskRdbStore::GetHasChild(const std::string &cloudId, bool &hasChild)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    CLOUDID_IS_NULL(cloudId);
    AbsRdbPredicates readDirPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    readDirPredicates.EqualTo(FileColumn::PARENT_CLOUD_ID, cloudId)
        ->And()->EqualTo(FileColumn::FILE_TIME_RECYCLED, "0")->And()
        ->NotEqualTo(FileColumn::DIRTY_TYPE, to_string(static_cast<int32_t>(DirtyType::TYPE_DELETED)));
    auto resultSet = rdbStore_->QueryByStep(readDirPredicates, {FileColumn::FILE_NAME});
    if (resultSet == nullptr) {
        LOGE("get nullptr result set");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() == E_OK) {
        hasChild = true;
    } else {
        hasChild = false;
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
    int32_t ret = rdbStore_->Update(changedRows, FileColumn::FILES_TABLE, updateValue, FileColumn::CLOUD_ID + " = ?",
        whereArgs);
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
    int32_t ret = rdbStore_->Delete(changedRows, FileColumn::FILES_TABLE, FileColumn::CLOUD_ID + " = ?", whereArgs);
    if (ret != E_OK) {
        LOGE("unlink local directory fail, ret %{public}d", ret);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::Unlink(const std::string &cloudId, const int32_t &position)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || cloudId == ROOT_CLOUD_ID) {
        LOGE("Unlink parameters is invalid");
        return E_INVAL_ARG;
    }
    if (position == LOCAL) {
        RETURN_ON_ERR(UnlinkLocal(cloudId));
    } else {
        RETURN_ON_ERR(UnlinkSynced(cloudId));
        CloudDiskSyncHelper::GetInstance().RegisterTriggerSync(bundleName_, userId_);
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::GetDirtyType(const std::string &cloudId, int32_t &dirtyType)
{
    AbsRdbPredicates predicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    predicates.EqualTo(FileColumn::CLOUD_ID, cloudId);
    auto resultSet = rdbStore_->QueryByStep(predicates, {FileColumn::DIRTY_TYPE});
    if (resultSet == nullptr) {
        LOGE("get null result");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        LOGE("get current node resultSet fail");
        return E_RDB;
    }

    int32_t ret = CloudDiskRdbUtils::GetInt(FileColumn::DIRTY_TYPE, dirtyType, resultSet);
    if (ret != E_OK) {
        LOGE("get file status fail");
        return ret;
    }
    return E_OK;
}

int32_t CloudDiskRdbStore::GetCurNode(const std::string &cloudId, CacheNode &curNode)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || cloudId == ROOT_CLOUD_ID) {
        LOGE("parameter invalid");
        return E_INVAL_ARG;
    }
    AbsRdbPredicates predicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    predicates.EqualTo(FileColumn::CLOUD_ID, cloudId);
    auto resultSet = rdbStore_->QueryByStep(
        predicates, {FileColumn::PARENT_CLOUD_ID, FileColumn::IS_DIRECTORY, FileColumn::FILE_NAME});
    if (resultSet == nullptr) {
        LOGE("get null result");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        LOGE("get current node resultSet fail");
        return E_RDB;
    }
    RowEntity rowEntity;
    if (resultSet->GetRow(rowEntity) != E_OK) {
        LOGE("result set to file info get row failed");
        return E_RDB;
    }

    int32_t isDirectory;
    rowEntity.Get(FileColumn::PARENT_CLOUD_ID).GetString(curNode.parentCloudId);
    rowEntity.Get(FileColumn::FILE_NAME).GetString(curNode.fileName);
    rowEntity.Get(FileColumn::IS_DIRECTORY).GetInt(isDirectory);
    curNode.isDir = isDirectory ? "directory" : "file";

    return E_OK;
}

int32_t CloudDiskRdbStore::GetParentNode(const std::string parentCloudId, std::string &nextCloudId,
    std::string &fileName)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (parentCloudId.empty()) {
        LOGE("parameter invalid");
        return E_INVAL_ARG;
    }
    AbsRdbPredicates predicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    predicates.EqualTo(FileColumn::CLOUD_ID, parentCloudId);
    auto resultSet = rdbStore_->QueryByStep(predicates, {FileColumn::PARENT_CLOUD_ID, FileColumn::FILE_NAME});
    if (resultSet == nullptr) {
        LOGE("get null result");
        return E_RDB;
    }
    if (resultSet->GoToNextRow() != E_OK) {
        LOGE("get current node resultSet fail");
        return E_RDB;
    }
    RowEntity rowEntity;
    if (resultSet->GetRow(rowEntity) != E_OK) {
        LOGE("result set to file info get row failed");
        return E_RDB;
    }
    rowEntity.Get(FileColumn::PARENT_CLOUD_ID).GetString(nextCloudId);
    rowEntity.Get(FileColumn::FILE_NAME).GetString(fileName);
    return E_OK;
}

int32_t CloudDiskRdbStore::GetUriFromDB(const std::string &parentCloudId, std::string &uri)
{
    string realPrefix = CLOUDDISK_URI_PREFIX;
    realPrefix.replace(realPrefix.find(BUNDLENAME_FLAG), BUNDLENAME_FLAG.length(), bundleName_);
    if (parentCloudId.empty() || parentCloudId == rootId_ || parentCloudId == ROOT_CLOUD_ID) {
        uri = realPrefix + BACKFLASH + uri;
        return E_OK;
    }

    string nextCloudId;
    string fileName;
    int32_t ret = GetParentNode(parentCloudId, nextCloudId, fileName);
    if (ret != E_OK) {
        LOGI("get parentnode fail, parentCloudId: %{public}s", parentCloudId.c_str());
        return ret;
    }
    uri = fileName + BACKFLASH + uri;
    uint32_t queryTimes = 0;
    while (nextCloudId != ROOT_CLOUD_ID) {
        ret = GetParentNode(nextCloudId, nextCloudId, fileName);
        if (ret != E_OK) {
            return E_OK;
        }
        uri = fileName + BACKFLASH + uri;
        queryTimes++;
        if (uri.length() > PATH_MAX || queryTimes > MAX_QUERY_TIMES) {
            return E_INVAL_ARG;
        }
    }
    uri = realPrefix + BACKFLASH + uri;
    return E_OK;
}

int32_t CloudDiskRdbStore::GetNotifyUri(const CacheNode &cacheNode, std::string &uri)
{
    int32_t ret = CheckRootIdValid();
    if (ret != E_OK) {
        LOGE("rootId is invalid");
        return ret;
    }
    ret = CloudDiskNotifyUtils::GetUriFromCache(bundleName_, rootId_, cacheNode, uri);
    if (ret == E_OK) {
        return ret;
    }
    LOGD("get uri from cache fail, name: %{public}s", GetAnonyString(cacheNode.fileName).c_str());
    uri = cacheNode.fileName;
    ret = GetUriFromDB(cacheNode.parentCloudId, uri);
    if (ret == E_OK) {
        return ret;
    }
    LOGI("get uri from db fail, name: %{public}s", GetAnonyString(cacheNode.fileName).c_str());
    return ret;
}

int32_t CloudDiskRdbStore::GetNotifyData(const CacheNode &cacheNode, NotifyData &notifyData)
{
    int32_t ret = GetNotifyUri(cacheNode, notifyData.uri);
    if (ret == E_OK) {
        notifyData.isDir = cacheNode.isDir == "directory";
    }
    return ret;
}

int32_t CloudDiskRdbStore::CheckRootIdValid()
{
    if (!rootId_.empty()) {
        return E_OK;
    }
    CloudPrefImpl cloudPrefImpl(userId_, bundleName_, FileColumn::FILES_TABLE);
    cloudPrefImpl.GetString(ROOT_CLOUD_ID, rootId_);
    if (rootId_.empty()) {
        LOGE("get rootId fail");
        return E_INVAL_ARG;
    }
    LOGI("load rootis succ, rootId: %{public}s", rootId_.c_str());
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
        " AND OLD.meta_time_edited != new.meta_time_edited" +
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
        " AND OLD.file_status NOT IN (0,1) AND new.file_status NOT IN (1,2)" +
        " BEGIN SELECT cloud_sync_func(" + "'" + userId + "', " + "'" + bundleName + "'); END;";
    return CREATE_FILES_LOCAL_CLOUD_SYNC;
}

static int32_t ExecuteSql(RdbStore &store)
{
    static const vector<string> onCreateSqlStrs = {
        FileColumn::CREATE_FILE_TABLE,
        FileColumn::CREATE_PARENT_CLOUD_ID_INDEX,
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

static void VersionAddFileStatus(RdbStore &store)
{
    const string addFileStatus = FileColumn::ADD_FILE_STATUS;
    int32_t ret = store.ExecuteSql(addFileStatus);
    if (ret != NativeRdb::E_OK) {
        LOGE("add file_status fail, err %{public}d", ret);
    }
}

static void VersionSetFileStatusDefault(RdbStore &store)
{
    const string setFileStatus = FileColumn::SET_FILE_STATUS_DEFAULT;
    int32_t ret = store.ExecuteSql(setFileStatus);
    if (ret != NativeRdb::E_OK) {
        LOGE("set file_status fail, err %{public}d", ret);
    }
}

static void VersionFixSyncMetatimeTrigger(RdbStore &store)
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
}

static void VersionFixRetryTrigger(RdbStore &store)
{
    const string dropFilesLocalTrigger = "DROP TRIGGER IF EXISTS files_local_cloud_sync_trigger";
    if (store.ExecuteSql(dropFilesLocalTrigger) != NativeRdb::E_OK) {
        LOGE("drop local file trigger fail");
    }
    const string addFilesLocalTrigger = LocalFileTriggerSync(store);
    int32_t ret = store.ExecuteSql(addFilesLocalTrigger);
    if (ret != NativeRdb::E_OK) {
        LOGE("add local file trigger fail, err %{public}d", ret);
    }
}

static void VersionRemoveCloudSyncFuncTrigger(RdbStore &store)
{
    const string dropNewFolderTrigger = "DROP TRIGGER IF EXISTS folders_new_cloud_sync_trigger";
    if (store.ExecuteSql(dropNewFolderTrigger) != NativeRdb::E_OK) {
        LOGE("drop folders_new_cloud_sync_trigger fail");
    }
    const string dropUpdateFileTrigger = "DROP TRIGGER IF EXISTS files_update_cloud_sync_trigger";
    if (store.ExecuteSql(dropUpdateFileTrigger) != NativeRdb::E_OK) {
        LOGE("drop files_update_cloud_sync_trigger fail");
    }
    const string dropFileDeleteTrigger = "DROP TRIGGER IF EXISTS files_delete_cloud_sync_trigger";
    if (store.ExecuteSql(dropFileDeleteTrigger) != NativeRdb::E_OK) {
        LOGE("drop files_delete_cloud_sync_trigger fail");
    }
    const string dropFileLocalTrigger = "DROP TRIGGER IF EXISTS files_local_cloud_sync_trigger";
    if (store.ExecuteSql(dropFileLocalTrigger) != NativeRdb::E_OK) {
        LOGE("drop files_local_cloud_sync_trigger fail");
    }
}

static void VersionAddThmFlag(RdbStore &store)
{
    const string addThmFlag = FileColumn::ADD_THM_FLAG;
    int32_t ret = store.ExecuteSql(addThmFlag);
    if (ret != NativeRdb::E_OK) {
        LOGE("add thm_flag fail, err %{public}d", ret);
    }
    const string addLcdFlag = FileColumn::ADD_LCD_FLAG;
    ret = store.ExecuteSql(addLcdFlag);
    if (ret != NativeRdb::E_OK) {
        LOGE("add lcd_flag fail, err %{public}d", ret);
    }
    const string addUploadFlag = FileColumn::ADD_UPLOAD_FLAG;
    ret = store.ExecuteSql(addUploadFlag);
    if (ret != NativeRdb::E_OK) {
        LOGE("add no_need_upload fail, err %{public}d", ret);
    }
}

static int32_t GetMetaBaseData(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    RETURN_ON_ERR(CloudDiskRdbUtils::GetString(FileColumn::CLOUD_ID, info.cloudId, resultSet));
    RETURN_ON_ERR(CloudDiskRdbUtils::GetString(FileColumn::FILE_NAME, info.name, resultSet));
    int32_t isDir = 0;
    RETURN_ON_ERR(CloudDiskRdbUtils::GetInt(FileColumn::IS_DIRECTORY, isDir, resultSet));
    info.IsDirectory = static_cast<bool>(isDir);
    int32_t position = 0;
    RETURN_ON_ERR(CloudDiskRdbUtils::GetInt(FileColumn::POSITION, position, resultSet));
    info.location = static_cast<uint32_t>(position);
    int64_t atime = 0;
    RETURN_ON_ERR(CloudDiskRdbUtils::GetLong(FileColumn::FILE_TIME_ADDED, atime, resultSet));
    info.atime = static_cast<uint64_t>(atime);
    int64_t mtime = 0;
    RETURN_ON_ERR(CloudDiskRdbUtils::GetLong(FileColumn::FILE_TIME_EDITED, mtime, resultSet));
    info.mtime = static_cast<uint64_t>(mtime);
    int64_t size = 0;
    RETURN_ON_ERR(CloudDiskRdbUtils::GetLong(FileColumn::FILE_SIZE, size, resultSet));
    info.size = static_cast<uint64_t>(size);
    int64_t rowId = 0;
    RETURN_ON_ERR(CloudDiskRdbUtils::GetLong(FileColumn::ROW_ID, rowId, resultSet));
    info.rowId = static_cast<uint64_t>(rowId);
    return E_OK;
}

static int32_t GetUserIdAndBundleName(RdbStore &store, uint32_t &userId, string &bundleName)
{
    string userIdStr;
    GenCloudSyncTriggerFuncParams(store, userIdStr, bundleName);
    bool isValid = std::all_of(userIdStr.begin(), userIdStr.end(), ::isdigit);
    if (!isValid) {
        LOGE("invalid user Id");
        return E_INVAL_ARG;
    }
    userId = static_cast<uint32_t>(std::stoi(userIdStr));
    return E_OK;
}

static int32_t GenerateDentryRecursively(RdbStore &store, const string &parentCloudId)
{
    LOGD("Generate dentry recursively parentCloudId:%{public}s", parentCloudId.c_str());
    uint32_t userId;
    string bundleName;
    RETURN_ON_ERR(GetUserIdAndBundleName(store, userId, bundleName));
    AbsRdbPredicates lookUpPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    lookUpPredicates.EqualTo(FileColumn::PARENT_CLOUD_ID, parentCloudId)
        ->And()->EqualTo(FileColumn::FILE_TIME_RECYCLED, "0");
    int32_t rowCount = 0;
    uint64_t offset = 0;
    do {
        lookUpPredicates.Limit(offset, CHECK_QUERY_LIMIT);
        auto resultSet = store.Query(lookUpPredicates, FileColumn::FILE_SYSTEM_QUERY_COLUMNS);
        if (resultSet == nullptr) {
            LOGE("failed to get result set at offset:%{public}" PRIu64 "", offset);
            continue;
        }
        int32_t ret = resultSet->GetRowCount(rowCount);
        if (ret != E_OK || rowCount < 0) {
            LOGE("failed to get row count at offset:%{public}" PRIu64 ", ret: %{public}d", offset, ret);
            continue;
        }
        if (rowCount == 0) {
            return E_OK;
        }
        CloudDiskFileInfo info;
        while (resultSet->GoToNextRow() == 0) {
            RETURN_ON_ERR(GetMetaBaseData(info, resultSet));
            MetaBase metaBase(info.name);
            auto callback = [info] (MetaBase &m) {
                m.cloudId = info.cloudId;
                m.atime = info.atime;
                m.mtime = info.mtime;
                m.size = info.size;
                m.mode = (info.IsDirectory) ? (S_IFDIR | STAT_MODE_DIR) : (S_IFREG | STAT_MODE_REG);
                m.position = info.location;
                m.fileType = FILE_TYPE_CONTENT;
            };
            auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, bundleName, parentCloudId);
            ret = metaFile->DoLookupAndUpdate(info.name, callback);
            if (ret != E_OK) {
                LOGE("insert new dentry failed, ret = %{public}d", ret);
                return ret;
            }
            if (info.IsDirectory) {RETURN_ON_ERR(GenerateDentryRecursively(store, info.cloudId));}
        }
        offset += CHECK_QUERY_LIMIT;
    } while (rowCount != 0);
    return E_OK;
}

static int32_t GenerateRecycleDentryRecursively(RdbStore &store)
{
    uint32_t userId;
    string bundleName;
    RETURN_ON_ERR(GetUserIdAndBundleName(store, userId, bundleName));
    AbsRdbPredicates lookUpPredicates = AbsRdbPredicates(FileColumn::FILES_TABLE);
    lookUpPredicates.NotEqualTo(FileColumn::FILE_TIME_RECYCLED, "0");
    int32_t rowCount = 0;
    uint64_t offset = 0;
    do {
        lookUpPredicates.Limit(offset, CHECK_QUERY_LIMIT);
        auto resultSet = store.Query(lookUpPredicates, FileColumn::FILE_SYSTEM_QUERY_COLUMNS);
        if (resultSet == nullptr) {
            LOGE("failed to get result set at offset:%{public}" PRIu64 "", offset);
            continue;
        }
        int32_t ret = resultSet->GetRowCount(rowCount);
        if (ret != E_OK || rowCount < 0) {
            LOGE("failed to get row count at offset:%{public}" PRIu64 ", ret: %{public}d", offset, ret);
            continue;
        }
        if (rowCount == 0) {
            return E_OK;
        }
        CloudDiskFileInfo info;
        while (resultSet->GoToNextRow() == 0) {
            RETURN_ON_ERR(GetMetaBaseData(info, resultSet));
            string uniqueName = info.name + "_" + std::to_string(info.rowId);
            MetaBase metaBase(uniqueName);
            auto callback = [info] (MetaBase &m) {
                m.cloudId = info.cloudId;
                m.atime = info.atime;
                m.mtime = info.mtime;
                m.size = info.size;
                m.mode = (info.IsDirectory) ? (S_IFDIR | STAT_MODE_DIR) : (S_IFREG | STAT_MODE_REG);
                m.position = info.location;
                m.fileType = FILE_TYPE_CONTENT;
            };
            RETURN_ON_ERR(MetaFileMgr::GetInstance().CreateRecycleDentry(userId, bundleName));
            auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, bundleName, RECYCLE_CLOUD_ID);
            ret = metaFile->DoLookupAndUpdate(uniqueName, callback);
            if (ret != E_OK) {
                LOGE("insert new dentry failed, ret = %{public}d", ret);
                return ret;
            }
        }
        offset += CHECK_QUERY_LIMIT;
    } while (rowCount != 0);
    return E_OK;
}

static void VersionAddCheckFlag(RdbStore &store)
{
    const string addCheckFlag = FileColumn::ADD_CHECK_FLAG;
    int32_t ret = store.ExecuteSql(addCheckFlag);
    if (ret != NativeRdb::E_OK) {
        LOGE("add check_flag fail, ret = %{public}d", ret);
    }
    ret = GenerateDentryRecursively(store, ROOT_CLOUD_ID);
    if (ret != E_OK) {
        LOGE("failed to generate dentry recursively, ret = %{public}d", ret);
    }
    ret = GenerateRecycleDentryRecursively(store);
    if (ret != E_OK) {
        LOGE("failed to generate recycle ndentry recursively, ret = %{public}d", ret);
    }
}

static void VersionAddRootDirectory(RdbStore &store)
{
    const string addRootDirectory = "ALTER Table " + FileColumn::FILES_TABLE +
        " ADD COLUMN " + FileColumn::ROOT_DIRECTORY + " TEXT";
    int32_t ret = store.ExecuteSql(addRootDirectory);
    if (ret != NativeRdb::E_OK) {
        LOGE("add root_directory fail, ret = %{public}d", ret);
    }
    ValuesBucket rootDirectory;
    rootDirectory.PutString(FileColumn::ROOT_DIRECTORY, system::GetParameter(FILEMANAGER_KEY, ""));
    int32_t changedRows = -1;
    vector<ValueObject> bindArgs;
    ret = store.Update(changedRows, FileColumn::FILES_TABLE, rootDirectory, "", bindArgs);
    if (ret != NativeRdb::E_OK) {
        LOGE("set root_directory fail, err %{public}d", ret);
    }
}

static void VersionAddAttribute(RdbStore &store)
{
    const string attrbute = FileColumn::ADD_ATTRIBUTE;
    int32_t ret = store.ExecuteSql(attrbute);
    if (ret != NativeRdb::E_OK) {
        LOGE("add attrbute fail, ret = %{public}d", ret);
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
    if (oldVersion < VERSION_ADD_FILE_STATUS) {
        VersionAddFileStatus(store);
    }
    if (oldVersion < VERSION_SET_FILE_STATUS_DEFAULT) {
        VersionSetFileStatusDefault(store);
    }
    if (oldVersion < VERSION_ADD_CHECK_FLAG) {
        VersionAddCheckFlag(store);
    }
    if (oldVersion < VERSION_ADD_ROOT_DIRECTORY) {
        VersionAddRootDirectory(store);
    }
    if (oldVersion < VERSION_FIX_SYNC_METATIME_TRIGGER) {
        VersionFixSyncMetatimeTrigger(store);
    }
    if (oldVersion < VERSION_FIX_RETRY_TRIGGER) {
        VersionFixRetryTrigger(store);
    }

    if (oldVersion < VERSION_REMOVE_CLOUD_SYNC_FUNC_TRIGGER) {
        VersionRemoveCloudSyncFuncTrigger(store);
    }

    if (oldVersion < VERSION_ADD_ATTRIBUTE) {
        VersionAddAttribute(store);
    }
    if (oldVersion < VERSION_ADD_THM_FLAG) {
        VersionAddThmFlag(store);
    }
    return NativeRdb::E_OK;
}
}