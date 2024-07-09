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
#include "downloadassetcallbackstub_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "cloud_fuzzer_helper.h"
#include "download_asset_callback_client.h"
#include "download_asset_callback_stub.h"

#include "message_parcel.h"
#include "utils_log.h"

namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t U64_AT_SIZE = 8;

using namespace OHOS::FileManagement::CloudSync;
using namespace std;

bool WriteInterfaceToken(MessageParcel &datas)
{
    if (!datas.WriteInterfaceToken(DownloadAssetCallbackStub::GetDescriptor())) {
        LOGE("Write token failed.");
        return false;
    }
    return true;
}

void HandleOnFinishedFuzzTest(std::shared_ptr<DownloadAssetCallbackStub> downloadAssetCallbackStubStr,
                              const uint8_t *data,
                              size_t size)
{
    FuzzData fuzzData(data, size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    int32_t taskId = fuzzData.GetData<uint64_t>();
    datas.WriteUint64(taskId);
    int32_t result = fuzzData.GetData<int32_t>();
    string uri = fuzzData.GetStringFromData(static_cast<int>(size - U32_AT_SIZE - U64_AT_SIZE));
    datas.WriteString(uri);
    datas.WriteInt32(result);
    datas.RewindRead(0);
    // SERVICE_CMD_ON_DOWNLOAD_FINSHED
    uint32_t code = 0;
    MessageParcel reply;
    MessageOption option;

    downloadAssetCallbackStubStr->OnRemoteRequest(code, datas, reply, option);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size < (OHOS::U32_AT_SIZE + OHOS::U64_AT_SIZE)) {
        return 0;
    }

    auto downloadAssetCallbackStubStr =
        std::make_shared<OHOS::FileManagement::CloudSync::DownloadAssetCallbackClient>();
    OHOS::HandleOnFinishedFuzzTest(downloadAssetCallbackStubStr, data, size);
    return 0;
}
