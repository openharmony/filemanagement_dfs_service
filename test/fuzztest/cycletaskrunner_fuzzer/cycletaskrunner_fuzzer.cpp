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
#include "cycletaskrunner_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <set>
#include <string>

#include "cloud_file_kit.h"
#include "cloud_fuzzer_helper.h"
#include "cycle_task_runner.h"
#include "optimize_storage_task.h"
#include "periodic_check_task.h"
#include "report_statistics_task.h"
#include "save_subscription_task.h"

namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;
constexpr int SPLITE_SIZE = 3;
using namespace std;
using namespace OHOS::FileManagement;
using namespace OHOS::FileManagement::CloudSync;

class CloudFileKitMock : public CloudFile::CloudFileKit {};

__attribute__((used)) static bool g_isInit =
    CloudFile::CloudFileKit::RegisterCloudInstance(new (std::nothrow) CloudFileKitMock());

void CycleTaskDerivedFuzzTest(const uint8_t *data, size_t size)
{
    FuzzData fuzzData(data, size);
    auto dataSyncManager = make_shared<CloudFile::DataSyncManager>();
    CycleTaskRunner cyclerunner(dataSyncManager);
    cyclerunner.StartTask();

    int32_t userId = fuzzData.GetData<int32_t>();
    auto remainSize = fuzzData.GetRemainSize();
    std::string bundleName = fuzzData.GetStringFromData(static_cast<int>(remainSize));
    std::set<std::string> bundleNames;
    bundleNames.insert(bundleName);
    auto bundleNamesPtr = make_shared<std::set<std::string>>(bundleNames);
    shared_ptr<CycleTask> saveSubscriptionTaskPtr = make_shared<SaveSubscriptionTask>(dataSyncManager);
    saveSubscriptionTaskPtr->SetRunnableBundleNames(bundleNamesPtr);
    saveSubscriptionTaskPtr->RunTask(userId);
    saveSubscriptionTaskPtr->RunTaskForBundle(userId, bundleName);

    shared_ptr<CycleTask> optimizeStorageTaskPtr = make_shared<OptimizeStorageTask>(dataSyncManager);
    optimizeStorageTaskPtr->SetRunnableBundleNames(bundleNamesPtr);
    optimizeStorageTaskPtr->RunTask(userId);
    optimizeStorageTaskPtr->RunTaskForBundle(userId, bundleName);

    shared_ptr<CycleTask> reportStatisticsTaskPtr = make_shared<ReportStatisticsTask>(dataSyncManager);
    reportStatisticsTaskPtr->SetRunnableBundleNames(bundleNamesPtr);
    reportStatisticsTaskPtr->RunTask(userId);
    reportStatisticsTaskPtr->RunTaskForBundle(userId, bundleName);

    shared_ptr<CycleTask> periodicCheckTaskPtr = make_shared<PeriodicCheckTask>(dataSyncManager);
    periodicCheckTaskPtr->SetRunnableBundleNames(bundleNamesPtr);
    periodicCheckTaskPtr->RunTask(userId);
    periodicCheckTaskPtr->RunTaskForBundle(userId, bundleName);
    periodicCheckTaskPtr->GetTaskName();
    periodicCheckTaskPtr->GetDataSyncManager();
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || (size <= OHOS::U32_AT_SIZE << OHOS::SPLITE_SIZE)) {
        return 0;
    }

    OHOS::CycleTaskDerivedFuzzTest(data, size);
    return 0;
}
