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
#include "filetransfer_callback_stub.h"

#include <fcntl.h>
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
FileTransferCallbackStub::FileTransferCallbackStub()
{
    memberFuncMap_[ON_SEND_FINISHED] = &FileTransferCallbackStub::CmdOnSendFinished;
    memberFuncMap_[ON_SEND_ERROR] = &FileTransferCallbackStub::CmdOnSendError;
    memberFuncMap_[ON_RECEIVE_FINISHED] = &FileTransferCallbackStub::CmdOnReceiveFinished;
    memberFuncMap_[ON_RECEIVE_ERROR] = &FileTransferCallbackStub::CmdOnReceiveError;
    memberFuncMap_[ON_DEVICE_ONLINE] = &FileTransferCallbackStub::CmdOnDeviceOnline;
    memberFuncMap_[ON_DEVICE_OFFLINE] = &FileTransferCallbackStub::CmdOnDeviceOffline;
    memberFuncMap_[ON_WRITE_FILE] = &FileTransferCallbackStub::CmdWriteFile;
}

FileTransferCallbackStub::~FileTransferCallbackStub() {}

int32_t FileTransferCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string myDescripter = FileTransferCallbackStub::GetDescriptor();
    std::u16string remoteDescripter = data.ReadInterfaceToken();
    if (myDescripter != remoteDescripter) {
        LOGE("Descriptor checked failed");
        return DFS_CALLBACK_DESCRIPTOR_IS_EMPTY;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t FileTransferCallbackStub::CmdOnDeviceOnline(MessageParcel &data, MessageParcel &reply)
{
    std::string cid = data.ReadString();
    int32_t result = DeviceOnline(cid);
    if (!reply.WriteInt32(result)) {
        LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t FileTransferCallbackStub::CmdOnDeviceOffline(MessageParcel &data, MessageParcel &reply)
{
    std::string cid = data.ReadString();
    int32_t result = DeviceOffline(cid);
    if (!reply.WriteInt32(result)) {
        LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t FileTransferCallbackStub::CmdOnSendFinished(MessageParcel &data, MessageParcel &reply)
{
    std::string cid = data.ReadString();
    std::string fileName = data.ReadString();
    int32_t result = SendFinished(cid, fileName);
    if (!reply.WriteInt32(result)) {
        LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t FileTransferCallbackStub::CmdOnSendError(MessageParcel &data, MessageParcel &reply)
{
    std::string cid = data.ReadString();
    int32_t result = SendError(cid);
    if (!reply.WriteInt32(result)) {
        LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t FileTransferCallbackStub::CmdOnReceiveFinished(MessageParcel &data, MessageParcel &reply)
{
    std::string cid = data.ReadString();
    std::string fileName = data.ReadString();
    uint32_t num = data.ReadUint32();
    int32_t result = ReceiveFinished(cid, fileName, num);
    if (!reply.WriteInt32(result)) {
        LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t FileTransferCallbackStub::CmdOnReceiveError(MessageParcel &data, MessageParcel &reply)
{
    std::string cid = data.ReadString();
    int32_t result = ReceiveError(cid);
    if (!reply.WriteInt32(result)) {
        LOGE("Write parcel failed");
        return result;
    }

    return ERR_NONE;
}

int32_t FileTransferCallbackStub::CmdWriteFile(MessageParcel &data, MessageParcel &reply)
{
    int32_t fileData = data.ReadFileDescriptor();
    int32_t fd = fcntl(fileData, F_GETFD, 0);
    if (fd < 0) {
        reply.WriteInt32(DFS_CALLBACK_ERR_SET_FD_FAIL);
        LOGE("FileTransferCallbackStub : Failed to get app device id, error: invalid device id");
        return DFS_CALLBACK_ERR_SET_FD_FAIL;
    }

    std::string fileName = data.ReadString();
    if (fileName.empty()) {
        LOGE("FileTransferCallbackStub : Failed to get app device id, error: invalid device id");
        return DFS_CALLBACK_DESCRIPTOR_IS_EMPTY;
    }

    int32_t result = WriteFile(fileData, fileName);
    if (!reply.WriteInt32(result)) {
        LOGE("fail to write parcel");
        return DFS_CALLBACK_WRITE_REPLY_FAIL;
    }

    return ERR_NONE;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS