/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cloud_disk_service_logfile.h"

#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <securec.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cloud_disk_service_callback_manager.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_service_manager.h"
#include "cloud_disk_sync_folder.h"
#include "cloud_file_utils.h"
#include "convertor.h"
#include "file_utils.h"
#include "utils_directory.h"
#include "utils_log.h"
#include "uuid_helper.h"

namespace OHOS::FileManagement::CloudDiskService {

const std::string LOGFILENAME = "history.log";
const unsigned int STAT_MODE_DIR = 0771;
const unsigned int MAX_CHANGEDATAS_SIZE = 20;
const uint64_t CHANGE_DATA_TIMEOUT_MS = 1000;

static std::string GetLogFileByPath(const int32_t userId, const uint32_t syncFolderIndex)
{
    std::string rootDir =
        "/data/service/el2/" + std::to_string(userId) +
        "/hmdfs/cache/account_cache/dentry_cache/clouddisk_service_cache/" +
        Convertor::ConvertToHex(syncFolderIndex) + "/";
    Storage::DistributedFile::Utils::ForceCreateDirectory(rootDir, STAT_MODE_DIR);
    return rootDir + LOGFILENAME;
}

static void GetBucketAndOffset(const uint64_t line, uint32_t &bucket, uint32_t &offset)
{
    uint32_t temp = line & LOGGROUP_INDEX_MASK;
    bucket = temp >> LOGGROUP_SHIFT;
    offset = temp & LOG_INDEX_MASK;
}

static off_t GetBucketaddr(const uint32_t bucket)
{
    return (static_cast<off_t>(bucket) << LOGGROUP_SIZE_SHIFT) + LOGGROUP_HEADER;
}

static std::unique_ptr<LogGroup> LoadCurrentPage(int fd, uint32_t bucket)
{
    auto logGroup = std::make_unique<LogGroup>();
    off_t pos = GetBucketaddr(bucket);
    auto ret = FileRangeLock::FilePosLock(fd, pos, LOGGROUP_SIZE, F_WRLCK);
    if (ret) {
        return nullptr;
    }
    ssize_t size = FileUtils::ReadFile(fd, pos, LOGGROUP_SIZE, logGroup.get());
    if (size != LOGGROUP_SIZE) {
        (void)FileRangeLock::FilePosLock(fd, pos, LOGGROUP_SIZE, F_UNLCK);
        return nullptr;
    }
    return logGroup;
}

static void UnlockCurrentPage(int fd, uint32_t bucket)
{
    off_t pos = GetBucketaddr(bucket);
    (void)FileRangeLock::FilePosLock(fd, pos, LOGGROUP_SIZE, F_UNLCK);
}

static int32_t SyncCurrentPage(LogGroup &logGroup, int fd, uint32_t bucket)
{
    off_t pos = GetBucketaddr(bucket);
    int size = FileUtils::WriteFile(fd, &logGroup, pos, LOGGROUP_SIZE);
    if (size != LOGGROUP_SIZE) {
        LOGD("WriteFile failed, size %{public}d != %{public}d", size, LOGGROUP_SIZE);
        (void)FileRangeLock::FilePosLock(fd, pos, LOGGROUP_SIZE, F_UNLCK);
        return EINVAL;
    }
    auto ret = FileRangeLock::FilePosLock(fd, pos, LOGGROUP_SIZE, F_UNLCK);
    if (ret) {
        return ret;
    }
    return E_OK;
}

static int32_t FillLogGroup(struct LogGroup &logGroup, const struct LogBlock &logBlock, uint32_t offset)
{
    if (logGroup.nsl[offset].timestamp != 0) {
        (void) memset_s(&logGroup, LOGGROUP_SIZE, 0, LOGGROUP_SIZE);
    }
    auto ret = memcpy_s(&logGroup.nsl[offset], sizeof(struct LogBlock), &logBlock, sizeof(struct LogBlock));
    if (ret != 0) {
        LOGE("memcpy_s struct LogBlock failed, errno = %{public}d", errno);
        return -1;
    }
    logGroup.logBlockCnt++;
    return E_OK;
}

static uint32_t GetCurrentLine(int fd)
{
    uint32_t startLine = 0;
    uint32_t offset = 0;
    for (uint32_t i = 0; i < LOGGROUP_MAX; i++) {
        auto logGroup = LoadCurrentPage(fd, i);
        if (logGroup == nullptr) {
            LOGE("load page failed");
            return 0;
        }
        if (logGroup->nsl[0].timestamp == 0) {
            UnlockCurrentPage(fd, i);
            break;
        } else if (logGroup->nsl[0].line != startLine + offset) {
            UnlockCurrentPage(fd, i);
            break;
        } else {
            startLine = logGroup->nsl[0].line;
            offset = logGroup->logBlockCnt;
            UnlockCurrentPage(fd, i);
        }
    }
    return startLine + offset;
}

int32_t CloudDiskServiceLogFile::WriteLogFile(const struct LogBlock &logBlock)
{
    uint32_t bucket;
    uint32_t offset;
    GetBucketAndOffset(logBlock.line, bucket, offset);

    auto logGroup = LoadCurrentPage(fd_, bucket);
    if (logGroup == nullptr) {
        LOGE("load page failed");
        return -1;
    }
    FillLogGroup(*logGroup, logBlock, offset);
    SyncCurrentPage(*logGroup, fd_, bucket);
    return E_OK;
}

int32_t CloudDiskServiceLogFile::ReadLogFile(const uint64_t line, struct LogBlock &logBlock)
{
    uint32_t bucket;
    uint32_t offset;
    GetBucketAndOffset(line, bucket, offset);

    auto logGroup = LoadCurrentPage(fd_, bucket);
    if (logGroup == nullptr) {
        LOGE("load page failed");
        return -1;
    }
    logBlock = logGroup->nsl[offset];
    UnlockCurrentPage(fd_, bucket);
    return E_OK;
}

int32_t CloudDiskServiceLogFile::OnDataChange()
{
    std::lock_guard<std::mutex> lock(vectorMtx_);

    if (changeDatas_.empty()) {
        return E_OK;
    }
    CloudDiskServiceCallbackManager::GetInstance().OnChangeData(syncFolderIndex_, changeDatas_);
    changeDatas_.clear();

    return E_OK;
}

CloudDiskServiceLogFile::CloudDiskServiceLogFile(const int32_t userId, const uint32_t syncFolderIndex)
{
    userId_ = userId;
    syncFolderIndex_ = syncFolderIndex;
    needCallback_ = false;
    CloudDiskSyncFolder::GetInstance().GetSyncFolderByIndex(syncFolderIndex_, syncFolderPath_);
    logFilePath_ = GetLogFileByPath(userId_, syncFolderIndex_);
    if (access(logFilePath_.c_str(), F_OK) == 0) {
        fd_ = UniqueFd{open(logFilePath_.c_str(), O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)};
        currentLine_ = GetCurrentLine(fd_);
    } else {
        fd_ = UniqueFd{open(logFilePath_.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)};
        ftruncate(fd_, LOGGROUP_SIZE * LOGGROUP_MAX + LOGGROUP_HEADER);
        currentLine_ = 0;
    }
}

void CloudDiskServiceLogFile::SetSyncFolderPath(const std::string &path)
{
    syncFolderPath_ = path;
}

int32_t CloudDiskServiceLogFile::GenerateLogBlock(const struct EventInfo &eventInfo, const uint64_t parentInode,
                                                  const struct LogGenerateCtx &ctx, const std::string &parentRecordId,
                                                  uint64_t &line)
{
    struct LogBlock logBlock;
    logBlock.line = currentLine_++;
    logBlock.timestamp = eventInfo.timestamp;
    logBlock.parentInode = parentInode;
    logBlock.hash = CloudDisk::CloudFileUtils::DentryHash(eventInfo.name);
    logBlock.operationType = static_cast<uint8_t>(eventInfo.operateType);
    logBlock.bidx = ctx.bidx;
    logBlock.bitPos = ctx.bitPos;
    auto ret = memcpy_s(logBlock.parentRecordId, RECORD_ID_LEN, parentRecordId.c_str(), parentRecordId.length());
    if (ret != 0) {
        LOGE("memcpy_s parentRecordId failed, errno = %{public}d", errno);
        return -1;
    }
    WriteLogFile(logBlock);
    line = logBlock.line;
    LOGD("Generate log line:%{public}llu, operationType:%{public}d", line, logBlock.operationType);
    return E_OK;
}

void CloudDiskServiceLogFile::GenerateChangeDataForInvalid(const struct EventInfo &eventInfo)
{
    std::lock_guard<std::mutex> lock(vectorMtx_);
    struct ChangeData changeData{};
    changeData.operationType = eventInfo.operateType;
    changeData.timeStamp = eventInfo.timestamp;
    changeDatas_.push_back(changeData);
    CloudDiskServiceCallbackManager::GetInstance().OnChangeData(syncFolderIndex_, changeDatas_);
}

int32_t CloudDiskServiceLogFile::GenerateChangeData(const struct EventInfo &eventInfo, uint64_t line,
                                                    const std::string &childRecordId, const std::string &parentRecordId)
{
    std::lock_guard<std::mutex> lock(vectorMtx_);
    std::string fullPath = eventInfo.path + "/" + eventInfo.name;
    struct ChangeData changeData{};
    changeData.updateSequenceNumber = line;
    changeData.fileId = childRecordId;
    changeData.parentFileId = parentRecordId;
    if (fullPath.compare(0, syncFolderPath_.size(), syncFolderPath_) == 0) {
        changeData.relativePath = fullPath.substr(syncFolderPath_.size());
    } else {
        changeData.relativePath = fullPath;
    }
    changeData.operationType = eventInfo.operateType;

    struct stat childStat;
    if (stat(fullPath.c_str(), &childStat) != 0) {
        changeData.size = 0;
        changeData.mtime = 0;
    } else {
        changeData.size = childStat.st_size;
        changeData.mtime = childStat.st_mtime;
    }

    changeData.timeStamp = eventInfo.timestamp;
    changeDatas_.push_back(changeData);
    LOGD("Generate changedata line:%{public}llu, operationType:%{public}d, size:%{public}zu", line,
        static_cast<uint8_t>(changeData.operationType), changeDatas_.size());
    if (changeDatas_.size() >= MAX_CHANGEDATAS_SIZE) {
        CloudDiskServiceCallbackManager::GetInstance().OnChangeData(syncFolderIndex_, changeDatas_);
        changeDatas_.clear();
    }
    return E_OK;
}

int32_t CloudDiskServiceLogFile::FillChildForDir(const std::string &path, const uint64_t timestamp)
{
    struct stat statInfo;
    if (stat(path.c_str(), &statInfo) != 0) {
        LOGE("stat parent failed for %{public}d", errno);
        return errno;
    }

    if (!S_ISDIR(statInfo.st_mode)) {
        return E_OK;
    }
    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGE("opendir failed errno = %{public}d", errno);
        return errno;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        struct EventInfo eventInfo(userId_, syncFolderIndex_, OperationType::CREATE, path + "/" + entry->d_name);
        LOGD("FillChildForDir for path: %{public}s", (path + "/" + entry->d_name).c_str());
        ProduceLog(eventInfo);
    }

