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

#include "dk_container.h"

#include <functional>
#include <gtest/gtest.h>
#include <iostream>

namespace DriveKit {

namespace {
constexpr int SCHED_NUM = 1;
constexpr int FILE_NUM = 2;
static int32_t g_nPrivateDatabaseNum = 0;
} // namespace

std::shared_ptr<DKDatabase> DKContainer::GetPrivateDatabase()
{
    GTEST_LOG_(INFO) << "GetPrivateDatabase";
    g_nPrivateDatabaseNum++;
    if (g_nPrivateDatabaseNum % FILE_NUM == SCHED_NUM) {
        GTEST_LOG_(INFO) << "GetPrivateDatabase is false";
        return nullptr;
    }
    GTEST_LOG_(INFO) << "GetPrivateDatabase is true";
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

void DKContainer::Init()
{
    if (driveKit_) {
        publicDatabase_ = std::make_shared<DKDatabase>(shared_from_this(), DKDatabaseScope::DK_PUBLIC_DATABASE);
        if (publicDatabase_) {
            publicDatabase_->Init();
        }
        privateDatabase_ = std::make_shared<DKDatabase>(shared_from_this(), DKDatabaseScope::DK_PRIVATE_DATABASE);
        if (privateDatabase_) {
            privateDatabase_->Init();
        }
        sharedDatabase_ = std::make_shared<DKDatabase>(shared_from_this(), DKDatabaseScope::DK_SHARED_DATABASE);
        if (sharedDatabase_) {
            sharedDatabase_->Init();
        }
    }
}

DKLocalErrorCode DKContainer::SaveSubscription(std::shared_ptr<DKContext> contex,
                                               DKSubscription &subscription,
                                               SaveSubscriptionCallback callback)
{
    GTEST_LOG_(INFO) << "SaveSubscription begin";
    if (callback) {
        return DKLocalErrorCode::IPC_CONNECT_FAILED;
    }
    return DKLocalErrorCode::NO_ERROR;
}

DKLocalErrorCode DKContainer::DeleteSubscription(std::shared_ptr<DKContext> contex,
                                                 DKSubscriptionId id,
                                                 DelSubscriptionCallback callback)
{
    GTEST_LOG_(INFO) << "DeleteSubscription begin";
    if (callback) {
        return DKLocalErrorCode::IPC_CONNECT_FAILED;
    }
    return DKLocalErrorCode::NO_ERROR;
}
void DKContainer::Release() {}
} // namespace DriveKit
