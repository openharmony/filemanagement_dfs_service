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

#include "placeholder_callback_manager.h"
#include "placeholder_progress_manager.h"
#include "placeholder_task_manager.h"

namespace OHOS::FileManagement::CloudDiskService {
namespace {
uint32_t callbackClearCount = 0;
uint32_t taskCancelCount = 0;
uint32_t cancellationRecordClearCount = 0;
uint32_t progressDrainCount = 0;
uint32_t progressClearCount = 0;
PlaceholderTaskCancelReason lastCancelReason = PlaceholderTaskCancelReason::USER_SWITCH;
} // namespace

namespace Test {
void ResetPlaceholderLifecycleCallCounts()
{
    callbackClearCount = 0;
    taskCancelCount = 0;
    cancellationRecordClearCount = 0;
    progressDrainCount = 0;
    progressClearCount = 0;
    lastCancelReason = PlaceholderTaskCancelReason::USER_SWITCH;
}

uint32_t GetPlaceholderCallbackClearCount()
{
    return callbackClearCount;
}

uint32_t GetPlaceholderTaskCancelCount()
{
    return taskCancelCount;
}

uint32_t GetPlaceholderCancellationRecordClearCount()
{
    return cancellationRecordClearCount;
}

uint32_t GetPlaceholderProgressDrainCount()
{
    return progressDrainCount;
}

uint32_t GetPlaceholderProgressClearCount()
{
    return progressClearCount;
}

PlaceholderTaskCancelReason GetLastPlaceholderTaskCancelReason()
{
    return lastCancelReason;
}
} // namespace Test

PlaceholderCallbackManager &PlaceholderCallbackManager::GetInstance()
{
    static PlaceholderCallbackManager instance;
    return instance;
}

void PlaceholderCallbackManager::ClearAll()
{
    ++callbackClearCount;
}

PlaceholderTaskManager &PlaceholderTaskManager::GetInstance()
{
    static PlaceholderTaskManager instance;
    return instance;
}

void PlaceholderTaskManager::CancelAllTasks(PlaceholderTaskCancelReason reason)
{
    ++taskCancelCount;
    lastCancelReason = reason;
}

void PlaceholderTaskManager::ClearCancellationRecords()
{
    ++cancellationRecordClearCount;
}

PlaceholderProgressManager &PlaceholderProgressManager::GetInstance()
{
    static PlaceholderProgressManager instance;
    return instance;
}

void PlaceholderProgressManager::Drain()
{
    ++progressDrainCount;
}

void PlaceholderProgressManager::Clear()
{
    ++progressClearCount;
}
} // namespace OHOS::FileManagement::CloudDiskService
