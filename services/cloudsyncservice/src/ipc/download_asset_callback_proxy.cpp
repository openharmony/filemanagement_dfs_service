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

#include "ipc/download_asset_callback_proxy.h"

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

void DownloadAssetCallbackProxy::OnFinished(const TaskId taskId, const std::string &uri, const int32_t result)
{
    LOGI("Start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return;
    }

    if (!data.WriteUint64(taskId)) {
        LOGE("Failed to send the uri");
        return;
    }

    if (!data.WriteString(uri)) {
        LOGE("Failed to send the uri");
        return;
    }

    if (!data.WriteInt32(result)) {
        LOGE("Failed to send the state");
        return;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return;
    }
    int32_t ret = remote->SendRequest(IDownloadAssetCallback::SERVICE_CMD_ON_DOWNLOAD_FINSHED, data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, ret:%{public}d", ret);
        return;
    }
    LOGI("End");
    return;
}
} // namespace OHOS::FileManagement::CloudSync