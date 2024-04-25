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
#include "account_status.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
bool AccountStatus::IsNeedCleanCache()
{
    AccountState tempPreAccountState = preAccountState_;
    AccountState tempAccountState = accountState_;
    preAccountState_ = AccountState::ACCOUNT_LOGIN;
    accountState_ = AccountState::ACCOUNT_LOGIN;
    return (tempPreAccountState == AccountState::ACCOUNT_LOGOUT && tempAccountState == AccountState::ACCOUNT_LOGIN) ||
        (tempPreAccountState == AccountState::ACCOUNT_LOGIN && tempAccountState == AccountState::ACCOUNT_LOGOUT);
}

void AccountStatus::SetAccountState(AccountState accountState)
{
    preAccountState_ = accountState_;
    accountState_ = accountState;
}

AccountStatus::AccountState AccountStatus::GetAccountState()
{
    return accountState_;
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS