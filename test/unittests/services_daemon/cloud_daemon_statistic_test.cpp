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

#include "cloud_daemon_statistic.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudFile::test {
using namespace testing;
using namespace testing::ext;
using namespace std;

#define MODEL_TIME 20240710
#define MODEL_TYPE 1
#define MODEL_SIZE 12

class CloudDaemonStatisticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDaemonStatisticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDaemonStatisticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDaemonStatisticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDaemonStatisticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(CloudDaemonStatisticTest, CloudDaemonStatisticTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_001 Start";
    try {
        CloudDaemonStatistic::GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_001  ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_001 End";
}

HWTEST_F(CloudDaemonStatisticTest, CloudDaemonStatisticTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_002 Start";
    try {
        CloudDaemonStatistic::GetInstance().UpdateStatData();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_002  ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_002 End";
}

HWTEST_F(CloudDaemonStatisticTest, CloudDaemonStatisticTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_003 Start";
    CloudDaemonStatistic cds;
    try {
        cds.UpdateOpenSizeStat(MODEL_SIZE);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_003  ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_003 End";
}

HWTEST_F(CloudDaemonStatisticTest, CloudDaemonStatisticTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_004 Start";
    CloudDaemonStatistic cds;
    try {
        cds.UpdateReadSizeStat(MODEL_SIZE);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_004  ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_004 End";
}

HWTEST_F(CloudDaemonStatisticTest, CloudDaemonStatisticTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_005 Start";
    CloudDaemonStatistic cds;
    try {
        cds.UpdateReadTimeStat(MODEL_TYPE, MODEL_TIME);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_005  ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_005 End";
}

HWTEST_F(CloudDaemonStatisticTest, CloudDaemonStatisticTest_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_006 Start";
    CloudDaemonStatistic cds;
    try {
        cds.UpdateStatData();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_006  ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_006 End";
}

HWTEST_F(CloudDaemonStatisticTest, CloudDaemonStatisticTest_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_007 Start";
    CloudDaemonStatistic &readStat = CloudDaemonStatistic::GetInstance();
    try {
        readStat.UpdateOpenTimeStat(MODEL_TYPE, MODEL_TIME);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_007  ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_007 End";
}
}