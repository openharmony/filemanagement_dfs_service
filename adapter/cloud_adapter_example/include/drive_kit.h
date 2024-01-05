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
enum class DKCloudSyncDemon {
    DK_CLOUD_DATA = 0, //数据子系统加载时传入
    DK_CLOUD_FILE, //文件子系统加载时传入
    DK_CLOUD_UNKNOWN,
};
struct DKRecordChangeEvent {
    std::string accountId;
    DKAppBundleName appBundleName;
    DKContainerName containerName;
    std::vector<DKDatabaseScope> databaseScopes;
    std::vector<DKRecordType> recordTypes;
    std::map<std::string, std::string> properties;
};
struct DKUserInfo {
    std::string accountId;     //云空间用户id
    DKCloudStatus cloudStatus; //云空间登录状态
    DKSpaceStatus spaceStatus; //云空间存储状态
    uint64_t totalSpace;       //用户总的云空间
    uint64_t remainSpace;      //用户剩余云空间
    std::string pushKeyId;
    std::string pushKeyValue;
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
    static std::shared_ptr<DriveKitNative> GetInstance(int userId, DKCloudSyncDemon syncDemon);
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
    //校验和解析通知消息，调用该函数前，要先调用GetCloudUserInfo,否则会报错
    DKError ResolveNotificationEvent(const std::string &extraData, DKRecordChangeEvent &event);
    int32_t OnUploadAsset(const std::string &request, const std::string &result);
    void ReleaseDefaultContainer(DKAppBundleName bundleName);
    void ReleaseContainer(DKAppBundleName bundleName, DKContainerName containerName);
    void ReleaseCloudUserInfo();
    void CleanCloudUserInfo();
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
    DKCloudSyncDemon syncDemon_;
};
} // namespace DriveKit
#endif
