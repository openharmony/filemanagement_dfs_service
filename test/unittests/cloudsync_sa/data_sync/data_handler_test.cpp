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
#include "gmock/gmock-spec-builders.h"
#include "sync_rule/battery_status.h"
#include "sync_rule/cloud_status.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DataHandlerMock final : public DataHandler {
public:
    DataHandlerMock(int32_t userId, const std::string &bundleName, const std::string &table)
        : DataHandler(userId, bundleName, table)
    {
    }

    MOCK_METHOD1(GetFetchCondition, void(FetchCondition &cond));
    MOCK_METHOD2(OnFetchRecords, int32_t(std::shared_ptr<std::vector<DriveKit::DKRecord>> &, OnFetchParams &));
    MOCK_METHOD1(GetRetryRecords, int32_t(std::vector<DriveKit::DKRecordId> &records));
    MOCK_METHOD1(GetCreatedRecords, int32_t(std::vector<DriveKit::DKRecord> &records));
    MOCK_METHOD1(GetDeletedRecords, int32_t(std::vector<DriveKit::DKRecord> &records));
    MOCK_METHOD1(GetMetaModifiedRecords, int32_t(std::vector<DriveKit::DKRecord> &records));
    MOCK_METHOD1(OnCreateRecords, int32_t(const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map));
    MOCK_METHOD1(OnDeleteRecords, int32_t(const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map));
    MOCK_METHOD1(OnModifyMdirtyRecords,
                 int32_t(const std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> &map));

public:
    void GetReturnTest();
    void OnRecordFailedTest();
    void FinishPullTest();
    void GetNextCursorTest();
};

void DataHandlerMock::GetReturnTest()
{
    int32_t retCode;
    int32_t error = E_STOP;
    GetReturn(error, retCode);
    EXPECT_EQ(retCode, E_STOP);
    error = E_CLOUD_STORAGE_FULL;
    GetReturn(error, retCode);
    EXPECT_EQ(retCode, E_CLOUD_STORAGE_FULL);
    error = E_SYNC_FAILED_NETWORK_NOT_AVAILABLE;
    GetReturn(error, retCode);
    EXPECT_EQ(retCode, E_SYNC_FAILED_NETWORK_NOT_AVAILABLE);
    retCode = E_OK;
    error = E_OK;
    GetReturn(error, retCode);
    EXPECT_EQ(retCode, E_OK);
}

void DataHandlerMock::OnRecordFailedTest()
{
    DriveKit::DKRecordOperResult result;
    DriveKit::DKError error;
    error.serverErrorCode = 0;
    error.errorDetails.clear();
    result.SetDKError(error);
    auto ret = OnRecordFailed({"", result});
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "OnRecordFailed other001";
    DriveKit::DKErrorDetail detail;
    detail.detailCode = 0;
    error.errorDetails.push_back(detail);
    result.SetDKError(error);
    ret = OnRecordFailed({"", result});
    EXPECT_EQ(ret, E_UNKNOWN);
    GTEST_LOG_(INFO) << "OnRecordFailed other002";
    detail.detailCode = static_cast<int>(DriveKit::DKDetailErrorCode::SPACE_FULL);
    error.errorDetails.clear();
    error.errorDetails.push_back(detail);
    result.SetDKError(error);
    ret = OnRecordFailed({"", result});
    EXPECT_EQ(ret, E_CLOUD_STORAGE_FULL);
    GTEST_LOG_(INFO) << "OnRecordFailed other003";
    error.serverErrorCode = static_cast<int>(DriveKit::DKServerErrorCode::NETWORK_ERROR);
    error.errorDetails.clear();
    result.SetDKError(error);
    ret = OnRecordFailed({"", result});
    EXPECT_EQ(ret, E_SYNC_FAILED_NETWORK_NOT_AVAILABLE);
    GTEST_LOG_(INFO) << "OnRecordFailed other004";
    error.serverErrorCode = static_cast<int>(DriveKit::DKServerErrorCode::UID_EMPTY);
    result.SetDKError(error);
    ret = OnRecordFailed({"", result});
    EXPECT_EQ(ret, E_STOP);
    GTEST_LOG_(INFO) << "OnRecordFailed other005";
    error.serverErrorCode = static_cast<int>(DriveKit::DKServerErrorCode::SWITCH_OFF);
    result.SetDKError(error);
    ret = OnRecordFailed({"", result});
    EXPECT_EQ(ret, E_STOP);
}

void DataHandlerMock::FinishPullTest()
{
    cursorMap_.clear();
    cursorFinishMap_.insert({0, true});
    isFinish_ = true;
    startCursor_.clear();
    FinishPull(0);
    GTEST_LOG_(INFO) << "FinishPull other001";
    cursorFinishMap_.clear();
    startCursor_ = "test";
    cursorFinishMap_.insert({1, true});
    FinishPull(0);
    GTEST_LOG_(INFO) << "FinishPull other002";
    isFinish_ = false;
    FinishPull(0);
    GTEST_LOG_(INFO) << "FinishPull other003";
    isFinish_ = false;
    DriveKit::DKQueryCursor cursor;
    cursorMap_.insert({1, cursor});
    FinishPull(0);
}
void DataHandlerMock::GetNextCursorTest()
{
    tempNextCursor_.clear();
    DriveKit::DKQueryCursor cursor;
    GetNextCursor(cursor);
    EXPECT_EQ(cursor, "");
    GTEST_LOG_(INFO) << "GetNextCursor other001";
    tempNextCursor_ = "test";
    GetNextCursor(cursor);
    EXPECT_EQ(cursor, "test");
}

class DataHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<DataHandlerMock> dataHandler_;
};
void DataHandlerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    string tableName = "test";
    string bundleName = "com.ohos.test";
    const int32_t userId = 100;
    dataHandler_ = make_shared<DataHandlerMock>(userId, bundleName, tableName);
}

void DataHandlerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    dataHandler_ = nullptr;
}

void DataHandlerTest::SetUp(void) {}

void DataHandlerTest::TearDown(void) {}

/**
 * @tc.name: GetNextCursorTest
 * @tc.desc: Verify the GetNextCursor function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, GetNextCursorTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNextCursorTest Start";
    try {
        dataHandler_->GetNextCursorTest();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNextCursorTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetNextCursorTest end";
}

/**
 * @tc.name: SetTempStartCursorTest
 * @tc.desc: Verify the SetTempStartCursor function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, SetTempStartCursorTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetTempStartCursorTest Start";
    try {
        DriveKit::DKQueryCursor cursor = "test";
        dataHandler_->SetTempStartCursor(cursor);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetTempStartCursorTest FAILED";
    }
    GTEST_LOG_(INFO) << "SetTempStartCursorTest end";
}

/**
 * @tc.name: GetTempStartCursorTest
 * @tc.desc: Verify the GetTempStartCursor function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, GetTempStartCursorTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetTempStartCursorTest Start";
    try {
        DriveKit::DKQueryCursor cursor;
        dataHandler_->GetTempStartCursor(cursor);
        EXPECT_EQ(cursor, "test");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetTempStartCursorTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetTempStartCursorTest end";
}

/**
 * @tc.name: SetTempNextCursorTest
 * @tc.desc: Verify the SetTempNextCursor function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, SetTempNextCursorTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetTempNextCursorTest Start";
    try {
        DriveKit::DKQueryCursor cursor;
        dataHandler_->SetTempNextCursor(cursor, true);
        GTEST_LOG_(INFO) << "SetTempNextCursorTest other";
        dataHandler_->SetTempNextCursor(cursor, false);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetTempNextCursorTest FAILED";
    }
    GTEST_LOG_(INFO) << "SetTempNextCursorTest end";
}

/**
 * @tc.name: GetBatchNoTest
 * @tc.desc: Verify the GetBatchNo function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, GetBatchNoTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBatchNoTest Start";
    try {
        auto ret = dataHandler_->GetBatchNo();
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBatchNoTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetBatchNoTest end";
}

/**
 * @tc.name: IsPullRecordsTest
 * @tc.desc: Verify the IsPullRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, IsPullRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPullRecordsTest Start";
    try {
        auto ret = dataHandler_->IsPullRecords();
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPullRecordsTest FAILED";
    }
    GTEST_LOG_(INFO) << "IsPullRecordsTest end";
}

/**
 * @tc.name: ClearCursorTest
 * @tc.desc: Verify the ClearCursor function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, ClearCursorTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearCursorTest Start";
    try {
        dataHandler_->ClearCursor();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ClearCursorTest FAILED";
    }
    GTEST_LOG_(INFO) << "ClearCursorTest end";
}

/**
 * @tc.name: FinishPullTest
 * @tc.desc: Verify the FinishPull function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, FinishPullTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FinishPullTest Start";
    try {
        dataHandler_->FinishPullTest();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FinishPullTest FAILED";
    }
    GTEST_LOG_(INFO) << "FinishPullTest end";
}

/**
 * @tc.name: SetRecordSizeTest
 * @tc.desc: Verify the SetRecordSize function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, SetRecordSizeTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRecordSizeTest Start";
    try {
        dataHandler_->SetRecordSize(1);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetRecordSizeTest FAILED";
    }
    GTEST_LOG_(INFO) << "SetRecordSizeTest end";
}

/**
 * @tc.name: GetRecordSizeTest
 * @tc.desc: Verify the GetRecordSize function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, GetRecordSizeTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRecordSizeTest Start";
    try {
        auto ret = dataHandler_->GetRecordSize();
        EXPECT_NE(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRecordSizeTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetRecordSizeTest end";
}

/**
 * @tc.name: GetCheckFlagTest
 * @tc.desc: Verify the GetCheckFlag function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, GetCheckFlagTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCheckFlagTest Start";
    try {
        auto ret = dataHandler_->GetCheckFlag();
        EXPECT_NE(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCheckFlagTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetCheckFlagTest end";
}

/**
 * @tc.name: SetCheckingTest
 * @tc.desc: Verify the SetChecking function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, SetCheckingTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetCheckingTest Start";
    try {
        dataHandler_->SetChecking();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetCheckingTest FAILED";
    }
    GTEST_LOG_(INFO) << "SetCheckingTest end";
}

/**
 * @tc.name: GetCheckRecordsTest
 * @tc.desc: Verify the GetCheckRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, GetCheckRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCheckRecordsTest Start";
    vector<DriveKit::DKRecordId> checkRecords;
    auto ret = dataHandler_->GetCheckRecords(checkRecords, nullptr);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetCheckRecordsTest end";
}

/**
 * @tc.name: GetFileModifiedRecordsTest
 * @tc.desc: Verify the GetFileModifiedRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, GetFileModifiedRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileModifiedRecordsTest Start";
    vector<DriveKit::DKRecord> records;
    auto ret = dataHandler_->GetFileModifiedRecords(records);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetFileModifiedRecordsTest end";
}

/**
 * @tc.name: OnModifyFdirtyRecordsTest
 * @tc.desc: Verify the OnModifyFdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, OnModifyFdirtyRecordsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecordsTest Start";
    auto ret = dataHandler_->OnModifyFdirtyRecords({});
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecordsTest end";
}

/**
 * @tc.name: GetAssetsToDownloadTest
 * @tc.desc: Verify the GetAssetsToDownload function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, GetAssetsToDownloadTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAssetsToDownloadTest Start";
    vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
    auto ret = dataHandler_->GetAssetsToDownload(outAssetsToDownload);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetAssetsToDownloadTest end";
}

/**
 * @tc.name: GetDownloadAssetTest
 * @tc.desc: Verify the GetDownloadAsset function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, GetDownloadAssetTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadAssetTest Start";
    vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
    auto ret = dataHandler_->GetDownloadAsset("100", outAssetsToDownload);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetDownloadAssetTest end";
}

/**
 * @tc.name: OnDownloadSuccessTest
 * @tc.desc: Verify the OnDownloadSuccess function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, OnDownloadSuccessTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadSuccessTest Start";
    DriveKit::DKDownloadAsset asset;
    auto ret = dataHandler_->OnDownloadSuccess(asset);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnDownloadSuccessTest end";
}

/**
 * @tc.name: OnDownloadAssetsTest
 * @tc.desc: Verify the OnDownloadAssets function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, OnDownloadAssetsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadAssetsTest Start";
    map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> resultMap;
    auto ret = dataHandler_->OnDownloadAssets(resultMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnDownloadAssetsTest end";
}

/**
 * @tc.name: OnDownloadAssetsTest002
 * @tc.desc: Verify the OnDownloadAssets function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, OnDownloadAssetsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadAssetsTest002 Start";
    DriveKit::DKDownloadAsset asset;
    auto ret = dataHandler_->OnDownloadAssets(asset);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnDownloadAssetsTest002 end";
}

/**
 * @tc.name: CleanTest
 * @tc.desc: Verify the Clean function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, CleanTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanTest Start";
    auto ret = dataHandler_->Clean(100);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "CleanTest end";
}

/**
 * @tc.name: OnRecordFailedTest
 * @tc.desc: Verify the OnRecordFailed function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, OnRecordFailedTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRecordFailedTest Start";
    dataHandler_->OnRecordFailedTest();
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "OnRecordFailedTest end";
}

/**
 * @tc.name: HandleCloudSpaceNotEnoughTest
 * @tc.desc: Verify the HandleCloudSpaceNotEnough function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, HandleCloudSpaceNotEnoughTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCloudSpaceNotEnoughTest Start";
    auto ret = dataHandler_->HandleCloudSpaceNotEnough();
    EXPECT_EQ(ret, E_CLOUD_STORAGE_FULL);
    GTEST_LOG_(INFO) << "HandleCloudSpaceNotEnoughTest end";
}

/**
 * @tc.name: HandleNotSupportSyncTest
 * @tc.desc: Verify the HandleNotSupportSync function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, HandleNotSupportSyncTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNotSupportSyncTest Start";
    auto ret = dataHandler_->HandleNotSupportSync();
    EXPECT_EQ(ret, E_STOP);
    GTEST_LOG_(INFO) << "HandleNotSupportSyncTest end";
}

/**
 * @tc.name: HandleNetworkErrTest
 * @tc.desc: Verify the HandleNetworkErr function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, HandleNetworkErrTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNetworkErrTest Start";
    auto ret = dataHandler_->HandleNetworkErr();
    EXPECT_EQ(ret, E_SYNC_FAILED_NETWORK_NOT_AVAILABLE);
    GTEST_LOG_(INFO) << "HandleNetworkErrTest end";
}

/**
 * @tc.name: HandleDetailcodeTest
 * @tc.desc: Verify the HandleDetailcode function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, HandleDetailcodeTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleDetailcodeTest Start";
    auto ret = dataHandler_->HandleDetailcode({});
    EXPECT_EQ(ret, E_UNKNOWN);
    GTEST_LOG_(INFO) << "HandleDetailcodeTest end";
}

/**
 * @tc.name: GetReturnTest
 * @tc.desc: Verify the GetReturn function
 * @tc.type: FUNC
 * @tc.require: #I7UVFF
 */
HWTEST_F(DataHandlerTest, GetReturnTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetReturnTest Start";
    dataHandler_->GetReturnTest();
    EXPECT_TRUE(true);
    GTEST_LOG_(INFO) << "GetReturnTest end";
}
} // namespace OHOS::FileManagement::CloudSync::Test