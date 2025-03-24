/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_CORE_H
#define OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_CORE_H

#include "cloud_sync_common.h"
#include "filemgmt_libfs.h"

namespace OHOS::FileManagement::CloudSync {
using namespace ModuleFileIO;

class CloudSyncManagerCore {
public:
    static FsResult<void> DoChangeAppCloudSwitch(
        const std::string &accountId, const std::string &bundleName, bool status);
    static FsResult<void> DoClean(const std::string &accoutId, const CleanOptions &cleanOptions);
    static FsResult<void> DoNotifyDataChange(const std::string &accountId, const std::string &bundleName);
    static FsResult<void> DoNotifyEventChange(
        int32_t userId, const std::string &eventId, const std::string &extraData);
    static FsResult<void> DoEnableCloud(const std::string &accoutId, const SwitchDataObj &switchData);
    static FsResult<void> DoDisableCloud(const std::string &accoutId);
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_CORE_H