    closedir(dir);
    return E_OK;
}


void CloudDiskServiceLogFile::StartCallback()
{
    LOGD("StartCallback, %{public}u", syncFolderIndex_);
    needCallback_ = true;
}

void CloudDiskServiceLogFile::StopCallback()
{
    LOGD("StopCallback, %{public}u", syncFolderIndex_);
    needCallback_ = false;
}

int32_t CloudDiskServiceLogFile::ProduceLog(const struct EventInfo &eventInfo)
{
    struct stat parentStat;
    if (stat(eventInfo.path.c_str(), &parentStat) != 0) {
        LOGE("stat parent failed for %{public}d", errno);
        return errno;
    }

    int32_t ret = 0;
    if (eventInfo.operateType == OperationType::SYNC_FOLDER_INVALID) {
        GenerateChangeDataForInvalid(eventInfo);
        ret = CloudDiskServiceManager::GetInstance().UnregisterForSa(eventInfo.path + "/" + eventInfo.name);
        if (ret != 0) {
            return ret;
        } else {
            return LogFileMgr::GetInstance().UnRegisterSyncFolder(userId_, syncFolderIndex_);
        }
    }

    auto parentMetaFile = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(userId_, syncFolderIndex_,
                                                                                 parentStat.st_ino);

    struct LogGenerateCtx ctx;
    ret = ProductLogForOperate(parentMetaFile, eventInfo.path, eventInfo.name, ctx, eventInfo.operateType);
    if (ret != 0) {
        LOGE("create log failed");
        return ret;
    }

    uint64_t line = 0;
    GenerateLogBlock(eventInfo, parentStat.st_ino, ctx, parentMetaFile->selfRecordId_, line);
    if (needCallback_) {
        GenerateChangeData(eventInfo, line, ctx.recordId, parentMetaFile->selfRecordId_);
    }

    if (eventInfo.operateType == OperationType::CREATE) {
        ffrt::submit(
            [eventInfo, this] { FillChildForDir(eventInfo.path + "/" + eventInfo.name, eventInfo.timestamp); });
    }
    return E_OK;
}

int32_t CloudDiskServiceLogFile::PraseLog(const uint64_t line, ChangeData &data, bool &isEof)
    __attribute__((no_sanitize("unsigned-integer-overflow")))
{
    LOGD("prase line:%{public}llu, currentline:%{public}llu", line, currentLine_.load());
    if (line == currentLine_.load()) {
        isEof = true;
        return E_OK;
    } else if (currentLine_.load() - line >= LOG_COUNT_MAX) {
        return E_INVALID_CHANGE_SEQUENCE;
    }

    struct LogBlock logBlock;
    auto ret = ReadLogFile(line, logBlock);
    if (ret != 0) {
        return ret;
    }

    auto parentMetaFile = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(userId_, syncFolderIndex_,
                                                                                 logBlock.parentInode);
    std::string relativePath;
    ret = MetaFileMgr::GetInstance().GetRelativePath(parentMetaFile, relativePath);
    if (ret != 0) {
        LOGE("get relative path failed");
    }

    MetaBase mBase;
    ret = parentMetaFile->DoLookupByOffset(mBase, logBlock.bidx, logBlock.bitPos);
    if (ret != 0) {
        LOGE("lookup by offset failed");
        return ret;
    }

    data.updateSequenceNumber = logBlock.line;
    data.fileId = mBase.recordId;
    data.parentFileId = std::string(reinterpret_cast<const char *>(logBlock.parentRecordId), RECORD_ID_LEN);
    data.relativePath = relativePath + mBase.name;
    data.operationType = static_cast<OperationType>(logBlock.operationType);
    data.size = mBase.size;
    data.mtime = mBase.mtime;
    data.timeStamp = logBlock.timestamp;
    return E_OK;
}

