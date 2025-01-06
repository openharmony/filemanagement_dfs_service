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

#ifndef FILEMANAGEMENT_DFS_SERVICE_CONNECT_COUNT_H
#define FILEMANAGEMENT_DFS_SERVICE_CONNECT_COUNT_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <vector>

#include "ipc/i_file_dfs_listener.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
struct Connect {
    Connect(uint32_t callingTokenId, const std::string &networkId, int32_t num, const sptr<IFileDfsListener> &listener)
        : callingTokenId(callingTokenId), networkId(networkId), num(num), listener(listener) {}

    uint32_t callingTokenId;
    std::string networkId;
    int32_t num;
    sptr<IFileDfsListener> listener;
};

class ConnectCount final {
public:
    ConnectCount() = default;
    ~ConnectCount() = default;
    static std::shared_ptr<ConnectCount> GetInstance();

    void AddConnect(uint32_t callingTokenId, const std::string &networkId, sptr<IFileDfsListener> &listener);
    bool CheckCount(const std::string &networkId);
    void RemoveAllConnect();
    std::vector<std::string> RemoveConnect(uint32_t callingTokenId);
    void RemoveConnect(const std::string &networkId);
    void RemoveConnect(uint32_t callingTokenId, const std::string &networkId);
    std::vector<std::string> GetNetworkIds(uint32_t callingTokenId);
    void NotifyRemoteReverseObj(const std::string &networkId, int32_t status);
    int32_t FindCallingTokenIdForListerner(const sptr<IRemoteObject> &listener, uint32_t &callingTokenId);

private:
    static std::shared_ptr<ConnectCount> instance_;
    std::recursive_mutex connectMutex_;
    std::unordered_set<std::shared_ptr<Connect>> connectList_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_CONNECT_COUNT_H
