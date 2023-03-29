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

#ifndef DRIVE_KIT_CONTAINER_H
#define DRIVE_KIT_CONTAINER_H

#include "dk_cloud_callback.h"
#include "dk_context.h"
#include "dk_database.h"
#include "dk_error.h"
#include "dk_result.h"
#include <map>
#include <string>

namespace DriveKit {

enum class DKDatabaseScope {
    DK_PUBLIC_DATABASE = 0,
    DK_PRIVATE_DATABASE,
    DK_SHARED_DATABASE,
};
using DKSubscriptionId = std::string;
struct DKSubscription {
    int64_t expirationTime;
};
class DKSubscriptionResult : public DKResult {
public:
    DKSubscription GetDKSubscription()
    {
        return subscription_;
    }
    DKSubscriptionId GetDKSubscriptionId()
    {
        return id_;
    }

    void SetDKSubscription(DKSubscription subscription)
    {
        subscription_ = subscription;
    }
    void SetDKSubscriptionId(DKSubscriptionId id)
    {
        id_ = id;
    }

private:
    DKSubscription subscription_;
    DKSubscriptionId id_;
};
class DriveKit;
class DKContainer : public std::enable_shared_from_this<DKContainer> {
public:
    DKContainer(DKAppBundleName bundleName, DKContainerName containerName, std::shared_ptr<DriveKit> driveKit)
    {
        bundleName_ = bundleName;
        containerName_ = containerName;
        driveKit_ = driveKit;
        if (driveKit_) {
            publicDatabase_ = std::make_shared<DKDatabase>(shared_from_this());
            privateDatabase_ = std::make_shared<DKDatabase>(shared_from_this());
            sharedDatabase_ = std::make_shared<DKDatabase>(shared_from_this());
        }
    }
    ~DKContainer() = default;
    std::shared_ptr<DKDatabase> GetPrivateDatabase();
    std::shared_ptr<DKDatabase> GetDatabaseWithdatabaseScope(DKDatabaseScope databaseScope);
    DKLocalErrorCode SaveSubscription(std::shared_ptr<DKContext> contex,
                                      DKSubscription &subscription,
                                      std::function<void(std::shared_ptr<DKContext>,
                                                         std::shared_ptr<const DKContainer>,
                                                         DKSubscriptionResult &,
                                                         const DKError &)> callback);
    DKLocalErrorCode DeleteSubscription(std::shared_ptr<DKContext> context,
                                        DKSubscriptionId id,
                                        std::function<void(std::shared_ptr<DKContext>, const DKError &)> callback);

    DKAppBundleName GetAppBundleName()
    {
        return bundleName_;
    }
    DKContainerName GetContainerName()
    {
        return containerName_;
    }
    std::shared_ptr<DriveKit> GetDriveKit()
    {
        return driveKit_;
    }

private:
    std::shared_ptr<DriveKit> driveKit_;
    std::shared_ptr<DKDatabase> publicDatabase_;
    std::shared_ptr<DKDatabase> privateDatabase_;
    std::shared_ptr<DKDatabase> sharedDatabase_;
    DKAppBundleName bundleName_;
    DKContainerName containerName_;
};
} // namespace DriveKit
#endif
