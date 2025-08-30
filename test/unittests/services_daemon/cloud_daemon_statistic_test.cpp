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

HWTEST_F(CloudDaemonStatisticTest, CloudDaemonStatisticTest_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_008 Start";
    CloudDaemonStatistic cds;
    cds.videoReadInfo_[CACHE_SUM] = 0;

    try {
        cds.UpdateReadInfo(CACHE_SUM);
        EXPECT_EQ(cds.videoReadInfo_[CACHE_SUM], 1);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_008 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_008 End";
}

HWTEST_F(CloudDaemonStatisticTest, CloudDaemonStatisticTest_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_009 Start";
    CloudDaemonStatistic cds;
    cds.videoReadInfo_[CACHE_SUM] = 0;

    try {
        cds.UpdateReadInfo(VIDEO_READ_INFO);
        EXPECT_EQ(cds.videoReadInfo_[CACHE_SUM], 0);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_009 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_009 End";
}

HWTEST_F(CloudDaemonStatisticTest, CloudDaemonStatisticTest_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_010 Start";
    CloudDaemonStatistic cds;
    cds.videoReadInfo_[CACHE_SUM] = UINT32_MAX;

    try {
        cds.UpdateReadInfo(CACHE_SUM);
        EXPECT_EQ(cds.videoReadInfo_[CACHE_SUM], UINT32_MAX);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_010 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDaemonStatisticTest_010 End";
}

/**
 * @tc.name: UpdateBundleNameTest_001
 * @tc.desc: Verify the UpdateBundleName function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDaemonStatisticTest, UpdateBundleNameTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateBundleNameTest_001 Start";
    CloudDaemonStatistic cds;
    string bundleName = "test.bundle.name";

    try {
        cds.UpdateBundleName(bundleName);
        EXPECT_EQ(cds.bundleName_, bundleName);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateBundleNameTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "UpdateBundleNameTest_001 End";
}

/**
 * @tc.name: ReportReadStatTest_001
 * @tc.desc: Verify the ReportReadStat function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDaemonStatisticTest, ReportReadStatTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReportReadStatTest_001 Start";
    CloudDaemonStatisticInfo info;
    info.bundleName = "test.bundle.name";

    CloudDaemonStatistic cds;
    try {
        auto ret = cds.ReportReadStat(info);
        EXPECT_EQ(ret, 0);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReportReadStatTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "ReportReadStatTest_001 End";
}

/**
 * @tc.name: HandleBundleNameTest_001
 * @tc.desc: Verify the HandleBundleName function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDaemonStatisticTest, HandleBundleNameTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleBundleNameTest_001 Start";
    CloudDaemonStatistic cds;
    cds.bundleName_ = "test.bundle.name";
    CloudDaemonStatisticInfo info;
    info.bundleName = "test.bundle.name";
    try {
        cds.HandleBundleName(info);
        EXPECT_EQ(cds.bundleName_, "test.bundle.name");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleBundleNameTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleBundleNameTest_001 End";
}

/**
 * @tc.name: HandleBundleNameTest_002
 * @tc.desc: Verify the HandleBundleName function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDaemonStatisticTest, HandleBundleNameTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleBundleNameTest_002 Start";
    CloudDaemonStatistic cds;
    cds.bundleName_ = "test.bundle.name";
    CloudDaemonStatisticInfo info;
    info.bundleName = "different.bundle.name";
    try {
        cds.HandleBundleName(info);
        EXPECT_EQ(cds.bundleName_, "test.bundle.name");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleBundleNameTest_002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleBundleNameTest_002 End";
}

/**
 * @tc.name: HandleBundleNameTest_003
 * @tc.desc: Verify the HandleBundleName function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDaemonStatisticTest, HandleBundleNameTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleBundleNameTest_003 Start";
    CloudDaemonStatistic cds;
    cds.bundleName_ = "test.bundle.name";
    CloudDaemonStatisticInfo info;
    info.bundleName = "";
    try {
        cds.HandleBundleName(info);
        EXPECT_EQ(cds.bundleName_, "test.bundle.name");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleBundleNameTest_003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleBundleNameTest_003 End";
}

/**
 * @tc.name: HandleBundleNameTest_004
 * @tc.desc: Verify the HandleBundleName function
 * @tc.type: FUNC
 * @tc.require: ICQTGD
 */
HWTEST_F(CloudDaemonStatisticTest, HandleBundleNameTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleBundleNameTest_004 Start";
    CloudDaemonStatistic cds;
    cds.bundleName_ = "";
    CloudDaemonStatisticInfo info;
    info.bundleName = "";
    try {
        cds.HandleBundleName(info);
        EXPECT_EQ(cds.bundleName_, "");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleBundleNameTest_004 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleBundleNameTest_004 End";
}
}