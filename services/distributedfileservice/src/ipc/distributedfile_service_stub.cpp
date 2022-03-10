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

#include "distributedfile_service_stub.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include "filetransfer_callback_proxy.h"
#include "ipc_skeleton.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
DistributedFileServiceStub::DistributedFileServiceStub()
{
    memberFuncMap_[SEND_FILE_DISTRIBUTED] = &DistributedFileServiceStub::SendFileStub;
    memberFuncMap_[OPEN_FILE_FD] = &DistributedFileServiceStub::OpenFileStub;
    memberFuncMap_[REGISTER_NOTIFY_CALLBACK] = &DistributedFileServiceStub::CmdRegisterNotifyCallback;
    memberFuncMap_[IS_DEVICE_ONLINE] = &DistributedFileServiceStub::CmdIsDeviceOnline;
}

DistributedFileServiceStub::~DistributedFileServiceStub()
{
    memberFuncMap_.clear();
}

int DistributedFileServiceStub::OnRemoteRequest(uint32_t code,
                                                MessageParcel &data,
                                                MessageParcel &reply,
                                                MessageOption &option)
{
    std::u16string myDescriptor = DistributedFileServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        LOGE("DistributedFileServiceStub descriptor information");
        return DFS_DESCRIPTOR_IS_EMPTY;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t DistributedFileServiceStub::SendFileStub(MessageParcel &data, MessageParcel &reply)
{
    std::string cid = data.ReadString();
    if (cid.empty()) {
        LOGE("SendFileStub : Failed to get app device id, error: invalid device id");
        return DFS_SESSION_ID_IS_EMPTY;
    }

    int32_t sourceListNumber = data.ReadInt32();
    if (sourceListNumber != 1) {
        return DFS_PARAM_FILE_COUNT_ERROR;
    }
    std::vector<std::string> srcList;
    for (int32_t index = 0; index < sourceListNumber; ++index) {
        srcList.push_back(data.ReadString());
    }

    int32_t destinationListNumber = data.ReadInt32();
    if (destinationListNumber > 1) {
        return DFS_PARAM_FILE_COUNT_ERROR;
    }
    std::vector<std::string> dstList;
    for (int32_t index = 0; index < destinationListNumber; ++index) {
        dstList.push_back(data.ReadString());
    }

    uint32_t fileCount = data.ReadUint32();
    if (fileCount != 1) {
        return DFS_PARAM_FILE_COUNT_ERROR;
    }

    int32_t result = SendFile(cid, srcList, dstList, fileCount);
    if (!reply.WriteInt32(result)) {
        LOGE("sendfilestub fail to write parcel");
        return DFS_WRITE_REPLY_FAIL;
    }

    return result;
}

int32_t DistributedFileServiceStub::OpenFileStub(MessageParcel &data, MessageParcel &reply)
{
    int32_t fileData = data.ReadFileDescriptor();
    int32_t fd = fcntl(fileData, F_GETFD, 0);
    if (fd < 0) {
        reply.WriteInt32(DFS_FILE_OP_ERROR);
        LOGE("DistributedFileServiceStub : Failed to get app device id, error: invalid device id");
        return DFS_FILE_OP_ERROR;
    }

    std::string fileName = data.ReadString();
    if (fileName.empty()) {
        LOGE("DistributedFileServiceStub : Failed to get app device id, error: invalid device id");
        return DFS_NO_SUCH_FILE;
    }

    int32_t modeData = data.ReadInt32();
    int32_t result = OpenFile(fileData, fileName, modeData);
    if (!reply.WriteInt32(result)) {
        LOGE("fail to write parcel");
        return DFS_WRITE_REPLY_FAIL;
    }

    return result;
}

int32_t DistributedFileServiceStub::CmdRegisterNotifyCallback(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        LOGE("Callback ptr is nullptr.");
        return DFS_REMOTE_ADDRESS_IS_NULL;
    }

    sptr<IFileTransferCallback> callback = iface_cast<IFileTransferCallback>(remote);
    LOGD("DistributedFileServiceStub::RegisterNotifyCallback: cb[%{public}p]", callback->AsObject().GetRefPtr());
    int32_t result = RegisterNotifyCallback(callback);
    reply.WriteInt32(result);
    return result;
}

int32_t DistributedFileServiceStub::CmdIsDeviceOnline(MessageParcel &data, MessageParcel &reply)
{
    std::string cid = data.ReadString();
    if (cid.empty()) {
        LOGE("DistributedFileServiceStub : Failed to get app device id, error: invalid device id");
        return DFS_SESSION_ID_IS_EMPTY;
    }

    int32_t result = IsDeviceOnline(cid);
    if (!reply.WriteInt32(result)) {
        LOGE("fail to write parcel");
        return DFS_WRITE_REPLY_FAIL;
    }

    return result;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS