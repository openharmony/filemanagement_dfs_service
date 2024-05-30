/*
* Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "network/softbus/softbus_asset_recv_listener.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

std::string SoftbusAssetRecvListener::path_ = "";
void SoftbusAssetRecvListener::OnFile(int32_t socket, FileEvent *event)
{
    if (event == nullptr) {
        LOGE("invalid parameter");
        return;
    }
    switch (event->type) {
        case FILE_EVENT_RECV_UPDATE_PATH:
            event->UpdateRecvPath = GetRecvPath;
            break;
        default:
            LOGI("Other situations");
            break;
    }
}

const char* SoftbusAssetRecvListener::GetRecvPath()
{
    const char* path = path_.c_str();
    LOGI("path: %s", path);
    return path;
}

void SoftbusAssetRecvListener::OnAssetRecvBind(int32_t sessionId, PeerSocketInfo info)
{
    LOGI("OnAssetRecvBind begin.");
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS