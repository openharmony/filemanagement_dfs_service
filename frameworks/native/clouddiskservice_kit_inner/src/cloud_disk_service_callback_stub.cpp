/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cloud_disk_service_callback_stub.h"

#include "cloud_disk_service_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;

CloudDiskServiceCallbackStub::CloudDiskServiceCallbackStub()
{
    opToInterfaceMap_[SERVICE_CMD_ON_CHANGE_DATA] = [this](MessageParcel &data, MessageParcel &reply) {
        return this->HandleOnChangeData(data, reply);
    };
}

int32_t CloudDiskServiceCallbackStub::OnRemoteRequest(uint32_t code,
                                                      MessageParcel &data,
                                                      MessageParcel &reply,
                                                      MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return E_INTERNAL_ERROR;
    }
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        LOGE("Cannot response request %d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    auto memberFunc = interfaceIndex->second;
    return memberFunc(data, reply);
}

int32_t CloudDiskServiceCallbackStub::HandleOnChangeData(MessageParcel &data, MessageParcel &reply)
{
    std::vector<ChangeData> changesData;

    std::string sandboxPath = data.ReadString();
    auto size = data.ReadInt32();
    if (size > static_cast<int32_t>(VECTOR_MAX_SIZE)) {
        return ERR_INVALID_DATA;
    }
    for (int i = 0; i < size; ++i) {
        std::unique_ptr<ChangeData> value(data.ReadParcelable<ChangeData>());
        if (!value) {
            LOGE("object of ChangeData is nullptr");
            return E_INVALID_ARG;
        }
        changesData.push_back(*value);
    }

    OnChangeData(sandboxPath, changesData);
    LOGI("HandleOnChangeData end");
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudDiskService