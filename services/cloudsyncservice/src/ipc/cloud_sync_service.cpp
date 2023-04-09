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
#include "ipc/cloud_sync_service.h"

#include <memory>

#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "ipc/cloud_sync_callback_manager.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/net_conn_callback_observer.h"
#include "sync_rule/network_status.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
using namespace OHOS;

REGISTER_SYSTEM_ABILITY_BY_ID(CloudSyncService, FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, false);

CloudSyncService::CloudSyncService(int32_t saID, bool runOnCreate) : SystemAbility(saID, runOnCreate)
{
    dataSyncManager_ = make_shared<DataSyncManager>();
    batteryStatusListener_ = make_shared<BatteryStatusListener>(dataSyncManager_);
}

void CloudSyncService::PublishSA()
{
    LOGI("Begin to init");
    if (!SystemAbility::Publish(this)) {
        throw runtime_error(" Failed to publish the daemon");
    }
    LOGI("Init finished successfully");
}

void CloudSyncService::Init()
{
    NetworkStatus::InitNetwork();
    /* Get Init Charging status */
    BatteryStatus::GetInitChargingStatus();
}

void CloudSyncService::OnStart()
{
    Init();
    LOGI("Begin to start service");
    try {
        PublishSA();
        AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    LOGI("Start service successfully");
}

void CloudSyncService::OnStop()
{
    LOGI("Stop finished successfully");
}

void CloudSyncService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    batteryStatusListener_->Start();
}

int32_t CloudSyncService::RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject)
{
    if (remoteObject == nullptr) {
        LOGE("remoteObject is nullptr");
        return E_INVAL_ARG;
    }

    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }

    auto callback = iface_cast<ICloudSyncCallback>(remoteObject);
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    CloudSyncCallbackManager::GetInstance().AddCallback(bundleName, callerUserId, callback);
    return E_OK;
}

int32_t CloudSyncService::StartSyncInner(bool forceFlag)
{
    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->TriggerStartSync(bundleName, callerUserId, forceFlag, SyncTriggerType::APP_TRIGGER);
}

int32_t CloudSyncService::StopSyncInner()
{
    string bundleName;
    if (DfsuAccessTokenHelper::GetCallerBundleName(bundleName)) {
        return E_INVAL_ARG;
    }
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->TriggerStopSync(bundleName, callerUserId, SyncTriggerType::APP_TRIGGER);
}

int32_t CloudSyncService::ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status)
{
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    if (status) {
        return dataSyncManager_->TriggerStartSync(bundleName, callerUserId, false, SyncTriggerType::CLOUD_TRIGGER);
    }
    return dataSyncManager_->TriggerStopSync(bundleName, callerUserId, SyncTriggerType::CLOUD_TRIGGER);
}

int32_t CloudSyncService::NotifyDataChange(const std::string &accoutId, const std::string &bundleName)
{
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    return dataSyncManager_->TriggerStartSync(bundleName, callerUserId, false, SyncTriggerType::CLOUD_TRIGGER);
}
} // namespace OHOS::FileManagement::CloudSync
