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

#include <functional>
#include <iostream>

#include "dk_container.h"

namespace DriveKit {

std::shared_ptr<DKDatabase> DKContainer::GetPrivateDatabase()
{
    return privateDatabase_;
}

std::shared_ptr<DKDatabase> DKContainer::GetDatabaseWithdatabaseScope(DKDatabaseScope databaseScope)
{
    std::shared_ptr<DKDatabase> database = nullptr;
    switch (databaseScope) {
        case DKDatabaseScope::DK_PUBLIC_DATABASE:
            database = publicDatabase_;
            break;
        case DKDatabaseScope::DK_PRIVATE_DATABASE:
            database = privateDatabase_;
            break;
        case DKDatabaseScope::DK_SHARED_DATABASE:
            database = sharedDatabase_;
            break;
        default:
            break;
    }
    return database;
}

DKLocalErrorCode DKContainer::SaveSubscription(
    std::shared_ptr<DKContext> contex,
    DKSubscription &subscription,
    std::function<
        void(std::shared_ptr<DKContext>, std::shared_ptr<const DKContainer>, DKSubscriptionResult &, const DKError &)>
        callback)
{
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKContainer::DeleteSubscription(
    std::shared_ptr<DKContext> context,
    DKSubscriptionId id,
    std::function<void(std::shared_ptr<DKContext>, const DKError &)> callback)
{
    return DKLocalErrorCode::NO_ERROR;
}
} // namespace DriveKit
