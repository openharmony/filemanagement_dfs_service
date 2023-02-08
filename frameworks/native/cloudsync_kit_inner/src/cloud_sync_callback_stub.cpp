/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "cloud_sync_callback_stub.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

CloudSyncCallbackStub::CloudSyncCallbackStub()
{
    opToInterfaceMap_[SERVICE_CMD_ON_SYNC_STATE_CHANGED] = &CloudSyncCallbackStub::HandleOnSyncStateChanged;
}

int32_t CloudSyncCallbackStub::OnRemoteRequest(uint32_t code,
                                               MessageParcel &data,
                                               MessageParcel &reply,
                                               MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return 1; // TODO
    }
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        LOGE("Cannot response request %d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*(interfaceIndex->second))(data, reply);
}

int32_t CloudSyncCallbackStub::HandleOnSyncStateChanged(MessageParcel &data, MessageParcel &reply)
{
    LOGI("start");
    SyncType type = SyncType(data.ReadInt32());
    SyncPromptState state = SyncPromptState(data.ReadInt32());
    OnSyncStateChanged(type, state);
    LOGI("end");
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync