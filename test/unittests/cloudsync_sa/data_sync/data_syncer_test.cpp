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

#include "data_sync/data_syncer.h"

#include "dfs_error.h"
#include "gallery_data_syncer.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/cloud_status.h"


namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DataSyncerMock final : public DataSyncer {
public:
    DataSyncerMock(const std::string bundleName, const int32_t userId);
    MOCK_METHOD0(Schedule,void());
};

DataSyncerMock::DataSyncerMock(const std::string bundleName, const int32_t userId): DataSyncer("com.ohos.test", 100)
{}

class DataSyncerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DataSyncerMock> datasyncer_;
};
void DataSyncerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DataSyncerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DataSyncerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    const std::string bundleName_;
    int32_t userId = 100;
    string bundleName = "com.ohos.test";
    datasyncer_ = make_shared<DataSyncerMock>(bundleName,userId);
}

void DataSyncerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StopSyncTest
 * @tc.desc: Verify the StopSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, StopSyncTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopSync Start";
    int res = datasyncer_->StopSync(SyncTriggerType::APP_TRIGGER);
    EXPECT_EQ(res, E_OK);
    res = datasyncer_->StopSync(SyncTriggerType::CLOUD_TRIGGER);
    EXPECT_EQ(res, E_OK);
    res = datasyncer_->StopSync(SyncTriggerType::PENDING_TRIGGER);
    EXPECT_EQ(res, E_OK);
    res = datasyncer_->StopSync(SyncTriggerType::BATTERY_OK_TRIGGER);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "StopSync end";
}

/**
 * @tc.name: DownloadSourceFileTest
 * @tc.desc: Verify the DownloadSourceFile function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
*/
HWTEST_F(DataSyncerTest, DownloadSourceFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadSourceFile Start";
    int res = datasyncer_->DownloadSourceFile("com.ohos.test",nullptr);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "DownloadSourceFile end";
}

/**
 * @tc.name: AbortTest
 * @tc.desc: Verify the Abort function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, AbortTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Abort Start";
    try {
        datasyncer_->Abort();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "Abort FAILED";
    }
    GTEST_LOG_(INFO) << "Abort end";
}

/**
 * @tc.name: SetSdkHelperTest
 * @tc.desc: Verify the SetSdkHelper function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, SetSdkHelperTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetSdkHelper Start";
    try {
        datasyncer_->SetSdkHelper(nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetSdkHelper FAILED";
    }
    GTEST_LOG_(INFO) << "SetSdkHelper end";
}

/**
 * @tc.name: SetSdkHelperTest1
 * @tc.desc: Verify the SetSdkHelper function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, SetSdkHelperTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetSdkHelper Start";
    shared_ptr<SdkHelper> sdkHelper;
    try {
        datasyncer_->SetSdkHelper(sdkHelper);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetSdkHelper FAILED";
    }
    GTEST_LOG_(INFO) << "SetSdkHelper end";
}

/**
 * @tc.name: BeginTransactionTest
 * @tc.desc: Verify the BeginTransaction function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, BeginTransactionTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BeginTransaction Start";
     try {
        datasyncer_->BeginTransaction();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BeginTransaction FAILED";
    }
    GTEST_LOG_(INFO) << "BeginTransaction end";
}

/**
 * @tc.name: EndTransactionTest
 * @tc.desc: Verify the EndTransaction function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, EndTransactionTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EndTransaction Start";
     try {
        datasyncer_->EndTransaction();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "EndTransaction FAILED";
    }
    GTEST_LOG_(INFO) << "EndTransaction end";
}

/**
 * @tc.name: GetBundleNameTest
 * @tc.desc: Verify the GetBundleName function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, GetBundleNameTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleName Start";
    string res = datasyncer_->GetBundleName();
    EXPECT_EQ(res,datasyncer_->bundleName_);
    GTEST_LOG_(INFO) << "GetBundleName end";
}

/**
 * @tc.name: GetUserIdTest
 * @tc.desc: Verify the GetUserId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, GetUserIdTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUserId Start";
    int res = datasyncer_->GetUserId();
    EXPECT_EQ(res,datasyncer_->userId_);
    GTEST_LOG_(INFO) << "GetUserId end";
}

} // namespace OHOS::FileManagement::CloudSync::Test