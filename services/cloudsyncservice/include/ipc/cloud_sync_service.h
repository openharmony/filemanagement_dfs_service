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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_H
#define OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_H

#include <map>

#include "cloud_sync_callback_manager.h"
#include "cloud_sync_service_stub.h"
#include "i_cloud_sync_callback.h"
#include "nocopyable.h"
#include "system_ability.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncService final : public SystemAbility, public CloudSyncServiceStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(CloudSyncService);

public:
    explicit CloudSyncService(int32_t saID, bool runOnCreate = true) : SystemAbility(saID, runOnCreate) {}
    virtual ~CloudSyncService() = default;

    int32_t RegisterCallbackInner(const std::string &appPackageName, const sptr<IRemoteObject> &remoteObject) override;
    int32_t StartSyncInner(const std::string &appPackageName, SyncType type, bool forceFlag) override;
    int32_t StopSyncInner(const std::string &appPackageName) override;

private:
    void OnStart() override;
    void OnStop() override;
    void PublishSA();

    CloudSyncCallbackManager callbackManager_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_H