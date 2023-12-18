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

#include "download_asset_callback_client.h"

#include <cinttypes>

#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
void DownloadAssetCallbackClient::OnFinished(const TaskId taskId, const std::string &uri, const int32_t result)
{
    auto callback = GetDownloadTaskCallback(taskId);
    if (callback) {
        LOGD("callback");
        callback(uri, result);
    }
    RemoveDownloadTaskCallback(taskId);
    LOGD("On Download finished, taskId:%{public}" PRIu64 ", uri:%{public}s, ret:%{public}d", taskId, uri.c_str(),
         result);
}

void DownloadAssetCallbackClient::AddDownloadTaskCallback(TaskId taskId, CloudSyncAssetManager::ResultCallback callback)
{
    LOGD("taskId = %{public}" PRIu64 "", taskId);
    callbackListMap_.EnsureInsert(taskId, std::move(callback));
}

void DownloadAssetCallbackClient::RemoveDownloadTaskCallback(TaskId taskId)
{
    CloudSyncAssetManager::ResultCallback callback;
    if (callbackListMap_.Find(taskId, callback)) {
        callbackListMap_.Erase(taskId);
        return;
    }
    LOGE("not find task callback, taskId = %{public}" PRIu64 "", taskId);
}

CloudSyncAssetManager::ResultCallback DownloadAssetCallbackClient::GetDownloadTaskCallback(TaskId taskId)
{
    CloudSyncAssetManager::ResultCallback callback;
    if (callbackListMap_.Find(taskId, callback)) {
        return callback;
    }
    LOGE("not find task callback, taskId = %{public}" PRIu64 "", taskId);
    return std::function<void(const std::string &, int32_t)>{}; // empty function
}

DownloadAssetCallback::TaskId DownloadAssetCallbackClient::GetTaskId()
{
    return taskId_.fetch_add(1);
}
} // namespace OHOS::FileManagement::CloudSync