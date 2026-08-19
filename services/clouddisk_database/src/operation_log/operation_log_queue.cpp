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

#include "operation_log_queue.h"

#include <chrono>
#include <cstring>

#include "operation_log_const.h"
#include "operation_log_entry.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

using namespace std;

void OperationLogQueue::Push(const OperationLogEntry& entry)
{
    unique_lock<mutex> lock(mutex_);
    if (stopping_) {
        return;
    }
    if (queue_.size() >= MAX_QUEUE_SIZE) {
        LOGE("queue is full, reject push");
        return;
    }
    queue_.push(entry);
    cv_.notify_all();
}

void OperationLogQueue::Shutdown()
{
    unique_lock<mutex> lock(mutex_);
    stopping_ = true;
    cv_.notify_all();
}

void OperationLogQueue::Reset()
{
    lock_guard<mutex> lock(mutex_);
    stopping_ = false;
    while (!queue_.empty()) {
        queue_.pop();
    }
}

bool OperationLogQueue::IsEmpty() const
{
    lock_guard<mutex> lock(mutex_);
    return queue_.empty();
}

bool OperationLogQueue::IsFull() const
{
    lock_guard<mutex> lock(mutex_);
    return queue_.size() >= MAX_QUEUE_SIZE;
}

bool OperationLogQueue::IsStopping() const
{
    lock_guard<mutex> lock(mutex_);
    return stopping_;
}

vector<OperationLogEntry> OperationLogQueue::PopBatch()
{
    unique_lock<mutex> lock(mutex_);
    if (queue_.empty() && !stopping_) {
        cv_.wait_for(lock, chrono::milliseconds(WAIT_TIMEOUT_MS), [this] {
            return !queue_.empty() || stopping_;
        });
    }
    vector<OperationLogEntry> batch;
    batch.reserve(BATCH_SIZE);
    while (!queue_.empty() && batch.size() < BATCH_SIZE) {
        batch.push_back(queue_.front());
        queue_.pop();
    }

    return batch;
}

} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
