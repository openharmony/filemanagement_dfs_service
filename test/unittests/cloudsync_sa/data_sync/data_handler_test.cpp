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

#include "data_handler.h"
#include "data_sync_manager.h"
#include "dfs_error.h"
#include "gallery_data_syncer.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/cloud_status.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DataHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<FileDataHandler> datahandler_;
};
void DataHandlerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DataHandlerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DataHandlerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    shared_ptr<DataSyncManager> dataSyncManager_ = make_shared<DataSyncManager>();
    int32_t userId = 100;
    string bundleName = "com.ohos.test";
    shared_ptr<DataSyncer> dataSyncer_ = dataSyncManager_->GetDataSyncer(bundleName, userId);
    shared_ptr<GalleryDataSyncer> galleryDataSyncer_ = static_pointer_cast<GalleryDataSyncer>(dataSyncer_);
    /* init handler */
    datahandler_ = galleryDataSyncer_->fileHandler_;
}

void DataHandlerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SetCursorTest
 * @tc.desc: Verify the SetCursor function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataHandlerTest, SetCursorTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetCursor Start";
    int res = datahandler_->SetCursor();
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "SetCursor End";
}

/**
 * @tc.name: GetCursorTest
 * @tc.desc: Verify the GetCursor function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataHandlerTest, GetCursorTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCursor Start";
    int res = datahandler_->GetCursor();
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "GetCursor End";
}

/**
 * @tc.name: GetFileModifiedRecordsTest
 * @tc.desc: Verify the GetFileModifiedRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataHandlerTest, GetFileModifiedRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileModifiedRecords Start";
    vector<DriveKit::DKRecord> records;
    int res = datahandler_->GetFileModifiedRecords(records);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "GetFileModifiedRecords End";
}

/**
 * @tc.name: OnModifyFdirtyRecordsTest
 * @tc.desc: Verify the OnModifyFdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataHandlerTest, OnModifyFdirtyRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords Start";
    map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> map;
    int res = datahandler_->OnModifyFdirtyRecords(map);
    EXPECT_EQ(res, E_OK);
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords End";
}

} // namespace OHOS::FileManagement::CloudSync::Test