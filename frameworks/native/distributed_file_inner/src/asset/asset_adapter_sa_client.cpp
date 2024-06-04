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

#include "asset/asset_adapter_sa_client.h"

#include "dfs_error.h"
#include "distributed_file_daemon_manager_impl.h"
#include "distributed_file_daemon_proxy.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace OHOS::FileManagement;

AssetAdapterSaClient &AssetAdapterSaClient::GetInstance()
{
    static AssetAdapterSaClient instance;
    return instance;
}

AssetAdapterSaClient::AssetAdapterSaClient()
{
    LOGI("AssetAdapterSaClient create.");
    if (!SubscribeAssetAdapterSA()) {
        LOGE("AssetAdapterSaClient subscribe asset adapter SA fail.");
    }
}

bool AssetAdapterSaClient::SubscribeAssetAdapterSA()
{
    LOGI("SubscribeAssetAdapterSA");
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LOGE("Asset adapter Sa client get Sa manager proxy fail.");
        return false;
    }
    sptr<AssetSystemAbilityStatusChange> callback = new AssetSystemAbilityStatusChange();
    int32_t ret = samgrProxy->SubscribeSystemAbility(FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID, callback);
    if (ret != E_OK) {
        LOGE("SA manager subscribe system ability fail, SA Id %{public}d, ret %{public}d",
             FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID, ret);
        return false;
    }
    return true;
}

int32_t AssetAdapterSaClient::AddListener(const sptr<IAssetRecvCallback> &listener)
{
    LOGI("Sa client add listener enter.");
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LOGE("Asset adapter Sa client get Sa manager proxy fail.");
        return E_SA_LOAD_FAILED;
    }

    std::lock_guard<std::mutex> lock(eventMutex_);
    listeners_.emplace_back(listener);

    if (!CheckSystemAbilityStatus()) {
        LOGI("Sa client add listener, check system ability status fail.");
        return E_OK;
    }

    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }

    int32_t ret = distributedFileDaemonProxy->RegisterAssetCallback(listener);
    if (ret != E_OK) {
        LOGE("Distributed file deamon proxy register listener fail, ret is %{public}d", ret);
        return ret;
    }
    return E_OK;
}

bool AssetAdapterSaClient::CheckSystemAbilityStatus()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LOGE("Asset adapter Sa client get Sa manager proxy fail.");
        return false;
    }
    if (!samgrProxy->CheckSystemAbility(FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID)) {
        LOGI("SA manager proxy check system ability fail, SA Id %{public}d",
             FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID);
        return false;
    }
    return true;
}

int32_t AssetAdapterSaClient::RemoveListener(const sptr<IAssetRecvCallback> &listener)
{
    LOGI("Sa client remove listener enter.");
    std::lock_guard<std::mutex> lock(eventMutex_);
    for (auto iter = listeners_.begin(); iter != listeners_.end();) {
        if ((*iter)->AsObject() == listener->AsObject()) {
            iter = listeners_.erase(iter);
        } else {
            iter++;
        }
    }

    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }

    int32_t ret = distributedFileDaemonProxy->UnRegisterAssetCallback(listener);
    if (ret != E_OK) {
        LOGE("Distributed file deamon proxy unRegister listener fail, ret is %{public}d", ret);
        return ret;
    }
    return E_OK;
}

void AssetAdapterSaClient::OnAddSystemAbility()
{
    LOGI("Sa client on add system ability enter.");
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return;
    }
    std::lock_guard<std::mutex> lock(eventMutex_);
    LOGI("Sa client on add system ability listener size %{public}d", (int32_t)listeners_.size());
    for (auto iter = listeners_.begin(); iter != listeners_.end(); iter++) {
        int32_t ret = distributedFileDaemonProxy->RegisterAssetCallback(*iter);
        if (ret != E_OK) {
            LOGE("Distributed file deamon proxy register listener fail, ret is %{public}d", ret);
        }
    }
}

AssetSystemAbilityStatusChange::AssetSystemAbilityStatusChange()
{
    LOGI("AssetSystemAbilityStatusChange create.");
}

AssetSystemAbilityStatusChange::~AssetSystemAbilityStatusChange()
{
    LOGI("AssetSystemAbilityStatusChange delete.");
}

void AssetSystemAbilityStatusChange::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("OnAddSystemAbility called, the systemAbilityId is %{public}d", systemAbilityId);
    if (systemAbilityId != FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID) {
        LOGE("systemAbilityId must be FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID, but it is %{public}d",
             systemAbilityId);
    }

    AssetAdapterSaClient::GetInstance().OnAddSystemAbility();
    LOGI("OnAddSystemAbility called end, the systemAbilityId is %{public}d", systemAbilityId);
}

void AssetSystemAbilityStatusChange::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("OnRemoveSystemAbility called, the systemAbilityId is %{public}d", systemAbilityId);
    if (systemAbilityId != FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID) {
        LOGE("systemAbilityId must be FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID, but it is %{public}d",
             systemAbilityId);
    }
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
