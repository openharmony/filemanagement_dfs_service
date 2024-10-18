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

#ifndef OHOS_STORAGE_ASSET_ADAPTER_SA_CLIENT_H
#define OHOS_STORAGE_ASSET_ADAPTER_SA_CLIENT_H

#include <cstdint>
#include <mutex>
#include "asset/i_asset_recv_callback.h"
#include "refbase.h"
#include "system_ability_status_change_stub.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class AssetAdapterSaClient {
public:
    static AssetAdapterSaClient &GetInstance();
    bool SubscribeAssetAdapterSA();
    int32_t AddListener(const sptr<IAssetRecvCallback> &listener);
    int32_t RemoveListener(const sptr<IAssetRecvCallback> &listener);
    bool CheckSystemAbilityStatus();
    void OnAddSystemAbility();

private:
    AssetAdapterSaClient();
    std::vector<sptr<IAssetRecvCallback>> listeners_;
    std::mutex eventMutex_;
};

class AssetSystemAbilityStatusChange : public SystemAbilityStatusChangeStub {
public:
    AssetSystemAbilityStatusChange();
    ~AssetSystemAbilityStatusChange();
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // OHOS_STORAGE_ASSET_ADAPTER_SA_CLIENT_H
