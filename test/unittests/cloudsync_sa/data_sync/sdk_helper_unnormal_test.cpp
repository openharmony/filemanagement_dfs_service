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

/* template link */
namespace OHOS::FileManagement::CloudSync {
template<typename T, typename RET, typename... ARGS>
std::function<RET(ARGS...)> DataSyncer::AsyncCallback(RET(T::*f)(ARGS...))
{
    return taskRunner_->AsyncCallback<DataSyncer>(f, this);
}
}

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

#undef LOGI
void LOGI(string content)
{
    GTEST_LOG_(INFO) << content;
}

void SdkHelperTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    dataSyncManager_ = make_shared<DataSyncManager>();
    int32_t userId = 100;
    string bundleName = "com.ohos.photos";
    dataSyncer_ = dataSyncManager_->GetDataSyncer(bundleName, userId);
    shared_ptr<GalleryDataSyncer> galleryDataSyncer_ = static_pointer_cast<GalleryDataSyncer>(dataSyncer_);
    /* init handler */
    galleryDataSyncer_->GetHandler();
    handler_ = galleryDataSyncer_->fileHandler_;
    sdkHelper_ = std::make_shared<SdkHelper>();
}

void SdkHelperTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FetchRecordsErrorTest
 * @tc.desc: Verify the FetchRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, FetchRecordsErrorTest, TestSize.Level1)
{
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler_);
    auto callback = dataSyncer_->AsyncCallback(&DataSyncer::OnFetchRecords);
    ASSERT_NE(callback, nullptr);
    FetchCondition cond;
    handler_->GetFetchCondition(cond);
    int32_t ret = sdkHelper_->FetchRecords(context, cond, "nextCursor", callback);
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: FetchDatabaseChangesErrorTest
 * @tc.desc: Verify the FetchDatabaseChanges function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, FetchDatabaseChangesErrorTest, TestSize.Level1)
{
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler_);
    auto callback = dataSyncer_->AsyncCallback(&DataSyncer::OnFetchDatabaseChanges);
    ASSERT_NE(callback, nullptr);
    FetchCondition cond;
    handler_->GetFetchCondition(cond);
    int32_t ret = sdkHelper_->FetchDatabaseChanges(context, cond, "nextCursor", callback);
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: CreateRecordsErrorTest
 * @tc.desc: Verify the CreateRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, CreateRecordsErrorTest, TestSize.Level1)
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
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: DeleteRecordsErrorTest
 * @tc.desc: Verify the DeleteRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, DeleteRecordsErrorTest, TestSize.Level1)
{
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler_);
    auto handler = context->GetHandler();
    ASSERT_NE(handler, nullptr);
    vector<DriveKit::DKRecord> records;
    int32_t ret = handler->GetDeletedRecords(records);
    EXPECT_EQ(E_STOP, ret);
    auto callback = dataSyncer_->AsyncCallback(&DataSyncer::OnDeleteRecords);
    ASSERT_NE(callback, nullptr);
    ret = sdkHelper_->DeleteRecords(context, records, callback);
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: ModifyRecordsErrorTest
 * @tc.desc: Verify the ModifyRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, ModifyRecordsErrorTest, TestSize.Level1)
{
    shared_ptr<TaskContext> context = make_shared<TaskContext>(handler_);
    auto handler = context->GetHandler();
    ASSERT_NE(handler, nullptr);
    vector<DriveKit::DKRecord> records;
    int32_t ret = handler->GetMetaModifiedRecords(records);
    EXPECT_EQ(E_STOP, ret);
    auto callback = dataSyncer_->AsyncCallback(&DataSyncer::OnModifyMdirtyRecords);
    ASSERT_NE(callback, nullptr);
    ret = sdkHelper_->ModifyRecords(context, records, callback);
    EXPECT_EQ(E_CLOUD_SDK, ret);
}

/**
 * @tc.name: GetAssetReadSessionErrorTest
 * @tc.desc: Verify the GetAssetReadSession function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SdkHelperTest, GetAssetReadSessionErrorTest, TestSize.Level1)
{
    DriveKit::DKRecordType recordType = "SdkHelperTest";
    DriveKit::DKRecordId recordId = "GetAssetReadSessionTest";
    DriveKit::DKFieldKey assetKey = "test";
    DriveKit::DKAssetPath assetPath = "/data/file";
    auto ret = sdkHelper_->GetAssetReadSession(recordType, recordId, assetKey, assetPath);
    EXPECT_EQ(ret, nullptr);
}
} // namespace OHOS::FileManagement::CloudSync::Test