/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "system_load.h"

#include "battery_status.h"
#include "dfs_error.h"
#include "ffrt_inner.h"
#include "parameters.h"
#include "task_state_manager.h"
#include "utils_log.h"
#ifdef support_thermal_manager
#include "thermal_mgr_client.h"
#endif

namespace OHOS::FileManagement::CloudSync {
#ifdef support_thermal_manager
std::shared_ptr<PowerMgr::ThermalMgrListener> SystemLoadStatus::thermalListener_ =
    std::make_shared<PowerMgr::ThermalMgrListener>();
std::shared_ptr<SystemLoadEvent> SystemLoadStatus::thermalEvent_ =  std::make_shared<SystemLoadEvent>();
std::atomic<PowerMgr::ThermalLevel> SystemLoadStatus::levelStatus_ = PowerMgr::ThermalLevel::NORMAL;
void SystemLoadEvent::SetDataSyncer(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    dataSyncManager_ = dataSyncManager;
}

void SystemLoadStatus::RegisterSystemloadCallback(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    if (thermalEvent_ == nullptr) {
        LOGE("thermalEvent_ is nullptr");
        return;
    }
    thermalEvent_->SetDataSyncer(dataSyncManager);
    int32_t ret = thermalListener_->SubscribeLevelEvent(thermalEvent_);
    if (ret != E_OK) {
        LOGE("SubscribeLevelEvent failed, ret:%{public}d", ret);
        return;
    }
}

void SystemLoadEvent::OnThermalLevelResult(const PowerMgr::ThermalLevel &level)
{
    LOGD("thermal level change, old is:%{public}d, new is %{public}d",
        static_cast<int32_t>(SystemLoadStatus::Getload()), static_cast<int32_t>(level));
    SystemLoadStatus::Setload(level);
    if (level >= PowerMgr::ThermalLevel::HOT) {
        LOGI("thermal over warm");
        if (dataSyncManager_ == nullptr) {
            LOGE("dataSyncManager_ is nullptr");
            return;
        }
        dataSyncManager_->StopDownloadAndUploadTask();
        return;
    }
    ffrt::submit([level, this]() {
        if (dataSyncManager_ == nullptr) {
            LOGE("dataSyncManager_ is nullptr");
            return;
        }

        std::string systemLoadSync = system::GetParameter(TEMPERATURE_SYSPARAM_SYNC, "");
        std::string systemLoadThumb = system::GetParameter(TEMPERATURE_SYSPARAM_THUMB, "");
        LOGI("thermal under warm, level:%{public}d", level);
        if (systemLoadSync == "true") {
            LOGI("SetParameter TEMPERATURE_SYSPARAM_SYNC false");
            system::SetParameter(TEMPERATURE_SYSPARAM_SYNC, "false");
            TaskStateManager::GetInstance().StartTask();
            dataSyncManager_->TriggerRecoverySync(SyncTriggerType::SYSTEM_LOAD_TRIGGER);
        }
        if (systemLoadThumb == "true") {
            if (BatteryStatus::IsCharging() && level > PowerMgr::ThermalLevel::WARM) {
                return;
            } else if (!BatteryStatus::IsCharging() && level > PowerMgr::ThermalLevel::NORMAL) {
                return;
            }
            LOGI("SetParameter TEMPERATURE_SYSPARAM_THUMB false");
            system::SetParameter(TEMPERATURE_SYSPARAM_THUMB, "false");
            TaskStateManager::GetInstance().StartTask();
            dataSyncManager_->TriggerDownloadThumb();
        }
    });
}

void SystemLoadStatus::GetSystemloadLevel()
{
    Setload(PowerMgr::ThermalMgrClient::GetInstance().GetThermalLevel());
    LOGI("GetThermalLevel finish, load level is:%{public}d", static_cast<int32_t>(Getload()));
}

void SystemLoadStatus::Setload(const PowerMgr::ThermalLevel &level)
{
    levelStatus_.store(level);
}

PowerMgr::ThermalLevel SystemLoadStatus::Getload()
{
    return levelStatus_.load();
}

void SystemLoadStatus::InitSystemload(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
{
    GetSystemloadLevel();
    RegisterSystemloadCallback(dataSyncManager);
}
#endif

bool SystemLoadStatus::IsSystemLoadAllowed(STOPPED_TYPE process, const PowerMgr::ThermalLevel &level)
{
#ifdef support_thermal_manager
    if (Getload() > level) {
        if (process == STOPPED_IN_THUMB) {
            LOGI("SetParameter TEMPERATURE_SYSPARAM_THUMB true");
            system::SetParameter(TEMPERATURE_SYSPARAM_THUMB, "true");
        } else if (process == STOPPED_IN_SYNC) {
            LOGI("SetParameter TEMPERATURE_SYSPARAM_SYNC true");
            system::SetParameter(TEMPERATURE_SYSPARAM_SYNC, "true");
        }
        return false;
    }
#endif
    return true;
}
}