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

#include "account_utils.h"
#include "os_account_manager.h"
#include "utils_log.h"

namespace OHOS::FileManagement {
using namespace std;
bool AccountUtils::IsAccountAvailableByUser(const int32_t userId)
{
    if (userId == MAIN_USER_ID) {
        return true;
    }

    AccountSA::OsAccountType accountType;
    auto ret = AccountSA::OsAccountManager::GetOsAccountType(userId, accountType);
    if (ret != 0) {
        LOGE("get accountType failed, ret: %{public}d, userId: %{public}d", ret, userId);
        return true;
    }
    if (accountType == AccountSA::OsAccountType::PRIVATE || accountType == AccountSA::OsAccountType::MAINTENANCE) {
        LOGE("accountType is private or maintenance, accountType: %{public}d, userId: %{public}d",
             accountType, userId);
        return false;
    }
    return true;
}

bool AccountUtils::IsAccountAvailable()
{
    int32_t userId = -1;
    auto result = AccountSA::OsAccountManager::GetOsAccountLocalIdFromProcess(userId);
    if (result != 0) {
        LOGE("get userId from process failed, ret: %{public}d", result);
        return true;
    }

    if (userId == MAIN_USER_ID) {
        return true;
    }

    AccountSA::OsAccountType accountType;
    auto ret = AccountSA::OsAccountManager::GetOsAccountType(userId, accountType);
    if (ret != 0) {
        LOGE("get accountType failed, ret: %{public}d, userId: %{public}d", ret, userId);
        return true;
    }
    if (accountType == AccountSA::OsAccountType::PRIVATE || accountType == AccountSA::OsAccountType::MAINTENANCE) {
        LOGE("accountType is private or maintenance, accountType: %{public}d, userId: %{public}d",
             accountType, userId);
        return false;
    }
    return true;
}
} // namespace OHOS::FileManagement