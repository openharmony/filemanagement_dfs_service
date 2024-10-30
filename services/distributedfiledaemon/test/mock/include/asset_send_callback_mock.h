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
#ifndef OHOS_STORAGE_FILE_DFS_IASSET_SEND_CALLBACK_MOCK_H
#define OHOS_STORAGE_FILE_DFS_IASSET_SEND_CALLBACK_MOCK_H

#include <gmock/gmock.h>

#include "asset/asset_obj.h"
#include "asset/i_asset_send_callback.h"
#include "iremote_stub.h"

namespace OHOS::Storage::DistributedFile {
class IAssetSendCallbackMock : public IRemoteStub<IAssetSendCallback> {
public:
    int code_ = 0;
    IAssetSendCallbackMock() : code_(0) {}
    virtual ~IAssetSendCallbackMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));
    MOCK_METHOD2(OnSendResult, int32_t(const sptr<AssetObj> &assetObj, int32_t result));
};
} // namespace OHOS::Storage::DistributedFile
#endif