/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef TEST_OHOS_FILEMANAGEMENT_CLOUDDISKSERVICE_ICLOUDDISKSERVICE_H
#define TEST_OHOS_FILEMANAGEMENT_CLOUDDISKSERVICE_ICLOUDDISKSERVICE_H

#include <cstdint>
#include <vector>
#include <iremote_broker.h>
#include <string_ex.h>
#include "cloud_disk_common.h"
#include "hilog/log.h"

using OHOS::FileManagement::CloudDiskService::ChangesResult;
using OHOS::FileManagement::CloudDiskService::FileSyncState;
using OHOS::FileManagement::CloudDiskService::FailedList;
using OHOS::FileManagement::CloudDiskService::ResultList;

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {

enum class ICloudDiskServiceIpcCode {
    COMMAND_REGISTER_SYNC_FOLDER_CHANGES_INNER = MIN_TRANSACTION_ID,
    COMMAND_UNREGISTER_SYNC_FOLDER_CHANGES_INNER,
    COMMAND_GET_SYNC_FOLDER_CHANGES_INNER,
    COMMAND_SET_FILE_SYNC_STATES_INNER,
    COMMAND_GET_FILE_SYNC_STATES_INNER,
    COMMAND_REGISTER_SYNC_FOLDER_INNER,
    COMMAND_UNREGISTER_SYNC_FOLDER_INNER,
    COMMAND_UNREGISTER_FOR_SA_INNER,
};

class ICloudDiskService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.FileManagement.CloudDiskService.ICloudDiskService");

    virtual ErrCode RegisterSyncFolderChangesInner(
        const std::string& syncFolder,
        const sptr<IRemoteObject>& remoteobject) = 0;
    
    virtual ErrCode UnregisterSyncFolderChangesInner(
        const std::string& syncFolder) = 0;

    virtual ErrCode GetSyncFolderChangesInner(
        const std::string& syncFolder,
        uint64_t count,
        uint64_t startUSN,
        ChangesResult& funcResult) = 0;

    virtual ErrCode SetFileSyncStatesInner(
        const std::string& syncFolder,
        const std::vector<FileSyncState>& fileSyncStates,
        std::vector<FailedList>& funcResult) = 0;

    virtual ErrCode GetFileSyncStatesInner(
        const std::string& syncFolder,
        const std::vector<std::string>& pathArray,
        std::vector<ResultList>& funcResult) = 0;

    virtual ErrCode RegisterSyncFolderInner(
        int32_t userId,
        const std::string& bundleName,
        const std::string& path) = 0;

    virtual ErrCode UnregisterSyncFolderInner(
        int32_t userId,
        const std::string& bundleName,
        const std::string& path) = 0;

    virtual ErrCode UnregisterForSaInner(
        const std::string& path) = 0;
protected:
    static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, 0xD003900, "CloudDiskService"};
    const int VECTOR_MAX_SIZE = 102400;
    const int LIST_MAX_SIZE = 102400;
    const int SET_MAX_SIZE = 102400;
    const int MAP_MAX_SIZE = 102400;
};
} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS
#endif // TEST_OHOS_FILEMANAGEMENT_CLOUDDISKSERVICE_ICLOUDDISKSERVICE_H