int32_t CloudDiskServiceLogFile::ProductLogForOperate(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile,
                                                      const std::string &path, const std::string &name,
                                                      struct LogGenerateCtx &ctx, OperationType operationType)
{
    switch (operationType) {
        case OperationType::CREATE:
            return ProduceCreateLog(parentMetaFile, path, name, ctx);
        case OperationType::DELETE:
            return ProduceUnlinkLog(parentMetaFile, name, ctx);
        case OperationType::MOVE_FROM:
            return ProduceRenameOldLog(parentMetaFile, name, ctx);
        case OperationType::MOVE_TO:
            return ProduceRenameNewLog(parentMetaFile, path, name, ctx);
        case OperationType::CLOSE_WRITE:
            return ProduceCloseAndWriteLog(parentMetaFile, path, name, ctx);
        case OperationType::SYNC_FOLDER_INVALID:
        case OperationType::OPERATION_MAX:
            return EINVAL;
    }
}

int32_t CloudDiskServiceLogFile::ProduceCreateLog(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile,
                                                  const std::string &path, const std::string &name,
                                                  struct LogGenerateCtx &ctx)
{
    LOGD("Begin ProduceCreateLog");
    ctx.recordId = UuidHelper::GenerateUuidOnly();
    MetaBase mBase(name, ctx.recordId);

    struct stat childStat;
    if (stat((path + "/" + name).c_str(), &childStat) != 0) {
        LOGE("stat child failed for %{public}d", errno);
        return errno;
    }
    mBase.mode = childStat.st_mode;
    mBase.atime = childStat.st_atime;
    mBase.mtime = childStat.st_mtime;
    mBase.size = childStat.st_size;
    auto ret = parentMetaFile->DoCreate(mBase, ctx.bidx, ctx.bitPos);
    if (ret != 0) {
        LOGE("create failed");
        return -1;
    }

    if (!S_ISDIR(childStat.st_mode)) {
        return E_OK;
    }
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(userId_, syncFolderIndex_, childStat.st_ino);
    metaFile->parentDentryFile_ = parentMetaFile->selfInode_;
    metaFile->selfRecordId_ = ctx.recordId;
    metaFile->selfHash_ = CloudDisk::CloudFileUtils::DentryHash(name);
    metaFile->GenericDentryHeader();
    return E_OK;
}

int32_t CloudDiskServiceLogFile::ProduceUnlinkLog(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile,
                                                  const std::string &name, struct LogGenerateCtx &ctx)
{
    LOGD("Begin ProduceUnlinkLog");
    MetaBase mBase(name);
    auto ret = parentMetaFile->DoRemove(mBase, ctx.recordId, ctx.bidx, ctx.bitPos);
    if (ret != 0) {
        LOGE("remove failed");
        return -1;
    }
    return E_OK;
}

int32_t CloudDiskServiceLogFile::ProduceRenameOldLog(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile,
                                                     const std::string &name, struct LogGenerateCtx &ctx)
{
    LOGD("Begin ProduceRenameOldLog");
    MetaBase mBase(name);
    auto ret = parentMetaFile->DoRenameOld(mBase, ctx.recordId, ctx.bidx, ctx.bitPos);
    if (ret != 0) {
        LOGE("renameold failed");
        return -1;
    }
    renameRecordId_ = ctx.recordId;
    return E_OK;
}

int32_t CloudDiskServiceLogFile::ProduceRenameNewLog(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile,
                                                     const std::string &path, const std::string &name,
                                                     struct LogGenerateCtx &ctx)
{
    LOGD("Begin ProduceRenameNewLog");
    MetaBase mBase(name);
    struct stat childStat;
    if (stat((path + "/" + name).c_str(), &childStat) != 0) {
        LOGE("stat child failed for %{public}d", errno);
        return errno;
    }
    mBase.mode = childStat.st_mode;
    mBase.atime = childStat.st_atime;
    mBase.mtime = childStat.st_mtime;
    mBase.size = childStat.st_size;
    auto ret = parentMetaFile->DoRenameNew(mBase, renameRecordId_, ctx.bidx, ctx.bitPos);
    if (ret != 0) {
        LOGE("renamenew failed");
        return -1;
    }
    ctx.recordId = renameRecordId_;

    if (!S_ISDIR(childStat.st_mode)) {
        return E_OK;
    }
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(userId_, syncFolderIndex_, childStat.st_ino);
    metaFile->parentDentryFile_ = parentMetaFile->selfInode_;
    metaFile->selfRecordId_ = ctx.recordId;
    metaFile->selfHash_ = CloudDisk::CloudFileUtils::DentryHash(name);
    metaFile->GenericDentryHeader();
    return E_OK;
}

int32_t CloudDiskServiceLogFile::ProduceCloseAndWriteLog(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile,
                                                         const std::string &path, const std::string &name,
                                                         struct LogGenerateCtx &ctx)
{
    LOGD("Begin ProduceCloseAndWriteLog");
    MetaBase mBase(name);
    struct stat childStat;
    if (stat((path + "/" + name).c_str(), &childStat) != 0) {
        LOGE("stat child failed for %{public}d", errno);
        return errno;
    }
    mBase.mode = childStat.st_mode;
    mBase.atime = childStat.st_atime;
    mBase.mtime = childStat.st_mtime;
    mBase.size = childStat.st_size;
    auto ret = parentMetaFile->DoUpdate(mBase, ctx.recordId, ctx.bidx, ctx.bitPos);
    if (ret != 0) {
        LOGE("update failed");
        return -1;
    }
    return E_OK;
}

LogFileMgr& LogFileMgr::GetInstance()
{
    static LogFileMgr instance_;
    return instance_;
}

int32_t LogFileMgr::ProduceRequest(const struct EventInfo &eventInfo)
{
    auto logFile = GetCloudDiskServiceLogFile(eventInfo.userId, eventInfo.syncFolderIndex);
    logFile->ProduceLog(eventInfo);
    return E_OK;
}

