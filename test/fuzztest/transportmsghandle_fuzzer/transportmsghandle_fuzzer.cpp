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
#include "transportmsghandle_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>

#include "cloud_fuzzer_helper.h"
#include "message_handler.h"

namespace OHOS {
constexpr size_t U16_AT_SIZE = 2;
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t U64_AT_SIZE = 8;
constexpr int SPLITE_SIZE = 3;

using namespace std;
using namespace OHOS::FileManagement::CloudSync;

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzData fuzzData(data, size);
    uint16_t msgType = fuzzData.GetData<uint16_t>();
    int32_t errorCode = fuzzData.GetData<int32_t>();
    int32_t userId = fuzzData.GetData<int32_t>();
    uint64_t taskId = fuzzData.GetData<uint64_t>();
    int len = static_cast<int>(fuzzData.GetRemainSize()) / SPLITE_SIZE;
    if (len <= 0) {
        return false;
    }
    std::string srcNetworkId = fuzzData.GetStringFromData(len);
    std::string dstNetworkId = fuzzData.GetStringFromData(len);
    std::string uri = fuzzData.GetStringFromData(len);
    MessageInputInfo msgInputInfo = {.srcNetworkId = srcNetworkId,
                                     .dstNetworkId = dstNetworkId,
                                     .uri = uri,
                                     .msgType = msgType,
                                     .errorCode = errorCode,
                                     .userId = userId,
                                     .taskId = taskId};
    MessageHandler msgHandle(msgInputInfo);
    auto dataSubPtr = std::make_unique<uint8_t[]>(size);
    msgHandle.PackData(dataSubPtr.get(), static_cast<uint32_t>(size));
    MessageHandler msgHandleSub;
    msgHandleSub.UnPackData(dataSubPtr.get(), msgHandle.GetDataSize());
    msgHandleSub.GetTaskId();
    msgHandleSub.GetMsgType();
    msgHandleSub.GetUserId();
    msgHandleSub.GetErrorCode();
    msgHandleSub.GetUri();
    msgHandleSub.GetSrcNetworkId();
    msgHandleSub.GetDstNetworkId();
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= (OHOS::U16_AT_SIZE + OHOS::U64_AT_SIZE + (OHOS::U32_AT_SIZE << 1))) {
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
