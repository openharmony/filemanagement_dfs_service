/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "asset/asset_send_callback_stub.h"

#include "asset/asset_callback_interface_code.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace FileManagement;
AssetSendCallbackStub::AssetSendCallbackStub()
{
    opToInterfaceMap_[static_cast<uint32_t>(AssetCallbackInterfaceCode::ASSET_CALLBACK_ON_SEND_RESULT)] =
        &AssetSendCallbackStub::HandleOnSendResult;
}

int32_t AssetSendCallbackStub::OnRemoteRequest(uint32_t code,
                                               MessageParcel &data,
                                               MessageParcel &reply,
                                               MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return ASSET_SEND_CALLBACK_DESCRIPTOR_IS_EMPTY;
    }
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        LOGE("Cannot response request %{public}d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*(interfaceIndex->second))(data, reply);
}

int32_t AssetSendCallbackStub::HandleOnSendResult(MessageParcel &data, MessageParcel &reply)
{
    sptr<AssetObj> assetObj = data.ReadParcelable<AssetObj>();
    if (!assetObj) {
        LOGE("object of AssetObj is nullptr");
        return E_INVAL_ARG;
    }

    int32_t result;
    if (!data.ReadInt32(result)) {
        LOGE("read result failed");
        return E_INVAL_ARG;
    }
    int32_t res = OnSendResult(assetObj, result);
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