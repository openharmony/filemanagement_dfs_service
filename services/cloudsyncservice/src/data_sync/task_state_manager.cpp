/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "task_state_manager.h"

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

const string TASK_ID = "unload";
const int32_t DELAY_TIME = 180000;

TaskStateManager &TaskStateManager::GetInstance()
{
    static TaskStateManager instance;
    return instance;
}

TaskStateManager::TaskStateManager()
{
    auto runner = AppExecFwk::EventRunner::Create(TASK_ID);
    if (unloadHandler_ == nullptr) {
        unloadHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    if (unloadHandler_ == nullptr) {
        LOGE("init unload handler failed");
    }
}

void TaskStateManager::StartTask(string bundleName, TaskType task)
{
    unloadHandler_->RemoveTask(TASK_ID);
    std::lock_guard<std::mutex> lock(taskMapsMutex_);
    auto iterator = taskMaps_.find(bundleName);
    if (iterator == taskMaps_.end()) {
        taskMaps_[bundleName] = static_cast<uint64_t>(task);
        return;
    }
    auto taskState = iterator->second | static_cast<uint64_t>(task);
    taskMaps_[bundleName] = taskState;
}

void TaskStateManager::CompleteTask(string bundleName, TaskType task)
{
    std::lock_guard<std::mutex> lock(taskMapsMutex_);
    auto iterator = taskMaps_.find(bundleName);
    if (iterator == taskMaps_.end()) {
        LOGE("task is not started");
    } else {
        taskMaps_[bundleName] = iterator->second & ~static_cast<uint64_t>(task);
        if (taskMaps_[bundleName] == 0) {
            taskMaps_.erase(bundleName);
        }
    }
    if (taskMaps_.empty()) {
        DelayUnloadTask();
    }
}

void TaskStateManager::StartTask()
{
    std::lock_guard<std::mutex> lock(taskMapsMutex_);
    if (taskMaps_.empty()) {
        DelayUnloadTask();
    }
}

bool TaskStateManager::HasTask(const string bundleName, TaskType task)
{
    std::lock_guard<std::mutex> lock(taskMapsMutex_);
    auto iterator = taskMaps_.find(bundleName);
    if (iterator != taskMaps_.end()) {
        if (taskMaps_[bundleName] & static_cast<uint64_t>(task)) {
            return true;
        }
    }
    return false;
}

void TaskStateManager::DelayUnloadTask()
{
    LOGI("delay unload task begin");
    auto task = [this]() {
        LOGI("do unload task");
        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            LOGE("get samgr failed");
            return;
        }
        int32_t ret = samgrProxy->UnloadSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID);
        if (ret != ERR_OK) {
            LOGE("remove system ability failed");
            return;
        }
    };
    unloadHandler_->RemoveTask(TASK_ID);
    unloadHandler_->PostTask(task, TASK_ID, DELAY_TIME);
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS