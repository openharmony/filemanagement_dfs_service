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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "cloud_file_fault_event.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudFileFultEventTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    CloudFaultReportStatus& status = CloudFaultReportStatus::GetInstance();
};

void CloudFileFultEventTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudFileFultEventTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudFileFultEventTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudFileFultEventTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(CloudFileFultEventTest, CloudSyncFaultReportTest_001, TestSize.Level1)
{
    std::string functionName = "TestFunction";
    int lineNumber = 123;
    CloudSyncFaultInfo event;
    event.bundleName_ = "com.example.app";
    event.faultScenario_ = FaultScenarioCode::CLOUD_FULL_SYNC;
    event.faultType_ = FaultType::TIMEOUT;
    event.faultErrorCode_ = -1;
    event.message_ = "Sync failed";
    int32_t result = CloudFileFaultEvent::CloudSyncFaultReport(functionName, lineNumber, event);
    EXPECT_EQ(result, event.faultErrorCode_);
}

HWTEST_F(CloudFileFultEventTest, IsAllowToReporttTest_001, TestSize.Level1)
{
    FaultType faultType = FaultType::OPEN_CLOUD_FILE_TIMEOUT;
    EXPECT_TRUE(status.IsAllowToReport(faultType));
}

HWTEST_F(CloudFileFultEventTest, IsAllowToReporttTest_002, TestSize.Level1)
{
    FaultType faultType = FaultType::OPEN_CLOUD_FILE_TIMEOUT;
    status.IsAllowToReport(faultType);
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    status.lastTimeMap_[faultType] = t.tv_sec;

    EXPECT_FALSE(status.IsAllowToReport(faultType));
}

}