int32_t LogFileMgr::PraseRequest(const int32_t userId, const uint32_t syncFolderIndex, const uint64_t start,
                                 const uint64_t count, struct ChangesResult &changesResult)
{
    int32_t ret;
    uint64_t nextUsn = start + count;
    auto logFile = GetCloudDiskServiceLogFile(userId, syncFolderIndex);
    for (uint64_t line = start; line < start + count; line++) {
        struct ChangeData changeData;
        ret = logFile->PraseLog(line, changeData, changesResult.isEof);
        if (ret == E_OK && !changesResult.isEof) {
            changesResult.changesData.push_back(changeData);
            continue;
        } else if (changesResult.isEof) {
            nextUsn = line;
            break;
        } else {
            continue;
        }
    }
    if (ret == E_OK) {
        changesResult.nextUsn = nextUsn;
    }
    return ret;
}

int32_t LogFileMgr::RegisterSyncFolder(const int32_t userId, const uint32_t syncFolderIndex, const std::string &path)
{
    auto logFile = GetCloudDiskServiceLogFile(userId, syncFolderIndex);
    logFile->SetSyncFolderPath(path);

    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        LOGE("stat failed for %{public}d", errno);
        return errno;
    }
    auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskServiceMetaFile(userId, syncFolderIndex, st.st_ino);
    metaFile->parentDentryFile_ = ROOT_PARENTDENTRYFILE;
    metaFile->selfRecordId_ = UuidHelper::GenerateUuidOnly();
    metaFile->selfHash_ = CloudDisk::CloudFileUtils::DentryHash(ROOT_PARENTDENTRYFILE);

    metaFile->GenericDentryHeader();

    ffrt::submit([logFile, path] { logFile->FillChildForDir(path, UTCTimeMilliSeconds()); });
    return E_OK;
}

int32_t LogFileMgr::UnRegisterSyncFolder(const int32_t userId, const uint32_t syncFolderIndex)
{
    std::lock_guard<std::mutex> lock(mtx_);
    LogFileKey key(userId, syncFolderIndex);
    LogFiles_.erase(key);
    MetaFileMgr::GetInstance().CloudDiskServiceClearAll();

    std::string rootDir =
        "/data/service/el2/" + std::to_string(userId) +
        "/hmdfs/cache/account_cache/dentry_cache/clouddisk_service_cache/" +
        Convertor::ConvertToHex(syncFolderIndex) + "/";
    if (!OHOS::Storage::DistributedFile::Utils::ForceRemoveDirectoryDeepFirst(rootDir)) {
        LOGW("remove photo dentry dir failed, errno: %{public}d", errno);
    }
    return E_OK;
}

void LogFileMgr::RegisterSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex)
{
    {
        std::lock_guard<std::mutex> lock(mtx_);
        if (registerChangeCount_ == 0) {
            handle_ = ffrt_timer_start(ffrt_qos_default, CHANGE_DATA_TIMEOUT_MS, nullptr, CallBack, true);
        }
        registerChangeCount_++;
    }

    auto logFile = GetCloudDiskServiceLogFile(userId, syncFolderIndex);
    logFile->StartCallback();
}

void LogFileMgr::UnRegisterSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex)
{
    auto logFile = GetCloudDiskServiceLogFile(userId, syncFolderIndex);
    logFile->StopCallback();

    {
        std::lock_guard<std::mutex> lock(mtx_);
        registerChangeCount_--;
        if (registerChangeCount_ == 0) {
            ffrt_timer_stop(ffrt_qos_default, handle_);
        }
    }
}

std::shared_ptr<CloudDiskServiceLogFile> LogFileMgr::GetCloudDiskServiceLogFile(const int32_t userId,
                                                                                const uint32_t syncFolderIndex)
{
    std::shared_ptr<CloudDiskServiceLogFile> lFile = nullptr;
    std::lock_guard<std::mutex> lock(mtx_);
    LogFileKey key(userId, syncFolderIndex);
    auto it = LogFiles_.find(key);
    if (it != LogFiles_.end()) {
        lFile = it->second;
    } else {
        lFile = std::make_shared<CloudDiskServiceLogFile>(userId, syncFolderIndex);
        LogFiles_[key] = lFile;
    }
    return lFile;
}

void LogFileMgr::CallBack(void *)
{
    LogFileMgr::GetInstance().OnDataChange();
}

int32_t LogFileMgr::OnDataChange()
{
    std::lock_guard<std::mutex> lock(mtx_);
    for (const auto &it : LogFiles_) {
        it.second->OnDataChange();
    }
    return E_OK;
}

} // namespace OHOS::FileManagement::CloudDiskService