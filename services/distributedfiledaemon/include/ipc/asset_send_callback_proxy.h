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

#ifndef ASSET_SEND_CALLBACK_PROXY_H
#define ASSET_SEND_CALLBACK_PROXY_H

#include "asset/i_asset_send_callback.h"
#include "iremote_proxy.h"
#include "asset/asset_obj.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class AssetSendCallbackProxy : public IRemoteProxy<IAssetSendCallback> {
public:
    explicit AssetSendCallbackProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IAssetSendCallback>(object) {}
    ~AssetSendCallbackProxy() override = default;

    int32_t OnSendResult(const sptr<AssetObj> &assetObj, int32_t result) override;

private:
    static inline BrokerDelegator<AssetSendCallbackProxy> delegator_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // ASSET_SEND_CALLBACK_PROXY_H