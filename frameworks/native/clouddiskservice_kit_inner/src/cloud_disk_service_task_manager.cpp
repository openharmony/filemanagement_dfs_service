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

#include "cloud_disk_service_task_manager.h"

#include "iservice_registry.h"
#include "mem_mgr_client.h"
#include "parameters.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {
using namespace std;

CloudDiskServiceTaskManager &CloudDiskServiceTaskManager::GetInstance()
{
    static CloudDiskServiceTaskManager instance;
    return instance;
}

CloudDiskServiceTaskManager::CloudDiskServiceTaskManager() : queue_("unloadTask") {}

void CloudDiskServiceTaskManager::StartTask(TaskKey key, TaskType task)
{
    CancelUnloadTask();
    std::lock_guard<std::mutex> lock(taskMapsMutex_);
    if (criticalStatus_ == false) {
        int32_t ret =
            Memory::MemMgrClient::GetInstance().SetCritical(getpid(), true, FILEMANAGEMENT_CLOUD_DISK_SERVICE_SA_ID);
        if (ret == ERR_OK) {
            criticalStatus_ = true;
        }
    }
    auto iterator = taskMaps_.find(key);
    if (iterator == taskMaps_.end()) {
        taskMaps_[key] = static_cast<uint64_t>(task);
        return;
    }
    auto taskState = iterator->second | static_cast<uint64_t>(task);
    taskMaps_[key] = taskState;
}

void CloudDiskServiceTaskManager::CompleteTask(TaskKey key, TaskType task)
{
    std::lock_guard<std::mutex> lock(taskMapsMutex_);
    auto iterator = taskMaps_.find(key);
    if (iterator == taskMaps_.end()) {
        LOGE("task is not started");
    } else {
        taskMaps_[key] = iterator->second & ~static_cast<uint64_t>(task);
        if (taskMaps_[key] == 0) {
            LOGI("start erase");
            taskMaps_.erase(key);
        }
    }
}

void CloudDiskServiceTaskManager::StartTask()
{
    std::lock_guard<std::mutex> lock(taskMapsMutex_);
}

bool CloudDiskServiceTaskManager::HasTask(TaskKey key, TaskType task)
{
    std::lock_guard<std::mutex> lock(taskMapsMutex_);
    auto iterator = taskMaps_.find(key);
    if (iterator != taskMaps_.end()) {
        if (taskMaps_[key] & static_cast<uint64_t>(task)) {
            return true;
        }
    }
    return false;
}

void CloudDiskServiceTaskManager::CancelUnloadTask()
{
    std::lock_guard<ffrt::mutex> lock(unloadTaskMutex_);
    if (unloadTaskHandle_ == nullptr) {
        return;
    }
    LOGD("cancel unload task");
    queue_.cancel(unloadTaskHandle_);
    unloadTaskHandle_ = nullptr;
}
} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS