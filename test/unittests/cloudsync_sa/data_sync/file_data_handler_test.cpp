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

#include <fstream>
#include <filesystem>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <sys/stat.h>

#include "abs_shared_result_set_mock.h"
#include "dfs_error.h"
#include "file_data_handler.h"
#include "gallery_file_const.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store_config.h"
#include "rdb_store_mock.h"
#include "result_set_mock.h"
#include "sync_rule/cloud_status.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;
using namespace DriveKit;
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

#include "file_data_handler_two_test.cpp"
#include "file_data_handler_three_test.cpp"
#include "file_data_handler_four_test.cpp"

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        fileDataHandler->isChecking_ = true;
        fileDataHandler->GetFetchCondition(cond);
        fileDataHandler->isChecking_ = false;
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(rowCount)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetDouble(_, _)).WillRepeatedly(Return(0));
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
 * @tc.name: PullRecordInsert001
 * @tc.desc: Verify the PullRecordInsert function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, PullRecordInsert001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordInsert001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        DKRecord record;
        OnFetchParams onFetchParams{true};
        int32_t ret = fileDataHandler->PullRecordInsert(record, onFetchParams);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " PullRecordInsert001 ERROR";
    }

    GTEST_LOG_(INFO) << "PullRecordInsert001 End";
}

/**
 * @tc.name: PullRecordInsert002
 * @tc.desc: Verify the PullRecordInsert function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, PullRecordInsert002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordInsert002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        DKRecord record;
        OnFetchParams onFetchParams{false};
        int32_t ret = fileDataHandler->PullRecordInsert(record, onFetchParams);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " PullRecordInsert002 ERROR";
    }

    GTEST_LOG_(INFO) << "PullRecordInsert002 End";
}

/**
 * @tc.name: SetRetry001
 * @tc.desc: Verify the SetRetry function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, SetRetry001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRetry001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(1));

        std::string recordId = "recordId";
        int32_t ret = fileDataHandler->SetRetry(recordId);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetRetry001 ERROR";
    }

    GTEST_LOG_(INFO) << "SetRetry001 End";
}

/**
 * @tc.name: SetRetry002
 * @tc.desc: Verify the SetRetry function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, SetRetry002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRetry002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));

        std::string recordId = "recordId";
        int32_t ret = fileDataHandler->SetRetry(recordId);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetRetry002 ERROR";
    }

    GTEST_LOG_(INFO) << "SetRetry002 End";
}

/**
 * @tc.name: RecycleFile001
 * @tc.desc: Verify the RecycleFile function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, RecycleFile001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleFile001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(1));

        std::string recordId;
        int32_t ret = fileDataHandler->RecycleFile(recordId);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RecycleFile001 ERROR";
    }

    GTEST_LOG_(INFO) << "RecycleFile001 End";
}

/**
 * @tc.name: RecycleFile002
 * @tc.desc: Verify the RecycleFile function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, RecycleFile002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleFile002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));

        std::string recordId;
        int32_t ret = fileDataHandler->RecycleFile(recordId);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RecycleFile002 ERROR";
    }

    GTEST_LOG_(INFO) << "RecycleFile002 End";
}

/**
 * @tc.name: GetFileModifiedRecords001
 * @tc.desc: Verify the GetFileModifiedRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetFileModifiedRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileModifiedRecords001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        vector<DKRecord> records;
        int32_t ret = fileDataHandler->GetFileModifiedRecords(records);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFileModifiedRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetFileModifiedRecords001 End";
}

/**
 * @tc.name: GetFileModifiedRecords002
 * @tc.desc: Verify the GetFileModifiedRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetFileModifiedRecords002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileModifiedRecords002 Begin";
    try {
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        vector<DKRecord> records;
        int32_t ret = fileDataHandler->GetFileModifiedRecords(records);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFileModifiedRecords002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetFileModifiedRecords002 End";
}

/**
 * @tc.name: IsTimeChanged001
 * @tc.desc: Verify the IsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, IsTimeChanged001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        DriveKit::DKRecord record;
        std::unordered_map<std::string, LocalInfo> localMap;
        std::string path = "";
        std::string type = "hh";
        bool ret = fileDataHandler->IsTimeChanged(record, localMap, path, type);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsTimeChanged001 ERROR";
    }
    GTEST_LOG_(INFO) << "IsTimeChanged001 End";
}

/**
 * @tc.name: IsTimeChanged002
 * @tc.desc: Verify the IsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, IsTimeChanged002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        std::unordered_map<std::string, LocalInfo> localMap;
        std::string path = "";
        std::string type = "hh";
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("123")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetRecordData(data);
        bool ret = fileDataHandler->IsTimeChanged(record, localMap, path, type);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsTimeChanged002 ERROR";
    }
    GTEST_LOG_(INFO) << "IsTimeChanged002 End";
}

/**
 * @tc.name: IsTimeChanged003
 * @tc.desc: Verify the IsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, IsTimeChanged003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        std::unordered_map<std::string, LocalInfo> localMap;
        std::string path = "";
        std::string type = Media::PhotoColumn::PHOTO_META_DATE_MODIFIED;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetRecordData(data);
        bool ret = fileDataHandler->IsTimeChanged(record, localMap, path, type);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsTimeChanged003 ERROR";
    }
    GTEST_LOG_(INFO) << "IsTimeChanged003 End";
}

/**
 * @tc.name: IsTimeChanged004
 * @tc.desc: Verify the IsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, IsTimeChanged004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        std::unordered_map<std::string, LocalInfo> localMap;
        std::string path = "hh";
        std::string type = Media::PhotoColumn::PHOTO_META_DATE_MODIFIED;
        localMap[path] = { 1111, 1111 };
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetEditedTime(1111000);
        record.SetRecordData(data);
        bool ret = fileDataHandler->IsTimeChanged(record, localMap, path, type);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsTimeChanged004 ERROR";
    }
    GTEST_LOG_(INFO) << "IsTimeChanged004 End";
}

/**
 * @tc.name: IsTimeChanged005
 * @tc.desc: Verify the IsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, IsTimeChanged005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        std::unordered_map<std::string, LocalInfo> localMap;
        std::string path = "hh";
        std::string type = Media::PhotoColumn::MEDIA_DATE_MODIFIED;

        localMap[path] = { 11, 11 };
        bool ret = fileDataHandler->IsTimeChanged(record, localMap, path, type);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsTimeChanged005 ERROR";
    }
    GTEST_LOG_(INFO) << "IsTimeChanged005 End";
}

/**
 * @tc.name: GetLocalInfo001
 * @tc.desc: Verify the GetLocalInfo function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetLocalInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalInfo001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> map;
        std::unordered_map<std::string, LocalInfo> cloudMap;
        auto result = std::make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();
        DriveKit::DKRecordData data2;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        record.SetRecordData(data2);
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
        fileDataHandler->GetLocalInfo(*result, cloudMap, "hh");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLocalInfo001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLocalInfo001 End";
}

/**
 * @tc.name: GetLocalInfo002
 * @tc.desc: Verify the GetLocalInfo function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetLocalInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalInfo002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> map;
        std::unordered_map<std::string, LocalInfo> cloudMap;
        auto result = std::make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();
        DriveKit::DKRecordData data2;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecord record;
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        DriveKit::DKRecordId recordId = "1";
        record.SetRecordData(data2);
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
        fileDataHandler->GetLocalInfo(*result, cloudMap, "cloud_id");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLocalInfo002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLocalInfo002 End";
}

/**
 * @tc.name: GetLocalInfo003
 * @tc.desc: Verify the GetLocalInfo function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetLocalInfo003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalInfo003 Begin";
    try {
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> map;
        std::unordered_map<std::string, LocalInfo> cloudMap;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        auto result = std::make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        DriveKit::DKRecordData data2;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("123")));
        data2.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        record.SetRecordData(data2);
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
        fileDataHandler->GetLocalInfo(*result, cloudMap, "hh");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLocalInfo003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLocalInfo003 End";
}

/**
 * @tc.name: GetLocalInfo004
 * @tc.desc: Verify the GetLocalInfo function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetLocalInfo004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalInfo004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> map;
        std::unordered_map<std::string, LocalInfo> cloudMap;
        auto result = std::make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();
        DriveKit::DKRecordData data2;
        DriveKit::DKRecordFieldMap prop;
        data2.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        record.SetRecordData(data2);
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
        fileDataHandler->GetLocalInfo(*result, cloudMap, "hh");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLocalInfo004 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLocalInfo004 End";
}

/**
 * @tc.name: GetLocalInfo005
 * @tc.desc: Verify the GetLocalInfo function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetLocalInfo005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalInfo005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> map;
        std::unordered_map<std::string, LocalInfo> cloudMap;
        auto result = std::make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();
        DriveKit::DKRecordData data2;
        DriveKit::DKRecordFieldMap prop;
        data2.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        record.SetRecordData(data2);
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
        fileDataHandler->GetLocalInfo(*result, cloudMap, "hh");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLocalInfo005 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLocalInfo005 End";
}

/**
 * @tc.name: BuildInfoMap001
 * @tc.desc: Verify the BuildInfoMap function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, BuildInfoMap001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildInfoMap Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        std::unordered_map<std::string, LocalInfo> cloudMap;
        const int count = 3;
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(count)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(0));
        fileDataHandler->BuildInfoMap(move(rset), cloudMap, Media::PhotoColumn::PHOTO_CLOUD_ID);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BuildInfoMap001 ERROR";
    }
    GTEST_LOG_(INFO) << "BuildInfoMap001 End";
}

/**
 * @tc.name: Reset001
 * @tc.desc: Verify the Reset function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, Reset001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Reset001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        fileDataHandler->modifyFailSet_.push_back("1");
        fileDataHandler->createFailSet_.push_back("3");
        fileDataHandler->Reset();
        EXPECT_TRUE(fileDataHandler->modifyFailSet_.empty());
        EXPECT_TRUE(fileDataHandler->createFailSet_.empty());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Reset001 ERROR";
    }
    GTEST_LOG_(INFO) << "Reset001 End";
}

/**
 * @tc.name: OnCreateRecordSuccess001
 * @tc.desc: Verify the OnCreateRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateRecordSuccess001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unordered_map<std::string, LocalInfo> localMap;
        DriveKit::DKRecordData data2;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        record.SetRecordData(data2);
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnCreateRecordSuccess(entry, localMap);
        EXPECT_EQ(static_cast<int32_t>(E_INVAL_ARG), ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateRecordSuccess001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess001 End";
}

/**
 * @tc.name: OnCreateRecordSuccess002
 * @tc.desc: Verify the OnCreateRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateRecordSuccess002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unordered_map<std::string, LocalInfo> localMap;
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        DriveKit::DKRecordFieldMap prop;
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnCreateRecordSuccess(entry, localMap);
        EXPECT_EQ(static_cast<int32_t>(E_INVAL_ARG), ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateRecordSuccess002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess002 End";
}

/**
 * @tc.name: OnCreateRecordSuccess003
 * @tc.desc: Verify the OnCreateRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateRecordSuccess003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unordered_map<std::string, LocalInfo> localMap;
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        std::string path = "hh";
        DriveKit::DKRecordFieldMap prop;
        localMap[path] = { 1111, 1111 };
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("hh")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnCreateRecordSuccess(entry, localMap);
        EXPECT_EQ(E_OK, ret);
        ret = fileDataHandler->OnCreateRecordSuccess(entry, localMap);
        EXPECT_EQ(E_OK, ret);
        localMap[path] = { 11, 1111 };
        ret = fileDataHandler->OnCreateRecordSuccess(entry, localMap);
        EXPECT_EQ(E_OK, ret);
        localMap[path] = { 1111, 11 };
        ret = fileDataHandler->OnCreateRecordSuccess(entry, localMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateRecordSuccess003 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess003 End";
}

/**
 * @tc.name: OnMdirtyRecordSuccess001
 * @tc.desc: Verify the OnMdirtyRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnMdirtyRecordSuccess001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnMdirtyRecordSuccess001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(1));
        std::unordered_map<std::string, LocalInfo> localMap;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnMdirtyRecordSuccess(entry, localMap);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnMdirtyRecordSuccess001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnMdirtyRecordSuccesss001 End";
}

/**
 * @tc.name: OnMdirtyRecordSuccess002
 * @tc.desc: Verify the OnMdirtyRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnMdirtyRecordSuccess002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnMdirtyRecordSuccess002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));
        std::unordered_map<std::string, LocalInfo> localMap;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnMdirtyRecordSuccess(entry, localMap);
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnMdirtyRecordSuccess002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnMdirtyRecordSuccess002 End";
}

/**
 * @tc.name: OnMdirtyRecordSuccess003
 * @tc.desc: Verify the OnMdirtyRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnMdirtyRecordSuccess003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnMdirtyRecordSuccess003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));
        std::unordered_map<std::string, LocalInfo> localMap;
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        DriveKit::DKRecordFieldMap prop;
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnMdirtyRecordSuccess(entry, localMap);
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnMdirtyRecordSuccess003 ERROR";
    }
    GTEST_LOG_(INFO) << "OnMdirtyRecordSuccess003 End";
}

/**
 * @tc.name: OnMdirtyRecordSuccess004
 * @tc.desc: Verify the OnMdirtyRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnMdirtyRecordSuccess004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnMdirtyRecordSuccess004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));
        std::unordered_map<std::string, LocalInfo> localMap;
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnMdirtyRecordSuccess(entry, localMap);
        EXPECT_EQ(1, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnMdirtyRecordSuccess004 ERROR";
    }
    GTEST_LOG_(INFO) << "OnMdirtyRecordSuccess004 End";
}

/**
 * @tc.name: OnMdirtyRecordSuccess005
 * @tc.desc: Verify the OnMdirtyRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnMdirtyRecordSuccess005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnMdirtyRecordSuccess005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(1)).WillOnce(Return(0));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));
        std::unordered_map<std::string, LocalInfo> localMap;
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnMdirtyRecordSuccess(entry, localMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnMdirtyRecordSuccess005 ERROR";
    }
    GTEST_LOG_(INFO) << "OnMdirtyRecordSuccess005 End";
}

/**
 * @tc.name: OnDeleteRecordSuccess001
 * @tc.desc: Verify the OnDeleteRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnDeleteRecordSuccess001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDeleteRecordSuccess001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Delete(_, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnDeleteRecordSuccess(entry);
        EXPECT_EQ(0, ret);
        ret = fileDataHandler->OnDeleteRecordSuccess(entry);
        EXPECT_EQ(0, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnDeleteRecordSuccess001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnDeleteRecordSuccess001 End";
}

/**
 * @tc.name: OnModifyFdirtyRecords001
 * @tc.desc: Verify the OnModifyFdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyFdirtyRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));

        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;
        int32_t ret = fileDataHandler->OnModifyFdirtyRecords(myMap);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyFdirtyRecords001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords001 End";
}

/**
 * @tc.name: OnModifyFdirtyRecords002
 * @tc.desc: Verify the OnModifyFdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyFdirtyRecords002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));

        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        DriveKit::DKRecordId recordId = "hh";
        DriveKit::DKRecordOperResult operResult;
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;

        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("filePath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        myMap.insert(std::make_pair(recordId, operResult));
        int32_t ret = fileDataHandler->OnModifyFdirtyRecords(myMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyFdirtyRecords002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords002 End";
}

/**
 * @tc.name: OnModifyFdirtyRecords003
 * @tc.desc: Verify the OnModifyFdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyFdirtyRecords003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));

        DriveKit::DKError error_;
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        DriveKit::DKRecordId recordId = "hh";
        DriveKit::DKRecordOperResult operResult;
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;

        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("filePath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        error_.isLocalError = false;
        operResult.SetDKError(error_);
        myMap.insert(std::make_pair(recordId, operResult));
        int32_t ret = fileDataHandler->OnModifyFdirtyRecords(myMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyFdirtyRecords003 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords003 End";
}

/**
 * @tc.name: OnDeleteRecords001
 * @tc.desc: Verify the OnDeleteRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnDeleteRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDeleteRecords001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;
        EXPECT_CALL(*rdb, Delete(_, _, _, A<const vector<string> &>())).WillOnce(Return(0));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        DriveKit::DKError error_;
        error_.isLocalError = false;
        error_.SetServerError(static_cast<int32_t>(DriveKit::DKServerErrorCode::UID_EMPTY));
        DriveKit::DKErrorDetail errorDetail;
        errorDetail.detailCode = static_cast<int32_t>(DriveKit::DKDetailErrorCode::PARAM_INVALID);
        error_.errorDetails.push_back(errorDetail);
        operResult.SetDKError(error_);
        myMap.insert(std::make_pair(recordId, operResult));
        int32_t ret = fileDataHandler->OnDeleteRecords(myMap);
        EXPECT_EQ(E_STOP, ret);

        myMap.clear();
        error_.isServerError = false;
        operResult.SetDKError(error_);
        myMap.insert(std::make_pair(recordId, operResult));
        ret = fileDataHandler->OnDeleteRecords(myMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnDeleteRecords001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnDeleteRecords001 End";
}

/**
 * @tc.name: OnCreateRecords001
 * @tc.desc: Verify the OnCreateRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecords001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));

        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;
        int32_t ret = fileDataHandler->OnCreateRecords(myMap);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateRecords001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateRecords001 End";
}

/**
 * @tc.name: OnCreateRecords002
 * @tc.desc: Verify the OnCreateRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateRecords002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecords002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        DriveKit::DKRecordId recordId = "hh";
        DriveKit::DKRecordOperResult operResult;
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;

        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("filePath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        myMap.insert(std::make_pair(recordId, operResult));
        int32_t ret = fileDataHandler->OnCreateRecords(myMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateRecords002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateRecords002 End";
}

/**
 * @tc.name: OnCreateRecords003
 * @tc.desc: Verify the OnCreateRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateRecords003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecords003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        DriveKit::DKError error_;
        DriveKit::DKRecord record;
        DriveKit::DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        DriveKit::DKRecordId recordId = "hh";
        DriveKit::DKRecordOperResult operResult;
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;

        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("filePath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        error_.isLocalError = false;
        operResult.SetDKError(error_);
        myMap.insert(std::make_pair(recordId, operResult));
        int32_t ret = fileDataHandler->OnCreateRecords(myMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateRecords003 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateRecords003 End";
}

/**
 * @tc.name: OnFetchRecords001
 * @tc.desc: Verify the OnFetchRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnFetchRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchRecords001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));

        shared_ptr<vector<DKRecord>> records = make_shared<vector<DKRecord>>();
        OnFetchParams onFetchParams;
        int32_t ret = fileDataHandler->OnFetchRecords(records, onFetchParams);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "OnFetchRecords001 End";
}

/**
 * @tc.name: OnFetchRecords002
 * @tc.desc: Verify the OnFetchRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnFetchRecords002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchRecords002 Begin";
    try {
        const int rowCount = 0;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _))
            .WillOnce(Return(ByMove(std::move(rset))))
            .WillOnce(Return(ByMove(nullptr)))
            .WillOnce(Return(ByMove(nullptr)))
            .WillOnce(Return(ByMove(nullptr)));

        shared_ptr<vector<DKRecord>> records = make_shared<vector<DKRecord>>();
        OnFetchParams onFetchParams;
        int32_t ret = fileDataHandler->OnFetchRecords(records, onFetchParams);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchRecords002 ERROR";
    }

    GTEST_LOG_(INFO) << "OnFetchRecords002 End";
}

/**
 * @tc.name: OnFetchRecords003
 * @tc.desc: Verify the OnFetchRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnFetchRecords003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchRecords003 Begin";
    try {
        int rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GoToRow(_)).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset)))).WillRepeatedly(Return(nullptr));
        DKRecord record;
        record.SetRecordId("1");
        shared_ptr<vector<DKRecord>> records = make_shared<vector<DKRecord>>();
        records->push_back(record);
        OnFetchParams onFetchParams;
        int32_t ret = fileDataHandler->OnFetchRecords(records, onFetchParams);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchRecords003 ERROR";
    }

    GTEST_LOG_(INFO) << "OnFetchRecords003 End";
}

/**
 * @tc.name: PullRecordUpdate001
 * @tc.desc: Verify the PullRecordUpdate function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, PullRecordUpdate001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordUpdate001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));

        DKRecord record;
        OnFetchParams onFetchParams;
        int32_t ret = fileDataHandler->PullRecordUpdate(record, *rset, onFetchParams);
        EXPECT_NE(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PullRecordUpdate001 ERROR";
    }

    GTEST_LOG_(INFO) << "PullRecordUpdate001 End";
}

/**
 * @tc.name: PullRecordDelete001
 * @tc.desc: Verify the PullRecordDelete function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, PullRecordDelete001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordDelete001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));

        DKRecord record;
        int32_t ret = fileDataHandler->PullRecordDelete(record, *rset);
        EXPECT_NE(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PullRecordDelete001 ERROR";
    }

    GTEST_LOG_(INFO) << "PullRecordDelete001 End";
}

/**
 * @tc.name: ClearCloudInfo001
 * @tc.desc: Verify the ClearCloudInfo function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, ClearCloudInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearCloudInfo001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(1));

        std::string cloudId;
        int32_t ret = fileDataHandler->ClearCloudInfo(cloudId);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ClearCloudInfo001 ERROR";
    }

    GTEST_LOG_(INFO) << "ClearCloudInfo001 End";
}

/**
 * @tc.name: ClearCloudInfo002
 * @tc.desc: Verify the ClearCloudInfo function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, ClearCloudInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearCloudInfo002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));

        std::string cloudId;
        int32_t ret = fileDataHandler->ClearCloudInfo(cloudId);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ClearCloudInfo002 ERROR";
    }

    GTEST_LOG_(INFO) << "ClearCloudInfo002 End";
}

/**
 * @tc.name: DeleteDentryFile001
 * @tc.desc: Verify the DeleteDentryFile function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, DeleteDentryFile001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteDentryFile001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        int32_t ret = fileDataHandler->DeleteDentryFile();
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " DeleteDentryFile001 ERROR";
    }

    GTEST_LOG_(INFO) << "DeleteDentryFile001 End";
}

/**
 * @tc.name: CleanNotDirtyData001
 * @tc.desc: Verify the CleanNotDirtyData function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, CleanNotDirtyData001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotDirtyData001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Delete(_, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));
        int32_t ret = fileDataHandler->CleanNotDirtyData();
        EXPECT_TRUE(E_OK == ret || E_RDB == ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotDirtyData001 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotDirtyData001 End";
}

/**
 * @tc.name: CleanPureCloudRecord001
 * @tc.desc: Verify the CleanPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, CleanPureCloudRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanPureCloudRecord001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));

        string filePath = "/test/pareantPath/thumbPpath";
        int32_t ret = fileDataHandler->CleanPureCloudRecord();
        EXPECT_TRUE(E_OK == ret || E_RDB == ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanPureCloudRecord001 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanPureCloudRecord001 End";
}

/**
 * @tc.name: CleanNotPureCloudRecord001
 * @tc.desc: Verify the CleanNotPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, CleanNotPureCloudRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotPureCloudRecord001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        string filePath = "/test/pareantPath/thumbPpath";
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(1));

        int32_t ret = fileDataHandler->CleanNotPureCloudRecord(CleanAction::RETAIN_DATA);
        EXPECT_TRUE(E_OK == ret || E_RDB == ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotPureCloudRecord001 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotPureCloudRecord001 End";
}

/**
 * @tc.name: CleanNotPureCloudRecord002
 * @tc.desc: Verify the CleanNotPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, CleanNotPureCloudRecord002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotPureCloudRecord002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        string filePath = "/test/pareantPath/thumbPpath";
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));

        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(1));

        int32_t ret = fileDataHandler->CleanNotPureCloudRecord(CleanAction::RETAIN_DATA);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotPureCloudRecord002 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotPureCloudRecord002 End";
}

/**
 * @tc.name: CleanNotPureCloudRecord003
 * @tc.desc: Verify the CleanNotPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, CleanNotPureCloudRecord003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotPureCloudRecord003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        string filePath = "/test/pareantPath/thumbPpath";
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));

        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));

        int32_t ret = fileDataHandler->CleanNotPureCloudRecord(CleanAction::RETAIN_DATA);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotPureCloudRecord003 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotPureCloudRecord003 End";
}

/**
 * @tc.name: CleanNotPureCloudRecord004
 * @tc.desc: Verify the CleanNotPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, CleanNotPureCloudRecord004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotPureCloudRecord004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        string filePath = "/test/pareantPath/thumbPpath";
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, Delete(_, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        int32_t ret = fileDataHandler->CleanNotPureCloudRecord(CleanAction::RETAIN_DATA);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotPureCloudRecord004 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotPureCloudRecord004 End";
}

/**
 * @tc.name: CleanNotPureCloudRecord005
 * @tc.desc: Verify the CleanNotPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, CleanNotPureCloudRecord005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotPureCloudRecord005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        string filePath = "/test/pareantPath/thumbPpath";
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));

        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(2), Return(0)));

        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(1));

        int32_t ret = fileDataHandler->CleanNotPureCloudRecord(CleanAction::RETAIN_DATA);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotPureCloudRecord005 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotPureCloudRecord005 End";
}

/**
 * @tc.name: AppendToDownload
 * @tc.desc: Verify the AppendToDownload function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, AppendToDownload, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppendToDownload Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        DKRecord record;
        std::string fieldKey;
        std::vector<DKDownloadAsset> assetsToDownload;
        fileDataHandler->AppendToDownload(record, fieldKey, assetsToDownload);
        EXPECT_TRUE(true);
        DKFieldValue fieldValue = 1;
        DKRecordField recordField(fieldValue);
        record.fields_.insert(std::pair<DKFieldKey, DKRecordField>("properties", recordField));
        fileDataHandler->AppendToDownload(record, fieldKey, assetsToDownload);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AppendToDownload ERROR";
    }

    GTEST_LOG_(INFO) << "AppendToDownload End";
}

/**
 * @tc.name: GetAssetsToDownload001
 * @tc.desc: Verify the GetAssetsToDownload function
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(FileDataHandlerTest, GetAssetsToDownload001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAssetsToDownload001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        int32_t ret = fileDataHandler->GetAssetsToDownload(outAssetsToDownload);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAssetsToDownload001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetAssetsToDownload001 End";
}

/**
 * @tc.name: GetAssetsToDownload002
 * @tc.desc: Verify the GetAssetsToDownload function
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(FileDataHandlerTest, GetAssetsToDownload002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAssetsToDownload002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GoToNextRow()).WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        int32_t ret = fileDataHandler->GetAssetsToDownload(outAssetsToDownload);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAssetsToDownload002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetAssetsToDownload002 End";
}

/**
 * @tc.name: QueryLocalByCloudId
 * @tc.desc: Verify the QueryLocalByCloudId function
 * @tc.type: FUNC
 * @tc.require: issueI7UYAL
 */
HWTEST_F(FileDataHandlerTest, QueryLocalByCloudId, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryLocalByCloudId Begin";
    try {
        const string recordId = "1";
        std::vector<std::string> recordIds;
        recordIds.push_back(recordId);
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        auto [db, mp] = fileDataHandler->QueryLocalByCloudId(recordIds);
        EXPECT_EQ(db, NULL);

        const int count = 0;
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(count), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        auto [ptr, ret] = fileDataHandler->QueryLocalByCloudId(recordIds);
        EXPECT_EQ(static_cast<int>(ret.size()), E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryLocalByCloudId ERROR";
    }

    GTEST_LOG_(INFO) << "QueryLocalByCloudId End";
}

/**
 * @tc.name: GetCreatedRecords001
 * @tc.desc: Verify the GetCreatedRecords001 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetCreatedRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreatedRecords001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        std::vector<DKRecord> records;
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        int32_t ret = fileDataHandler->GetCreatedRecords(records);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetCreatedRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetCreatedRecords001 End";
}

/**
 * @tc.name: GetCreatedRecords002
 * @tc.desc: Verify the GetCreatedRecords002 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetCreatedRecords002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreatedRecords002 Begin";
    try {
        const int rowCount = 3;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        std::vector<DKRecord> records;
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(rowCount)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _))
            .WillOnce(Return(ByMove(std::move(rset))))
            .WillOnce(Return(nullptr));
        int32_t ret = fileDataHandler->GetCreatedRecords(records);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetCreatedRecords002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetCreatedRecords002 End";
}

/**
 * @tc.name: GetDeletedRecords001
 * @tc.desc: Verify the GetDeletedRecords001 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetDeletedRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeletedRecords001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        std::vector<DKRecord> records;
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        int32_t ret = fileDataHandler->GetDeletedRecords(records);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetDeletedRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetDeletedRecords001 End";
}

/**
 * @tc.name: GetDeletedRecords002
 * @tc.desc: Verify the GetDeletedRecords002 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetDeletedRecords002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeletedRecords002 Begin";
    try {
        const int rowCount = 3;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        std::vector<DKRecord> records;
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(rowCount)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetDouble(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        int32_t ret = fileDataHandler->GetDeletedRecords(records);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetDeletedRecords002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetDeletedRecords002 End";
}

/**
 * @tc.name: GetMetaModifiedRecords001
 * @tc.desc: Verify the GetMetaModifiedRecords001 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetMetaModifiedRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMetaModifiedRecords001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        vector<DKRecord> records;
        int32_t ret = fileDataHandler->GetMetaModifiedRecords(records);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetMetaModifiedRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetMetaModifiedRecords001 End";
}

/**
 * @tc.name: GetMetaModifiedRecords002
 * @tc.desc: Verify the GetMetaModifiedRecords002 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetMetaModifiedRecords002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMetaModifiedRecords002 Begin";
    try {
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        vector<DKRecord> records;
        int32_t ret = fileDataHandler->GetMetaModifiedRecords(records);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetMetaModifiedRecords002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetMetaModifiedRecords002 End";
}

/**
 * @tc.name: InsertTest
 * @tc.desc: Verify the Insert function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, InsertTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InsertTest Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Insert(_, _, _)).WillRepeatedly(Return(0));

        int64_t outRowId;
        NativeRdb::ValuesBucket initiavalues;
        int32_t ret = fileDataHandler->Insert(outRowId, initiavalues);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InsertTest ERROR";
    }

    GTEST_LOG_(INFO) << "InsertTest End";
}

/**
 * @tc.name: UpdateTest
 * @tc.desc: Verify the Update function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, UpdateTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateTest Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        int changedRows;
        NativeRdb::ValuesBucket values;
        int32_t ret = fileDataHandler->Update(changedRows, values, "", {""});
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateTest ERROR";
    }

    GTEST_LOG_(INFO) << "UpdateTest End";
}

/**
 * @tc.name: DeleteTest
 * @tc.desc: Verify the Delete function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, DeleteTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteTest Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Delete(_, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        int deletedRows;
        int32_t ret = fileDataHandler->Delete(deletedRows, "", {""});
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteTest ERROR";
    }

    GTEST_LOG_(INFO) << "DeleteTest End";
}

/**
 * @tc.name: QueryTest
 * @tc.desc: Verify the Query function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, QueryTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryTest Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));

        std::string tableName = "test_name";
        NativeRdb::AbsRdbPredicates predicates(tableName);
        std::vector<std::string> columns;
        auto ret = fileDataHandler->Query(predicates, columns);
        EXPECT_EQ(ret, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryTest ERROR";
    }

    GTEST_LOG_(INFO) << "QueryTest End";
}

} // namespace OHOS::FileManagement::CloudSync::Test
