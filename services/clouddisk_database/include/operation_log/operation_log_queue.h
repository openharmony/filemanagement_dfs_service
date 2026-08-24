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

#ifndef OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_QUEUE_H
#define OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_QUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>

#include "operation_log_entry.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

class OperationLogQueue {
public:
    void Push(const OperationLogEntry& entry);
    std::vector<OperationLogEntry> PopBatch();
    void Shutdown();
    void Reset();
    bool IsEmpty() const;
    bool IsFull() const;
    bool IsStopping() const;

    static constexpr uint32_t BATCH_SIZE = 200;
    //队列等待刷新时间
    static constexpr uint64_t WAIT_TIMEOUT_MS = 1000;
    static constexpr uint32_t MAX_QUEUE_SIZE = 10000;

private:
    std::queue<OperationLogEntry> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    bool stopping_ = false;
};

} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_QUEUE_H
