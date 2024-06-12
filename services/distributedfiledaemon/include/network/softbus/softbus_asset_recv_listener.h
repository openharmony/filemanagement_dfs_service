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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_ASSET_RECV_LISTENER_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_ASSET_RECV_LISTENER_H

#include <cstdint>
#include <map>
#include <mutex>
#include <string>

#include "asset/asset_obj.h"
#include "transport/socket.h"
#include "transport/trans_type.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftbusAssetRecvListener {
public:
    static void OnFile(int32_t socket, FileEvent *event);
    static const char* GetRecvPath();

    static void OnRecvAssetStart(int32_t socketId, const char **fileList, int32_t fileCnt);
    static void OnRecvAssetFinished(int32_t socketId, const char **fileList, int32_t fileCnt);
    static void OnRecvAssetError(int32_t socketId, int32_t errorCode);

    static void OnAssetRecvBind(int32_t sessionId, PeerSocketInfo info);

private:
    static int32_t HandleOneFile(int32_t socketId, const std::string &filePath, const sptr<AssetObj> &assetObj);
    static int32_t HandleMoreFile(int32_t socketId, const std::string &filePath, const sptr<AssetObj> &assetObj);
    static bool JudgeSingleFile(const std::string &filePath);

    static int32_t GetCurrentUserId();
    static bool MoveAsset(const std::vector<std::string> &fileList, bool isSingleFile);
    static bool RemoveAsset(const std::string &file);
    static inline const std::string SERVICE_NAME{"ohos.storage.distributedfile.daemon"};
    static inline std::string path_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_ASSET_RECV_LISTENER_H
