/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "file_dfs_listener_stub.h"

#include "file_dfs_listener_interface_code.h"
#include "utils_log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD004315
#define LOG_TAG "distributedfile_daemon"

namespace OHOS {
namespace FileManagement {
namespace ModuleFileIO {
using namespace FileManagement;
namespace {
    constexpr int NO_ERROR = 0;
    constexpr int E_INVAL_ARG = 1;
}
FileDfsListenerStub::FileDfsListenerStub()
{
    opToInterfaceMap_[static_cast<uint32_t>
        (Storage::DistributedFile::FileDfsListenerInterfaceCode::FILE_DFS_LISTENER_ON_STATUS)] =
        &FileDfsListenerStub::HandleOnStatus;
}

int32_t FileDfsListenerStub::OnRemoteRequest(uint32_t code,
                                             MessageParcel &data,
                                             MessageParcel &reply,
                                             MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return FILE_DFS_LISTENER_DESCRIPTOR_IS_EMPTY;
    }
    switch (code) {
        case static_cast<uint32_t>(Storage::DistributedFile::FileDfsListenerInterfaceCode::FILE_DFS_LISTENER_ON_STATUS):
            return HandleOnStatus(data, reply);
        default:
            LOGE("Cannot response request %{public}d: unknown tranction", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t FileDfsListenerStub::HandleOnStatus(MessageParcel &data, MessageParcel &reply)
{
    std::string networkId;
    if (!data.ReadString(networkId)) {
        LOGE("read networkId failed");
        return E_INVAL_ARG;
    }
    int32_t status;
    if (!data.ReadInt32(status)) {
        LOGE("read status failed");
        return E_INVAL_ARG;
    }
    if (networkId.empty() || status < 0) {
        LOGE("Invalid arguments");
        return E_INVAL_ARG;
    }
    OnStatus(networkId, status);
    return NO_ERROR;
}

} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS