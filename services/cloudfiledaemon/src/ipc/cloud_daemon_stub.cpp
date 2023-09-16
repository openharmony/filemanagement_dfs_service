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

#include "ipc/cloud_daemon_stub.h"

#include "cloud_file_daemon_interface_code.h"
#include "dfsu_memory_guard.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
CloudDaemonStub::CloudDaemonStub()
{
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileDaemonInterfaceCode::CLOUD_DAEMON_CMD_START_FUSE)] =
        &CloudDaemonStub::HandleStartFuseInner;
}

int32_t CloudDaemonStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                         MessageParcel &reply, MessageOption &option)
{
    CloudSync::DfsuMemoryGuard cacheGuard;
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return CLOUD_DAEMON_DESCRIPTOR_IS_EMPTY;
    }
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        LOGE("Cannot response request %d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*(interfaceIndex->second))(data, reply);
}

int32_t CloudDaemonStub::HandleStartFuseInner(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin StartFuseInner");
    auto userId = data.ReadInt32();
    auto fd = data.ReadFileDescriptor();
    auto path = data.ReadString();
    int32_t res = StartFuse(userId, int32_t(fd), path);
    reply.WriteInt32(res);
    LOGI("End StartFuseInner");
    return res;
}
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS
