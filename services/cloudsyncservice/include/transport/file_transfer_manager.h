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

#ifndef OHOS_FILEMGMT_FILE_TRANSFER_MANAGER_H
#define OHOS_FILEMGMT_FILE_TRANSFER_MANAGER_H

#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "message_handler.h"
#include "session_manager.h"
namespace OHOS::FileManagement::CloudSync {
class FileTransferManager : public RecieveDataHandler, public std::enable_shared_from_this<FileTransferManager> {
public:
    explicit FileTransferManager(std::shared_ptr<SessionManager> sessionManager);
    ~FileTransferManager(){};

    void Init();

    void DownloadFileFromRemoteDevice(const std::string &networkId,
                                      const int32_t userId,
                                      const uint64_t taskId,
                                      const std::string &uri);
    void OnMessageHandle(const std::string &senderNetworkId,
                         int receiverSessionId,
                         const void *data,
                         unsigned int dataLen) override;
    void OnFileRecvHandle(const std::string &senderNetworkId, const char *filePath, int result) override;
    void OnSessionClosed() override;
    void HandleDownloadFileRequest(MessageHandler &msgHandler,
                                  const std::string &senderNetworkId,
                                  int receiverSessionId);
    void HandleDownloadFileResponse(MessageHandler &msgHandler);
    void HandleRecvFileFinished();

private:
    struct TaskInfo {
        std::string uri;
        std::string relativePath;
        uint64_t taskId;
    };
    bool IsFileExists(const std::string &filePath);
    std::tuple<std::string, std::string>
        UriToPath(const std::string &uri, const int32_t userId, bool isCheckFileExists = true);
    void AddTransTask(const std::string &uri, const int32_t userId, uint64_t taskId);
    void FinishTransTask(const std::string &relativePath, int result);
    void RemoveTransTask(uint64_t taskId);
    void IncTransTaskCount();
    void DecTransTaskCount();
    std::mutex taskMutex_;
    std::vector<TaskInfo> taskInfos_;
    std::atomic<int> taskCount_{0};

    std::shared_ptr<SessionManager> sessionManager_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_FILE_TRANSFER_MANAGER_H