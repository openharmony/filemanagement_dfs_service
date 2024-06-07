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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SDK_ASSET_CALLBACK_MANANGER_H
#define FILEMANAGEMENT_DFS_SERVICE_SDK_ASSET_CALLBACK_MANANGER_H

#include <vector>
#include <map>
#include <mutex>

#include "asset/i_asset_recv_callback.h"
#include "asset/i_asset_send_callback.h"
#include "refbase.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

class AssetCallbackMananger {
public:
    static AssetCallbackMananger &GetInstance();

    void AddRecvCallback(const sptr<IAssetRecvCallback> &recvCallback);
    void RemoveRecvCallback(const sptr<IAssetRecvCallback> &recvCallback);
    void AddSendCallback(std::string taskId, const sptr<IAssetSendCallback> &sendCallback);
    void RemoveSendCallback(std::string taskId);

    void NotifyAssetRecvStart(const std::string &srcNetworkId,
                              const std::string &dstNetworkId,
                              const std::string &sessionId,
                              const std::string &dstBundleName);
    void NotifyAssetRecvFinished(const std::string &srcNetworkId,
                                 const sptr<AssetObj> &assetObj,
                                 int32_t result);
    void NotifyAssetSendResult(std::string &taskId,
                               const sptr<AssetObj> &assetObj,
                               int32_t result);

private:
    std::mutex recvCallbackListMutex_;
    std::vector<sptr<IAssetRecvCallback>> recvCallbackList_;
    std::mutex sendCallbackMapMutex_;
    std::map<std::string, sptr<IAssetSendCallback>> sendCallbackMap_;
};

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // FILEMANAGEMENT_DFS_SERVICE_SDK_ASSET_CALLBACK_MANANGER_H
