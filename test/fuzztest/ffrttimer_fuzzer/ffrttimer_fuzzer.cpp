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
#include "ffrttimer_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "cloud_fuzzer_helper.h"
#include "ffrt_timer.h"

namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;

using namespace OHOS::FileManagement::CloudFile;
using namespace std;

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzData fuzzData(data, size);
    int len = static_cast<int>(size - (U32_AT_SIZE << 1));
    string name = fuzzData.GetStringFromData(len + 1);
    FfrtTimer fTimer{name};
    uint32_t interval = fuzzData.GetData<uint32_t>();
    uint32_t repatTimes = fuzzData.GetData<uint32_t>();
    fTimer.Start([]() -> void {}, interval, repatTimes);
    fTimer.Start([]() -> void {}, interval, repatTimes);
    fTimer.Stop();
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= (OHOS::U32_AT_SIZE << 1)) {
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}
