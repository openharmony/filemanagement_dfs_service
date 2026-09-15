/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "cloud_disk_service_callback_table_stub.h"

#include "cloud_disk_service_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
CloudDiskServiceCallbackTableStub::CloudDiskServiceCallbackTableStub()
{
    opToInterfaceMap_[SERVICE_CMD_ON_CALLBACK] = [this](MessageParcel &data, MessageParcel &reply) {
        return HandleOnCallback(data, reply);
    };
}

int32_t CloudDiskServiceCallbackTableStub::OnRemoteRequest(uint32_t code,
                                                           MessageParcel &data,
                                                           MessageParcel &reply,
                                                           MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LOGE("Invalid callback table interface token");
        return E_INVALID_ARG;
    }
    auto interface = opToInterfaceMap_.find(code);
    if (interface == opToInterfaceMap_.end() || !interface->second) {
        LOGE("Unknown callback table transaction: %{public}u", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return interface->second(data, reply);
}

int32_t CloudDiskServiceCallbackTableStub::HandleOnCallback(MessageParcel &data, MessageParcel &reply)
{
    CallbackParcelStorage storage;
    CloudDiskCallbackReqHead reqHead{};
    CloudDiskCallbackContext reqContext{};
    if (!ReadCallbackParcel(data, reqHead, reqContext, storage)) {
        LOGE("Failed to read callback parcel");
        return E_INVALID_ARG;
    }

    OnCallback(reqHead, reqContext);
    if (reqHead.callbackType == CloudDiskCallbackType::FETCH_DATA ||
        reqHead.callbackType == CloudDiskCallbackType::CANCEL_FETCH_DATA) {
        return E_OK;
    }
    if (!WriteCallbackReply(reply, reqHead.callbackType, reqContext)) {
        LOGE("Failed to write callback response");
        return E_INVALID_ARG;
    }
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudDiskService
