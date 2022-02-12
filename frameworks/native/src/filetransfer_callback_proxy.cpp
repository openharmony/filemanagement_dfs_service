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
FileTransferCallbackProxy::FileTransferCallbackProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<IFileTransferCallback>(impl) {}

FileTransferCallbackProxy::~FileTransferCallbackProxy() {}

int32_t FileTransferCallbackProxy::DeviceOnline(const std::string &cid)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(FileTransferCallbackProxy::GetDescriptor())) {
        LOGE("xhl [---FileTransferCallbackProxy---] WriteInterfaceToken failed");
        return DFS_CALLBACK_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("xhl [---FileTransferCallbackProxy---] Remote is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteString(cid);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(ON_DEVICE_ONLINE, data, reply, option);
    if (ret != ERR_NONE) {
        LOGE("[---FileTransferCallbackProxy---] Proxy SendRequest failed, ret code:[%{public}d]", ret);
    }
    return ret;
}

int32_t FileTransferCallbackProxy::DeviceOffline(const std::string &cid)
{
    MessageParcel data;
    if (!data.WriteInterfaceToken(FileTransferCallbackProxy::GetDescriptor())) {
        LOGE("xhl [---FileTransferCallbackProxy---] WriteInterfaceToken failed");
        return 0;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        LOGE("xhl [---FileTransferCallbackProxy---] Remote is null");
        return ERR_NULL_OBJECT;
    }

    data.WriteString(cid);
    MessageParcel reply;
    MessageOption option;
    int32_t ret = remote->SendRequest(ON_DEVICE_OFFLINE, data, reply, option);
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
        return 0;
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
        return 0;
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
        return 0;
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
        return 0;
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
    LOGI("xhl WriteFile enter");
    LOGI("xhl WriteFile fd %{public}d, %{public}s", fd, fileName.c_str());
    MessageParcel data;
    if (!data.WriteInterfaceToken(FileTransferCallbackProxy::GetDescriptor())) {
        return DFS_CALLBACK_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    data.WriteFileDescriptor(fd);
    data.WriteString(fileName);

    if (Remote() == nullptr) {
        LOGE("xhl WriteFile remote object address is null");
        return DFS_CALLBACK_REMOTE_ADDRESS_IS_NULL;
    }

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(ON_WRITE_FILE, data, reply, option);
    LOGI("xhl WriteFile end");
    return reply.ReadInt32();
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS