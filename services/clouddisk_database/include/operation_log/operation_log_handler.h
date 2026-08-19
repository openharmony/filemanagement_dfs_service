/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_HANDLER_H
#define OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_HANDLER_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "operation_log_entry.h"
#include "operation_log_queue.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

class OperationLogHandler {
public:
    static OperationLogHandler &GetInstance();
    int32_t Init(int32_t userId);
    void Start();
    void Stop();
    int32_t RecordDelete(int64_t opTime, const std::string& filePath,
        int64_t fileInode, int64_t fileUid,
        const std::string& processName, int64_t processPid, int64_t processUid);
    int32_t CleanOldRecords();

private:
    OperationLogHandler() = default;
    ~OperationLogHandler();
    OperationLogHandler(const OperationLogHandler &) = delete;
    OperationLogHandler &operator=(const OperationLogHandler &) = delete;

    void WriteThreadLoop();
    int32_t WriteBatch(const std::vector<OperationLogEntry> &batch);
    int32_t DoInsert(const std::vector<OperationLogEntry> &batch);
    bool IsRecoverableError(int32_t errCode);
    int32_t CheckAndCleanRecords();
    int32_t GetRecordCount(int32_t &count);

    OperationLogQueue queue_;
    std::thread writeThread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> isInited_{false};
    std::mutex lifecycleMutex_;
    int64_t lastCleanTime_{0};
    int32_t userId_{0};
};

} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_HANDLER_H
