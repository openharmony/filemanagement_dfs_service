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
#include "downloadassetcbmgr_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "cloud_fuzzer_helper.h"
#include "download_asset_callback_manager.h"
#include "iremote_stub.h"

namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t U64_AT_SIZE = 8;

using namespace std;
using namespace OHOS::FileManagement::CloudSync;

class IDownloadAssetCallbackTest : public IRemoteStub<IDownloadAssetCallback> {
public:
    using TaskId = uint64_t;
    void OnFinished(const TaskId taskId, const std::string &uri, const int32_t result) override {}
};

bool DoSomethingInterestingWithMyAPI(const uint8_t *data, size_t size)
{
    FuzzData fuzzData(data, size);
    sptr<IDownloadAssetCallbackTest> callback = new (std::nothrow) IDownloadAssetCallbackTest();
    if (callback == nullptr) {
        return false;
    }
    uint64_t taskId = fuzzData.GetData<uint64_t>();
    int32_t result = fuzzData.GetData<int32_t>();
    size_t remainSize = fuzzData.GetRemainSize();
    string uri = fuzzData.GetStringFromData(static_cast<int>(remainSize));
    DownloadAssetCallbackManager::GetInstance().AddCallback(callback);
    DownloadAssetCallbackManager::GetInstance().OnDownloadFinshed(taskId, uri, result);
    return true;
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= OHOS::U32_AT_SIZE + OHOS::U64_AT_SIZE) {
        return 0;
    }

    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}