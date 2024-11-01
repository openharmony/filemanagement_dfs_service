/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef FILEMANAGEMENT_DFS_SERVICE_EVENTHANDLE_DAEMON_EVENT_H
#define FILEMANAGEMENT_DFS_SERVICE_EVENTHANDLE_DAEMON_EVENT_H

#include "asset/asset_obj.h"
#include "asset/i_asset_send_callback.h"
#include "block_object.h"
#include "hmdfs_info.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
typedef enum {
    DEAMON_EXECUTE_PUSH_ASSET = 1,
    DEAMON_EXECUTE_REQUEST_SEND_FILE,
    DEAMON_EXECUTE_PREPARE_SESSION,
} DaemonEventType;

struct PushAssetData {
    PushAssetData(int32_t userId, const sptr<AssetObj> &assetObj)
        : userId_(userId), assetObj_(assetObj)
    {
    }

    int32_t userId_;
    const sptr<AssetObj> assetObj_;
};

struct RequestSendFileData {
    RequestSendFileData(const std::string &srcUri,
                        const std::string &dstPath,
                        const std::string &dstDeviceId,
                        const std::string &sessionName,
                        const std::shared_ptr<BlockObject<int32_t>> &requestSendFileBlock)
        : srcUri_(srcUri),
          dstPath_(dstPath),
          dstDeviceId_(dstDeviceId),
          sessionName_(sessionName),
          requestSendFileBlock_(requestSendFileBlock)
    {
    }

    const std::string srcUri_;
    const std::string dstPath_;
    const std::string dstDeviceId_;
    const std::string sessionName_;
    const std::shared_ptr<BlockObject<int32_t>> requestSendFileBlock_;
};

struct PrepareSessionData {
    PrepareSessionData(const std::string &srcUri,
                       const std::string &physicalPath,
                       const std::string &sessionName,
                       const sptr<IDaemon> daemon,
                       HmdfsInfo &info,
                       const std::shared_ptr<BlockObject<int32_t>> &prepareSessionBlock)
        : srcUri_(srcUri),
          physicalPath_(physicalPath),
          sessionName_(sessionName),
          daemon_(daemon),
          info_(info),
          prepareSessionBlock_(prepareSessionBlock)
    {
    }

    const std::string srcUri_;
    const std::string physicalPath_;
    const std::string sessionName_;
    const sptr<IDaemon> daemon_;
    HmdfsInfo &info_;
    const std::shared_ptr<BlockObject<int32_t>> prepareSessionBlock_;
};

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // FILEMANAGEMENT_DFS_SERVICE_EVENTHANDLE_DAEMON_EVENT_H
