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

#include "file_transfer_manager.h"

#include <cinttypes>

#include "dfs_error.h"
#include "ipc/download_asset_callback_manager.h"
#include "sandbox_helper.h"
#include "task_state_manager.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
static const string CALLER_NAME = "distributeddata";

FileTransferManager::FileTransferManager(std::shared_ptr<SessionManager> sessionManager)
    : sessionManager_(sessionManager)
{
}

void FileTransferManager::Init()
{
    sessionManager_->RegisterDataHandler(shared_from_this());
}

void FileTransferManager::DownloadFileFromRemoteDevice(const std::string &networkId,
                                                       const int32_t userId,
                                                       const uint64_t taskId,
                                                       const std::string &uri)
{
    IncTransTaskCount();
    MessageInputInfo info = {.srcNetworkId = "",
                             .dstNetworkId = networkId,
                             .uri = uri,
                             .msgType = MSG_DOWNLOAD_FILE_REQ,
                             .errorCode = 0,
                             .userId = userId,
                             .taskId = taskId};
    MessageHandler msgHandler(info);
    uint32_t dataLen = msgHandler.GetDataSize();
    auto data = make_unique<uint8_t[]>(dataLen);
    msgHandler.PackData(data.get(), dataLen);
    LOGI("send data, dataLen:%{public}d, taskId: %{public}" PRIu64 "", dataLen, taskId);
    auto ret = sessionManager_->SendData(networkId, data.get(), dataLen);
    if (ret != E_OK) {
        LOGE("download file failed, uri:%{public}s, ret:%{public}d", uri.c_str(), ret);
        DownloadAssetCallbackManager::GetInstance().OnDownloadFinshed(taskId, uri, ret);
        DecTransTaskCount();
    } else {
        AddTransTask(uri, userId, taskId);
    }
}

void FileTransferManager::HandleDownloadFileRequest(MessageHandler &msgHandler,
                                                    const std::string &senderNetworkId,
                                                    int receiverSessionId)
{
    IncTransTaskCount();
    auto uri = msgHandler.GetUri();
    auto userId = msgHandler.GetUserId();
    auto [physicalPath, relativePath] = UriToPath(uri, userId);
    uint32_t errorCode = E_OK;
    if (!relativePath.empty()) {
        auto result = sessionManager_->SendFile(senderNetworkId, {physicalPath}, {relativePath});
        if (result != E_OK) {
            LOGE("send file failed, relativePath:%{public}s, ret:%{public}d", relativePath.c_str(), result);
            errorCode = E_SEND_FILE;
            DecTransTaskCount();
        }
    } else {
        errorCode = E_FILE_NOT_EXIST;
        DecTransTaskCount();
    }
    auto taskId = msgHandler.GetTaskId();
    MessageInputInfo info = {.srcNetworkId = "",
                             .dstNetworkId = senderNetworkId,
                             .uri = uri,
                             .msgType = MSG_DOWNLOAD_FILE_RSP,
                             .errorCode = errorCode,
                             .userId = userId,
                             .taskId = taskId};
    MessageHandler resp(info);
    uint32_t dataLen = resp.GetDataSize();
    auto data = make_unique<uint8_t[]>(dataLen);
    resp.PackData(data.get(), dataLen);
    auto ret = sessionManager_->SendData(receiverSessionId, data.get(), dataLen);
    if (ret != E_OK) {
        LOGE("response failed: %{public}d, sessionId:%{public}d", ret, receiverSessionId);
    }
    LOGD("send response, sessionId:%{public}d", receiverSessionId);
}

void FileTransferManager::HandleDownloadFileResponse(MessageHandler &msgHandler)
{
    LOGD("recviceve response msg");
    auto errorCode = msgHandler.GetErrorCode();
    if (errorCode == E_OK) {
        LOGD("callback after file recv finished");
        return;
    }
    auto uri = msgHandler.GetUri();
    auto taskId = msgHandler.GetTaskId();
    DownloadAssetCallbackManager::GetInstance().OnDownloadFinshed(taskId, uri, errorCode);
    DecTransTaskCount();
    RemoveTransTask(taskId);
}

void FileTransferManager::HandleRecvFileFinished()
{
    LOGI("file transfer finished");
    DecTransTaskCount(); // task finished, may be can unload sa
}

void FileTransferManager::OnMessageHandle(const std::string &senderNetworkId,
                                          int receiverSessionId,
                                          const void *data,
                                          unsigned int dataLen)
{
    MessageHandler msgHandler;
    bool ret = msgHandler.UnPackData((uint8_t *)data, dataLen);
    if (!ret) {
        LOGE("package data invalid");
        return;
    }
    auto msgType = msgHandler.GetMsgType();
    if (msgType == MSG_DOWNLOAD_FILE_REQ) {
        HandleDownloadFileRequest(msgHandler, senderNetworkId, receiverSessionId);
    } else if (msgType == MSG_DOWNLOAD_FILE_RSP) {
        HandleDownloadFileResponse(msgHandler);
    } else if (msgType == MSG_FINISH_FILE_RECV) {
        HandleRecvFileFinished();
    } else {
        LOGE("error msg type:%{public}d", msgType);
    }
}

