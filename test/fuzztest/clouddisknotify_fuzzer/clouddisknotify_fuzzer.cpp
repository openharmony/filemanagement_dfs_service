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
#include "clouddisknotify_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "cloud_fuzzer_helper.h"
#include "clouddisk_notify.h"

namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;
constexpr int SPLITE_SIZE = 5;
constexpr size_t DIVIDED_NUM = 2;
constexpr uint32_t OPS_TYE_SIZE = 11;

using namespace OHOS::FileManagement::CloudDisk;
using namespace std;

void TryNotifyServiceFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    int len = static_cast<int>((size - U32_AT_SIZE)) / SPLITE_SIZE;
    if (len <= 0) {
        return;
    }
    NotifyParamService paramService;
    paramService.notifyType = static_cast<NotifyType>(fuzzData.GetData<uint32_t>());
    CacheNode cacheNode;
    cacheNode.cloudId = fuzzData.GetStringFromData(len);
    cacheNode.fileName = fuzzData.GetStringFromData(len);
    cacheNode.isDir = (size % DIVIDED_NUM == 0) ? "file" : "directory";
    cacheNode.parentCloudId = fuzzData.GetStringFromData(len);
    paramService.node = cacheNode;
    ParamServiceOther paramOthers;
    paramOthers.userId = fuzzData.GetData<int32_t>();
    paramOthers.bundleName = fuzzData.GetStringFromData(len);
    NotifyData notifyData;
    notifyData.uri = fuzzData.GetStringFromData(len);
    notifyData.isDir = (size % DIVIDED_NUM == 0);
    notifyData.type = paramService.notifyType;
    paramOthers.notifyDataList.emplace_back(notifyData);
    paramService.opsType = static_cast<NotifyOpsType>(fuzzData.GetData<uint32_t>() % OPS_TYE_SIZE);
    CloudDiskNotify::GetInstance().TryNotifyService(paramService, paramOthers);
}

void GetDeleteNotifyDataFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    ParamServiceOther paramOthers;
    paramOthers.userId = fuzzData.GetData<int32_t>();
    vector<NativeRdb::ValueObject> deleteIds;
    int len = static_cast<int>((size - U32_AT_SIZE) >> 1);
    string cloudId = fuzzData.GetStringFromData(len);
    deleteIds.emplace_back(cloudId);
    paramOthers.bundleName = fuzzData.GetStringFromData(len);
    vector<NotifyData> notifyDataList;
    CloudDiskNotify::GetInstance().GetDeleteNotifyData(deleteIds, notifyDataList, paramOthers);
}

void AddNotifyFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    NotifyData notifyData;
    notifyData.type = static_cast<NotifyType>(fuzzData.GetData<uint32_t>());
    int len = static_cast<int>(size - U32_AT_SIZE);
    notifyData.uri = fuzzData.GetStringFromData(len);
    notifyData.isDir = (size % DIVIDED_NUM == 0);
    CloudDiskNotify::GetInstance().AddNotify(notifyData);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= (OHOS::U32_AT_SIZE << 1) + static_cast<size_t>(OHOS::SPLITE_SIZE)) {
        return 0;
    }

    OHOS::FuzzData fuzzdata(data, size);
    OHOS::TryNotifyServiceFuzzTest(fuzzdata, size);
    OHOS::GetDeleteNotifyDataFuzzTest(fuzzdata, size);
    OHOS::AddNotifyFuzzTest(fuzzdata, size);
    return 0;
}
