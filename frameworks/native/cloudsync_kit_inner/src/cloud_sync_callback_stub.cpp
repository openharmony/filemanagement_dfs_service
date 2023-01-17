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
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

CloudSyncCallbackStub::CloudSyncCallbackStub()
{
    opToInterfaceMap_[SERVICE_CMD_ON_START_SYNC_FINISHED] = &CloudSyncCallbackStub::HandleOnStartSyncFinished;
    opToInterfaceMap_[SERVICE_CMD_ON_STOP_SYNC_FINISHED] = &CloudSyncCallbackStub::HandleOnStopSyncFinished;
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

int32_t CloudSyncCallbackStub::HandleOnStartSyncFinished(MessageParcel &data, MessageParcel &reply)
{
    LOGI("hello, world");
    return ERR_NONE;
}

int32_t CloudSyncCallbackStub::HandleOnStopSyncFinished(MessageParcel &data, MessageParcel &reply)
{
    LOGI("hello, world");
    return ERR_NONE;
}
} // namespace OHOS::FileManagement::CloudSync