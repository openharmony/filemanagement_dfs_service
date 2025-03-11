/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "cloud_report_cacher.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudReportCacherTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    CloudReportCacher& Cacher = CloudReportCacher::GetInstance();
};

void CloudReportCacherTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudReportCacherTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudReportCacherTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudReportCacherTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(CloudReportCacherTest, InitTest, TestSize.Level1)
{
    std::string bundleName;
    std::string traceId;
    uint32_t scenarioCode = 1;

    Cacher.Init(bundleName, traceId, scenarioCode);

    bundleName = "bundleName";
    traceId = "traceId";
    Cacher.Init(bundleName, traceId, scenarioCode);
    EXPECT_EQ(bundleName, "bundleName");
}

HWTEST_F(CloudReportCacherTest, SetScenarioCodeTest, TestSize.Level1)
{
    std::string bundleName;
    uint32_t scenarioCode = 1;

    Cacher.SetScenarioCode(bundleName, scenarioCode);

    bundleName = "bundleName";
    Cacher.SetScenarioCode(bundleName, scenarioCode);
    EXPECT_EQ(bundleName, "bundleName");
}

HWTEST_F(CloudReportCacherTest, PushEventTest, TestSize.Level1)
{
    std::string bundleName = "";
    CloudFaultInfo event;

    Cacher.PushEvent(bundleName, event);
    EXPECT_EQ(bundleName, "");
}

HWTEST_F(CloudReportCacherTest, ReportTest, TestSize.Level1)
{
    std::string bundleName = "";
    int scheduleType = 0;

    Cacher.Report(bundleName, scheduleType);
    scheduleType = 1;
    Cacher.Report(bundleName, scheduleType);
    EXPECT_EQ(scheduleType, 1);
}

} // namespace OHOS::FileManagement::CloudFile