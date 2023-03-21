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
#include "dk_cloud_callback.h"
#include "dk_container.h"
#include "dk_error.h"
#include "dk_record.h"
#include <map>
#include <mutex>
#include <string>
#include <time.h>
#include <vector>

namespace DriveKit {
struct DKSchemaField {
    DKFieldKey name;
    DKRecordFieldType type;
};
struct DKSchemaNode {
    DKRecordType recordType;
    std::map<DKFieldKey, DKSchemaField> fields;
};
struct DKSchema {
    std::map<DKRecordType, DKSchemaNode> schemaNodes;
};
struct DKContainerInfo {
    DKAppBundleName bundleName;
    DKContainerName containerName;
    int schemaVersion;
    DKSchema schema;
};
struct DKAppInfo {
    DKAppBundleName bundleName;
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
    std::string cloudUserId;
    DKCloudStatus cloudStatus;
    DKSpaceStatus spaceStatus;
    uint64_t totalSpace;
    uint64_t remainSpace;
};
class DriveKit {
public:
    std::shared_ptr<DKContainer> GetDefaultContainer(DKAppBundleName bundleName);
    std::shared_ptr<DKContainer> GetContainer(DKAppBundleName bundleName, DKContainerName containerName);
    DKError GetCloudUserInfo(DKUserInfo &userInfo);
    DKError GetCloudAppInfo(const std::vector<DKAppBundleName> &bundleNames,
                            std::map<DKAppBundleName, DKAppInfo> &appinfos);
    DKError GetCloudAppSwitches(const std::vector<DKAppBundleName> &bundleNames,
                               std::map<DKAppBundleName, DKAppSwitchStatus> &appSwitchs);
    DKError GetServerTime(time_t &time);
    DKError GetSchemaRawData(const DKAppBundleName &bundleNames, std::string &schemaData);
    DKError GetSchemaRecordTypes(const DKAppBundleName &bundleNames, std::vector<DKRecordType> recordType);

private:
    std::mutex containMutex_;
    std::map<std::string, std::shared_ptr<DKContainer>> containers_;
    std::map<DKAppBundleName, DKAppInfo> appInfos_;
    static std::mutex drivekitMutex_;
    static std::map<int, std::shared_ptr<DriveKit>> driveKits_;
    DKUserInfo userInfo_;
    int userId_;

public:
    DriveKit(const DriveKit &) = delete;
    DriveKit(DriveKit &&) = delete;
    DriveKit &operator=(const DriveKit &) = delete;
    DriveKit &operator=(DriveKit &&) = delete;
    static std::shared_ptr<DriveKit> getInstance(int userId);
    virtual ~DriveKit();

private:
    DriveKit(int userId);
};
}; // namespace DriveKit
#endif
