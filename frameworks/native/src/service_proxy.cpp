/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

#include "service_proxy.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
ServiceProxy::ServiceProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IDistributedFileService>(impl) {}

ServiceProxy::~ServiceProxy() {}

int32_t ServiceProxy::SendFile(const std::string &cid,
                               const std::vector<std::string> &sourceFileList,
                               const std::vector<std::string> &destinationFileList,
                               const uint32_t fileCount)
{
    LOGI("xhl sendFile enter");
    MessageParcel data;
    if (!data.WriteInterfaceToken(ServiceProxy::GetDescriptor())) {
        return DFS_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteString(cid);
    int32_t sourceListNumber = sourceFileList.size();
    data.WriteInt32(sourceListNumber);
    for (int32_t index = 0; index < sourceListNumber; ++index) {
        data.WriteString(sourceFileList.at(index));
    }
    int32_t destinationListNumber = destinationFileList.size();
    data.WriteInt32(destinationListNumber);
    for (int32_t index = 0; index < destinationListNumber; ++index) {
        data.WriteString(destinationFileList.at(index));
    }
    data.WriteUint32(fileCount);

    if (Remote() == nullptr) {
        LOGE("xhl unregister sendfile remote object address is null");
        return DFS_REMOTE_ADDRESS_IS_NULL;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = Remote()->SendRequest(SEND_FILE_DISTRIBUTED, data, reply, option);
    if (result != DFS_SENDFILE_SUCCESS) {
        LOGE("xhl sendfile error code : %{public}d", result);
        return result;
    }
    LOGE("xhl sendfile sendRequest done %{public}d", result);

    return reply.ReadInt32();
}

int32_t ServiceProxy::OpenFile(int32_t fd, const std::string &fileName, int32_t mode)
{
    LOGI("xhl OpenFile enter");
    LOGI("xhl OpenFile fd %{public}d", fd);
    MessageParcel data;
    if (!data.WriteInterfaceToken(ServiceProxy::GetDescriptor())) {
        return DFS_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    data.WriteFileDescriptor(fd);
    data.WriteString(fileName);
    data.WriteInt32(mode);

    if (Remote() == nullptr) {
        LOGE("xhl openfile remote object address is null");
        return DFS_REMOTE_ADDRESS_IS_NULL;
    }

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(OPEN_FILE_FD, data, reply, option);
    LOGI("xhl RegisterNotifyCallback end");
    return reply.ReadInt32();
}

int32_t ServiceProxy::RegisterNotifyCallback(sptr<IFileTransferCallback> &callback)
{
    LOGI("xhl OpenFile enter");
    MessageParcel data;
    if (!data.WriteInterfaceToken(ServiceProxy::GetDescriptor())) {
        return DFS_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    if (callback == nullptr) {
        LOGE("xhl The parameter of callback is nullptr");
        return DFS_CALLBACK_PARAM_ERROR;
    }

    if (!data.WriteRemoteObject(callback->AsObject().GetRefPtr())) {
        LOGE("xhl write remote object failed");
        return DFS_WRITE_REMOTE_OBJECT_FAIL;
    }

    if (Remote() == nullptr) {
        LOGE("xhl register filetransfer callback remote object address is null");
        return DFS_REMOTE_ADDRESS_IS_NULL;
    }

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(REGISTER_NOTIFY_CALLBACK, data, reply, option);
    LOGI("xhl RegisterNotifyCallback end");
    return reply.ReadInt32();
}

int32_t ServiceProxy::UnRegisterNotifyCallback()
{
    LOGI("xhl UnRegisterNotifyCallback enter");
    MessageParcel data;
    if (!data.WriteInterfaceToken(ServiceProxy::GetDescriptor())) {
        return DFS_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }

    if (Remote() == nullptr) {
        LOGE("xhl unregister filetransfer callback remote object address is null");
        return DFS_REMOTE_ADDRESS_IS_NULL;
    }

    MessageParcel reply;
    MessageOption option;
    Remote()->SendRequest(UN_REGISTER_NOTIFY_CALLBACK, data, reply, option);
    LOGI("xhl UnRegisterNotifyCallback end");
    return reply.ReadInt32();
}

int32_t ServiceProxy::IsDeviceOnline(const std::string &cid)
{
    LOGE("xhl IsDeviceOnline enter");
    MessageParcel data;
    if (!data.WriteInterfaceToken(ServiceProxy::GetDescriptor())) {
        return DFS_WRITE_DESCRIPTOR_TOKEN_FAIL;
    }
    data.WriteString(cid);

    if (Remote() == nullptr) {
        LOGE("Remote object address is null");
        return DFS_REMOTE_ADDRESS_IS_NULL;
    }

    MessageParcel reply;
    MessageOption option;
    int32_t result = Remote()->SendRequest(IS_DEVICE_ONLINE, data, reply, option);
    if (result != DFS_NO_ERROR) {
        LOGE("Function RemoveBundleDistributedDirs! errCode:%{public}d", result);
        return DFS_NO_DEVICE_ONLINE;
    }
    LOGE("xhl IsDeviceOnline done %{public}d", result);

    return reply.ReadInt32();
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS