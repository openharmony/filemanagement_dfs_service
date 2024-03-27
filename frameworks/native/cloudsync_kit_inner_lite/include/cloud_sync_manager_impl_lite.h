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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_IMPL_LITE_H
#define OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_IMPL_LITE_H

#include <atomic>

#include "nocopyable.h"

#include "cloud_sync_manager_lite.h"
#include "svc_death_recipient_lite.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncManagerImplLite final : public CloudSyncManagerLite, public NoCopyable {
public:
    static CloudSyncManagerImplLite &GetInstance();

    int32_t TriggerSync(const std::string &bundleName, const int32_t &userId) override;

private:
    CloudSyncManagerImplLite() = default;
    void SetDeathRecipient(const sptr<IRemoteObject> &remoteObject);

    std::atomic_flag isFirstCall_{false};
    sptr<SvcDeathRecipientLite> deathRecipient_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_IMPL_LITE_H
