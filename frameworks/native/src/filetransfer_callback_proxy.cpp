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

#include "filetransfer_callback_proxy.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
int32_t FileTransferCallbackProxy::SessionOpened(const std::string &cid)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(FileTransferCallbackProxy::GetDescriptor())) {
        LOGE("[---FileTransferCallbackProxy---] WriteInterfaceToken failed");
        return DFS_CALLBACK_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("[---FileTransferCallbackProxy---] Remote is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteString(cid);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(ON_SESSION_OPENED, data, reply, option);
    if (ret != ERR_NONE) {
        LOGE("[---FileTransferCallbackProxy---] Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t FileTransferCallbackProxy::SessionClosed(const std::string &cid)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(FileTransferCallbackProxy::GetDescriptor())) {
        LOGE("[---FileTransferCallbackProxy---] WriteInterfaceToken failed");
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("[---FileTransferCallbackProxy---] Remote is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteString(cid);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(ON_SESSION_CLOSED, data, reply, option);
    if (ret != ERR_NONE) {
        LOGE("[---FileTransferCallbackProxy---] Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t FileTransferCallbackProxy::SendFinished(const std::string &cid, std::string fileName)
{
    LOGI("[---FileTransferCallbackProxy---] Proxy SendFinished");
    MessageParcel data;
    if (!data.WriteInterfaceToken(FileTransferCallbackProxy::GetDescriptor())) {
        LOGE("[---FileTransferCallbackProxy---] WriteInterfaceToken failed");
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("[---FileTransferCallbackProxy---] Remote is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteString(cid);
    data.WriteString(fileName);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(ON_SEND_FINISHED, data, reply, option);
    if (ret != ERR_NONE) {
        LOGE("[---FileTransferCallbackProxy---] Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t FileTransferCallbackProxy::SendError(const std::string &cid)
{
    LOGI("[---FileTransferCallbackProxy---] Proxy SendError");
    MessageParcel data;
    if (!data.WriteInterfaceToken(FileTransferCallbackProxy::GetDescriptor())) {
        LOGE("[---FileTransferCallbackProxy---] WriteInterfaceToken failed");
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("[---FileTransferCallbackProxy---] Remote is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteString(cid);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(ON_SEND_ERROR, data, reply, option);
    if (ret != ERR_NONE) {
        LOGE("[---FileTransferCallbackProxy---] Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t FileTransferCallbackProxy::ReceiveFinished(const std::string &cid, const std::string &fileName, uint32_t num)
{
    LOGI("[---FileTransferCallbackProxy---] Proxy ReceiveFinished, %{public}s", cid.c_str());
    MessageParcel data;
    if (!data.WriteInterfaceToken(FileTransferCallbackProxy::GetDescriptor())) {
        LOGE("[---FileTransferCallbackProxy---] WriteInterfaceToken failed");
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("[---FileTransferCallbackProxy---] Remote is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteString(cid);
    data.WriteString(fileName);
    data.WriteUint32(num);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(ON_RECEIVE_FINISHED, data, reply, option);
    if (ret != ERR_NONE) {
        LOGE("[---FileTransferCallbackProxy---] Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t FileTransferCallbackProxy::ReceiveError(const std::string &cid)
{
    LOGI("[---FileTransferCallbackProxy---] Proxy ReceiveError");
    MessageParcel data;
    if (!data.WriteInterfaceToken(FileTransferCallbackProxy::GetDescriptor())) {
        LOGE("WriteInterfaceToken failed");
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("[---FileTransferCallbackProxy---] Remote is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteString(cid);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(ON_RECEIVE_ERROR, data, reply, option);
    if (ret != ERR_NONE) {
        LOGE("[---FileTransferCallbackProxy---] Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t FileTransferCallbackProxy::WriteFile(int32_t fd, const std::string &fileName)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(FileTransferCallbackProxy::GetDescriptor())) {
        return ERR_INVALID_DATA;
    }

    data.WriteFileDescriptor(fd);
    data.WriteString(fileName);

    if (Remote() == nullptr) {
        LOGE("WriteFile remote object address is null");
        return ERR_NULL_OBJECT;
    }

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(ON_WRITE_FILE, data, reply, option);

    return reply.ReadInt32();
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS