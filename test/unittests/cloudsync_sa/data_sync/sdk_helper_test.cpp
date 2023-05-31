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

#include "data_sync_manager.h"
#include "dfs_error.h"
#include "dk_record.h"
#include "gallery_data_syncer.h"
#include "sdk_helper.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class SdkHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<DataSyncManager> dataSyncManager_;
    shared_ptr<DataSyncer> dataSyncer_;
    shared_ptr<SdkHelper> sdkHelper_;
    shared_ptr<FileDataHandler> handler_;
};
void SdkHelperTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SdkHelperTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void LOGI(string content)
{
    GTEST_LOG_(INFO) << content;
}

void SdkHelperTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    dataSyncManager_ = make_shared<DataSyncManager>();
    int32_t userId = 100;
    string bundleName = "com.ohos.test";
    dataSyncer_ = dataSyncManager_->GetDataSyncer(bundleName, userId);
    shared_ptr<GalleryDataSyncer> galleryDataSyncer_ = static_pointer_cast<GalleryDataSyncer>(dataSyncer_);
    /* init handler */
    handler_ = galleryDataSyncer_->fileHandler_;
    sdkHelper_ = std::make_shared<SdkHelper>();
}

void SdkHelperTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: InitDriveKitInstanceFailTest
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, InitDriveKitInstanceFailTest, TestSize.Level1)
{
    int32_t userId = -1;
    string appBundleName = "com.ohos.test";
    auto ret = sdkHelper_->Init(userId, appBundleName);
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: InitContainerFailTest
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, InitContainerFailTest, TestSize.Level1)
{
    int32_t userId = 100;
    string appBundleName = "com.ohos.test.mock";
    auto ret = sdkHelper_->Init(userId, appBundleName);
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: InitOKTest
 * @tc.desc: Verify the Init function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, InitOKTest, TestSize.Level1)
{
    int32_t userId = 100;
    string appBundleName = "com.ohos.test12312";
    auto ret = sdkHelper_->Init(userId, appBundleName);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: GetLockOKTest
 * @tc.desc: Verify the GetLock function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, GetLockOKTest, TestSize.Level1)
{
    DriveKit::DKLock lock_;
    auto ret = sdkHelper_->GetLock(lock_);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: FetchRecordsOKTest
 * @tc.desc: Verify the FetchRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, FetchRecordsOKTest, TestSize.Level1)
{
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler_);
    auto callback = dataSyncer_->AsyncCallback(&DataSyncer::OnFetchRecords);
    ASSERT_NE(callback, nullptr);
    int32_t ret = sdkHelper_->FetchRecords(context, dataSyncer_->nextCursor_, callback);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: GetStartCursorOKTest
 * @tc.desc: Verify the GetStartCursor function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, GetStartCursorOKTest, TestSize.Level1)
{
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler_);
    std::string tempStartCursor;
    int32_t ret = sdkHelper_->GetStartCursor(context, tempStartCursor);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: FetchRecordWithIdOKTest
 * @tc.desc: Verify the FetchRecordWithId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, FetchRecordWithIdOKTest, TestSize.Level1)
{
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler_);
    auto callback = dataSyncer_->AsyncCallback(&DataSyncer::OnFetchRetryRecord);
    ASSERT_NE(callback, nullptr);
    int32_t ret = sdkHelper_->FetchRecordWithId(context, "", callback);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: FetchDatabaseChangesOKTest
 * @tc.desc: Verify the FetchDatabaseChanges function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, FetchDatabaseChangesOKTest, TestSize.Level1)
{
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler_);
    auto callback = dataSyncer_->AsyncCallback(&DataSyncer::OnFetchDatabaseChanges);
    ASSERT_NE(callback, nullptr);
    int32_t ret = sdkHelper_->FetchDatabaseChanges(context, dataSyncer_->nextCursor_, callback);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: CreateRecordsOKTest
 * @tc.desc: Verify the CreateRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, CreateRecordsOKTest, TestSize.Level1)
{
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler_);
    auto handler = context->GetHandler();
    ASSERT_NE(handler, nullptr);
    vector<DriveKit::DKRecord> records;
    int32_t ret = handler->GetCreatedRecords(records);
    EXPECT_EQ(E_OK, ret);
    auto callback = dataSyncer_->AsyncCallback(&DataSyncer::OnCreateRecords);
    ASSERT_NE(callback, nullptr);
    ret = sdkHelper_->CreateRecords(context, records, callback);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: DeleteRecordsOKTest
 * @tc.desc: Verify the DeleteRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, DeleteRecordsOKTest, TestSize.Level1)
{
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler_);
    auto handler = context->GetHandler();
    ASSERT_NE(handler, nullptr);
    vector<DriveKit::DKRecord> records;
    int32_t ret = handler->GetDeletedRecords(records);
    EXPECT_EQ(E_OK, ret);
    auto callback = dataSyncer_->AsyncCallback(&DataSyncer::OnDeleteRecords);
    ASSERT_NE(callback, nullptr);
    ret = sdkHelper_->DeleteRecords(context, records, callback);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: ModifyRecordsOKTest
 * @tc.desc: Verify the ModifyRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, ModifyRecordsOKTest, TestSize.Level1)
{
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler_);
    auto handler = context->GetHandler();
    ASSERT_NE(handler, nullptr);
    vector<DriveKit::DKRecord> records;
    int32_t ret = handler->GetMetaModifiedRecords(records);
    EXPECT_EQ(E_OK, ret);
    auto callback = dataSyncer_->AsyncCallback(&DataSyncer::OnModifyMdirtyRecords);
    ASSERT_NE(callback, nullptr);
    ret = sdkHelper_->ModifyRecords(context, records, callback);
    EXPECT_EQ(E_OK, ret);
}

/**
 * @tc.name: CancelDownloadAssetsOKTest
 * @tc.desc: Verify the CancelDownloadAssets function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, CancelDownloadAssetsOKTest, TestSize.Level1)
{
    int32_t id = 0;
    int32_t ret = sdkHelper_->CancelDownloadAssets(id);
    EXPECT_EQ(E_OK, ret);
}
} // namespace OHOS::FileManagement::CloudSync::Test