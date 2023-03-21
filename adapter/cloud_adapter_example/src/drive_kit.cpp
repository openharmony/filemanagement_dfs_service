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

#include "drive_kit.h"

namespace DriveKit {
std::mutex DriveKit::drivekitMutex_;
std::map<int, std::shared_ptr<DriveKit>> DriveKit::driveKits_;
std::shared_ptr<DKContainer> DriveKit::GetDefaultContainer(DKAppBundleName bundleName)
{
    std::lock_guard<std::mutex> lck(containMutex_);
    auto it = containers_.find(bundleName);
    if (it != containers_.end()) {
        return it->second;
    }
    std::shared_ptr<DKContainer> container = std::make_shared<DKContainer>(bundleName);
    containers_[bundleName] = container;
    return container;
}

std::shared_ptr<DKContainer> DriveKit::GetContainer(DKAppBundleName bundleName, DKContainerName containerName)
{
    auto it = containers_.find(bundleName);
    if (it != containers_.end()) {
        return it->second;
    }
    std::shared_ptr<DKContainer> container = std::make_shared<DKContainer>(bundleName);
    containers_[bundleName] = container;
    return container;
}

DKError DriveKit::GetCloudUserInfo(DKUserInfo &userInfo)
{
    return DKError();
}

DKError DriveKit::GetCloudAppInfo(const std::vector<DKAppBundleName> &bundleNames,
                                  std::map<DKAppBundleName, DKAppInfo> &appInfos)
{
    return DKError();
}

DKError DriveKit::GetCloudAppSwitches(const std::vector<DKAppBundleName> &bundleNames,
                                      std::map<DKAppBundleName, DKAppSwitchStatus> &appSwitchs)
{
    return DKError();
}

DKError DriveKit::GetServerTime(time_t &time)
{
    return DKError();
}

DKError DriveKit::GetSchemaRawData(const DKAppBundleName &bundleNames, std::string &schemaData)
{
    return DKError();
}

DKError DriveKit::GetSchemaRecordTypes(const DKAppBundleName &bundleNames, std::vector<DKRecordType> recordType)
{
    return DKError();
}

std::shared_ptr<DriveKit> DriveKit::getInstance(int userId)
{
    std::lock_guard<std::mutex> lck(drivekitMutex_);
    auto it = driveKits_.find(userId);
    if (it != driveKits_.end()) {
        return it->second;
    }
    std::shared_ptr<DriveKit> driveKit(new DriveKit(userId));
    driveKits_[userId] = driveKit;
    return driveKit;
}

DriveKit::DriveKit(int userId)
{
    userId_ = userId;
}

DriveKit::~DriveKit() {}
} // namespace DriveKit
