/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "os_account_manager.h"

#include "mock_other_method.h"

namespace OHOS {
using namespace OHOS::Storage::DistributedFile;

namespace AccountSA {
/**
 * Interfaces for ohos account subsystem.
 */
class OhosAccountKitsImpl : public OhosAccountKits {
public:
    std::pair<bool, OhosAccountInfo> QueryOhosAccountInfo()
    {
        return {};
    }

    ErrCode GetOhosAccountInfo(OhosAccountInfo &accountInfo);

    ErrCode GetOsAccountDistributedInfo(int32_t localId, OhosAccountInfo &accountInfo)
    {
        return 0;
    }

    std::pair<bool, OhosAccountInfo> QueryOsAccountDistributedInfo(std::int32_t localId)
    {
        return {};
    }

    ErrCode UpdateOhosAccountInfo(const std::string& accountName, const std::string& uid,
        const std::string& eventStr)
    {
        return 0;
    }

    ErrCode SetOhosAccountInfo(const OhosAccountInfo &ohosAccountInfo,
        const std::string &eventStr)
    {
        return 0;
    }

    ErrCode SetOsAccountDistributedInfo(
        const int32_t localId, const OhosAccountInfo& ohosAccountInfo, const std::string& eventStr)
    {
        return 0;
    }

    ErrCode QueryDeviceAccountId(std::int32_t& accountId)
    {
        return 0;
    }

    std::int32_t GetDeviceAccountIdByUID(std::int32_t& uid)
    {
        return 0;
    }

    ErrCode SubscribeDistributedAccountEvent(const DISTRIBUTED_ACCOUNT_SUBSCRIBE_TYPE type,
        const std::shared_ptr<DistributedAccountSubscribeCallback> &callback)
    {
        return 0;
    }

    ErrCode UnsubscribeDistributedAccountEvent(const DISTRIBUTED_ACCOUNT_SUBSCRIBE_TYPE type,
        const std::shared_ptr<DistributedAccountSubscribeCallback> &callback)
    {
        return 0;
    }
};
} // namespace AccountSA


ErrCode AccountSA::OsAccountManager::QueryActiveOsAccountIds(std::vector<int32_t>& ids)
{
    return DfsDeviceOtherMethod::otherMethod->QueryActiveOsAccountIds(ids);
}

ErrCode AccountSA::OhosAccountKitsImpl::GetOhosAccountInfo(AccountSA::OhosAccountInfo &accountInfo)
{
    return DfsDeviceOtherMethod::otherMethod->GetOhosAccountInfo(accountInfo);
}
} // namespace OHOS

namespace OHOS::AccountSA {
OhosAccountKits &OhosAccountKits::GetInstance()
{
    static OhosAccountKitsImpl test;
    return test;
}
}