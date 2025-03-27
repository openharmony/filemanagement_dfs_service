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
#include "cloudsyncmanager_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>

#include "cloud_fuzzer_helper.h"
#include "cloud_sync_manager.h"
#include "cloud_sync_manager_impl.h"
#include "service_proxy.h"
#include "i_cloud_download_callback.h"
#include "i_cloud_sync_callback.h"

namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t BOOL_AT_SIZE = 1;

using namespace OHOS::FileManagement::CloudSync;
using namespace std;
class ICloudSyncCallbackTest : public IRemoteStub<ICloudSyncCallback> {
public:
    void OnSyncStateChanged(SyncType type, SyncPromptState state) override {}
    void OnSyncStateChanged(CloudSyncState state, ErrorType error) override {}
};

class ICloudDownloadCallbackTest : public IRemoteStub<ICloudDownloadCallback> {
public:
    void OnDownloadProcess(const DownloadProgressObj &progress) override {}
};

void RegisterCallbackFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::string bundleName = fuzzData.GetStringFromData(static_cast<int>(size));
    auto cloudSyncCallback = make_shared<ICloudSyncCallbackTest>();
    CloudSyncManager::GetInstance().RegisterCallback(cloudSyncCallback, bundleName);
}

void UnRegisterCallbackFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::string bundleName = fuzzData.GetStringFromData(static_cast<int>(size));
    CloudSyncManager::GetInstance().UnRegisterCallback(bundleName);
}

void StartSyncFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::string bundleName = fuzzData.GetStringFromData(static_cast<int>(size));
    CloudSyncManager::GetInstance().StartSync(bundleName);
}

void GetSyncTimeFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    int64_t syncTime{0};
    std::string bundleName = fuzzData.GetStringFromData(static_cast<int>(size));
    CloudSyncManager::GetInstance().GetSyncTime(syncTime, bundleName);
}

void StartSyncCallbackFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    bool forceFlag = fuzzData.GetData<bool>();
    auto cloudSyncCallback = make_shared<ICloudSyncCallbackTest>();
    CloudSyncManager::GetInstance().StartSync(forceFlag, cloudSyncCallback);
}

void TriggerSyncFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    int32_t userId = 100;
    std::string bundleName = fuzzData.GetStringFromData(static_cast<int>(size));
    CloudSyncManager::GetInstance().TriggerSync(bundleName, userId);

    fuzzData.ResetData(size);
    userId = fuzzData.GetData<int32_t>();
    bundleName = fuzzData.GetStringFromData(static_cast<int>(size - U32_AT_SIZE));
    CloudSyncManager::GetInstance().TriggerSync(bundleName, userId);
}

void StopSyncFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::string bundleName = fuzzData.GetStringFromData(static_cast<int>(size));
    CloudSyncManager::GetInstance().StopSync(bundleName);
}

void ChangeAppSwitchFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    bool status = fuzzData.GetData<bool>();
    int len = static_cast<int>((size - BOOL_AT_SIZE) >> 1);
    std::string accoutId = fuzzData.GetStringFromData(len);
    string bundleName = fuzzData.GetStringFromData(static_cast<int>(len));
    CloudSyncManager::GetInstance().ChangeAppSwitch(accoutId, bundleName, status);
}

void NotifyDataChangeFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    int len = static_cast<int>(size >> 1);
    std::string accoutId = fuzzData.GetStringFromData(len);
    std::string bundleName = fuzzData.GetStringFromData(len);
    CloudSyncManager::GetInstance().NotifyDataChange(accoutId, bundleName);
}

void StartDownloadFileFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    string uri = fuzzData.GetStringFromData(static_cast<int>(size));
    CloudSyncManager::GetInstance().StartDownloadFile(uri);
}

void StartFileCacheFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    string uri = fuzzData.GetStringFromData(static_cast<int>(size));
    CloudSyncManager::GetInstance().StartFileCache(uri);
}

void StopDownloadFileFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    bool needClean = fuzzData.GetData<bool>();
    string uri = fuzzData.GetStringFromData(static_cast<int>(size - BOOL_AT_SIZE));
    CloudSyncManager::GetInstance().StopDownloadFile(uri, needClean);
}

void RegisterDownloadFileCallbackFuzzTest(FuzzData &fuzzData, size_t size)
{
    auto downloadCallback = make_shared<ICloudDownloadCallbackTest>();
    CloudSyncManager::GetInstance().RegisterDownloadFileCallback(downloadCallback);
    CloudSyncManager::GetInstance().UnregisterDownloadFileCallback();
}

void EnableCloudFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    int len = static_cast<int>(size >> 1);
    std::string accoutId = fuzzData.GetStringFromData(len);
    string itemStr = fuzzData.GetStringFromData(len - static_cast<int>(BOOL_AT_SIZE));
    bool itemBool = fuzzData.GetData<bool>();
    SwitchDataObj switchDataObj;
    switchDataObj.switchData.insert({itemStr, itemBool});
    CloudSyncManager::GetInstance().EnableCloud(accoutId, switchDataObj);
    auto proxy = ServiceProxy::GetInstance();
    CloudSyncManagerImpl::GetInstance().SetDeathRecipient(proxy->AsObject());
}

void DisableCloudFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    std::string accoutId = fuzzData.GetStringFromData(static_cast<int>(size));
    CloudSyncManager::GetInstance().DisableCloud(accoutId);
}

void CleanFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    int len = static_cast<int>(size >> 1);
    std::string accoutId = fuzzData.GetStringFromData(len);
    int32_t itemInt = fuzzData.GetData<int32_t>();
    auto remainSize = fuzzData.GetRemainSize();
    string itemStr = fuzzData.GetStringFromData(static_cast<int>(remainSize));
    CleanOptions cleanOptions;
    cleanOptions.appActionsData.insert({itemStr, itemInt});
    CloudSyncManager::GetInstance().Clean(accoutId, cleanOptions);
}

void CleanCacheFuzzTest(FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    string uri = fuzzData.GetStringFromData(static_cast<int>(size));
    CloudSyncManager::GetInstance().CleanCache(uri);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= (OHOS::U32_AT_SIZE << 1)) {
        return 0;
    }

    OHOS::FuzzData fuzzData(data, size);
    OHOS::RegisterCallbackFuzzTest(fuzzData, size);
    OHOS::UnRegisterCallbackFuzzTest(fuzzData, size);
    OHOS::StartSyncFuzzTest(fuzzData, size);
    OHOS::GetSyncTimeFuzzTest(fuzzData, size);
    OHOS::StartSyncCallbackFuzzTest(fuzzData, size);
    OHOS::TriggerSyncFuzzTest(fuzzData, size);
    OHOS::StopSyncFuzzTest(fuzzData, size);
    OHOS::ChangeAppSwitchFuzzTest(fuzzData, size);
    OHOS::NotifyDataChangeFuzzTest(fuzzData, size);
    OHOS::StartDownloadFileFuzzTest(fuzzData, size);
    OHOS::StartFileCacheFuzzTest(fuzzData, size);
    OHOS::StopDownloadFileFuzzTest(fuzzData, size);
    OHOS::RegisterDownloadFileCallbackFuzzTest(fuzzData, size);
    OHOS::EnableCloudFuzzTest(fuzzData, size);
    OHOS::DisableCloudFuzzTest(fuzzData, size);
    OHOS::CleanFuzzTest(fuzzData, size);
    OHOS::CleanCacheFuzzTest(fuzzData, size);
    return 0;
}
