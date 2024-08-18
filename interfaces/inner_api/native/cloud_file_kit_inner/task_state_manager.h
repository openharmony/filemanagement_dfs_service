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

#ifndef OHOS_FILEMGMT_TASK_STATE_MANAGER_H
#define OHOS_FILEMGMT_TASK_STATE_MANAGER_H

#include <nocopyable.h>
#include "ffrt_inner.h"

namespace OHOS::FileManagement::CloudSync {
static const std::string CLOUD_FILE_SERVICE_SA_STATUS_FLAG = "persist.kernel.medialibrarydata.stopflag";
static const std::string CLOUD_FILE_SERVICE_SA_START = "0";
static const std::string CLOUD_FILE_SERVICE_SA_END = "1";
enum class TaskType : uint64_t {
    SYNC_TASK = 1,
    DOWNLOAD_TASK = 1 << 1,
    CLEAN_TASK = 1 << 2,
    UPLOAD_ASSET_TASK = 1 << 3,
    DOWNLOAD_ASSET_TASK = 1 << 4,
    DOWNLOAD_REMOTE_ASSET_TASK = 1 << 5,
    DOWNLOAD_THUMB_TASK = 1 << 6,
    DISABLE_CLOUD_TASK = 1 << 7,
    CACHE_VIDEO_TASK = 1 << 8,
};
class TaskStateManager : public NoCopyable {
public:
    static TaskStateManager &GetInstance();
    ~TaskStateManager() = default;
    void StartTask(std::string bundleName, TaskType task);
    void CompleteTask(std::string bundleName, TaskType task);
    bool HasTask(std::string bundleName, TaskType task);
    void StartTask();
private:
    TaskStateManager();
    void DelayUnloadTask();
    void CancelUnloadTask();

    std::mutex taskMapsMutex_;
    std::unordered_map<std::string, uint64_t> taskMaps_;
    ffrt::queue queue_;
    ffrt::task_handle unloadTaskHandle_;
    ffrt::mutex unloadTaskMutex_;
};
}
#endif // OHOS_FILEMGMT_TASK_STATE_MANAGER_H