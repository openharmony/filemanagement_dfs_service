/*
* Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "asset_callback_mananger.h"
#include "dfs_error.h"
#include "network/softbus/softbus_handler_asset.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
bool SoftBusAssetSendListener::isSingleFile_;
void SoftBusAssetSendListener::OnFile(int32_t socket, FileEvent *event)
{
    if (event == nullptr) {
        LOGE("invalid paramter");
        return;
    }
    switch (event->type) {
        case FILE_EVENT_SEND_FINISH:
            OnSendAssetFinished(socket, event->files);
            break;
        case FILE_EVENT_SEND_ERROR:
            OnSendAssetError(socket, event->files, event->errorCode);
            break;
        default:
            LOGI("Other situations");
            break;
    }
}
void SoftBusAssetSendListener::OnSendAssetFinished(int32_t socketId, const char **fileList)
{
    LOGE("Push asset finished.");
    auto assetObj = SoftBusHandlerAsset::GetInstance().GetAssetObj(socketId);
    if (assetObj == nullptr) {
        LOGE("OnSendAssetFinished get assetObj is nullptr");
        return;
    }
    auto taskId = assetObj->srcBundleName_ + assetObj->sessionId_;
    AssetCallbackMananger::GetInstance().NotifyAssetSendResult(taskId, assetObj, FileManagement::E_OK);
    SoftBusHandlerAsset::GetInstance().closeAssetBind(socketId);
    AssetCallbackMananger::GetInstance().RemoveSendCallback(taskId);
    SoftBusHandlerAsset::GetInstance().RemoveFile(fileList[0], !SoftBusAssetSendListener::isSingleFile_);
}

void SoftBusAssetSendListener::OnSendAssetError(int32_t socketId, const char **fileList, int32_t errorCode)
{
    LOGE("SendAssetError errorCode %{public}d", errorCode);
    auto assetObj = SoftBusHandlerAsset::GetInstance().GetAssetObj(socketId);
    if (assetObj == nullptr) {
        LOGE("OnSendAssetError  get assetObj is nullptr");
        return;
    }
    auto taskId = assetObj->srcBundleName_ + assetObj->sessionId_;
    AssetCallbackMananger::GetInstance().NotifyAssetSendResult(taskId, assetObj, FileManagement::E_SEND_FILE);
    SoftBusHandlerAsset::GetInstance().closeAssetBind(socketId);
    AssetCallbackMananger::GetInstance().RemoveSendCallback(taskId);
    SoftBusHandlerAsset::GetInstance().RemoveFile(fileList[0], !SoftBusAssetSendListener::isSingleFile_);
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS