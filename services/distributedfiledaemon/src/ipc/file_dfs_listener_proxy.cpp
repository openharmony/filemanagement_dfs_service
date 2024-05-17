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

#include "file_dfs_listener_proxy.h"
#include "dfs_error.h"
#include "iservice_registry.h"
#include "ipc_types.h"
#include "system_ability_definition.h"
#include "utils_log.h"
#include "file_dfs_listener_interface_code.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
using namespace OHOS::Storage;
using namespace OHOS::FileManagement;

void FileDfsListenerProxy::OnStatus(const std::string &networkId, int32_t status)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return;
    }
    if (!data.WriteString(networkId)) {
        LOGE("Failed to send networkId");
        return;
    }
    if (!data.WriteInt32(status)) {
        LOGE("Failed to send status");
        return;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(FileDfsListenerInterfaceCode::FILE_DFS_LISTENER_ON_STATUS), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno:%{public}d", errno);
        return;
    }
    return;
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS