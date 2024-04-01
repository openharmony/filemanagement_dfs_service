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

const int USER_ID = 100;
const std::string BUND_NAME = "com.ohos.photos";
const std::string BUND_NAME_TEST = "com.ohos.photos";
class DataSyncManagerTest : public testing::Test {
public:
    enum Action {
        RETAIN_DATA = 0,
        CLEAR_DATA
    };
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
    auto dataSyncer = dataSyncManager_->GetDataSyncer(BUND_NAME_TEST, USER_ID);
    EXPECT_EQ(USER_ID, dataSyncer->GetUserId());
    EXPECT_EQ(BUND_NAME_TEST, dataSyncer->GetBundleName());
}

/**
 * @tc.name: GetExistDataSyncerTest
 * @tc.desc: Verify the GetDataSyncer function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, GetExistDataSyncerTest, TestSize.Level1)
{
    auto dataSyncer = dataSyncManager_->GetDataSyncer(BUND_NAME_TEST, USER_ID);
    EXPECT_EQ(USER_ID, dataSyncer->GetUserId());
    EXPECT_EQ(BUND_NAME_TEST, dataSyncer->GetBundleName());

    dataSyncer = dataSyncManager_->GetDataSyncer(BUND_NAME_TEST, USER_ID);
    EXPECT_EQ(USER_ID, dataSyncer->GetUserId());
    EXPECT_EQ(BUND_NAME_TEST, dataSyncer->GetBundleName());
    EXPECT_EQ(dataSyncManager_->dataSyncersMap_.Size(), 1);
}

/**
 * @tc.name: GetDataSyncerdifferentBundleNameTest
 * @tc.desc: Verify the GetDataSyncer function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, GetDataSyncerdifferentBundleNameTest, TestSize.Level1)
{
    auto dataSyncer = dataSyncManager_->GetDataSyncer(BUND_NAME_TEST, USER_ID);
    EXPECT_EQ(USER_ID, dataSyncer->GetUserId());
    EXPECT_EQ(BUND_NAME_TEST, dataSyncer->GetBundleName());

    string bundleName2 = "com.ohos.test2";

    dataSyncer = dataSyncManager_->GetDataSyncer(bundleName2, USER_ID);
    EXPECT_EQ(USER_ID, dataSyncer->GetUserId());
    EXPECT_EQ(bundleName2, dataSyncer->GetBundleName());
    EXPECT_EQ(dataSyncManager_->dataSyncersMap_.Size(), 2);
}

/**
 * @tc.name: IsSkipSyncOnBatteryNoOkTest
 * @tc.desc: Verify the GetDataSyncer function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, IsSkipSyncOnBatteryNoOkTest, TestSize.Level1)
{
    BatteryStatus::level_ = BatteryStatus::CapacityLevel::LEVEL_TOO_LOW;
    auto ret = dataSyncManager_->IsSkipSync(BUND_NAME_TEST, USER_ID, false);
    BatteryStatus::level_ = BatteryStatus::CapacityLevel::LEVEL_NORMAL;
    EXPECT_EQ(E_SYNC_FAILED_BATTERY_TOO_LOW, ret);
}

/**
 * @tc.name: IsSkipSyncOKTest
 * @tc.desc: Verify the IsSkipSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, IsSkipSyncOKTest, TestSize.Level1)
{
    auto ret = dataSyncManager_->IsSkipSync(BUND_NAME_TEST, USER_ID, false);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: TriggerStartSyncOKTest
 * @tc.desc: Verify the TriggerStartSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, TriggerStartSyncNoOKTest, TestSize.Level1)
{
    BatteryStatus::level_ = BatteryStatus::CapacityLevel::LEVEL_TOO_LOW;
    auto ret = dataSyncManager_->TriggerStartSync(BUND_NAME, USER_ID,
        false, SyncTriggerType::APP_TRIGGER);
    BatteryStatus::level_ = BatteryStatus::CapacityLevel::LEVEL_NORMAL;
    EXPECT_EQ(E_SYNC_FAILED_BATTERY_TOO_LOW, ret);
}

/**
 * @tc.name: TriggerStartSyncOKTest
 * @tc.desc: Verify the GetDataSyncer function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, TriggerStartSyncOKTest, TestSize.Level1)
{
    auto ret = dataSyncManager_->TriggerStartSync(BUND_NAME, USER_ID,
        false, SyncTriggerType::APP_TRIGGER);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: TriggerStartSyncOKTest
 * @tc.desc: Verify the TriggerStopSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, TriggerStopSyncOKTest, TestSize.Level1)
{
    auto ret = dataSyncManager_->TriggerStopSync(BUND_NAME, USER_ID, SyncTriggerType::APP_TRIGGER);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: StartDownloadFileSyncOKTest
 * @tc.desc: Verify the StartDownloadFile function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, StartDownloadFileOKTest, TestSize.Level1)
{
    string path = "/test";
    auto ret = dataSyncManager_->StartDownloadFile(BUND_NAME, USER_ID, path);
    EXPECT_TRUE(E_OK == ret || ret == E_RDB);
}

/**
 * @tc.name: StopDownloadFileOKTest
 * @tc.desc: Verify the StopDownloadFile function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, StopDownloadFileOKTest, TestSize.Level1)
{
    string path = "/test";
    auto ret = dataSyncManager_->StopDownloadFile(BUND_NAME, USER_ID, path);
    EXPECT_TRUE(E_OK == ret || ret == E_RDB);
}

/**
 * @tc.name: UnregisterDownloadFileCallbackOKTest
 * @tc.desc: Verify the UnregisterDownloadFileCallback function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, UnregisterDownloadFileCallbackOKTest, TestSize.Level1)
{
    auto ret = dataSyncManager_->UnregisterDownloadFileCallback(BUND_NAME, USER_ID);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: RegisterDownloadFileCallbackOKTest
 * @tc.desc: Verify the RegisterDownloadFileCallback function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncManagerTest, RegisterDownloadFileCallbackOKTest, TestSize.Level1)
{
    auto ret = dataSyncManager_->RegisterDownloadFileCallback(BUND_NAME, USER_ID, nullptr);
    EXPECT_EQ(E_OK, ret);
}
} // namespace OHOS::FileManagement::CloudSync::Test