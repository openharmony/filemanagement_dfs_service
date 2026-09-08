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
PlaceholderCallbackManager &PlaceholderCallbackManager::GetInstance()
{
    static PlaceholderCallbackManager instance;
    return instance;
}

void PlaceholderCallbackManager::ClearAll() {}

PlaceholderTaskManager &PlaceholderTaskManager::GetInstance()
{
    static PlaceholderTaskManager instance;
    return instance;
}

void PlaceholderTaskManager::CancelAllTasks(PlaceholderTaskCancelReason reason)
{
    (void)reason;
}

void PlaceholderTaskManager::ClearTombstones() {}

PlaceholderProgressManager &PlaceholderProgressManager::GetInstance()
{
    static PlaceholderProgressManager instance;
    return instance;
}

void PlaceholderProgressManager::Drain() {}

void PlaceholderProgressManager::Clear() {}
} // namespace OHOS::FileManagement::CloudDiskService
