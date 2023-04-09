/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "data_sync/data_sync_manager.h"
#include "dfs_error.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/cloud_status.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DataSyncManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<DataSyncManager> dataSyncManager_;
};
void DataSyncManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DataSyncManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DataSyncManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    dataSyncManager_ = make_shared<DataSyncManager>();
}

void DataSyncManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetDataSyncerTest
 * @tc.desc: Verify the GetDataSyncer function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, GetDataSyncerTest, TestSize.Level1)
{
    int32_t userId = 100;
    string bundleName = "com.ohos.test";
    auto dataSyncer = dataSyncManager_->GetDataSyncer(bundleName, userId);
    EXPECT_EQ(userId, dataSyncer->GetUserId());
    EXPECT_EQ(bundleName, dataSyncer->GetBundleName());
}

/**
 * @tc.name: GetExistDataSyncerTest
 * @tc.desc: Verify the GetDataSyncer function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, GetExistDataSyncerTest, TestSize.Level1)
{
    int32_t userId = 100;
    string bundleName = "com.ohos.test";

    auto dataSyncer = dataSyncManager_->GetDataSyncer(bundleName, userId);
    EXPECT_EQ(userId, dataSyncer->GetUserId());
    EXPECT_EQ(bundleName, dataSyncer->GetBundleName());

    dataSyncer = dataSyncManager_->GetDataSyncer(bundleName, userId);
    EXPECT_EQ(userId, dataSyncer->GetUserId());
    EXPECT_EQ(bundleName, dataSyncer->GetBundleName());
    EXPECT_EQ(dataSyncManager_->dataSyncers_.size(), 1);
}

/**
 * @tc.name: GetDataSyncerdifferentBundleNameTest
 * @tc.desc: Verify the GetDataSyncer function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, GetDataSyncerdifferentBundleNameTest, TestSize.Level1)
{
    int32_t userId = 100;
    string bundleName = "com.ohos.test";

    auto dataSyncer = dataSyncManager_->GetDataSyncer(bundleName, userId);
    EXPECT_EQ(userId, dataSyncer->GetUserId());
    EXPECT_EQ(bundleName, dataSyncer->GetBundleName());

    string bundleName2 = "com.ohos.test2";

    dataSyncer = dataSyncManager_->GetDataSyncer(bundleName2, userId);
    EXPECT_EQ(userId, dataSyncer->GetUserId());
    EXPECT_EQ(bundleName2, dataSyncer->GetBundleName());
    EXPECT_EQ(dataSyncManager_->dataSyncers_.size(), 2);
}

/**
 * @tc.name: IsSkipSyncOnBatteryNoOkTest
 * @tc.desc: Verify the GetDataSyncer function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, IsSkipSyncOnBatteryNoOkTest, TestSize.Level1)
{
    int32_t userId = 100;
    string bundleName = "com.ohos.test";
    BatteryStatus::level_ = BatteryStatus::CapacityLevel::LEVEL_TOO_LOW;
    auto ret = dataSyncManager_->IsSkipSync(bundleName, userId);
    BatteryStatus::level_ = BatteryStatus::CapacityLevel::LEVEL_NORMAL;
    EXPECT_EQ(E_SYNC_FAILED_BATTERY_TOO_LOW, ret);
}

/**
 * @tc.name: IsSkipSyncOKTest
 * @tc.desc: Verify the GetDataSyncer function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, IsSkipSyncOKTest, TestSize.Level1)
{
    int32_t userId = 100;
    string bundleName = "com.ohos.test";

    auto ret = dataSyncManager_->IsSkipSync(bundleName, userId);
    EXPECT_EQ(E_OK, ret);
}
} // namespace OHOS::FileManagement::CloudSync::Test