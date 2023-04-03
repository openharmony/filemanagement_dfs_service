/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <thread>

#include "ipc/cloud_sync_callback_manager.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "system_ability_definition.h"
#include "utils_log.h"
#include "sync_rule/net_conn_callback_observer.h"
#include "sync_rule/network_status.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
using namespace OHOS;

REGISTER_SYSTEM_ABILITY_BY_ID(CloudSyncService, FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, false);

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
}

void CloudSyncService::OnStart()
{
    Init();
    LOGI("Begin to start service");
    try {
        PublishSA();
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    LOGI("Start service successfully");
}

void CloudSyncService::OnStop()
{
    LOGI("Stop finished successfully");
}

int32_t CloudSyncService::RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject)
{
    if (remoteObject == nullptr) {
        LOGE("remoteObject is nullptr");
        return E_INVAL_ARG;
    }

    string appPackageName;
    if (DfsuAccessTokenHelper::GetCallerPackageName(appPackageName)) {
        return E_INVAL_ARG;
    }

    auto callback = iface_cast<ICloudSyncCallback>(remoteObject);
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    CloudSyncCallbackManager::GetInstance().AddCallback(appPackageName, callerUserId, callback);
    return E_OK;
}

int32_t CloudSyncService::StartSyncInner(bool forceFlag)
{
    string appPackageName;
    if (DfsuAccessTokenHelper::GetCallerPackageName(appPackageName)) {
        return E_INVAL_ARG;
    }
    return StartSync(appPackageName, forceFlag, SyncTriggerType::APP_TRIGGER);
}

int32_t CloudSyncService::StopSyncInner()
{
    string appPackageName;
    if (DfsuAccessTokenHelper::GetCallerPackageName(appPackageName)) {
        return E_INVAL_ARG;
    }
    return StopSync(appPackageName, SyncTriggerType::APP_TRIGGER);
}

int32_t CloudSyncService::ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status)
{
    if (status) {
        return StartSync(bundleName, false, SyncTriggerType::CLOUD_TRIGGER);
    }
    return StopSync(bundleName, SyncTriggerType::CLOUD_TRIGGER);
}

int32_t CloudSyncService::NotifyDataChange(const std::string &accoutId, const std::string &bundleName)
{
    return StartSync(bundleName, false, SyncTriggerType::CLOUD_TRIGGER);
}

int32_t CloudSyncService::StartSync(const std::string &bundleName, bool forceFlag, SyncTriggerType triggerType)
{
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    auto ret = dataSyncManager_.Init(callerUserId);
    if (ret != E_OK) {
        return ret;
    }
    ret = dataSyncManager_.IsSkipSync(bundleName, callerUserId);
    if (ret != E_OK) {
        return ret;
    }
    auto dataSyncer = dataSyncManager_.GetDataSyncer(bundleName, callerUserId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, appPackageName: %{private}s", bundleName.c_str());
        return E_SYNCER_NUM_OUT_OF_RANGE;
    }
    std::thread([dataSyncerSptr{dataSyncer}, forceFlag, triggerType]() {
        dataSyncerSptr->StartSync(forceFlag, triggerType);
    }).detach();
    return E_OK;
}

int32_t CloudSyncService::StopSync(const std::string &bundleName, SyncTriggerType triggerType)
{
    auto callerUserId = DfsuAccessTokenHelper::GetUserId();
    auto dataSyncer = dataSyncManager_.GetDataSyncer(bundleName, callerUserId);
    if (!dataSyncer) {
        LOGE("Get dataSyncer failed, appPackageName: %{private}s", bundleName.c_str());
        return E_SYNCER_NUM_OUT_OF_RANGE;
    }
    std::thread([dataSyncerSptr{dataSyncer}, triggerType]() {
        dataSyncerSptr->StopSync(triggerType);
    }).detach();
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
