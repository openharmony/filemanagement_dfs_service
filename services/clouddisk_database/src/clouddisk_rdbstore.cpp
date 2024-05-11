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
#include "rdb_errno.h"
#include "rdb_sql_utils.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using namespace DriveKit;
using namespace OHOS::NativeRdb;
using namespace CloudSync;

enum XATTR_CODE {
    ERROR_CODE = -1,
    CLOUD_LOCATION = 1,
    CLOUD_RECYCLE,
    IS_FAVORITE,
    FILE_SYNC_STATUS
};
static constexpr int32_t LOOKUP_QUERY_LIMIT = 1;
static const uint32_t SET_STATE = 1;
static const uint32_t CANCEL_STATE = 0;
static const uint32_t MAX_FILE_NAME_SIZE = 246;
static const uint32_t MAX_QUERY_TIMES = 1024;
static const uint32_t STAT_MODE_DIR = 0771;
const string BUNDLENAME_FLAG = "<BundleName>";
const string CLOUDDISK_URI_PREFIX = "file://<BundleName>/data/storage/el2/cloud";
const string BACKFLASH = "/";
static const string RECYCLE_FILE_NAME = ".trash";

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
    config_.SetReadConSize(CONNECT_SIZE);
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

int32_t CloudDiskRdbStore::SetAttr(const std::string &fileName, const std::string &parentCloudId,
    const std::string &cloudId, const unsigned long long &size)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty()) {
        LOGE("cloudId is empty");
        return E_INVAL_ARG;
    }
    if (cloudId == "rootId") {
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

static int64_t Timespec2Milliseconds(const struct timespec &time)
{
    return time.tv_sec * SECOND_TO_MILLISECOND + time.tv_nsec / MILLISECOND_TO_NANOSECOND;
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
        if (fileName[fileName.length() - 1] == ' ' || fileName == RECYCLE_FILE_NAME) {
            LOGI("Illegal name");
            return EINVAL;
        }
    }
    return E_OK;
}

static int32_t CheckName(const std::string &fileName)
{
    if (fileName.empty()) {
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
        {'%', true},
        {'&', true},
        {'#', true},
        {';', true},
        {'!', true},
        {'\'', true}
    };
    for (char c : fileName) {
        if (illegalCharacter.find(c) != illegalCharacter.end()) {
            LOGI("Illegal name");
            return EINVAL;
        }
    }
    size_t lastDot = fileName.rfind('.');
    if (lastDot == std::string::npos) {
        lastDot = fileName.length();
    }
    std::string realFileName = fileName.substr(0, lastDot);
    if (realFileName.find(".") != std::string::npos ||
        ((fileName.find("emoji") != std::string::npos) && realFileName != "emoji") ||
        fileName.length() > MAX_FILE_NAME_SIZE) {
        LOGI("Illegal name");
        return EINVAL;
    }
    return E_OK;
}

static int32_t CreateFile(const std::string &fileName, const std::string &filePath, ValuesBucket &fileInfo,
    struct stat *statInfo)
{
    int32_t ret = stat(filePath.c_str(), statInfo);
    if (ret) {
        LOGE("filePath %{private}s is invalid", filePath.c_str());
        return E_PATH;
    }
    fileInfo.PutInt(FileColumn::IS_DIRECTORY, FILE);
    fileInfo.PutLong(FileColumn::FILE_SIZE, statInfo->st_size);
    fileInfo.PutLong(FileColumn::FILE_TIME_EDITED, Timespec2Milliseconds(statInfo->st_mtim));
    fileInfo.PutLong(FileColumn::META_TIME_EDITED, Timespec2Milliseconds(statInfo->st_mtim));
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
    };
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(userId, bundleName, parentCloudId);
    int32_t ret = metaFile->DoLookupAndUpdate(fileName, callback);
    if (ret != E_OK) {
        LOGE("update new dentry failed, ret = %{public}d", ret);
        return ret;
    }
    return E_OK;
}

static void UpdateDatabase(MetaBase &metaBase, int64_t fileTimeAdded, struct stat *statInfo)
{
    metaBase.atime = fileTimeAdded;
    metaBase.mtime = Timespec2Milliseconds(statInfo->st_mtim);
    metaBase.mode = statInfo->st_mode;
    metaBase.size = statInfo->st_size;
    metaBase.position = LOCAL;
    metaBase.fileType = FILE_TYPE_CONTENT;
}

int32_t CloudDiskRdbStore::Create(const std::string &cloudId, const std::string &parentCloudId,
    const std::string &fileName)
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
    fileInfo.PutString(FileColumn::CLOUD_ID, cloudId);
    fileInfo.PutString(FileColumn::FILE_NAME, fileName);
    int64_t fileTimeAdded = UTCTimeMilliSeconds();
    fileInfo.PutLong(FileColumn::FILE_TIME_ADDED, fileTimeAdded);
    fileInfo.PutString(FileColumn::PARENT_CLOUD_ID, parentCloudId);
    fileInfo.PutInt(FileColumn::DIRTY_TYPE, static_cast<int32_t>(DirtyType::TYPE_NO_NEED_UPLOAD));
    fileInfo.PutLong(FileColumn::OPERATE_TYPE, static_cast<int64_t>(OperationType::NEW));
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
    MetaBase metaBase(fileName, cloudId);
    UpdateDatabase(metaBase, fileTimeAdded, &statInfo);
    ret = CreateDentry(metaBase, userId_, bundleName_, fileName, parentCloudId);
    if (ret != E_OK) {
        LOGE("create new dentry failed, ret = %{public}d", ret);
        return ret;
    }
    rdbTransaction.Finish();
    return E_OK;
}

