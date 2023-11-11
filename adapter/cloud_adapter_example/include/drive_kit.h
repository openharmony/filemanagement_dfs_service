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

#ifndef DRIVE_KIT_H
#define DRIVE_KIT_H
#include <map>
#include <mutex>
#include <string>
#include <time.h>
#include <vector>

#include "dk_cloud_callback.h"
#include "dk_container.h"
#include "dk_error.h"
#include "dk_record.h"

namespace DriveKit {
using DKAppId = std::string;
using DKDatabaseName = std::string;
struct DKContainerInfo {
    DKAppBundleName appBundleName;
    DKContainerName containerName;
    DKDatabaseName databaseName;
    DKSchema schema;
};
struct DKAppInfo {
    DKAppBundleName appBundleName;
    DKAppId appId;
    std::string appFinger;
    int schemaGroupVersion;
    int enableCloud;
    DKAppSwitchStatus switchStatus;
    DKContainerInfo defaultContainer;
    std::vector<DKContainerInfo> shareContainer;
};
enum class DKSpaceStatus {
    DK_SPACE_STATUS_NORMAL = 0,
    DK_SPACE_STATUS_ALREADY_FULL,
};
struct DKUserInfo {
    std::string accountId;
    DKCloudStatus cloudStatus;
    DKSpaceStatus spaceStatus;
    uint64_t totalSpace;
    uint64_t remainSpace;
};
class DriveKitNative : public std::enable_shared_from_this<DriveKitNative> {
    friend class DKContainer;
    friend class DKDatabase;

public:
    DriveKitNative(const DriveKitNative &) = delete;
    DriveKitNative(DriveKitNative &&) = delete;
    DriveKitNative &operator=(const DriveKitNative &) = delete;
    DriveKitNative &operator=(DriveKitNative &&) = delete;
    static std::shared_ptr<DriveKitNative> GetInstance(int userId);
    virtual ~DriveKitNative();

    std::shared_ptr<DKContainer> GetDefaultContainer(DKAppBundleName bundleName);
    std::shared_ptr<DKContainer> GetContainer(DKAppBundleName bundleName, DKContainerName containerName);
    DKError GetCloudUserInfo(DKUserInfo &userInfo);
    DKError GetCloudAppInfo(const std::vector<DKAppBundleName> &bundleNames,
                            std::map<DKAppBundleName, DKAppInfo> &appInfos);
    DKError GetCloudAppSwitches(const std::vector<DKAppBundleName> &bundleNames,
                                std::map<DKAppBundleName, DKAppSwitchStatus> &appSwitchs);
    DKError GetAppConfigParams(const DKAppBundleName &bundleNames,
                               std::map<std::string, std::string> &param);
    DKError GetServerTime(time_t &time);
    int32_t OnUploadAsset(const std::string &request, const std::string &result);
    void ReleaseDefaultContainer(DKAppBundleName bundleName);
    void ReleaseContainer(DKAppBundleName bundleName, DKContainerName containerName);
    void ReleaseCloudUserInfo();
private:
    DriveKitNative(int userId);

    std::mutex containMutex_;
    std::map<std::string, std::shared_ptr<DKContainer>> containers_;
    std::mutex appInfoMutex_;
    std::map<DKAppBundleName, DKAppInfo> appInfos_;
    static std::mutex drivekitMutex_;
    static std::map<int, std::shared_ptr<DriveKitNative>> driveKits_;
    DKUserInfo userInfo_;
    int userId_;
};
} // namespace DriveKit
#endif
