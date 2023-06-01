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
    MOCK_METHOD0(Schedule, void());
};

DataSyncerMock::DataSyncerMock(const std::string bundleName, const int32_t userId) : DataSyncer("com.ohos.test", 100) {}

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
    datasyncer_ = make_shared<DataSyncerMock>(bundleName, userId);
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
        EXPECT_TRUE(true);
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
    shared_ptr<SdkHelper> sdkHelper;
    try {
        datasyncer_->SetSdkHelper(sdkHelper);
        EXPECT_TRUE(true);
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
        EXPECT_TRUE(true);
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
        EXPECT_TRUE(true);
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
    EXPECT_EQ(res, datasyncer_->bundleName_);
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
    EXPECT_EQ(res, datasyncer_->userId_);
    GTEST_LOG_(INFO) << "GetUserId end";
}

/**
 * @tc.name: GetSyncStateTest
 * @tc.desc: Verify the GetSyncState function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, GetSyncStateTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncState Start";
    try {
        datasyncer_->GetSyncState();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncState FAILED";
    }
    GTEST_LOG_(INFO) << "GetSyncState end";
}

/**
 * @tc.name: CompletePullTest
 * @tc.desc: Verify the CompletePull function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, CompletePullTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompletePull Start";
    try {
        datasyncer_->CompletePull();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CompletePull FAILED";
    }
    GTEST_LOG_(INFO) << "CompletePull end";
}

/**
 * @tc.name: CompletePushTest
 * @tc.desc: Verify the CompletePush function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, CompletePushTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompletePush Start";
    try {
        datasyncer_->CompletePush();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CompletePush FAILED";
    }
    GTEST_LOG_(INFO) << "CompletePush end";
}

/**
 * @tc.name: CompleteTaskTest
 * @tc.desc: Verify the CompleteTask function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, CompleteTaskTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompleteTask Start";
    shared_ptr<Task> task = make_shared<Task>(nullptr, nullptr);
    try {
        datasyncer_->CompleteTask(task);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CompleteTask FAILED";
    }
    GTEST_LOG_(INFO) << "CompleteTask end";
}

/**
 * @tc.name: StartDownloadFileTest
 * @tc.desc: Verify the StartDownloadFileTask function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, StartDownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDownloadFile Start";
    int res = datasyncer_->StartDownloadFile("/test", 100);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "StartDownloadFile end";
}

/**
 * @tc.name: StopDownloadFileTest
 * @tc.desc: Verify the StopDownloadFileTask function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, StopDownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDownloadFile Start";
    int res = datasyncer_->StopDownloadFile("/test", 100);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "StopDownloadFile end";
}

/**
 * @tc.name: RegisterDownloadFileCallbackTest
 * @tc.desc: Verify the RegisterDownloadFileCallback function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, RegisterDownloadFileCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallback Start";
    int res = datasyncer_->RegisterDownloadFileCallback(100, nullptr);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallback end";
}

/**
 * @tc.name: UnregisterDownloadFileCallbackTest
 * @tc.desc: Verify the UnregisterDownloadFileCallback function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, UnregisterDownloadFileCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallback Start";
    int res = datasyncer_->UnregisterDownloadFileCallback(100);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallback end";
}

/**
 * @tc.name: PullTest
 * @tc.desc: Verify the Pull function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataSyncerTest, PullTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Pull Start";
    int res = datasyncer_->Pull(nullptr);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "Pull end";
}

} // namespace OHOS::FileManagement::CloudSync::Test