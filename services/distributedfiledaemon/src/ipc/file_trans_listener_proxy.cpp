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

#include "file_trans_listener_proxy.h"

#include "dfs_error.h"
#include "file_trans_listener_interface_code.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils_log.h"
namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
using namespace OHOS::Storage;
using namespace OHOS::FileManagement;

int32_t FileTransListenerProxy::OnFileReceive(uint64_t totalBytes, uint64_t processedBytes)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }
    if (!data.WriteUint64(totalBytes)) {
        LOGE("Failed to send bytesTotal");
        return E_INVAL_ARG;
    }
    if (!data.WriteUint64(processedBytes)) {
        LOGE("Failed to send bytesUpload");
        return E_INVAL_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(FileTransListenerInterfaceCode::FILE_TRANS_LISTENER_ON_PROGRESS), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno:%{public}d", errno);
        return E_BROKEN_IPC;
    }
    return reply.ReadInt32();
}

int32_t FileTransListenerProxy::OnFailed(const std::string &sessionName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }
    if (!data.WriteString(sessionName)) {
        LOGE("Failed to send sessionName");
        return E_INVAL_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(FileTransListenerInterfaceCode::FILE_TRANS_LISTENER_ON_FAILED), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno:%{public}d", errno);
        return E_BROKEN_IPC;
    }
    return reply.ReadInt32();
}

int32_t FileTransListenerProxy::OnFinished(const std::string &sessionName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }
    if (!data.WriteString(sessionName)) {
        LOGE("Failed to send sessionName");
        return E_INVAL_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(FileTransListenerInterfaceCode::FILE_TRANS_LISTENER_ON_FINISHED), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno:%{public}d", errno);
        return E_BROKEN_IPC;
    }
    return reply.ReadInt32();
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS