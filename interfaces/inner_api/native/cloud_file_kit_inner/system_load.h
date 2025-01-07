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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_SYSTEM_LOAD_H
#define OHOS_FILEMGMT_CLOUD_SYNC_SYSTEM_LOAD_H

#include "data_sync_manager.h"
#include <functional>
#include "res_sched_systemload_notifier_client.h"
#include "res_type.h"
#include "sync_state_manager.h"

namespace OHOS::FileManagement::CloudSync {
constexpr int32_t SYSTEMLOADLEVEL_HOT = ResourceSchedule::ResType::SystemloadLevel::HIGH;
constexpr int32_t SYSTEMLOADLEVEL_WARM = ResourceSchedule::ResType::SystemloadLevel::MEDIUM;
constexpr int32_t SYSTEMLOADLEVEL_NORMAL = ResourceSchedule::ResType::SystemloadLevel::NORMAL;
const std::string TEMPERATURE_SYSPARAM_SYNC = "persist.kernel.cloudsync.temperature_abnormal_sync";
const std::string TEMPERATURE_SYSPARAM_THUMB = "persist.kernel.cloudsync.temperature_abnormal_thumb";

enum STOPPED_TYPE {
    STOPPED_IN_SYNC = 0,
    STOPPED_IN_THUMB = 1,
    STOPPED_IN_OTHER = 2,
};

class SystemLoadStatus {
public:
    SystemLoadStatus() = default;
    virtual ~SystemLoadStatus() = default;
    static void RegisterSystemloadCallback(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
    static void GetSystemloadLevel();
    static bool IsLoadStatusUnderHot(STOPPED_TYPE process = STOPPED_IN_OTHER, int32_t level = SYSTEMLOADLEVEL_WARM);
    static bool IsLoadStatusUnderNormal(STOPPED_TYPE process);
    static void InitSystemload(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
    static void Setload(int32_t load);
    static inline int32_t loadstatus_ = 0;
};

class SystemLoadListener : public ResourceSchedule::ResSchedSystemloadNotifierClient {
public:
    SystemLoadListener() = default;
    void OnSystemloadLevel(int32_t level) override;
    void SetDataSycner(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
private:
    std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager_ = nullptr;
};
} // OHOS

#endif // OHOS_FILEMGMT_CLOUD_SYNC_SYSTEM_LOAD_H