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
#include "dk_container.h"

namespace DriveKit {
std::mutex DriveKit::drivekitMutex_;
std::map<int, std::shared_ptr<DriveKit>> DriveKit::driveKits_;
std::shared_ptr<DKContainer> DriveKit::GetDefaultContainer(DKAppBundleName bundleName)
{
    DKContainerName containerName;
    {
        std::lock_guard<std::mutex> lck(appInfoMutex_);
        auto itor = appInfos_.find(bundleName);
        if (itor != appInfos_.end()) {
            containerName = itor->second.defaultContainer.containerName;
        }
    }
    if (containerName.empty()) {
        std::map<DKAppBundleName, DKAppInfo> appInfo;
        if (GetCloudAppInfo({bundleName}, appInfo).HasError()) {
            return nullptr;
        }
        auto itor = appInfo.find(bundleName);
        if (itor == appInfo.end()) {
            return nullptr;
        }
        containerName = itor->second.defaultContainer.containerName;
    }
    std::string key = bundleName + containerName;
    std::lock_guard<std::mutex> lck(containMutex_);
    auto it = containers_.find(key);
    if (it != containers_.end()) {
        return it->second;
    }
    std::shared_ptr<DKContainer> container =
        std::make_shared<DKContainer>(bundleName, containerName, shared_from_this());
    containers_[key] = container;
    return container;
}

std::shared_ptr<DKContainer> DriveKit::GetContainer(DKAppBundleName bundleName, DKContainerName containerName)
{
    std::string key = bundleName + containerName;
    auto it = containers_.find(key);
    if (it != containers_.end()) {
        return it->second;
    }
    std::shared_ptr<DKContainer> container =
        std::make_shared<DKContainer>(bundleName, containerName, shared_from_this());
    containers_[key] = container;
    return container;
}

DKError DriveKit::GetCloudUserInfo(DKUserInfo &userInfo)
{
    return DKError();
}

DKError DriveKit::GetCloudAppInfo(const std::vector<DKAppBundleName> &bundleNames,
                                  std::map<DKAppBundleName, DKAppInfo> &appInfos)
{
    DKAppInfo appInfo;
    appInfo.defaultContainer.containerName = "defaultContainer";
    for (auto &name : bundleNames) {
        appInfos[name] = appInfo;
    }
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

std::shared_ptr<DriveKit> DriveKit::getInstance(int userId)
{
    std::lock_guard<std::mutex> lck(drivekitMutex_);
    auto it = driveKits_.find(userId);
    if (it != driveKits_.end()) {
        return it->second;
    }
    struct EnableMakeShared : public DriveKit {
        explicit EnableMakeShared(int userId) : DriveKit(userId) {}
    };
    std::shared_ptr<DriveKit> driveKit = std::make_shared<EnableMakeShared>(userId);
    driveKits_[userId] = driveKit;
    return driveKit;
}

DriveKit::DriveKit(int userId)
{
    userId_ = userId;
}

DriveKit::~DriveKit() {}
} // namespace DriveKit
