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

#include "gallery_download_file_stat.h"
#include "iservice_registry.h"
#include "parameters.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

const int32_t DELAY_TIME = 90000; // ms
const int32_t SYSTEM_LOAD_DELAY_TIME = 600000; // ms

TaskStateManager &TaskStateManager::GetInstance()
{
    static TaskStateManager instance;
    return instance;
}

TaskStateManager::TaskStateManager() : queue_("unloadTask")
{
}

void TaskStateManager::StartTask(string bundleName, TaskType task)
{
    CancelUnloadTask();
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


void TaskStateManager::CancelUnloadTask()
{
    std::lock_guard<ffrt::mutex> lock(unloadTaskMutex_);
    if (unloadTaskHandle_ == nullptr) {
        return;
    }
    LOGD("cancel unload task");
    queue_.cancel(unloadTaskHandle_);
    unloadTaskHandle_ = nullptr;
}

void TaskStateManager::DelayUnloadTask()
{
    const std::string temperatureSysparamSync = "persist.kernel.cloudsync.temperature_abnormal_sync";
    const std::string temperatureSysparamThumb = "persist.kernel.cloudsync.temperature_abnormal_thumb";
    string systemLoadSync = system::GetParameter(temperatureSysparamSync, "");
    string systemLoadThumb = system::GetParameter(temperatureSysparamThumb, "");
    LOGI("delay unload task begin");
    auto delayTime = DELAY_TIME;
    if (systemLoadSync == "true" || systemLoadThumb == "true") {
        LOGE("temperatureSysparam is true, unload task in 10 minutes");
        delayTime = SYSTEM_LOAD_DELAY_TIME;
    }
    auto task = [this]() {
        LOGI("do unload task");
        {
            std::lock_guard<ffrt::mutex> lock(unloadTaskMutex_);
            unloadTaskHandle_ = nullptr;
        }

        /* for big data statistics */
        CloudFile::GalleryDownloadFileStat::GetInstance().OutputToFile();

        auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (samgrProxy == nullptr) {
            LOGE("get samgr failed");
            return;
        }
        system::SetParameter(CLOUD_FILE_SERVICE_SA_STATUS_FLAG, CLOUD_FILE_SERVICE_SA_END);
        int32_t ret = samgrProxy->UnloadSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID);
        if (ret != ERR_OK) {
            LOGE("remove system ability failed");
            return;
        }
    };

    CancelUnloadTask();
    std::lock_guard<ffrt::mutex> lock(unloadTaskMutex_);
    std::chrono::milliseconds ms(delayTime);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(ms);
    unloadTaskHandle_ = queue_.submit_h(task, ffrt::task_attr().delay(us.count()));
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS