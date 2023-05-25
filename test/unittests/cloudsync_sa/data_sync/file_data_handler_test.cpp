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

#include "abs_shared_result_set_mock.h"
#include "dfs_error.h"
#include "file_data_handler.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store_config.h"
#include "rdb_store_mock.h"
#include "result_set_mock.h"
#include "sync_rule/cloud_status.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;
const int USER_ID = 0;
const std::string BUND_NAME = "com.ohos.photos";
class FileDataHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void FileDataHandlerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileDataHandlerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileDataHandlerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileDataHandlerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetFetchCondition001
 * @tc.desc: Verify the GetFetchCondition function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetFetchCondition001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFetchCondition001 Begin";
    try {
        FetchCondition cond;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        fileDataHandler->GetFetchCondition(cond);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFetchCondition001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetFetchCondition001 End";
}

/**
 * @tc.name: GetRetryRecords001
 * @tc.desc: Verify the GetRetryRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetRetryRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRetryRecords001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        std::vector<DriveKit::DKRecordId> records;
        int32_t ret = fileDataHandler->GetRetryRecords(records);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetRetryRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetRetryRecords001 End";
}

/**
 * @tc.name: GetRetryRecords002
 * @tc.desc: Verify the GetRetryRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetRetryRecords002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRetryRecords002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        const int count = 3;
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(count)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));

        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));

        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));

        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        std::vector<DriveKit::DKRecordId> records;
        int32_t ret = fileDataHandler->GetRetryRecords(records);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetRetryRecords002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetRetryRecords002 End";
}

/**
 * @tc.name: GetDownloadAsset001
 * @tc.desc: Verify the GetDownloadAsset function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetDownloadAsset001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadAsset001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));

        std::string cloudId = "cloudId";
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        int32_t ret = fileDataHandler->GetDownloadAsset(cloudId, outAssetsToDownload);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetDownloadAsset001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetDownloadAsset001 End";
}

/**
 * @tc.name: GetDownloadAsset002
 * @tc.desc: Verify the GetDownloadAsset function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetDownloadAsset002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadAsset002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        std::string cloudId = "cloudId";
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        int32_t ret = fileDataHandler->GetDownloadAsset(cloudId, outAssetsToDownload);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetDownloadAsset002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetDownloadAsset002 End";
}

/**
 * @tc.name: GetDownloadAsset003
 * @tc.desc: Verify the GetDownloadAsset function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetDownloadAsset003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadAsset003 Begin";
    try {
        const int rowCount = 3;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(rowCount)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));

        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        std::string cloudId = "cloudId";
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        int32_t ret = fileDataHandler->GetDownloadAsset(cloudId, outAssetsToDownload);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetDownloadAsset003 ERROR";
    }

    GTEST_LOG_(INFO) << "GetDownloadAsset003 End";
}

/**
 * @tc.name: GetDownloadAsset004
 * @tc.desc: Verify the GetDownloadAsset function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetDownloadAsset004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadAsset004 Begin";
    try {
        const int times = 2;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GetRowCount(_)).Times(times).WillOnce(Return(0)).WillOnce(Return(1));

        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        std::string cloudId = "cloudId";
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        int32_t ret = fileDataHandler->GetDownloadAsset(cloudId, outAssetsToDownload);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetDownloadAsset004 ERROR";
    }

    GTEST_LOG_(INFO) << "GetDownloadAsset004 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test