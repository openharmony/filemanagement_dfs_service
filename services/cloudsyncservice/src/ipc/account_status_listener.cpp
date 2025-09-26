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

#include "account_status_listener.h"

#include <fcntl.h>
#include "common_event_manager.h"
#include "common_event_support.h"
#include "iservice_registry.h"
#include "parameters.h"
#include "system_ability_definition.h"
#include "task_state_manager.h"

#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {

using namespace AccountSA;

void AccountStatusSubscriber::OnStateChanged(const OsAccountStateData &data)
{
    if (data.state == OsAccountState::STOPPING) {
        LOGI("Stopped user");
        UnloadSa();
    }
}

void AccountStatusSubscriber::UnloadSa()
{
    auto samgrProxy = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgrProxy == nullptr) {
        LOGE("get samgr failed");
        return;
    }
    system::SetParameter(CLOUD_FILE_SERVICE_SA_STATUS_FLAG, CLOUD_FILE_SERVICE_SA_END);
    int32_t ret = samgrProxy->UnloadSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID);
    if (ret != ERR_OK) {
        LOGE("remove system ability failed");
        return;
    }
    LOGI("unload cloudfileservice end");
}

AccountStatusListener::~AccountStatusListener()
{
    Stop();
}

void AccountStatusListener::Start()
{
    std::set<OsAccountState> states = {OsAccountState::STOPPING};
    OsAccountSubscribeInfo subscribeInfo(states);
    osAccountSubscriber_ = std::make_shared<AccountStatusSubscriber>(subscribeInfo);
    ErrCode errCode = OsAccountManager::SubscribeOsAccount(osAccountSubscriber_);
    LOGI("account subscribe errCode:%{public}d", errCode);
}

void AccountStatusListener::Stop()
{
    if (osAccountSubscriber_!= nullptr) {
        ErrCode errCode = OsAccountManager::UnsubscribeOsAccount(osAccountSubscriber_);
        osAccountSubscriber_ = nullptr;
    }
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