void FileTransferManager::OnFileRecvHandle(const std::string &senderNetworkId, const char *filePath, int result)
{
    LOGE("received file, file path:%{public}s, result:%{public}d", filePath, result);
    DecTransTaskCount(); // allways dec task count when finsh one task
    if (filePath != nullptr) {
        FinishTransTask(string(filePath), result);
    }

    MessageInputInfo info = {.msgType = MSG_FINISH_FILE_RECV};
    MessageHandler req(info);
    uint32_t dataLen = req.GetDataSize();
    auto data = make_unique<uint8_t[]>(dataLen);
    req.PackData(data.get(), dataLen);
    auto ret = sessionManager_->SendData(senderNetworkId, data.get(), dataLen);
    if (ret != E_OK) {
        LOGE("response failed: %{public}d", ret);
    }
    LOGI("send file recv finished msg");
}

void FileTransferManager::OnSessionClosed() // avoid sa cannot unload when session disconnect
{
    taskCount_.store(0);
    TaskStateManager::GetInstance().CompleteTask(CALLER_NAME, TaskType::DOWNLOAD_REMOTE_ASSET_TASK);
}

bool FileTransferManager::IsFileExists(const std::string &filePath)
{
    if (access(filePath.c_str(), F_OK) != E_OK) {
        LOGE("file is not exist, path:%{public}s, error:%{public}s", filePath.c_str(), strerror(errno));
        return false;
    }
    return true;
}

std::tuple<std::string, std::string> FileTransferManager::UriToPath(const std::string &uri,
                                                                    const int32_t userId,
                                                                    bool isCheckFileExists)
{
    string physicalPath = "";
    int ret = AppFileService::SandboxHelper::GetPhysicalPath(uri, std::to_string(userId), physicalPath);
    if (ret != 0) {
        LOGE("Get physical path failed with %{public}d", ret);
        return {"", ""};
    }

    if (isCheckFileExists) {
        if (!this->IsFileExists(physicalPath)) {
            return {"", ""};
        }
    }

    const string HMDFS_DIR = "/mnt/hmdfs/";
    const string DATA_DIR = HMDFS_DIR + to_string(userId) + "/account/device_view/local/data";

    std::string relativePath;
    size_t fileDirPos = physicalPath.find(DATA_DIR);
    if (fileDirPos == std::string::npos) {
        return {"", ""};
    }
    fileDirPos += DATA_DIR.length();
    relativePath = physicalPath.substr(fileDirPos);

    return {physicalPath, relativePath};
}

void FileTransferManager::AddTransTask(const std::string &uri, const int32_t userId, uint64_t taskId)
{
    lock_guard<mutex> lock(taskMutex_);
    auto [physicalPath, relativePath] = UriToPath(uri, userId, false);
    TaskInfo info = {uri, relativePath, taskId};
    taskInfos_.push_back(info);
}

void FileTransferManager::RemoveTransTask(uint64_t taskId)
{
    LOGI("remove task:%{public}" PRIu64 "", taskId);
    lock_guard<mutex> lock(taskMutex_);
    for (auto iter = taskInfos_.begin(); iter != taskInfos_.end();) {
        if ((*iter).taskId == taskId) {
            iter = taskInfos_.erase(iter);
        } else {
            ++iter;
        }
    }
}

void FileTransferManager::FinishTransTask(const std::string &relativePath, int result)
{
    lock_guard<mutex> lock(taskMutex_);
    for (auto iter = taskInfos_.begin(); iter != taskInfos_.end();) {
        if ((*iter).relativePath == relativePath) {
            DownloadAssetCallbackManager::GetInstance().OnDownloadFinshed((*iter).taskId, (*iter).uri, result);
            iter = taskInfos_.erase(iter);
            return; // match the first one
        } else {
            ++iter;
        }
    }
    LOGE("not found task, relativePath:%{public}s", relativePath.c_str());
}

void FileTransferManager::IncTransTaskCount()
{
    TaskStateManager::GetInstance().StartTask(CALLER_NAME, TaskType::DOWNLOAD_REMOTE_ASSET_TASK);
    taskCount_.fetch_add(1);
}

void FileTransferManager::DecTransTaskCount()
{
    auto count = taskCount_.fetch_sub(1);
    if (count == 1) {
        TaskStateManager::GetInstance().CompleteTask(CALLER_NAME, TaskType::DOWNLOAD_REMOTE_ASSET_TASK);
    } else if (count < 1) {
        taskCount_.store(0);
    }
}
} // namespace OHOS::FileManagement::CloudSync
