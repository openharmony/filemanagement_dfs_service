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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_PROXY_LITE_H
#define OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_PROXY_LITE_H

#include "iremote_proxy.h"
#include "system_ability_load_callback_stub.h"

#include "i_cloud_sync_service_lite.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncServiceProxyLite : public IRemoteProxy<ICloudSyncServiceLite> {
public:
    explicit CloudSyncServiceProxyLite(
        const sptr<IRemoteObject> &impl) : IRemoteProxy<ICloudSyncServiceLite>(impl) {}
    ~CloudSyncServiceProxyLite() override {}

    static sptr<ICloudSyncServiceLite> GetInstance();
    static void InvalidInstance();
    int32_t TriggerSyncInner(const std::string &bundleName, const int32_t &userId) override;

    class ServiceProxyLoadCallbackLite : public SystemAbilityLoadCallbackStub {
    public:
        void OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
            const sptr<IRemoteObject> &remoteObject) override;
        void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;

        std::condition_variable proxyConVar_;
        std::atomic<bool> isLoadSuccess_{false};
    };
private:
    static inline std::mutex proxyMutex_;
    static inline std::mutex instanceMutex_;
    static inline sptr<ICloudSyncServiceLite> serviceProxy_;
    static inline BrokerDelegator<CloudSyncServiceProxyLite> delegator_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_PROXY_LITE_H
