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

#ifndef CLOUD_DISK_SERVICE_LOGFILE_H
#define CLOUD_DISK_SERVICE_LOGFILE_H

#include <atomic>
#include <fcntl.h>
#include <map>
#include <queue>
#include <sys/stat.h>
#include <vector>

#include "cloud_disk_common.h"
#include "cloud_disk_service_metafile.h"
#include "disk_types.h"
#include "ffrt_inner.h"
#include "unique_fd.h"

namespace OHOS::FileManagement::CloudDiskService {

constexpr uint32_t LOGBLOCK_RESERVED_LENGTH = 6;
constexpr uint32_t LOGGROUP_RESERVED_LENGTH = 62;
constexpr uint32_t LOGBLOCK_PER_GROUP = 64;
constexpr uint32_t LOGGROUP_SHIFT = 0X6;
constexpr uint32_t LOGGROUP_SIZE = 4096;
constexpr uint32_t LOGGROUP_SIZE_SHIFT = 0XC;
constexpr uint32_t LOGGROUP_MAX = 1024;
constexpr uint32_t LOGGROUP_INDEX_MASK = 0XFFFF;
constexpr uint32_t LOGGROUP_HEADER = 4096;
constexpr uint32_t LOG_INDEX_MASK = 0X3F;
constexpr uint32_t LOG_COUNT_MAX = 65536;

struct LogGenerateCtx {
    std::string recordId{};
    unsigned long bidx{0};
    uint32_t bitPos{0};
};

#pragma pack(push, 1)
struct LogBlock {
    uint64_t line;
    uint64_t timestamp;
    uint64_t parentInode;
    uint32_t hash;
    uint8_t operationType;
    uint64_t bidx{0};
    uint32_t bitPos{0};
    uint8_t parentRecordId[RECORD_ID_LEN];
    uint8_t reserved[LOGBLOCK_RESERVED_LENGTH];
};

struct LogGroup {
    uint8_t logVersion;
    uint8_t logBlockCnt;
    struct LogBlock nsl[LOGBLOCK_PER_GROUP];
    uint8_t reserved[LOGGROUP_RESERVED_LENGTH];
};
static_assert(sizeof(LogGroup) == LOGGROUP_SIZE);
#pragma pack(pop)

class CloudDiskServiceLogFile {
public:
    CloudDiskServiceLogFile() = delete;
    ~CloudDiskServiceLogFile() = default;
    explicit CloudDiskServiceLogFile(const int32_t userId, const uint32_t syncFolderIndex);
    void SetSyncFolderPath(const std::string &path);

    int32_t ProduceLog(const struct EventInfo &eventInfo);
    int32_t PraseLog(const uint64_t line, ChangeData &data, bool &isEof);
    int32_t FillChildForDir(const std::string &path, const uint64_t timestamp);
    void StartCallback();
    void StopCallback();

    int32_t OnDataChange();
    uint64_t GetStartLine();

private:
    int32_t ProductLogForOperate(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile,
                                 const std::string &path, const std::string &name, struct LogGenerateCtx &ctx,
                                 OperationType operationType);
    int32_t ProduceCreateLog(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile, const std::string &path,
                             const std::string &name, struct LogGenerateCtx &ctx);
    int32_t ProduceUnlinkLog(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile, const std::string &name,
                             struct LogGenerateCtx &ctx);
    int32_t ProduceRenameOldLog(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile, const std::string &name,
                                struct LogGenerateCtx &ctx);
    int32_t ProduceRenameNewLog(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile, const std::string &path,
                                const std::string &name, struct LogGenerateCtx &ctx);
    int32_t ProduceCloseAndWriteLog(const std::shared_ptr<CloudDiskServiceMetaFile> parentMetaFile,
                                    const std::string &path, const std::string &name, struct LogGenerateCtx &ctx);

    int32_t WriteLogFile(const struct LogBlock &logBlock);
    int32_t ReadLogFile(const uint64_t line, struct LogBlock &logBlock);
    int32_t GenerateLogBlock(const struct EventInfo &eventInfo, const uint64_t parentInode,
                             const struct LogGenerateCtx &ctx, const std::string &parentRecordId, uint64_t &line);
    int32_t GenerateChangeData(const struct EventInfo &eventInfo, uint64_t line,
                               const std::string &childRecordId, const std::string &parentRecordId);
    void GenerateChangeDataForInvalid(const struct EventInfo &eventInfo);
    bool CheckLineIsValid(const uint64_t line);

    int32_t userId_;
    uint32_t syncFolderIndex_;
    bool needCallback_;
    std::string syncFolderPath_;
    std::string logFilePath_;
    std::string renameRecordId_;
    UniqueFd fd_;

    std::atomic<uint64_t> currentLine_;
    std::atomic<bool> reversal_;

    std::mutex fileMtx_;
    std::mutex vectorMtx_;
    std::vector<struct ChangeData> changeDatas_;
};

typedef std::pair<uint32_t, uint32_t> LogFileKey;

class LogFileMgr {
public:
    static LogFileMgr& GetInstance();
    std::shared_ptr<CloudDiskServiceLogFile> GetCloudDiskServiceLogFile(const int32_t userId,
                                                                        const uint32_t syncFolderIndex);

    int32_t ProduceRequest(const struct EventInfo &eventInfo);
    int32_t PraseRequest(const int32_t userId, const uint32_t syncFolderIndex, const uint64_t start,
                         const uint64_t count, struct ChangesResult &changesResult);

    int32_t RegisterSyncFolder(const int32_t userId, const uint32_t syncFolderIndex, const std::string &path);
    int32_t UnRegisterSyncFolder(const int32_t userId, const uint32_t syncFolderIndex);
    void RegisterSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex);
    void UnRegisterSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex);
    void CloudDiskServiceClearAll();

private:
    LogFileMgr() = default;
    ~LogFileMgr() = default;
    LogFileMgr(const LogFileMgr &m) = delete;
    const LogFileMgr &operator=(const LogFileMgr &m) = delete;

    static void CallBack(void *data);
    int32_t OnDataChange();

    bool running_;
    std::mutex mtx_{};
    std::map<LogFileKey, std::shared_ptr<CloudDiskServiceLogFile>> LogFiles_;
    int32_t registerChangeCount_{0};
    ffrt_timer_t handle_{-1};
};

} // namespace OHOS::FileManagement::CloudDiskService

#endif // CLOUD_DISK_SERVICE_LOGFILE_H