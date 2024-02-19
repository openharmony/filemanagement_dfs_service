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
#include "drive_kit.h"

namespace DriveKit {
std::mutex DriveKitNative::drivekitMutex_;
std::map<int, std::shared_ptr<DriveKitNative>> DriveKitNative::driveKits_;
std::shared_ptr<DKContainer> DriveKitNative::GetDefaultContainer(DKAppBundleName bundleName)
{
    if (bundleName == "com.ohos.test.mock") {
        return nullptr;
    }
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
    container->Init();
    containers_[key.c_str()] = container;
    return container;
}

std::shared_ptr<DKContainer> DriveKitNative::GetContainer(DKAppBundleName bundleName, DKContainerName containerName)
{
    std::string key = bundleName + containerName;
    auto it = containers_.find(key);
    if (it != containers_.end()) {
        return it->second;
    }
    std::shared_ptr<DKContainer> container =
        std::make_shared<DKContainer>(bundleName, containerName, shared_from_this());
    container->Init();
    containers_[key.c_str()] = container;
    return container;
}

DKError DriveKitNative::GetCloudUserInfo(DKUserInfo &userInfo)
{
    return DKError();
}

DKError DriveKitNative::GetCloudAppInfo(const std::vector<DKAppBundleName> &bundleNames,
                                        std::map<DKAppBundleName, DKAppInfo> &appInfos)
{
    DKAppInfo appInfo;
    appInfo.defaultContainer.containerName = "defaultContainer";
    for (const auto &name : bundleNames) {
        appInfos[name] = appInfo;
    }
    return DKError();
}

DKError DriveKitNative::GetCloudAppSwitches(const std::vector<DKAppBundleName> &bundleNames,
                                            std::map<DKAppBundleName, DKAppSwitchStatus> &appSwitchs)
{
    return DKError();
}

DKError DriveKitNative::GetServerTime(time_t &time)
{
    return DKError();
}

std::shared_ptr<DriveKitNative> DriveKitNative::GetInstance(int userId)
{
    if (userId < 0) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lck(drivekitMutex_);
    auto it = driveKits_.find(userId);
    if (it != driveKits_.end()) {
        return it->second;
    }
    struct EnableMakeShared : public DriveKitNative {
        explicit EnableMakeShared(int userId) : DriveKitNative(userId) {}
    };
    std::shared_ptr<DriveKitNative> driveKit = std::make_shared<EnableMakeShared>(userId);
    driveKits_[userId] = driveKit;
    return driveKit;
}

std::shared_ptr<DriveKitNative> DriveKitNative::GetInstance(int userId, DKCloudSyncDemon syncDemon)
{
    auto drivekit = GetInstance(userId);
    if (drivekit) {
        drivekit->syncDemon_ = syncDemon;
    }
    return drivekit;
}

DriveKitNative::DriveKitNative(int userId)
{
    userId_ = userId;
}

DriveKitNative::~DriveKitNative() {}

int32_t DriveKitNative::OnUploadAsset(const std::string &request, const std::string &result)
{
    return 0;
}

void DriveKitNative::CleanCloudUserInfo()
{
    return;
}
} // namespace DriveKit
