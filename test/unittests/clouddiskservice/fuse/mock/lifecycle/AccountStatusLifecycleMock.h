/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef ACCOUNT_STATUS_LIFECYCLE_MOCK_H
#define ACCOUNT_STATUS_LIFECYCLE_MOCK_H

#include <cstdint>
#include <memory>
#include <vector>

namespace OHOS::AccountSA {
class OsAccountSubscriber;
}

namespace OHOS::FileManagement::CloudDiskService::Test {

class AccountStatusLifecycleMock final {
public:
    enum class Operation {
        SUBSCRIBE,
        UNSUBSCRIBE,
    };

    static void Reset()
    {
        subscribeCalls_ = 0;
        unsubscribeCalls_ = 0;
        subscribed_.reset();
        unsubscribed_.reset();
        operations_.clear();
    }

    static void RecordSubscribe(const std::shared_ptr<AccountSA::OsAccountSubscriber> &subscriber)
    {
        ++subscribeCalls_;
        subscribed_ = subscriber;
        operations_.emplace_back(Operation::SUBSCRIBE);
    }

    static void RecordUnsubscribe(const std::shared_ptr<AccountSA::OsAccountSubscriber> &subscriber)
    {
        ++unsubscribeCalls_;
        unsubscribed_ = subscriber;
        operations_.emplace_back(Operation::UNSUBSCRIBE);
    }

    static uint32_t GetSubscribeCalls()
    {
        return subscribeCalls_;
    }

    static uint32_t GetUnsubscribeCalls()
    {
        return unsubscribeCalls_;
    }

    static const std::shared_ptr<AccountSA::OsAccountSubscriber> &GetSubscribed()
    {
        return subscribed_;
    }

    static const std::shared_ptr<AccountSA::OsAccountSubscriber> &GetUnsubscribed()
    {
        return unsubscribed_;
    }

    static const std::vector<Operation> &GetOperations()
    {
        return operations_;
    }

private:
    static inline uint32_t subscribeCalls_{0};
    static inline uint32_t unsubscribeCalls_{0};
    static inline std::shared_ptr<AccountSA::OsAccountSubscriber> subscribed_;
    static inline std::shared_ptr<AccountSA::OsAccountSubscriber> unsubscribed_;
    static inline std::vector<Operation> operations_;
};

} // namespace OHOS::FileManagement::CloudDiskService::Test

#endif // ACCOUNT_STATUS_LIFECYCLE_MOCK_H
