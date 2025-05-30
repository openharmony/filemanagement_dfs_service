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
#include "concurrent_queue.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
ConcurrentQueue &ConcurrentQueue::GetInstance()
{
    static ConcurrentQueue instance_;
    return instance_;
}

void ConcurrentQueue::Init(int32_t qos, int32_t maxConcurrencyNum)
{
    queue_ = std::make_unique<ffrt::queue>(ffrt::queue_concurrent, "ConcurrencyQueue",
                                           ffrt::queue_attr().qos(qos).max_concurrency(maxConcurrencyNum));
}

void ConcurrentQueue::Deinit()
{
    queue_ = nullptr;
}

void ConcurrentQueue::Submit(std::function<void()> &&func, const ffrt::task_attr &attr)
{
    if (queue_ == nullptr) {
        LOGE("concurrent queue instance is null, submit task failed.");
        return;
    }
    queue_->submit_h(func, attr);
}

void ConcurrentQueue::Submit(const std::function<void()> &func, const ffrt::task_attr &attr)
{
    if (queue_ == nullptr) {
        LOGE("concurrent queue instance is null, submit task failed.");
        return;
    }
    queue_->submit_h(func, attr);
}
} // namespace FileManagement
} // namespace OHOS