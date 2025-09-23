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

#ifndef CLOUD_DISK_SERVICE_ACCOUNT_STATUS_LISTENER_H
#define CLOUD_DISK_SERVICE_ACCOUNT_STATUS_LISTENER_H

#include "cloud_file_utils.h"
#include "disk_monitor.h"
#include "os_account_manager.h"
#include "system_ability_load_callback_stub.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {
class AccountStatusListener {
public:
    explicit AccountStatusListener() = default;
    ~AccountStatusListener();
    void Start();
    void Stop();

private:
    std::shared_ptr<AccountSA::OsAccountSubscriber> osAccountSubscriber_ = nullptr;
};

class AccountStatusSubscriber : public AccountSA::OsAccountSubscriber {
public:
    AccountStatusSubscriber(const AccountSA::OsAccountSubscribeInfo &info) : OsAccountSubscriber(info) {};
    ~AccountStatusSubscriber() override {}
    void OnStateChanged(const AccountSA::OsAccountStateData &data) override;
    void UnloadSa();
};
} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_DISK_SERVICE_ACCOUNT_STATUS_LISTENER_H