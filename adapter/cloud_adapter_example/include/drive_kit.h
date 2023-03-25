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
using DKAppId = std::string;
using DKDatabaseName = std::string;
using DKSchemaRawData = std::string;
struct DKSchemaField {
    DKFieldKey name;
    DKRecordFieldType type;
    bool primary;
    bool nullable;
    bool sortable;
    bool searchable;
    bool queryable;
    DKRecordFieldType listType;
    DKRecordType refRecordType;
};
struct DKSchemaNode {
    DKRecordType recordType;
    std::string tableName;
    std::map<DKFieldKey, DKSchemaField> fields;
    std::vector<DKFieldKey> dupCheckFields;
};
struct DKOrderTable {
    DKRecordType recordType;
    std::string tableName;
};
struct DKSchema {
    int version;
    std::map<DKRecordType, DKSchemaNode> recordTypes;
    DKSchemaRawData schemaData;
    std::vector<DKOrderTable> orderTables;
};
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
class DriveKit : public std::enable_shared_from_this<DriveKit> {
    friend class DKContainer;
    friend class DKDatabase;

public:
    DriveKit(const DriveKit &) = delete;
    DriveKit(DriveKit &&) = delete;
    DriveKit &operator=(const DriveKit &) = delete;
    DriveKit &operator=(DriveKit &&) = delete;
    static std::shared_ptr<DriveKit> getInstance(int userId);
    virtual ~DriveKit();

    std::shared_ptr<DKContainer> GetDefaultContainer(DKAppBundleName bundleName);
    std::shared_ptr<DKContainer> GetContainer(DKAppBundleName bundleName, DKContainerName containerName);
    DKError GetCloudUserInfo(DKUserInfo &userInfo);
    DKError GetCloudAppInfo(const std::vector<DKAppBundleName> &bundleNames,
                            std::map<DKAppBundleName, DKAppInfo> &appInfos);
    DKError GetCloudAppSwitches(const std::vector<DKAppBundleName> &bundleNames,
                                std::map<DKAppBundleName, DKAppSwitchStatus> &appSwitchs);
    DKError GetServerTime(time_t &time);

private:
    DriveKit(int userId);

    std::mutex containMutex_;
    std::map<std::string, std::shared_ptr<DKContainer>> containers_;
    std::mutex appInfoMutex_;
    std::map<DKAppBundleName, DKAppInfo> appInfos_;
    static std::mutex drivekitMutex_;
    static std::map<int, std::shared_ptr<DriveKit>> driveKits_;
    DKUserInfo userInfo_;
    int userId_;
};
} // namespace DriveKit
#endif
