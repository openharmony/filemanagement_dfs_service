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
#include "clouddownloadcallbackstub_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "cloud_download_callback_client.h"
#include "cloud_download_callback_stub.h"
#include "cloud_fuzzer_helper.h"
#include "cloud_sync_common.h"

#include "message_parcel.h"
#include "utils_log.h"

namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t U64_AT_SIZE = 8;
constexpr size_t SPLITE_SIZE = 3;

using namespace OHOS::FileManagement::CloudSync;
using namespace std;

class CloudDownloadCallbackTest : public CloudDownloadCallback {
public:
    void OnDownloadProcess(const DownloadProgressObj &progress) override {}
};

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(CloudDownloadCallbackStub::GetDescriptor())) {
        LOGE("Write token failed.");
        return false;
    }
    return true;
}

void HandleOnProcessFuzzTest(std::shared_ptr<CloudDownloadCallbackStub> cloudDownloadCallbackStubStr,
                             const uint8_t *data,
                             size_t size)
{
    FuzzData fuzzData(data, size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    DownloadProgressObj downloadProgressObj;
    downloadProgressObj.refCount = fuzzData.GetData<uint32_t>();
    downloadProgressObj.downloadId = fuzzData.GetData<int64_t>();
    downloadProgressObj.state =
        static_cast<DownloadProgressObj::Status>(fuzzData.GetData<int32_t>() % (SPLITE_SIZE + 1));
    downloadProgressObj.downloadErrorType =
        static_cast<DownloadProgressObj::DownloadErrorType>(fuzzData.GetData<int32_t>() % (SPLITE_SIZE + SPLITE_SIZE));
    downloadProgressObj.downloadedSize = fuzzData.GetData<int64_t>();
    downloadProgressObj.totalSize = fuzzData.GetData<int64_t>();
    int len = static_cast<int>(size - (OHOS::U32_AT_SIZE * SPLITE_SIZE + OHOS::U64_AT_SIZE * SPLITE_SIZE));
    downloadProgressObj.path = fuzzData.GetStringFromData(len);
    datas.WriteParcelable(&downloadProgressObj);
    datas.RewindRead(0);
    // SERVICE_CMD_ON_PROCESS
    uint32_t code = 0;
    MessageParcel reply;
    MessageOption option;

    cloudDownloadCallbackStubStr->OnRemoteRequest(code, datas, reply, option);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size < (OHOS::U32_AT_SIZE * OHOS::SPLITE_SIZE + OHOS::U64_AT_SIZE * OHOS::SPLITE_SIZE)) {
        return 0;
    }

    auto cloudDownloadCallbackPtr = std::make_shared<OHOS::CloudDownloadCallbackTest>();
    auto cloudDownloadCallbackStubStr =
        std::make_shared<OHOS::FileManagement::CloudSync::CloudDownloadCallbackClient>(cloudDownloadCallbackPtr);
    OHOS::HandleOnProcessFuzzTest(cloudDownloadCallbackStubStr, data, size);
    return 0;
}
