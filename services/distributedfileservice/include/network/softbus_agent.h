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

#include "dfsu_singleton.h"
#include "i_filetransfer_callback.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftbusAgent : public std::enable_shared_from_this<SoftbusAgent>, public Utils::DfsuSingleton<SoftbusAgent> {
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
    int SendFile(const std::string &cid, const std::vector<std::string> &sourceFileList,
        const std::vector<std::string> &destinationFileList, uint32_t fileCount);
    int SendFile(const std::string &cid, const char *sFileList[], const char *dFileList[], uint32_t fileCnt);
    void OnSendFileFinished(const int sessionId, const std::string firstFile);
    void OnFileTransError(const int sessionId);
    void OnReceiveFileFinished(const int sessionId, const std::string files, int fileCnt);
    void SetTransCallback(sptr<IFileTransferCallback> &callback);
    void RemoveTransCallbak();

protected:
    void StartInstance() override;
    void StopInstance() override;
    void OpenSession(const std::string &cid);
    void CloseSession(const std::string &cid);
    std::string GetPeerDevId(const int sessionId);
private:
    void ClearResources(char **fileList, int32_t &fileCount);
private:
    std::string sessionName_ { "DistributedFileService" };
    std::mutex sessionMapMux_;
    std::unordered_map<std::string, std::list<int>> cidToSessionID_;

    std::mutex getSessionCVMut_;
    std::condition_variable getSessionCV_;
    sptr<IFileTransferCallback> notifyCallback_ = nullptr;
    char **sFileList_ = nullptr;
    char **dFileList_ = nullptr;
    int32_t sFileCount_ = 0;
    int32_t dFileCount_ = 0;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DFS_SOFTBUS_AGENT_H