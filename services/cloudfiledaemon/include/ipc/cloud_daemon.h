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

#ifndef CLOUD_DAEMON_H
#define CLOUD_DAEMON_H

#include <memory>
#include <mutex>
#include <string>

#include "iremote_stub.h"
#include "nocopyable.h"
#include "refbase.h"
#include "system_ability.h"

#include "cloud_daemon_stub.h"
#include "i_cloud_daemon.h"
#include "account_status_listener.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class CloudDaemon final : public SystemAbility, public CloudDaemonStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(CloudDaemon);

public:
    explicit CloudDaemon(int32_t saID, bool runOnCreate = true);
    virtual ~CloudDaemon() = default;

    void OnStart() override;
    void OnStop() override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }
    int32_t StartFuse(int32_t userId, int32_t deviceFd, const std::string &path) override;

private:
    CloudDaemon();
    ServiceRunningState state_ { ServiceRunningState::STATE_NOT_START };
    static sptr<CloudDaemon> instance_;
    static std::mutex instanceLock_;
    bool registerToService_ { false };
    void PublishSA();
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    std::shared_ptr<CloudDisk::AccountStatusListener> accountStatusListener_ = nullptr;
};
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_DAEMON_H
