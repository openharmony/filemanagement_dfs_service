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

#ifndef DRIVE_KIT_CLOUD_CALLBACK_H
#define DRIVE_KIT_CLOUD_CALLBACK_H

#include <map>
#include <string>
#include <vector>

namespace DriveKit {
enum class DKCloudStatus {
    DK_CLOUD_STATUS_UNKNOWN = 0,
    DK_CLOUD_STATUS_UNACTIVATED,
    DK_CLOUD_STATUS_LOGIN,
    DK_CLOUD_STATUS_LOGOUT,
    DK_CLOUD_STATUS_MAX,
};

enum class DKAppSwitchStatus {
    DK_APP_SWITCH_STATUS_CLOSE = 0,
    DK_APP_SWITCH_STATUS_OPEN,
    DK_APP_SWITCH_STATUS_NOTENABLE, // 未启用
    DK_APP_SWITCH_STATUS_MAX,
};

enum class DKAppDataClearPolicy {
    DK_CLEAR_CLOUD_DATA = 0,
    DK_RESERVE_CLOUD_DATA,
};

enum class DKRecordsChangeType {
    DK_RECORDS_CHANGE = 0,
    DK_SCHEMA_CHANGE,
};

using DKAppBundleName = std::string;
using DKContainerName = std::string;
} // namespace DriveKit
#endif
