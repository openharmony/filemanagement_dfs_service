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
#include "clouddaemonmanager_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "cloud_daemon_manager.h"
#include "cloud_daemon_manager_impl.h"
#include "cloud_fuzzer_helper.h"
namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;

using namespace OHOS::FileManagement::CloudFile;
using namespace std;

void StartFuseFuzzTest(const uint8_t *data, size_t size)
{
    FuzzData fuzzData(data, size);
    int32_t userId = fuzzData.GetData<int32_t>();
    int32_t devFd = fuzzData.GetData<int32_t>();
    int len = static_cast<int>(fuzzData.GetRemainSize());
    std::string path = fuzzData.GetStringFromData(len);
    CloudDaemonManager::GetInstance().StartFuse(userId, devFd, path);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= (OHOS::U32_AT_SIZE << 1)) {
        return 0;
    }

    OHOS::StartFuseFuzzTest(data, size);
    return 0;
}
