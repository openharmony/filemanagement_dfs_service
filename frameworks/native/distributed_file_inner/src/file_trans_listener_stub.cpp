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

#include "file_trans_listener_stub.h"

#include "dfs_error.h"
#include "file_trans_listener_interface_code.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace FileManagement;
FileTransListenerStub::FileTransListenerStub()
{
    opToInterfaceMap_[static_cast<uint32_t>(FileTransListenerInterfaceCode::FILE_TRANS_LISTENER_ON_PROGRESS)] =
        &FileTransListenerStub::HandleOnFileReceive;
    opToInterfaceMap_[static_cast<uint32_t>(FileTransListenerInterfaceCode::FILE_TRANS_LISTENER_ON_FAILED)] =
        &FileTransListenerStub::HandleOnFailed;
    opToInterfaceMap_[static_cast<uint32_t>(FileTransListenerInterfaceCode::FILE_TRANS_LISTENER_ON_FINISHED)] =
        &FileTransListenerStub::HandleOnFinished;
}

int32_t FileTransListenerStub::OnRemoteRequest(uint32_t code,
                                               MessageParcel &data,
                                               MessageParcel &reply,
                                               MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return File_Trans_Listener_DESCRIPTOR_IS_EMPTY;
    }
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        LOGE("Cannot response request %d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*(interfaceIndex->second))(data, reply);
}

int32_t FileTransListenerStub::HandleOnFileReceive(MessageParcel &data, MessageParcel &reply)
{
    uint64_t totalBytes = 0;
    if (!data.ReadUint64(totalBytes)) {
        LOGE("read totalBytes failed");
        return E_INVAL_ARG;
    }
    uint64_t processedBytes = 0;
    if (!data.ReadUint64(processedBytes)) {
        LOGE("read processedBytes failed");
        return E_INVAL_ARG;
    }
    int32_t res = OnFileReceive(totalBytes, processedBytes);
    if (res != E_OK) {
        LOGE("OnFileReceive call failed");
        return E_BROKEN_IPC;
    }
    reply.WriteInt32(res);
    return res;
}

int32_t FileTransListenerStub::HandleOnFailed(MessageParcel &data, MessageParcel &reply)
{
    std::string sessionName;
    if (!data.ReadString(sessionName)) {
        LOGE("read sessionName failed");
        return E_INVAL_ARG;
    }
    int32_t res = OnFailed(sessionName);
    if (res != E_OK) {
        LOGE("OnFailed call failed");
        return E_BROKEN_IPC;
    }
    reply.WriteInt32(res);
    return res;
}

int32_t FileTransListenerStub::HandleOnFinished(MessageParcel &data, MessageParcel &reply)
{
    std::string sessionName;
    if (!data.ReadString(sessionName)) {
        LOGE("read sessionName failed");
        return E_INVAL_ARG;
    }
    int32_t res = OnFinished(sessionName);
    if (res != E_OK) {
        LOGE("OnFinished call failed");
        return E_BROKEN_IPC;
    }
    reply.WriteInt32(res);
    return res;
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS