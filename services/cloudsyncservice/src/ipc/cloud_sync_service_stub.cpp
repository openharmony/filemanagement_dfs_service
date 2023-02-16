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
#include "ipc/cloud_sync_service_stub.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

CloudSyncServiceStub::CloudSyncServiceStub()
{
    opToInterfaceMap_[SERVICE_CMD_REGISTER_CALLBACK] = &CloudSyncServiceStub::HandleRegisterCallbackInner;
    opToInterfaceMap_[SERVICE_CMD_START_SYNC] = &CloudSyncServiceStub::HandleStartSyncInner;
    opToInterfaceMap_[SERVICE_CMD_STOP_SYNC] = &CloudSyncServiceStub::HandleStopSyncInner;
}

int32_t CloudSyncServiceStub::OnRemoteRequest(uint32_t code,
                                              MessageParcel &data,
                                              MessageParcel &reply,
                                              MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return E_SERVICE_DESCRIPTOR_IS_EMPTY;
    }
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        LOGE("Cannot response request %d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    return (this->*(interfaceIndex->second))(data, reply);
}

int32_t CloudSyncServiceStub::HandleRegisterCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin RegisterCallbackInner");
    auto remoteObj = data.ReadRemoteObject();
    std::string appPackageName = data.ReadString();
    int32_t res = RegisterCallbackInner(appPackageName, remoteObj);
    reply.WriteInt32(res);
    LOGI("End RegisterCallbackInner");
    return res;
}

int32_t CloudSyncServiceStub::HandleStartSyncInner(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin StartSyncInner");
    auto appPackageName = data.ReadString();
    SyncType type = SyncType(data.ReadInt32());
    auto forceFlag = data.ReadBool();
    int32_t res = StartSyncInner(appPackageName, type, forceFlag);
    reply.WriteInt32(res);
    LOGI("End StartSyncInner");
    return res;
}

int32_t CloudSyncServiceStub::HandleStopSyncInner(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin StopSyncInner");
    auto appPackageName = data.ReadString();
    int32_t res = StopSyncInner(appPackageName);
    reply.WriteInt32(res);
    LOGI("End StopSyncInner");
    return res;
}
} // namespace OHOS::FileManagement::CloudSync