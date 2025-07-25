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

#ifndef MOCK_I_CLOUD_SYNC_CALLBACK_H
#define MOCK_I_CLOUD_SYNC_CALLBACK_H


#include <gmock/gmock.h>

#include "i_cloud_sync_callback.h"
#include "i_downgrade_dl_callback.h"
#include "iremote_stub.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudSync {

class CloudSyncCallbackMock : public IRemoteStub<ICloudSyncCallback> {
public:
    int code_;
    CloudSyncCallbackMock() : code_(0) {}
    virtual ~CloudSyncCallbackMock() {}

    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));

    int32_t InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return E_OK;
    }

    void OnSyncStateChanged(CloudSyncState state, ErrorType error) override {}
};

class DowngradeDlCallbackMock : public IRemoteStub<IDowngradeDlCallback> {
public:
    DowngradeDlCallbackMock() = default;
    virtual ~DowngradeDlCallbackMock() = default;

    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));
    MOCK_METHOD1(OnDownloadProcess, void(const DowngradeProgress &progress));
};
} // namespace OHOS::FileManagement::CloudSync

#endif // MOCK_I_CLOUD_SYNC_CALLBACK_H
