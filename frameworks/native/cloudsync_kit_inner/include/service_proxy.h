/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_SERVICE_PROXY_H
#define OHOS_FILEMGMT_SERVICE_PROXY_H

#include "icloud_sync_service.h"
#include "iremote_proxy.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS::FileManagement::CloudSync {
class ServiceProxy : public IRemoteProxy<ICloudSyncService> {
public:
    static sptr<ICloudSyncService> GetInstance();
    static void InvaildInstance();

    class ServiceProxyLoadCallback : public SystemAbilityLoadCallbackStub {
    public:
        void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject> &remoteObject) override;
        void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;

        std::condition_variable proxyConVar_;
        std::atomic<bool> isLoadSuccess_{false};
    };

private:
    static inline std::mutex proxyMutex_;
    static inline std::mutex instanceMutex_;
    static inline sptr<ICloudSyncService> serviceProxy_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_SERVICE_PROXY_H
