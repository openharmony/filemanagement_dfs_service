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
#include "ipc/download_asset_callback_manager.h"

#include <cinttypes>

#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

DownloadAssetCallbackManager &DownloadAssetCallbackManager::GetInstance()
{
    static DownloadAssetCallbackManager instance;
    return instance;
}

void DownloadAssetCallbackManager::AddCallback(const sptr<IDownloadAssetCallback> &callback)
{
    if (callback == nullptr) {
        return;
    }
    callbackProxy_ = callback;
    auto remoteObject = callback->AsObject();
    auto deathCb = [this](const wptr<IRemoteObject> &obj) {
        callbackProxy_ = nullptr;
        LOGE("client died");
    };
    deathRecipient_ = sptr(new SvcDeathRecipient(deathCb));
    remoteObject->AddDeathRecipient(deathRecipient_);
}

void DownloadAssetCallbackManager::OnDownloadFinshed(const TaskId taskId, const std::string &uri, const int32_t result)
{
    auto callback = callbackProxy_;
    if (callback == nullptr) {
        LOGE("callbackProxy_ is nullptr");
        return;
    }
    LOGD("On Download finished, taskId:%{public}" PRIu64 ", uri:%{public}s, ret:%{public}d", taskId, uri.c_str(),
         result);
    callback->OnFinished(taskId, uri, result);
}
} // namespace OHOS::FileManagement::CloudSync