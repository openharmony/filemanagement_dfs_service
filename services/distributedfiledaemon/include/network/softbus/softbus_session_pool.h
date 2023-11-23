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
#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_POOL_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_POOL_H

#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftBusSessionPool {
public:
    struct SessionInfo {
        int sessionId{};
        std::string srcUri;
        std::string dstPath;
        int uid{};
    };

    static SoftBusSessionPool &GetInstance();
    std::string GenerateSessionName();
    void AddSessionInfo(const std::string &sessionName, const SessionInfo &sessionInfo);
    void DeleteSessionInfo(const std::string &sessionName);
    bool GetSessionInfo(const std::string &sessionName, SessionInfo &sessionInfo);

private:
    const std::string SESSION_NAME_PREFIX = "DistributedFileService";
    std::map<std::string, SessionInfo> sessionMap_;
    std::mutex sessionMutex_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_POOL_H