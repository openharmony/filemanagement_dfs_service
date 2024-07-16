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
#include "cloudsynccallbackstub_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "cloud_fuzzer_helper.h"
#include "cloud_sync_callback_client.h"
#include "cloud_sync_callback_stub.h"

#include "message_parcel.h"
#include "utils_log.h"

namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;

using namespace OHOS::FileManagement::CloudSync;
using namespace std;

class CloudSyncCallbackTest : public CloudSyncCallbackStub {
public:
    void OnSyncStateChanged(SyncType type, SyncPromptState state) override {}
    void OnSyncStateChanged(CloudSyncState state, ErrorType error) override {}
};

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(CloudSyncCallbackStub::GetDescriptor())) {
        LOGE("Write token failed.");
        return false;
    }
    return true;
}

void HandleOnSyncStateChangedFuzzTest(std::shared_ptr<CloudSyncCallbackStub> cloudSyncCallbackStubStr,
                                      const uint8_t *data,
                                      size_t size)
{
    FuzzData fuzzData(data, size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    int32_t state = fuzzData.GetData<int32_t>();
    datas.WriteInt32(state);
    int32_t error = fuzzData.GetData<int32_t>();
    datas.WriteInt32(error);
    datas.RewindRead(0);
    // SERVICE_CMD_ON_SYNC_STATE_CHANGED
    uint32_t code = 0;
    MessageParcel reply;
    MessageOption option;

    cloudSyncCallbackStubStr->OnRemoteRequest(code, datas, reply, option);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size < OHOS::U32_AT_SIZE) {
        return 0;
    }

    auto cloudSyncCallackPtr = std::make_shared<OHOS::CloudSyncCallbackTest>();
    auto cloudSyncCallbackStubStr =
        std::make_shared<OHOS::FileManagement::CloudSync::CloudSyncCallbackClient>(cloudSyncCallackPtr);
    OHOS::HandleOnSyncStateChangedFuzzTest(cloudSyncCallbackStubStr, data, size);
    return 0;
}
