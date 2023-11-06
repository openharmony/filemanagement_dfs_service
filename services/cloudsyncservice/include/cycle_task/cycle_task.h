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

#ifndef OHOS_CLOUD_SYNC_SERVICE_CYCLE_TASK_H
#define OHOS_CLOUD_SYNC_SERVICE_CYCLE_TASK_H
#include <string>
namespace OHOS {
namespace FileManagement {
namespace CloudSync {
class CycleTask {
public:
    CycleTask(std::string taskName, std::string bundleName, int32_t intervalTime);
    virtual ~CycleTask() = default;
    virtual int32_t RunTask(int32_t userId) = 0;
    std::string GetTaskName() const;
    std::string GetBundleName() const;
    int32_t GetIntervalTime() const;
private:
    std::string taskName_;
    std::string bundleName_;
    int32_t intervalTime_;
};

} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_SERVICE_CYCLE_TASK_H