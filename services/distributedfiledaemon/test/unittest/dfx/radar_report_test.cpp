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

#include "radar_report.h"

namespace OHOS::Storage::DistributedFile {
using namespace std;
using namespace OHOS;
using namespace OHOS::FileManagement;
using namespace OHOS::Storage::DistributedFile;
using namespace testing::ext;
using namespace testing;

class RadarReportAdapterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void RadarReportAdapterTest::SetUpTestCase()
{
}

void RadarReportAdapterTest::TearDownTestCase()
{
}

void RadarReportAdapterTest::SetUp()
{
}

void RadarReportAdapterTest::TearDown()
{
}

/**
 * @tc.name: RadarReportAdapterTest_StorageRadarThd_001
 * @tc.desc: verify StorageRadarThd.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_StorageRadarThd_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_StorageRadarThd_001 begin";
    RadarReportAdapter::GetInstance().InitRadar();
    sleep(1);
    RadarReportAdapter::GetInstance().stopRadarReport_ = true;
    RadarReportAdapter::GetInstance().UnInitRadar();
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_StorageRadarThd_001 end";
}

/**
 * @tc.name: RadarReportAdapterTest_SetUserStatistics_001
 * @tc.desc: verify SetUserStatistics.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_SetUserStatistics_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_SetUserStatistics_001 begin";
    RadarReportAdapter::GetInstance().SetUserStatistics(FileManagement::RadarStatisticInfoType::CONNECT_DFS_SUCC_CNT);
    EXPECT_EQ(RadarReportAdapter::GetInstance().opStatistics_.connectSuccCount, 1);
    RadarReportAdapter::GetInstance().SetUserStatistics(FileManagement::RadarStatisticInfoType::CONNECT_DFS_FAIL_CNT);
    EXPECT_EQ(RadarReportAdapter::GetInstance().opStatistics_.connectFailCount, 1);
    RadarReportAdapter::GetInstance().SetUserStatistics(FileManagement::RadarStatisticInfoType::GENERATE_DIS_URI_SUCC_CNT);
    EXPECT_EQ(RadarReportAdapter::GetInstance().opStatistics_.generateUriSuccCount, 1);
    RadarReportAdapter::GetInstance().SetUserStatistics(FileManagement::RadarStatisticInfoType::GENERATE_DIS_URI_FAIL_CNT);
    EXPECT_EQ(RadarReportAdapter::GetInstance().opStatistics_.generateUriFailCount, 1);
    RadarReportAdapter::GetInstance().SetUserStatistics(FileManagement::RadarStatisticInfoType::FILE_ACCESS_SUCC_CNT);
    EXPECT_EQ(RadarReportAdapter::GetInstance().opStatistics_.fileAccessSuccCount, 1);
    RadarReportAdapter::GetInstance().SetUserStatistics(FileManagement::RadarStatisticInfoType::FILE_ACCESS_FAIL_CNT);
    EXPECT_EQ(RadarReportAdapter::GetInstance().opStatistics_.fileAccessFailCount, 1);
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().SetUserStatistics(static_cast<RadarStatisticInfoType>(6)));
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_SetUserStatistics_001 end";
}

/**
 * @tc.name: RadarReportAdapterTest_ReportDfxStatistics_001
 * @tc.desc: verify ReportDfxStatistics.
 * @tc.type: FUNC
 * @tc.require: 
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_ReportDfxStatistics_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportDfxStatistics_001 begin";
    RadarReportAdapter::GetInstance().opStatistics_ = {0, 0, 0, 0, 0, 0};
    RadarReportAdapter::GetInstance().ReportDfxStatistics();
    EXPECT_TRUE(RadarReportAdapter::GetInstance().opStatistics_.empty());
    RadarReportAdapter::GetInstance().SetUserStatistics(FileManagement::RadarStatisticInfoType::CONNECT_DFS_SUCC_CNT);
    RadarReportAdapter::GetInstance().ReportDfxStatistics();
    EXPECT_TRUE(RadarReportAdapter::GetInstance().opStatistics_.empty());
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportDfxStatistics_001 end";
}
}