int32_t CloudDiskRdbStore::MkDir(const std::string &cloudId, const std::string &parentCloudId,
    const std::string &directoryName)
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
    MetaBase metaBase(directoryName, cloudId);
    metaBase.atime = fileTimeAdded;
    metaBase.mtime = fileTimeEdited;
    metaBase.mode = S_IFDIR | STAT_MODE_DIR;
    metaBase.position = LOCAL;
    metaBase.fileType = FILE_TYPE_CONTENT;
    ret = CreateDentry(metaBase, userId_, bundleName_, directoryName, parentCloudId);
    if (ret != E_OK) {
        LOGE("create new dentry failed, ret = %{public}d", ret);
        return ret;
    }
    rdbTransaction.Finish();
    return E_OK;
}

static void HandleWriteValue(ValuesBucket &write, int32_t position, struct stat &statInfo)
{
    write.PutLong(FileColumn::FILE_SIZE, statInfo.st_size);
    write.PutLong(FileColumn::FILE_TIME_EDITED, Timespec2Milliseconds(statInfo.st_mtim));
    write.PutLong(FileColumn::META_TIME_EDITED, Timespec2Milliseconds(statInfo.st_mtim));
    write.PutLong(FileColumn::FILE_TIME_VISIT, Timespec2Milliseconds(statInfo.st_atim));
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
    int32_t ret = metaFile->DoLookupAndUpdate(fileName, callback);
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
    metaBase.mtime = Timespec2Milliseconds(statInfo.st_mtim);
    metaBase.size = statInfo.st_size;
    ret = WriteUpdateDentry(metaBase, userId_, bundleName_, fileName, parentCloudId);
    if (ret != E_OK) {
        LOGE("write update dentry failed, ret %{public}d", ret);
        return E_RDB;
    }
    rdbTransaction.Finish();
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
    } else if (val == 1) {
        setXAttr.PutInt(FileColumn::OPERATE_TYPE, static_cast<int32_t>(OperationType::DELETE));
        setXAttr.PutLong(FileColumn::FILE_TIME_RECYCLED, UTCTimeMilliSeconds());
        setXAttr.PutInt(FileColumn::DIRECTLY_RECYCLED, SET_STATE);
    } else {
        LOGE("invalid value");
        return E_RDB;
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
    } else if (key == IS_FILE_STATUS_XATTR) {
        return FILE_SYNC_STATUS;
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

int32_t CloudDiskRdbStore::FileStatusGetXattr(const std::string &cloudId, const std::string &key, std::string &value)
{
    RDBPTR_IS_NULLPTR(rdbStore_);
    if (cloudId.empty() || cloudId == "rootId" || key != IS_FILE_STATUS_XATTR) {
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
        case FILE_SYNC_STATUS:
            return FileStatusGetXattr(cloudId, key, value);
            break;
    }
    if (cloudId.empty() || cloudId == "rootId") {
        LOGE("getxattr parameter is invalid");
        return E_INVAL_ARG;
    } else {
        return E_OK;
    }
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
    CacheNode newNode = {.fileName = newFileName, .parentCloudId = newParentCloudId};
    CacheNode oldNode = {.fileName = oldFileName, .parentCloudId = oldParentCloudId};
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
    if (cloudId.empty() || cloudId == "rootId") {
        LOGE("Unlink parameters is invalid");
        return E_INVAL_ARG;
    }
    if (position == LOCAL) {
        RETURN_ON_ERR(UnlinkLocal(cloudId));
    } else {
        RETURN_ON_ERR(UnlinkSynced(cloudId));
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
    if (cloudId.empty() || cloudId == "rootId") {
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
    if (parentCloudId.empty() || parentCloudId == rootId_ || parentCloudId == "rootId") {
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
    while (nextCloudId != "rootId") {
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
    LOGD("get uri from cache fail, name: %{public}s", cacheNode.fileName.c_str());
    uri = cacheNode.fileName;
    ret = GetUriFromDB(cacheNode.parentCloudId, uri);
    if (ret == E_OK) {
        return ret;
    }
    LOGI("get uri from db fail, name: %{public}s", cacheNode.fileName.c_str());
    return ret;
}

int32_t CloudDiskRdbStore::GetNotifyData(const DriveKit::DKRecord &record, NotifyData &notifyData)
{
    CacheNode cacheNode;
    DKRecordData data;
    record.GetRecordData(data);
    cacheNode.cloudId = record.GetRecordId();
    if (data.find(DK_PARENT_CLOUD_ID) != data.end()) {
        data.at(DK_PARENT_CLOUD_ID).GetString(cacheNode.parentCloudId);
    }
    if (data.find(DK_FILE_NAME) != data.end()) {
        data.at(DK_FILE_NAME).GetString(cacheNode.fileName);
    }
    if (data.find(DK_IS_DIRECTORY) != data.end()) {
        data.at(DK_IS_DIRECTORY).GetString(cacheNode.isDir);
    }
    if (cacheNode.parentCloudId.empty() || cacheNode.fileName.empty() || cacheNode.isDir.empty()) {
        LOGE("get field from data fail, parent: %{public}s, fileName: %{public}s, isDir: %{public}s",
             cacheNode.parentCloudId.c_str(), cacheNode.fileName.c_str(), cacheNode.isDir.c_str());
        return E_INVAL_ARG;
    }

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
    cloudPrefImpl.GetString("rootId", rootId_);
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
    return NativeRdb::E_OK;
}
}