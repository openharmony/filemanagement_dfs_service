/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef DFS_SOFTBUS_AGENT_H
#define DFS_SOFTBUS_AGENT_H

#include <condition_variable>
#include <list>
#include <unordered_map>
#include "utils_singleton.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftbusAgent : public std::enable_shared_from_this<SoftbusAgent>, public Utils::Singleton<SoftbusAgent> {
    DECLARE_SINGLETON(SoftbusAgent);

public:
    void RegisterSessionListener();
    void RegisterFileListener();
    void UnRegisterSessionListener();
    void OnDeviceOnline(const std::string &cid);
    void OnDeviceOffline(const std::string &cid);
    void AllDevicesOffline();
    void OnSessionOpened(const int sessionId, const int result);
    void OnSessionClosed(int sessionId);
    int SendFile(const std::string &cid, const char *sFileList[], const char *dFileList[], uint32_t fileCnt);
    int OnSendFileFinished(const int sessionId, const std::string firstFile);
    int OnFileTransError(const int sessionId);
    void OnReceiveFileFinished(const int sessionId, const std::string files, int fileCnt);

protected:
    void StartInstance() override;
    void StopInstance() override;
    void OpenSession(const std::string &cid);
    int CloseSession(const std::string &cid);
    std::string GetPeerDevId(const int sessionId);

private:
    std::string sessionName_ { "DistributedFileService" };
    std::mutex sessionMapMux_;
    std::unordered_map<std::string, std::list<int>> cidToSessionID_;

    std::mutex getSessionCVMut_;
    std::condition_variable getSessionCV_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DFS_SOFTBUS_AGENT_H