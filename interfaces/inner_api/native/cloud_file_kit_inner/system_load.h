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
#include "sync_state_manager.h"
#include <mutex>
#ifdef support_thermal_manager
#include "thermal_mgr_listener.h"
#else
namespace OHOS::PowerMgr {
enum class ThermalLevel : int32_t {
        COOL,
        NORMAL,
        WARM,
        HOT,
        OVERHEATED,
        WARNING,
        EMERGENCY,
        ESCAPE,
};
}
#endif

namespace OHOS::FileManagement::CloudSync {
enum STOPPED_TYPE {
    STOPPED_IN_SYNC = 0,
    STOPPED_IN_THUMB = 1,
    STOPPED_IN_OTHER = 2,
};
#ifdef support_thermal_manager
const std::string TEMPERATURE_SYSPARAM_SYNC = "persist.kernel.cloudsync.temperature_abnormal_sync";
const std::string TEMPERATURE_SYSPARAM_THUMB = "persist.kernel.cloudsync.temperature_abnormal_thumb";

class SystemLoadEvent : public PowerMgr::ThermalMgrListener::ThermalLevelEvent {
public:
    SystemLoadEvent() = default;
    void OnThermalLevelResult(const PowerMgr::ThermalLevel &level) override;
    void SetDataSyncer(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
private:
    std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager_ = nullptr;
};
#endif

class SystemLoadStatus {
public:
    SystemLoadStatus() = default;
    virtual ~SystemLoadStatus() = default;
    static bool IsSystemLoadAllowed(STOPPED_TYPE process = STOPPED_IN_OTHER,
        const PowerMgr::ThermalLevel &level = PowerMgr::ThermalLevel::HOT);
    static void Setload(const PowerMgr::ThermalLevel &level);
    static PowerMgr::ThermalLevel Getload();
#ifdef support_thermal_manager
    static void RegisterSystemloadCallback(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
    static void GetSystemloadLevel();
    static void InitSystemload(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager);
private:
    static std::atomic<PowerMgr::ThermalLevel> levelStatus_;
    static std::shared_ptr<PowerMgr::ThermalMgrListener> thermalListener_;
    static std::shared_ptr<SystemLoadEvent> thermalEvent_;
#endif
};
} // OHOS
#endif // OHOS_FILEMGMT_CLOUD_SYNC_SYSTEM_LOAD_H