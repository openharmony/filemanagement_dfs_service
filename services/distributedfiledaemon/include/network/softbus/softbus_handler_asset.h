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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_ASSET_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_ASSET_H

#include "transport/socket.h"
#include "transport/trans_type.h"
#include <map>
#include <mutex>
#include <cstdint>
#include <string>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
typedef enum {
    DFS_ASSET_ROLE_SEND = 0,
    DFS_ASSET_ROLE_RECV = 1,
} DFS_ASSET_ROLE;

class SoftBusHandlerAsset {
public:
    SoftBusHandlerAsset();
    ~SoftBusHandlerAsset();
    static SoftBusHandlerAsset &GetInstance();
    void CreateAssetLocalSessionServer();
    void DeleteAssetLocalSessionServer();

    static bool IsSameAccount(const std::string &networkId);
private:
    std::mutex serverIdMapMutex_;
    std::map<std::string, int32_t> serverIdMap_;
    std::map<DFS_ASSET_ROLE, ISocketListener> sessionListener_;
    static inline const std::string SERVICE_NAME {"ohos.storage.distributedfile.daemon"};
    static inline const std::string ASSET_LOCAL_SESSION_NAME {"DistributedFileService_assetListener"};
};

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_ASSET_H
