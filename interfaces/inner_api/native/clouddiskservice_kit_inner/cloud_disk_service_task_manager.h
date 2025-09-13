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

#ifndef OHOS_FILEMGMT_CLOUD_DISK_SERVICE_TASK_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_DISK_SERVICE_TASK_MANAGER_H

#include "ffrt_inner.h"
#include <functional>
#include <nocopyable.h>

namespace OHOS::FileManagement::CloudDiskService {
enum class TaskType : uint64_t {
    REGISTER_TASK = 1,
    UNREGISTER_TASK = 1 << 1,
    GET_TASK = 1 << 2,
};

struct TaskKey {
    std::string bundleName;
    std::string syncChronousRootPath;

    bool operator==(const TaskKey &other) const
    {
        return bundleName == other.bundleName && syncChronousRootPath == other.syncChronousRootPath;
    }

    struct Hash {
        size_t operator()(const TaskKey &key) const
        {
            auto hasher = std::hash<std::string>();
            return hasher(key.bundleName) ^ (hasher(key.syncChronousRootPath) << 1);
        }
    };
};

class CloudDiskServiceTaskManager : public NoCopyable {
public:
    static CloudDiskServiceTaskManager &GetInstance();
    ~CloudDiskServiceTaskManager() = default;
    void StartTask(TaskKey key, TaskType task);
    void CompleteTask(TaskKey key, TaskType task);
    bool HasTask(TaskKey key, TaskType task);
    void StartTask();

private:
    CloudDiskServiceTaskManager();
    void DelayUnloadTask(bool needSetCritical);
    void CancelUnloadTask();

    bool criticalStatus_ = true;
    std::mutex taskMapsMutex_;
    std::unordered_map<TaskKey, uint64_t, TaskKey::Hash> taskMaps_;
    ffrt::queue queue_;
    ffrt::task_handle unloadTaskHandle_;
    ffrt::mutex unloadTaskMutex_;
};
} // namespace OHOS::FileManagement::CloudDiskService
#endif // OHOS_FILEMGMT_CLOUD_DISK_SERVICE_TASK_MANAGER_H