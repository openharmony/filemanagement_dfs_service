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

#include "asset_callback_mananger.h"
#include "network/softbus/softbus_handler_asset.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

AssetCallbackMananger &AssetCallbackMananger::GetInstance()
{
    static AssetCallbackMananger instance;
    return instance;
}

void AssetCallbackMananger::AddRecvCallback(const sptr<IAssetRecvCallback> &recvCallback)
{
    std::lock_guard<std::mutex> lock(recvCallbackListMutex_);
    for (auto callback : recvCallbackList_) {
        if (recvCallback->AsObject() == callback->AsObject()) {
            LOGI("recvCallback registered!");
            return;
        }
    }
    recvCallbackList_.emplace_back(recvCallback);
}

void AssetCallbackMananger::RemoveRecvCallback(const sptr<IAssetRecvCallback> &recvCallback)
{
    std::lock_guard<std::mutex> lock(recvCallbackListMutex_);
    for (auto iter = recvCallbackList_.begin(); iter != recvCallbackList_.end();) {
        if ((*iter)->AsObject() == recvCallback->AsObject()) {
            iter = recvCallbackList_.erase(iter);
        } else {
            iter++;
        }
    }
}

void AssetCallbackMananger::AddSendCallback(std::string taskId, const sptr<IAssetSendCallback> &sendCallback)
{
    std::lock_guard<std::mutex> lock(sendCallbackMapMutex_);
    auto iter = sendCallbackMap_.find(taskId);
    if (iter != sendCallbackMap_.end()) {
        LOGI("socketId exist.");
        return;
    }
    sendCallbackMap_.insert(std::pair<std::string, sptr<IAssetSendCallback>>(taskId, sendCallback));
}

void AssetCallbackMananger::RemoveSendCallback(std::string taskId)
{
    std::lock_guard<std::mutex> lock(sendCallbackMapMutex_);
    auto iter = sendCallbackMap_.find(taskId);
    if (iter == sendCallbackMap_.end()) {
        LOGE("socketId not exist.");
        return;
    }
    sendCallbackMap_.erase(iter);
}
void AssetCallbackMananger::NotifyAssetRecvStart(const std::string &srcNetworkId,
                                                 const std::string &dstNetworkId,
                                                 const std::string &sessionId,
                                                 const std::string &dstBundleName)
{
    std::lock_guard<std::mutex> lock(recvCallbackListMutex_);
    for (auto callback : recvCallbackList_) {
        if (callback == nullptr) {
            LOGE("IAssetRecvCallback is empty!");
            return;
        }
        callback->OnStart(srcNetworkId, dstNetworkId, sessionId, dstBundleName);
    }
}
void AssetCallbackMananger::NotifyAssetRecvFinished(const std::string &srcNetworkId,
                                                    const sptr<AssetObj> &assetObj,
                                                    int32_t result)
{
    std::lock_guard<std::mutex> lock(recvCallbackListMutex_);
    for (auto callback : recvCallbackList_) {
        if (callback == nullptr) {
            LOGE("IAssetRecvCallback is empty!");
            return;
        }
        callback->OnFinished(srcNetworkId, assetObj, result);
    }
}
void AssetCallbackMananger::NotifyAssetSendResult(std::string &taskId,
                                                  const sptr<AssetObj> &assetObj,
                                                  int32_t result)
{
    std::lock_guard<std::mutex> lock(sendCallbackMapMutex_);
    auto iter = sendCallbackMap_.find(taskId);
    if (iter == sendCallbackMap_.end()) {
        LOGE("taskId not exist.");
        return;
    }
    if (iter->second == nullptr) {
        LOGE("IAssetSendCallback is empty!");
        return;
    }
    iter->second->OnSendResult(assetObj, result);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS