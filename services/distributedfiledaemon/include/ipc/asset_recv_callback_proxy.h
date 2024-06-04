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

#ifndef ASSET_RECV_CALLBACK_PROXY_H
#define ASSET_RECV_CALLBACK_PROXY_H

#include <string>
#include "asset/asset_obj.h"
#include "asset/i_asset_recv_callback.h"
#include "iremote_proxy.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class AssetRecvCallbackProxy : public IRemoteProxy<IAssetRecvCallback> {
public:
    explicit AssetRecvCallbackProxy(const sptr<IRemoteObject> &object) : IRemoteProxy<IAssetRecvCallback>(object) {}
    ~AssetRecvCallbackProxy() override = default;
    int32_t OnStart(const std::string &srcNetworkId,
                    const std::string &destNetworkId,
                    const std::string &sessionId,
                    const std::string &destBundleName) override;
    int32_t OnFinished(const std::string &srcNetworkId,
                       const sptr<AssetObj> &assetObj,
                       int32_t result) override;
private:
   static inline BrokerDelegator<AssetRecvCallbackProxy> delegator_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // ASSET_RECV_CALLBACK_PROXY_H
