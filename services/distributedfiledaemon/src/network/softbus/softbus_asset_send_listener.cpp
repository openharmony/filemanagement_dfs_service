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

#include "network/softbus/softbus_asset_send_listener.h"

#include "asset_callback_manager.h"
#include "dfs_error.h"
#include "network/softbus/softbus_handler_asset.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
std::mutex SoftBusAssetSendListener::taskIsSingleFileMapMutex_;
std::map<std::string, bool> SoftBusAssetSendListener::taskIsSingleFileMap_;
void SoftBusAssetSendListener::OnFile(int32_t socket, FileEvent *event)
{
    if (event == nullptr) {
        LOGE("invalid paramter");
        return;
    }
    switch (event->type) {
        case FILE_EVENT_SEND_FINISH:
            OnSendAssetFinished(socket, event->files, event->fileCnt);
            break;
        case FILE_EVENT_SEND_ERROR:
            OnSendAssetError(socket, event->files, event->fileCnt, event->errorCode);
            break;
        default:
            LOGI("Other situations");
            break;
    }
}

void SoftBusAssetSendListener::OnSendAssetFinished(int32_t socketId, const char **fileList, int32_t fileCnt)
{
    LOGI("Push asset finished, socketId is %{public}d", socketId);
    if (fileCnt == 0) {
        LOGE("fileList has no file");
        return;
    }
    auto assetObj = SoftBusHandlerAsset::GetInstance().GetAssetObj(socketId);
    if (assetObj == nullptr) {
        LOGE("OnSendAssetFinished get assetObj is nullptr");
        return;
    }
    auto taskId = assetObj->srcBundleName_ + assetObj->sessionId_;
    AssetCallbackManager::GetInstance().NotifyAssetSendResult(taskId, assetObj, FileManagement::E_OK);
    SoftBusHandlerAsset::GetInstance().closeAssetBind(socketId);
    AssetCallbackManager::GetInstance().RemoveSendCallback(taskId);
    SoftBusHandlerAsset::GetInstance().RemoveFile(fileList[0], GetIsZipFile(taskId));
    RemoveFileMap(taskId);
}

void SoftBusAssetSendListener::OnSendAssetError(int32_t socketId,
                                                const char **fileList,
                                                int32_t fileCnt,
                                                int32_t errorCode)
{
    LOGE("SendAssetError, socketId is %{public}d, errorCode %{public}d", socketId, errorCode);
    if (fileCnt == 0) {
        LOGE("fileList has no file");
        return;
    }
    auto assetObj = SoftBusHandlerAsset::GetInstance().GetAssetObj(socketId);
    if (assetObj == nullptr) {
        LOGE("OnSendAssetError  get assetObj is nullptr");
        return;
    }
    auto taskId = assetObj->srcBundleName_ + assetObj->sessionId_;
    AssetCallbackManager::GetInstance().NotifyAssetSendResult(taskId, assetObj, FileManagement::E_SEND_FILE);
    SoftBusHandlerAsset::GetInstance().closeAssetBind(socketId);
    AssetCallbackManager::GetInstance().RemoveSendCallback(taskId);
    SoftBusHandlerAsset::GetInstance().RemoveFile(fileList[0], GetIsZipFile(taskId));
    RemoveFileMap(taskId);
}

void SoftBusAssetSendListener::OnSendShutdown(int32_t sessionId, ShutdownReason reason)
{
    LOGI("OnSessionClosed, sessionId = %{public}d, reason = %{public}d", sessionId, reason);
}
void SoftBusAssetSendListener::AddFileMap(const std::string &taskId, bool isSingleFile)
{
    std::lock_guard<std::mutex> lock(taskIsSingleFileMapMutex_);
    auto it = taskIsSingleFileMap_.find(taskId);
    if (it != taskIsSingleFileMap_.end()) {
        LOGE("taskId already exist, %{public}s", taskId.c_str());
        return;
    }
    taskIsSingleFileMap_.insert(std::make_pair(taskId, isSingleFile));
}

bool SoftBusAssetSendListener::GetIsZipFile(const std::string &taskId)
{
    std::lock_guard<std::mutex> lock(taskIsSingleFileMapMutex_);
    auto it = taskIsSingleFileMap_.find(taskId);
    if (it == taskIsSingleFileMap_.end()) {
        LOGI("taskId not exist, %{public}s", taskId.c_str());
        return false;
    }
    return !it->second;
}

void SoftBusAssetSendListener::RemoveFileMap(const std::string &taskId)
{
    std::lock_guard<std::mutex> lock(taskIsSingleFileMapMutex_);
    auto it = taskIsSingleFileMap_.find(taskId);
    if (it == taskIsSingleFileMap_.end()) {
        LOGI("taskId not exist, %{public}s", taskId.c_str());
        return;
    }
    taskIsSingleFileMap_.erase(it);
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS