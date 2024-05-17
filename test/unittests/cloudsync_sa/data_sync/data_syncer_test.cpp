/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include <memory>

#include "data_sync/data_syncer.h"

#include "data_sync_manager.h"
#include "gallery_data_syncer.h"

#include "data_handler_method_mock.h"
#include "dfs_error.h"
#include "dk_error.h"
#include "gallery_data_syncer.h"
#include "gmock/gmock-actions.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/cloud_status.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DataSyncerMock final : public DataSyncer {
public:
    DataSyncerMock(const string bundleName, const int32_t userId);
    MOCK_METHOD0(Schedule, void());
    MOCK_METHOD0(Reset, void());
    MOCK_METHOD1(ScheduleByType, void(SyncTriggerType syncTriggerType));
    MOCK_METHOD1(Complete, int(bool));
};

DataSyncerMock::DataSyncerMock(const string bundleName, const int32_t userId) : DataSyncer("com.ohos.test", 100) {}

class DataSyncerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<DataSyncManager> dataSyncManager_;
    static inline shared_ptr<DataSyncerMock> datasyncer_;
    static inline shared_ptr<GalleryDataSyncer> galleryDataSyncer_;
    static inline shared_ptr<FileDataHandler> handler_;
    static inline shared_ptr<DataHandlerMethodMock> dataHandler_;
};

void DataSyncerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    const int32_t userId = 100;
    const string bundleName = "com.ohos.test";
    datasyncer_ = make_shared<DataSyncerMock>(bundleName, userId);
    EXPECT_CALL(*datasyncer_, Schedule()).Times(testing::AnyNumber());
    EXPECT_CALL(*datasyncer_, Reset()).Times(testing::AnyNumber());
    dataSyncManager_ = make_shared<DataSyncManager>();
    auto sdkHelper = make_shared<SdkHelper>();
    datasyncer_->SetSdkHelper(sdkHelper);
    string test = "test";
    dataHandler_ = make_shared<DataHandlerMethodMock>(userId, test, test);
    EXPECT_CALL(*dataHandler_, GetFetchCondition(_)).Times(testing::AnyNumber());
    EXPECT_CALL(*dataHandler_, FinishPull(_)).Times(testing::AnyNumber());
    EXPECT_CALL(*dataHandler_, GetTempStartCursor(_)).Times(testing::AnyNumber());
    EXPECT_CALL(*dataHandler_, GetNextCursor(_)).Times(testing::AnyNumber());
    EXPECT_CALL(*dataHandler_, SetTempStartCursor(_)).Times(testing::AnyNumber());
    EXPECT_CALL(*dataHandler_, SetTempNextCursor(_, _)).Times(testing::AnyNumber());
}

void DataSyncerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    dataSyncManager_ = nullptr;
    datasyncer_ = nullptr;
    galleryDataSyncer_ = nullptr;
    handler_ = nullptr;
    dataHandler_ = nullptr;
}

void DataSyncerTest::SetUp(void) {}

void DataSyncerTest::TearDown(void) {}

/**
 * @tc.name: AsyncRunTest
 * @tc.desc: Verify the AsyncRun function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, AsyncRunTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AsyncRun Start";
    shared_ptr<SdkHelper> sdkHelper;
    try {
        datasyncer_->AsyncRun({}, {});
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AsyncRun FAILED";
    }
    GTEST_LOG_(INFO) << "AsyncRun end";
}

/**
 * @tc.name: StartSyncTest
 * @tc.desc: Verify the StartSync function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, StartSyncTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSync Start";
    bool forceFlag = false;
    int res = datasyncer_->StartSync(forceFlag, SyncTriggerType::PENDING_TRIGGER);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "StartSync end";
}

/**
 * @tc.name: StartSyncErrTest
 * @tc.desc: Verify the StartSync function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, StartSyncErrTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSync Start";
    try {
        bool forceFlag = true;
        int res = datasyncer_->StartSync(forceFlag, SyncTriggerType::PENDING_TRIGGER);
        EXPECT_EQ(res, E_PENDING);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSync FAILED";
    }
    GTEST_LOG_(INFO) << "StartSync end";
}

/**
 * @tc.name: StopSyncTest
 * @tc.desc: Verify the StopSync function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, StopSyncTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopSync Start";
    int res = datasyncer_->StopSync(SyncTriggerType::PENDING_TRIGGER);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "StopSync end";
}

/**
 * @tc.name: LockTest
 * @tc.desc: Verify the Lock function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, LockTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Lock Start";
    datasyncer_->lock_.count = -100;
    datasyncer_->lock_.lock.lockInterval = 100;
    int res = datasyncer_->Lock();
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "Lock GetLock";
    datasyncer_->lock_.count = 0;
    datasyncer_->lock_.lock.lockInterval = 0;
    res = datasyncer_->Lock();
    EXPECT_EQ(res, E_OK);
    res = datasyncer_->Lock();
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "Lock end";
}

/**
 * @tc.name: UnlockTest
 * @tc.desc: Verify the Unlock function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, UnlockTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Unlock Start";
    try {
        datasyncer_->Unlock();
        datasyncer_->Unlock();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "Unlock FAILED";
    }
    GTEST_LOG_(INFO) << "Unlock end";
}

/**
 * @tc.name: ForceUnlockTest
 * @tc.desc: Verify the ForceUnlock function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, ForceUnlockTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForceUnlock Start";
    try {
        datasyncer_->lock_.count = 1;
        datasyncer_->ForceUnlock();
        datasyncer_->ForceUnlock();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForceUnlock FAILED";
    }
    GTEST_LOG_(INFO) << "ForceUnlock end";
}

/**
 * @tc.name: StartDownloadFileTest
 * @tc.desc: Verify the StartDownloadFileTask function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
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
 * @tc.require: #I7UU3Z
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
 * @tc.require: #I7UU3Z
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
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, UnregisterDownloadFileCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallback Start";
    int res = datasyncer_->UnregisterDownloadFileCallback(100);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallback end";
}

/**
 * @tc.name: PullTest001
 * @tc.desc: Verify the Pull function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, PullTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullTest001 Start";
    try {
        EXPECT_CALL(*dataHandler_, GetRetryRecords(_)).Times(1).WillOnce(Return(E_OK));
        EXPECT_CALL(*dataHandler_, GetBatchNo()).Times(1).WillOnce(Return(0));
        EXPECT_CALL(*dataHandler_, IsPullRecords()).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*dataHandler_, GetAssetsToDownload(_)).Times(1).WillOnce(Return(false));
        int32_t result = datasyncer_->Pull(dataHandler_);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PullTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "PullTest001 end";
}

/**
 * @tc.name: PullTest002
 * @tc.desc: Verify the Pull function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, PullTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullTest002 Start";
    try {
        EXPECT_CALL(*dataHandler_, GetBatchNo()).Times(1).WillOnce(Return(0));
        EXPECT_CALL(*dataHandler_, GetRetryRecords(_)).Times(1).WillOnce(Return(1));
        EXPECT_CALL(*dataHandler_, IsPullRecords()).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*dataHandler_, GetAssetsToDownload(_)).Times(1).WillOnce(Return(false));
        int32_t result = datasyncer_->Pull(dataHandler_);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PullTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "PullTest002 end";
}

/**
 * @tc.name: PullRecordsTest
 * @tc.desc: Verify the PullRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, PullRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecords Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(dataHandler_);
        EXPECT_CALL(*dataHandler_, GetCheckFlag()).Times(1).WillOnce(Return(true));
        datasyncer_->PullRecords(context);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PullRecords FAILED";
    }
    GTEST_LOG_(INFO) << "PullRecords end";
}

/**
 * @tc.name: PullRecordsTest002
 * @tc.desc: Verify the PullRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, PullRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordsTest002 Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(nullptr);
        datasyncer_->PullRecords(context);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PullRecordsTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "PullRecordsTest002 end";
}

/**
 * @tc.name: PullRecordsTest003
 * @tc.desc: Verify the PullRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, PullRecordsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordsTest003 Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(dataHandler_);
        EXPECT_CALL(*dataHandler_, GetCheckFlag()).Times(1).WillOnce(Return(false));
        char data[] = "123";
        context->data_ = data;
        datasyncer_->PullRecords(context);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PullRecordsTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "PullRecordsTest003 end";
}

/**
 * @tc.name: PullDatabaseChangesTest
 * @tc.desc: Verify the PullDatabaseChanges function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, PullDatabaseChangesTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullDatabaseChanges Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(dataHandler_);
        datasyncer_->PullDatabaseChanges(context);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PullDatabaseChanges FAILED";
    }
    GTEST_LOG_(INFO) << "PullDatabaseChanges end";
}

/**
 * @tc.name: PullDatabaseChangesTest002
 * @tc.desc: Verify the PullDatabaseChanges function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, PullDatabaseChangesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullDatabaseChangesTest002 Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(dataHandler_);
        char data[] = "123";
        context->data_ = data;
        datasyncer_->PullDatabaseChanges(context);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PullDatabaseChangesTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "PullDatabaseChangesTest002 end";
}

/**
 * @tc.name: PullDatabaseChangesTest003
 * @tc.desc: Verify the PullDatabaseChanges function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, PullDatabaseChangesTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullDatabaseChangesTest003 Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(nullptr);
        datasyncer_->PullDatabaseChanges(context);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PullDatabaseChangesTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "PullDatabaseChangesTest003 end";
}

/**
 * @tc.name: HandleOnFetchRecordsTest
 * @tc.desc: Verify the HandleOnFetchRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, HandleOnFetchRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest Start";
    try {
        shared_ptr<vector<DriveKit::DKRecord>> records = make_shared<vector<DriveKit::DKRecord>>();
        auto res = datasyncer_->HandleOnFetchRecords(nullptr, nullptr, records, false);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest FAILED";
    }
    GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest end";
}

/**
 * @tc.name: HandleOnFetchRecordsTest002
 * @tc.desc: Verify the HandleOnFetchRecordsTest002 function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, HandleOnFetchRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest002 Start";
    try {
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(nullptr, 1);
        shared_ptr<vector<DriveKit::DKRecord>> records = make_shared<vector<DriveKit::DKRecord>>();
        records->push_back({});
        auto res = datasyncer_->HandleOnFetchRecords(context, nullptr, records, false);
        EXPECT_EQ(res, E_CONTEXT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest002 end";
}

/**
 * @tc.name: HandleOnFetchRecordsTest003
 * @tc.desc: Verify the HandleOnFetchRecordsTest002 function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, HandleOnFetchRecordsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest003 Start";
    try {
        EXPECT_CALL(*dataHandler_, IsPullRecords()).Times(1).WillOnce(Return(true));
        EXPECT_CALL(*dataHandler_, GetRecordSize()).Times(1).WillOnce(Return(1));
        EXPECT_CALL(*dataHandler_, OnFetchRecords(_, _)).Times(1).WillOnce(Return(1));
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(dataHandler_, 0);
        shared_ptr<vector<DriveKit::DKRecord>> records = make_shared<vector<DriveKit::DKRecord>>();
        records->push_back({});
        auto res = datasyncer_->HandleOnFetchRecords(context, nullptr, records, true);
        EXPECT_NE(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest003 end";
}

/**
 * @tc.name: HandleOnFetchRecordsTest004
 * @tc.desc: Verify the HandleOnFetchRecordsTest004 function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, HandleOnFetchRecordsTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest004 Start";
    try {
        EXPECT_CALL(*dataHandler_, IsPullRecords()).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*dataHandler_, OnFetchRecords(_, _)).Times(1).WillOnce(Return(0));
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(dataHandler_, 0);
        shared_ptr<vector<DriveKit::DKRecord>> records = make_shared<vector<DriveKit::DKRecord>>();
        records->push_back({});
        auto res = datasyncer_->HandleOnFetchRecords(context, nullptr, records, true);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest004 end";
}

/**
 * @tc.name: HandleOnFetchRecordsTest005
 * @tc.desc: Verify the HandleOnFetchRecordsTest005 function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, HandleOnFetchRecordsTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest005 Start";
    try {
        EXPECT_CALL(*dataHandler_, IsPullRecords()).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*dataHandler_, OnFetchRecords(_, _)).Times(1).WillOnce(Return(0));
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(dataHandler_, 0);
        shared_ptr<vector<DriveKit::DKRecord>> records = make_shared<vector<DriveKit::DKRecord>>();
        records->push_back({});
        auto res = datasyncer_->HandleOnFetchRecords(context, nullptr, records, false);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "HandleOnFetchRecordsTest005 end";
}

/**
 * @tc.name: OnFetchRecordsTest
 * @tc.desc: Verify the OnFetchRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnFetchRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchRecordsTest Start";
    try {
        DriveKit::DKError err;
        err.isLocalError = true;
        err.serverErrorCode = static_cast<int>(DriveKit::DKServerErrorCode::NETWORK_ERROR);
        datasyncer_->OnFetchRecords(nullptr, nullptr, nullptr, {}, err);
        GTEST_LOG_(INFO) << "OnFetchRecordsTest else";
        err.serverErrorCode = static_cast<int>(DriveKit::DKServerErrorCode::ACCESS_DENIED);
        datasyncer_->OnFetchRecords(nullptr, nullptr, nullptr, {}, err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchRecordsTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnFetchRecordsTest end";
}

/**
 * @tc.name: OnFetchRecordsTest002
 * @tc.desc: Verify the OnFetchRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnFetchRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchRecordsTest002 Start";
    try {
        DriveKit::DKError err;
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(nullptr, 0);
        datasyncer_->OnFetchRecords(context, nullptr, nullptr, {}, err);
        GTEST_LOG_(INFO) << "OnFetchRecordsTest002 other";
        EXPECT_CALL(*dataHandler_, OnFetchRecords(_, _)).Times(1).WillOnce(Return(0));
        EXPECT_CALL(*dataHandler_, SetRecordSize(_)).Times(1);
        EXPECT_CALL(*dataHandler_, IsPullRecords()).Times(1).WillOnce(Return(true));
        shared_ptr<DownloadTaskContext> contextErr = make_shared<DownloadTaskContext>(dataHandler_, 0);
        shared_ptr<vector<DriveKit::DKRecord>> records = make_shared<vector<DriveKit::DKRecord>>();
        records->push_back({});
        datasyncer_->OnFetchRecords(contextErr, nullptr, records, {}, err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchRecordsTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "OnFetchRecordsTest002 end";
}

/**
 * @tc.name: OnFetchRecordsTest003
 * @tc.desc: Verify the OnFetchRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnFetchRecordsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchRecordsTest003 Start";
    try {
        DriveKit::DKError err;
        EXPECT_CALL(*dataHandler_, GetBatchNo()).Times(1).WillOnce(Return(0));
        shared_ptr<DownloadTaskContext> contextErr = make_shared<DownloadTaskContext>(dataHandler_, 1);
        shared_ptr<vector<DriveKit::DKRecord>> records = make_shared<vector<DriveKit::DKRecord>>();
        datasyncer_->OnFetchRecords(contextErr, nullptr, records, "", err);
        GTEST_LOG_(INFO) << "OnFetchRecordsTest003 other";
        records->push_back({});
        EXPECT_CALL(*dataHandler_, IsPullRecords()).Times(1).WillOnce(Return(false));
        EXPECT_CALL(*dataHandler_, OnFetchRecords(_, _)).Times(1).WillOnce(Return(1));
        datasyncer_->OnFetchRecords(contextErr, nullptr, records, "test", err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchRecordsTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "OnFetchRecordsTest003 end";
}

/**
 * @tc.name: DownloadInnerTest
 * @tc.desc: Verify the DownloadInner function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, DownloadInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadInnerTest Start";
    try {
        int32_t userId = 100;
        EXPECT_CALL(*dataHandler_, GetDownloadAsset(_, _, _)).Times(1).WillOnce(Return(1));
        auto ret = datasyncer_->DownloadInner(dataHandler_, "test", userId);
        EXPECT_NE(ret, E_OK);
        EXPECT_CALL(*dataHandler_, GetDownloadAsset(_, _, _)).Times(1).WillOnce(Return(0));
        auto res = datasyncer_->DownloadInner(dataHandler_, "test", userId);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadInnerTest FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadInnerTest end";
}

/**
 * @tc.name: OnFetchDatabaseChangesTest
 * @tc.desc: Verify the OnFetchDatabaseChanges function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnFetchDatabaseChangesTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchDatabaseChangesTest Start";
    try {
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(nullptr, 0);
        DriveKit::DKError err;
        datasyncer_->OnFetchDatabaseChanges(context, nullptr, nullptr, "", false, err);
        GTEST_LOG_(INFO) << "OnFetchDatabaseChanges other 001";
        err.isLocalError = true;
        err.serverErrorCode = static_cast<int>(DriveKit::DKServerErrorCode::NETWORK_ERROR);
        shared_ptr<DownloadTaskContext> contextTest = make_shared<DownloadTaskContext>(dataHandler_, 0);
        datasyncer_->OnFetchDatabaseChanges(contextTest, nullptr, nullptr, "", false, err);
        GTEST_LOG_(INFO) << "OnFetchDatabaseChanges other 002";
        EXPECT_CALL(*dataHandler_, GetAssetsToDownload(_)).Times(2).WillRepeatedly(Return(0));
        EXPECT_CALL(*dataHandler_, IsPullRecords()).Times(2).WillRepeatedly(Return(false));
        EXPECT_CALL(*dataHandler_, GetRetryRecords(_)).Times(2).WillRepeatedly(Return(E_OK));
        EXPECT_CALL(*dataHandler_, SetChecking()).Times(2);
        EXPECT_CALL(*dataHandler_, GetBatchNo()).Times(2).WillRepeatedly(Return(0));
        err.serverErrorCode = {};
        DriveKit::DKErrorDetail detailErr{};
        detailErr.detailCode = static_cast<int>(DriveKit::DKDetailErrorCode::PARAM_INVALID);
        err.errorDetails.push_back(detailErr);
        datasyncer_->OnFetchDatabaseChanges(contextTest, nullptr, nullptr, "", false, err);
        GTEST_LOG_(INFO) << "OnFetchDatabaseChanges other 003";
        err.errorDetails.clear();
        detailErr.detailCode = static_cast<int>(DriveKit::DKDetailErrorCode::CURSOR_EXPIRED);
        err.errorDetails.push_back(detailErr);
        datasyncer_->OnFetchDatabaseChanges(contextTest, nullptr, nullptr, "", false, err);
        GTEST_LOG_(INFO) << "OnFetchDatabaseChanges other 004";
        err.errorDetails.clear();
        detailErr.detailCode = static_cast<int>(DriveKit::DKDetailErrorCode::FLOW_ID_NOT_MATCH);
        err.errorDetails.push_back(detailErr);
        datasyncer_->OnFetchDatabaseChanges(contextTest, nullptr, nullptr, "", false, err);
        GTEST_LOG_(INFO) << "OnFetchDatabaseChanges other 005";
        err.errorDetails.clear();
        datasyncer_->OnFetchDatabaseChanges(contextTest, nullptr, nullptr, "", false, err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchDatabaseChangesTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnFetchDatabaseChangesTest end";
}

/**
 * @tc.name: OnFetchDatabaseChangesTest002
 * @tc.desc: Verify the OnFetchDatabaseChanges function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnFetchDatabaseChangesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchDatabaseChangesTest002 Start";
    try {
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(dataHandler_, 0);
        DriveKit::DKError err;
        EXPECT_CALL(*dataHandler_, GetBatchNo()).Times(1).WillRepeatedly(Return(0));
        EXPECT_CALL(*dataHandler_, IsPullRecords()).Times(2).WillRepeatedly(Return(0));
        shared_ptr<vector<DriveKit::DKRecord>> records = make_shared<vector<DriveKit::DKRecord>>();
        datasyncer_->OnFetchDatabaseChanges(context, nullptr, records, "", false, err);
        GTEST_LOG_(INFO) << "OnFetchDatabaseChangesTest002 other 002";
        records->push_back({});
        EXPECT_CALL(*dataHandler_, OnFetchRecords(_, _)).Times(1).WillOnce(Return(0));
        datasyncer_->OnFetchDatabaseChanges(context, nullptr, records, "", false, err);
        GTEST_LOG_(INFO) << "OnFetchDatabaseChangesTest002 other 003";

        EXPECT_CALL(*dataHandler_, OnFetchRecords(_, _)).Times(1).WillOnce(Return(0));
        datasyncer_->OnFetchDatabaseChanges(context, nullptr, records, "", true, err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchDatabaseChangesTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "OnFetchDatabaseChangesTest002 end";
}

/**
 * @tc.name: OnFetchCheckRecordsTest
 * @tc.desc: Verify the OnFetchCheckRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnFetchCheckRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchCheckRecordsTest Start";
    try {
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(nullptr, 0);
        DriveKit::DKError err;
        err.isLocalError = true;
        datasyncer_->OnFetchCheckRecords(context, nullptr, nullptr, "", err);
        err.serverErrorCode = static_cast<int>(DriveKit::DKServerErrorCode::NETWORK_ERROR);
        datasyncer_->OnFetchCheckRecords(context, nullptr, nullptr, "", err);
        err.isLocalError = false;
        datasyncer_->OnFetchCheckRecords(context, nullptr, nullptr, "", err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchCheckRecordsTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnFetchCheckRecordsTest end";
}

/**
 * @tc.name: OnFetchCheckRecordsTest002
 * @tc.desc: Verify the OnFetchCheckRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnFetchCheckRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchCheckRecordsTest002 Start";
    try {
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(dataHandler_, 0);
        DriveKit::DKError err;
        EXPECT_CALL(*dataHandler_, GetBatchNo()).Times(1).WillRepeatedly(Return(0));
        EXPECT_CALL(*dataHandler_, GetCheckRecords(_, _)).Times(1).WillOnce(Return(1));
        datasyncer_->OnFetchCheckRecords(context, nullptr, nullptr, "", err);
        GTEST_LOG_(INFO) << "OnFetchCheckRecordsTest002 other";
        EXPECT_CALL(*dataHandler_, GetCheckRecords(_, _)).Times(1).WillOnce(Return(0));
        datasyncer_->OnFetchCheckRecords(context, nullptr, nullptr, "test", err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchCheckRecordsTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "OnFetchCheckRecordsTest002 end";
}

/**
 * @tc.name: PullRecordsWithIdTest
 * @tc.desc: Verify the PullRecordsWithId function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, PullRecordsWithIdTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordsWithIdTest Start";
    try {
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(nullptr, 0);
        vector<DriveKit::DKRecordId> records;
        datasyncer_->PullRecordsWithId(context, records, false);
        GTEST_LOG_(INFO) << "PullRecordsWithIdTest other 001";
        shared_ptr<DownloadTaskContext> contextOther = make_shared<DownloadTaskContext>(dataHandler_, 0);
        datasyncer_->PullRecordsWithId(contextOther, records, false);
        GTEST_LOG_(INFO) << "PullRecordsWithIdTest other 002";
        records.push_back("test");
        datasyncer_->PullRecordsWithId(contextOther, records, true);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PullRecordsWithIdTest FAILED";
    }
    GTEST_LOG_(INFO) << "PullRecordsWithIdTest end";
}

/**
 * @tc.name: OnFetchRecordWithIdTest
 * @tc.desc: Verify the OnFetchRecordWithId function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnFetchRecordWithIdTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchRecordWithIdTest Start";
    try {
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(nullptr, 0);
        vector<DriveKit::DKRecordId> records;
        DriveKit::DKError err;
        DriveKit::DKRecord record;
        datasyncer_->OnFetchRecordWithId(context, nullptr, "test", record, err);
        GTEST_LOG_(INFO) << "OnFetchRecordWithIdTest other 001";
        err.isLocalError = true;
        datasyncer_->OnFetchRecordWithId(context, nullptr, "test", record, err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchRecordWithIdTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnFetchRecordWithIdTest end";
}

/**
 * @tc.name: RetryDownloadRecordsTest
 * @tc.desc: Verify the RetryDownloadRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, RetryDownloadRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RetryDownloadRecordsTest Start";
    try {
        shared_ptr<DownloadTaskContext> context = make_shared<DownloadTaskContext>(nullptr, 0);
        datasyncer_->RetryDownloadRecords(context);
        shared_ptr<DownloadTaskContext> contextTest = make_shared<DownloadTaskContext>(dataHandler_, 0);
        EXPECT_CALL(*dataHandler_, GetAssetsToDownload(_)).Times(1).WillOnce(Return(1));
        datasyncer_->RetryDownloadRecords(contextTest);
        EXPECT_CALL(*dataHandler_, GetAssetsToDownload(_)).Times(1).WillOnce(Return(0));
        datasyncer_->RetryDownloadRecords(contextTest);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RetryDownloadRecordsTest FAILED";
    }
    GTEST_LOG_(INFO) << "RetryDownloadRecordsTest end";
}

/**
 * @tc.name: InitTest
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, InitTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitTest Start";
    try {
        auto ret = datasyncer_->Init("test", 100);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitTest FAILED";
    }
    GTEST_LOG_(INFO) << "InitTest end";
}

/**
 * @tc.name: CleanTest
 * @tc.desc: Verify the Clean function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, CleanTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanTest Start";
    try {
        auto ret = datasyncer_->Clean(0);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanTest FAILED";
    }
    GTEST_LOG_(INFO) << "CleanTest end";
}

/**
 * @tc.name: CleanInnerTest
 * @tc.desc: Verify the CleanInner function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, CleanInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanInnerTest Start";
    try {
        EXPECT_CALL(*dataHandler_, ClearCursor()).Times(2);
        EXPECT_CALL(*dataHandler_, Clean(_)).Times(1).WillOnce(Return(1));
        auto ret = datasyncer_->CleanInner(dataHandler_, 0);
        EXPECT_NE(ret, E_OK);
        GTEST_LOG_(INFO) << "CleanInnerTest other";
        EXPECT_CALL(*dataHandler_, Clean(_)).Times(1).WillOnce(Return(0));
        ret = datasyncer_->CleanInner(dataHandler_, 0);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanInnerTest FAILED";
    }
    GTEST_LOG_(INFO) << "CleanInnerTest end";
}

/**
 * @tc.name: CreateRecordsTest
 * @tc.desc: Verify the CreateRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, CreateRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateRecords Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(nullptr);
        datasyncer_->CreateRecords(context);
        GTEST_LOG_(INFO) << "CreateRecords other 001";
        EXPECT_CALL(*dataHandler_, GetCreatedRecords(_)).Times(1).WillOnce(Return(1));
        shared_ptr<TaskContext> contextTest = make_shared<TaskContext>(dataHandler_);
        datasyncer_->CreateRecords(contextTest);
        GTEST_LOG_(INFO) << "CreateRecords other 002";
        EXPECT_CALL(*dataHandler_, GetCreatedRecords(_)).Times(1).WillOnce(Return(0));
        datasyncer_->CreateRecords(contextTest);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateRecords FAILED";
    }
    GTEST_LOG_(INFO) << "CreateRecords end";
}

/**
 * @tc.name: DeleteRecordsTest
 * @tc.desc: Verify the DeleteRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, DeleteRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteRecords Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(nullptr);
        datasyncer_->DeleteRecords(context);
        GTEST_LOG_(INFO) << "DeleteRecords other 001";
        EXPECT_CALL(*dataHandler_, GetDeletedRecords(_)).Times(1).WillOnce(Return(1));
        shared_ptr<TaskContext> contextTest = make_shared<TaskContext>(dataHandler_);
        datasyncer_->DeleteRecords(contextTest);
        GTEST_LOG_(INFO) << "DeleteRecords other 002";
        EXPECT_CALL(*dataHandler_, GetDeletedRecords(_)).Times(1).WillOnce(Return(0));
        datasyncer_->DeleteRecords(contextTest);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteRecords FAILED";
    }
    GTEST_LOG_(INFO) << "DeleteRecords end";
}

/**
 * @tc.name: ModifyMdirtyRecordsTest
 * @tc.desc: Verify the ModifyMdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, ModifyMdirtyRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModifyMdirtyRecordsTest Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(nullptr);
        datasyncer_->ModifyMdirtyRecords(context);
        GTEST_LOG_(INFO) << "ModifyMdirtyRecordsTest other 001";
        EXPECT_CALL(*dataHandler_, GetMetaModifiedRecords(_)).Times(1).WillOnce(Return(1));
        shared_ptr<TaskContext> contextTest = make_shared<TaskContext>(dataHandler_);
        datasyncer_->ModifyMdirtyRecords(contextTest);
        GTEST_LOG_(INFO) << "ModifyMdirtyRecordsTest other 002";
        EXPECT_CALL(*dataHandler_, GetMetaModifiedRecords(_)).Times(1).WillOnce(Return(0));
        datasyncer_->ModifyMdirtyRecords(contextTest);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ModifyMdirtyRecordsTest FAILED";
    }
    GTEST_LOG_(INFO) << "ModifyMdirtyRecordsTest end";
}

/**
 * @tc.name: ModifyFdirtyRecordsTest
 * @tc.desc: Verify the ModifyFdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, ModifyFdirtyRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ModifyFdirtyRecordsTest Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(nullptr);
        datasyncer_->ModifyFdirtyRecords(context);
        GTEST_LOG_(INFO) << "ModifyFdirtyRecordsTest other 001";
        EXPECT_CALL(*dataHandler_, GetFileModifiedRecords(_)).Times(1).WillOnce(Return(1));
        shared_ptr<TaskContext> contextTest = make_shared<TaskContext>(dataHandler_);
        datasyncer_->ModifyFdirtyRecords(contextTest);
        GTEST_LOG_(INFO) << "ModifyFdirtyRecordsTest other 002";
        EXPECT_CALL(*dataHandler_, GetFileModifiedRecords(_)).Times(1).WillOnce(Return(0));
        datasyncer_->ModifyFdirtyRecords(contextTest);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ModifyFdirtyRecordsTest FAILED";
    }
    GTEST_LOG_(INFO) << "ModifyFdirtyRecordsTest end";
}

/**
 * @tc.name: OnCreateRecordsTest
 * @tc.desc: Verify the OnCreateRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnCreateRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecordsTest Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(dataHandler_);
        DriveKit::DKError err;
        shared_ptr<map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>> map =
            make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();

        EXPECT_CALL(*dataHandler_, OnCreateRecords(_)).Times(1).WillOnce(Return(1));
        datasyncer_->OnCreateRecords(context, nullptr, map, err);
        GTEST_LOG_(INFO) << "OnCreateRecordsTest other 001";
        EXPECT_CALL(*dataHandler_, OnCreateRecords(_)).Times(1).WillOnce(Return(12));
        datasyncer_->OnCreateRecords(context, nullptr, map, err);
        GTEST_LOG_(INFO) << "OnCreateRecordsTest other 002";
        EXPECT_CALL(*dataHandler_, OnCreateRecords(_)).Times(1).WillOnce(Return(24));
        datasyncer_->OnCreateRecords(context, nullptr, map, err);
        GTEST_LOG_(INFO) << "OnCreateRecordsTest other 003";
        EXPECT_CALL(*dataHandler_, OnCreateRecords(_)).Times(1).WillOnce(Return(0));
        datasyncer_->OnCreateRecords(context, nullptr, map, err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnCreateRecordsTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnCreateRecordsTest end";
}

/**
 * @tc.name: OnDeleteRecordsTest
 * @tc.desc: Verify the OnDeleteRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnDeleteRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDeleteRecordsTest Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(dataHandler_);
        DriveKit::DKError err;
        shared_ptr<map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>> map =
            make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();

        EXPECT_CALL(*dataHandler_, OnDeleteRecords(_)).Times(1).WillOnce(Return(1));
        datasyncer_->OnDeleteRecords(context, nullptr, map, err);
        GTEST_LOG_(INFO) << "OnDeleteRecordsTest other 001";
        EXPECT_CALL(*dataHandler_, OnDeleteRecords(_)).Times(1).WillOnce(Return(12));
        datasyncer_->OnDeleteRecords(context, nullptr, map, err);
        GTEST_LOG_(INFO) << "OnDeleteRecordsTest other 002";
        EXPECT_CALL(*dataHandler_, OnDeleteRecords(_)).Times(1).WillOnce(Return(24));
        datasyncer_->OnDeleteRecords(context, nullptr, map, err);
        GTEST_LOG_(INFO) << "OnDeleteRecordsTest other 003";
        EXPECT_CALL(*dataHandler_, OnDeleteRecords(_)).Times(1).WillOnce(Return(0));
        datasyncer_->OnDeleteRecords(context, nullptr, map, err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnDeleteRecordsTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnDeleteRecordsTest end";
}

/**
 * @tc.name: OnModifyMdirtyRecordsTest
 * @tc.desc: Verify the OnModifyMdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnModifyMdirtyRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecordsTest Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(dataHandler_);
        DriveKit::DKError err;
        shared_ptr<map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>> saveMap =
            make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();

        EXPECT_CALL(*dataHandler_, OnModifyMdirtyRecords(_)).Times(1).WillOnce(Return(1));
        datasyncer_->OnModifyMdirtyRecords(context, nullptr, saveMap, nullptr, err);
        GTEST_LOG_(INFO) << "OnModifyMdirtyRecordsTest other 001";
        EXPECT_CALL(*dataHandler_, OnModifyMdirtyRecords(_)).Times(1).WillOnce(Return(12));
        datasyncer_->OnModifyMdirtyRecords(context, nullptr, saveMap, nullptr, err);
        GTEST_LOG_(INFO) << "OnModifyMdirtyRecordsTest other 002";
        EXPECT_CALL(*dataHandler_, OnModifyMdirtyRecords(_)).Times(1).WillOnce(Return(24));
        datasyncer_->OnModifyMdirtyRecords(context, nullptr, saveMap, nullptr, err);
        GTEST_LOG_(INFO) << "OnModifyMdirtyRecordsTest other 003";
        EXPECT_CALL(*dataHandler_, OnModifyMdirtyRecords(_)).Times(1).WillOnce(Return(0));
        datasyncer_->OnModifyMdirtyRecords(context, nullptr, saveMap, nullptr, err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnModifyMdirtyRecordsTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecordsTest end";
}

/**
 * @tc.name: OnModifyFdirtyRecordsTest
 * @tc.desc: Verify the OnModifyFdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
 */
HWTEST_F(DataSyncerTest, OnModifyFdirtyRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecordsTest Start";
    try {
        shared_ptr<TaskContext> context = make_shared<TaskContext>(dataHandler_);
        DriveKit::DKError err;
        shared_ptr<map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>> saveMap =
            make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();

        EXPECT_CALL(*dataHandler_, OnModifyFdirtyRecords(_)).Times(1).WillOnce(Return(1));
        datasyncer_->OnModifyFdirtyRecords(context, nullptr, saveMap, nullptr, err);
        GTEST_LOG_(INFO) << "OnModifyFdirtyRecordsTest other 001";
        EXPECT_CALL(*dataHandler_, OnModifyFdirtyRecords(_)).Times(1).WillOnce(Return(12));
        datasyncer_->OnModifyFdirtyRecords(context, nullptr, saveMap, nullptr, err);
        GTEST_LOG_(INFO) << "OnModifyFdirtyRecordsTest other 002";
        EXPECT_CALL(*dataHandler_, OnModifyFdirtyRecords(_)).Times(1).WillOnce(Return(24));
        datasyncer_->OnModifyFdirtyRecords(context, nullptr, saveMap, nullptr, err);
        GTEST_LOG_(INFO) << "OnModifyFdirtyRecordsTest other 003";
        EXPECT_CALL(*dataHandler_, OnModifyFdirtyRecords(_)).Times(1).WillOnce(Return(0));
        datasyncer_->OnModifyFdirtyRecords(context, nullptr, saveMap, nullptr, err);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnModifyFdirtyRecordsTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecordsTest end";
}

/**
 * @tc.name: BeginTransactionTest
 * @tc.desc: Verify the BeginTransaction function
 * @tc.type: FUNC
 * @tc.require: #I7UU3Z
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
 * @tc.require: #I7UU3Z
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
 * @tc.require: #I7UU3Z
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
 * @tc.require: #I7UU3Z
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
 * @tc.require: #I7UU3Z
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
 * @tc.require: #I7VDFI
 */
HWTEST_F(DataSyncerTest, CompletePullTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompletePull Start";
    try {
        datasyncer_->CompletePull();
        datasyncer_->errorCode_ = 1;
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
 * @tc.require: #I7VDFI
 */
HWTEST_F(DataSyncerTest, CompletePushTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompletePush Start";
    try {
        datasyncer_->CompletePush();
        datasyncer_->errorCode_ = 1;
        datasyncer_->CompletePush();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CompletePush FAILED";
    }
    GTEST_LOG_(INFO) << "CompletePush end";
}

/**
 * @tc.name: CompleteAllTest
 * @tc.desc: Verify the CompleteAll function
 * @tc.type: FUNC
 * @tc.require: #I7VDFI
 */
HWTEST_F(DataSyncerTest, CompleteAllTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompleteAllTest Start";
    try {
        datasyncer_->lock_.count = 0;
        datasyncer_->errorCode_ = 0;
        datasyncer_->CompleteAll(true);
        datasyncer_->errorCode_ = 1;
        datasyncer_->CompleteAll(false);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CompleteAllTest FAILED";
    }
    GTEST_LOG_(INFO) << "CompleteAllTest end";
}

/**
 * @tc.name: SyncStateChangedNotifyTest
 * @tc.desc: Verify the SyncStateChangedNotify function
 * @tc.type: FUNC
 * @tc.require: #I7VDFI
 */
HWTEST_F(DataSyncerTest, SyncStateChangedNotifyTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateChangedNotify Start";
    try {
        datasyncer_->SyncStateChangedNotify(CloudSyncState::STOPPED, ErrorType::NO_ERROR);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateChangedNotify FAILED";
    }
    GTEST_LOG_(INFO) << "SyncStateChangedNotify end";
}

/**
 * @tc.name: NotifyCurrentSyncStateTest
 * @tc.desc: Verify the NotifyCurrentSyncState function
 * @tc.type: FUNC
 * @tc.require: #I7VDFI
 */
HWTEST_F(DataSyncerTest, NotifyCurrentSyncStateTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyCurrentSyncStateTest Start";
    try {
        datasyncer_->NotifyCurrentSyncState();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NotifyCurrentSyncStateTest FAILED";
    }
    GTEST_LOG_(INFO) << "NotifyCurrentSyncStateTest end";
}

/**
 * @tc.name: SetErrorCodeMaskTest
 * @tc.desc: Verify the SetErrorCodeMask function
 * @tc.type: FUNC
 * @tc.require: #I7VDFI
 */
HWTEST_F(DataSyncerTest, SetErrorCodeMaskTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetErrorCodeMaskTest Start";
    try {
        ErrorType errorType = NO_ERROR;
        datasyncer_->errorCode_ = 0;
        datasyncer_->SetErrorCodeMask(errorType);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetErrorCodeMaskTest FAILED";
    }
    GTEST_LOG_(INFO) << "SetErrorCodeMaskTest end";
}

/**
 * @tc.name: GetErrorTypeTest
 * @tc.desc: Verify the GetErrorType function
 * @tc.type: FUNC
 * @tc.require: #I7VDFI
 */
HWTEST_F(DataSyncerTest, GetErrorTypeTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetErrorType Start";
    datasyncer_->errorCode_ = E_OK;
    ErrorType result = datasyncer_->GetErrorType();
    EXPECT_EQ(ErrorType::NO_ERROR, result);

    datasyncer_->errorCode_ = 11;
    BatteryStatus::level_ = BatteryStatus::LEVEL_LOW;
    result = datasyncer_->GetErrorType();
    EXPECT_NE(ErrorType::NO_ERROR, result);

    datasyncer_->errorCode_ = 222;
    BatteryStatus::level_ = BatteryStatus::LEVEL_TOO_LOW;
    result = datasyncer_->GetErrorType();
    EXPECT_NE(ErrorType::NO_ERROR, result);

    BatteryStatus::level_ = BatteryStatus::LEVEL_NORMAL;
    result = datasyncer_->GetErrorType();
    EXPECT_NE(ErrorType::NO_ERROR, result);
    GTEST_LOG_(INFO) << "GetErrorType end";
}

/**
 * @tc.name: SaveSubscriptionTest
 * @tc.desc: Verify the SaveSubscription function
 * @tc.type: FUNC
 * @tc.require: #I7VDFI
 */
HWTEST_F(DataSyncerTest, SaveSubscriptionTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveSubscriptionTest Start";
    try {
        datasyncer_->SaveSubscription();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SaveSubscriptionTest FAILED";
    }
    GTEST_LOG_(INFO) << "SaveSubscriptionTest end";
}

/**
 * @tc.name: DeleteSubscriptionTest
 * @tc.desc: Verify the DeleteSubscription function
 * @tc.type: FUNC
 * @tc.require: #I7VDFI
 */
HWTEST_F(DataSyncerTest, DeleteSubscriptionTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteSubscriptionTest Start";
    try {
        datasyncer_->DeleteSubscription();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteSubscriptionTest FAILED";
    }
    GTEST_LOG_(INFO) << "DeleteSubscriptionTest end";
}
} // namespace OHOS::FileManagement::CloudSync::Test