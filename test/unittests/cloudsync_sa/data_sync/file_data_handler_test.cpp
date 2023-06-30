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

/**
 * @tc.name: PullRecordInsert001
 * @tc.desc: Verify the GetDownloadAsset function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, PullRecordInsert001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordInsert001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
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
 * @tc.desc: Verify the GetDownloadAsset function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, PullRecordInsert002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordInsert002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(1));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(0));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(1));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(0));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

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
 * @tc.name: OnCreateIsTimeChanged001
 * @tc.desc: Verify the OnCreateIsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateIsTimeChangedd001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateIsTimeChanged001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        DriveKit::DKRecordData data;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        std::string path = "";
        std::string type = "hh";
        int32_t ret = fileDataHandler->OnCreateIsTimeChanged(data, localMap, path, type);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateIsTimeChanged001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateIsTimeChanged001 End";
}

/**
 * @tc.name: OnCreateIsTimeChangedd002
 * @tc.desc: Verify the OnCreateIsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateIsTimeChangedd002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateIsTimeChanged002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        DriveKit::DKRecordData data;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        std::string path = "";
        std::string type = "hh";
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("123")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        int32_t ret = fileDataHandler->OnCreateIsTimeChanged(data, localMap, path, type);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateIsTimeChanged002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateIsTimeChanged002 End";
}

/**
 * @tc.name: OnCreateIsTimeChanged003
 * @tc.desc: Verify the OnCreateIsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateIsTimeChanged003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateIsTimeChanged003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        DriveKit::DKRecordData data;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        std::string path = "";
        std::string type = Media::PhotoColumn::PHOTO_META_DATE_MODIFIED;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        int32_t ret = fileDataHandler->OnCreateIsTimeChanged(data, localMap, path, type);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateIsTimeChanged003 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateIsTimeChanged003 End";
}

/**
 * @tc.name: OnCreateIsTimeChanged004
 * @tc.desc: Verify the OnCreateIsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateIsTimeChanged004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateIsTimeChanged004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        DriveKit::DKRecordData data;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        std::string path = "hh";
        std::string type = Media::PhotoColumn::PHOTO_META_DATE_MODIFIED;
        localMap[path] = std::make_pair(1111, 1111);
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        int32_t ret = fileDataHandler->OnCreateIsTimeChanged(data, localMap, path, type);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateIsTimeChanged004 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateIsTimeChanged004 End";
}

/**
 * @tc.name: OnCreateIsTimeChanged005
 * @tc.desc: Verify the OnCreateIsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateIsTimeChanged005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateIsTimeChanged005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecordData data;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        std::string path = "hh";
        std::string type = Media::PhotoColumn::MEDIA_DATE_MODIFIED;

        localMap[path] = std::make_pair(11, 11);
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));

        int32_t ret = fileDataHandler->OnCreateIsTimeChanged(data, localMap, path, type);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnCreateIsTimeChanged005 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateIsTimeChanged005 End";
}

/**
 * @tc.name: OnModifyIsTimeChanged001
 * @tc.desc: Verify the OnModifyIsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyIsTimeChanged001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyIsTimeChanged001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        DriveKit::DKRecordData data;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        std::string cloudId = "";
        std::string type = "hh";
        int32_t ret = fileDataHandler->OnModifyIsTimeChanged(data, localMap, cloudId, type);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyIsTimeChanged001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyIsTimeChanged001 End";
}

/**
 * @tc.name: OnModifyIsTimeChanged002
 * @tc.desc: Verify the OnModifyIsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyIsTimeChanged002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyIsTimeChanged001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        DriveKit::DKRecordData data;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        std::string cloudId = "";
        std::string type = "hh";
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("123")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        int32_t ret = fileDataHandler->OnModifyIsTimeChanged(data, localMap, cloudId, type);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyIsTimeChanged002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyIsTimeChanged002 End";
}

/**
 * @tc.name: OnModifyIsTimeChanged003
 * @tc.desc: Verify the OnModifyIsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyIsTimeChanged003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyIsTimeChanged003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        DriveKit::DKRecordData data;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        std::string cloudId = "";
        std::string type = Media::PhotoColumn::PHOTO_META_DATE_MODIFIED;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        int32_t ret = fileDataHandler->OnModifyIsTimeChanged(data, localMap, cloudId, type);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyIsTimeChanged003 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyIsTimeChanged003 End";
}

/**
 * @tc.name: OnModifyIsTimeChanged004
 * @tc.desc: Verify the OnModifyIsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyIsTimeChanged004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyIsTimeChanged004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        DriveKit::DKRecordData data;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        std::string cloudId = "hh";
        std::string type = Media::PhotoColumn::PHOTO_META_DATE_MODIFIED;
        localMap[cloudId] = std::make_pair(1111, 1111);
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        int32_t ret = fileDataHandler->OnModifyIsTimeChanged(data, localMap, cloudId, type);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyIsTimeChanged004 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyIsTimeChanged004 End";
}

/**
 * @tc.name: OnModifyIsTimeChanged005
 * @tc.desc: Verify the OnModifyIsTimeChanged function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyIsTimeChanged005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyIsTimeChanged005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        DriveKit::DKRecordData data;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        std::string cloudId = "hh";
        std::string type = Media::PhotoColumn::MEDIA_DATE_MODIFIED;
        localMap[cloudId] = std::make_pair(11, 11);
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        int32_t ret = fileDataHandler->OnModifyIsTimeChanged(data, localMap, cloudId, type);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyIsTimeChanged005 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyIsTimeChanged005 End";
}

/**
 * @tc.name: GetLocalTimeMap001
 * @tc.desc: Verify the GetLocalTimeMap function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetLocalTimeMap001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalTimeMap001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> map;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> cloudMap;
        auto result = std::make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();
        DriveKit::DKRecordData data2;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        record.SetRecordData(data2);
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
        fileDataHandler->GetLocalTimeMap(*result, cloudMap, "hh");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLocalTimeMap001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLocalTimeMap001 End";
}

/**
 * @tc.name: GetLocalTimeMap002
 * @tc.desc: Verify the GetLocalTimeMap function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetLocalTimeMap002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalTimeMap002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> map;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> cloudMap;
        auto result = std::make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();
        DriveKit::DKRecordData data2;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecord record;
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        DriveKit::DKRecordId recordId = "1";
        record.SetRecordData(data2);
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
        fileDataHandler->GetLocalTimeMap(*result, cloudMap, "cloud_id");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLocalTimeMap002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLocalTimeMap002 End";
}

/**
 * @tc.name: GetLocalTimeMap003
 * @tc.desc: Verify the GetLocalTimeMap function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetLocalTimeMap003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalTimeMap003 Begin";
    try {
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> map;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> cloudMap;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        auto result = std::make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        DriveKit::DKRecordData data2;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("123")));
        data2.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        record.SetRecordData(data2);
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
        fileDataHandler->GetLocalTimeMap(*result, cloudMap, "hh");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLocalTimeMap003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLocalTimeMap003 End";
}

/**
 * @tc.name: GetLocalTimeMap004
 * @tc.desc: Verify the GetLocalTimeMap function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetLocalTimeMap004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalTimeMap004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> map;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> cloudMap;
        auto result = std::make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();
        DriveKit::DKRecordData data2;
        DriveKit::DKRecordFieldMap prop;
        data2.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        record.SetRecordData(data2);
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
        fileDataHandler->GetLocalTimeMap(*result, cloudMap, "hh");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLocalTimeMap004 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLocalTimeMap004 End";
}

/**
 * @tc.name: GetLocalTimeMap005
 * @tc.desc: Verify the GetLocalTimeMap function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, GetLocalTimeMap005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalTimeMap005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> map;
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> cloudMap;
        auto result = std::make_shared<std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult>>();
        DriveKit::DKRecordData data2;
        DriveKit::DKRecordFieldMap prop;
        data2.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        record.SetRecordData(data2);
        operResult.SetDKRecord(std::move(record));
        (*result)[recordId] = operResult;
        fileDataHandler->GetLocalTimeMap(*result, cloudMap, "hh");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLocalTimeMap005 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLocalTimeMap005 End";
}

/**
 * @tc.name: OnResultSetConvertToMap001
 * @tc.desc: Verify the OnResultSetConvertToMap function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnResultSetConvertToMap001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnResultSetConvertToMap Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> cloudMap;
        const int count = 3;
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(count)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(0));
        fileDataHandler->OnResultSetConvertToMap(move(rset), cloudMap, Media::PhotoColumn::PHOTO_CLOUD_ID);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnResultSetConvertToMap001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnResultSetConvertToMap001 End";
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
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
 * @tc.name: HandleCloudSpaceNotEnough001
 * @tc.desc: Verify the HandleCloudSpaceNotEnough function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, HandleCloudSpaceNotEnough001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCloudSpaceNotEnough001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry;
        DriveKit::DKError error_;
        error_.serverErrorCode = static_cast<int32_t>(DriveKit::DKServerErrorCode::ACCESS_DENIED);
        DriveKit::DKErrorDetail errorDetail2;
        errorDetail2.errorCode = SPACE_NOT_ENOUGH;
        error_.errorDetails.push_back(errorDetail2);
        entry.second.SetDKError(error_);
        int32_t result = fileDataHandler->OnRecordFailed(entry);
        EXPECT_EQ(result, static_cast<int32_t>(fileDataHandler->HandleCloudSpaceNotEnough()));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleCloudSpaceNotEnough001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleCloudSpaceNotEnough001 End";
}

/**
 * @tc.name: HandleATFailed001
 * @tc.desc: Verify the HandleATFailed function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, HandleATFailed001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleATFailed001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry;
        DriveKit::DKError error_;
        error_.serverErrorCode = static_cast<int32_t>(DriveKit::DKServerErrorCode::AUTHENTICATION_FAILED);
        DriveKit::DKErrorDetail errorDetail2;
        errorDetail2.errorCode = AT_FAILED;
        error_.errorDetails.push_back(errorDetail2);
        entry.second.SetDKError(error_);
        int32_t result = fileDataHandler->OnRecordFailed(entry);
        EXPECT_EQ(result, static_cast<int32_t>(fileDataHandler->HandleATFailed()));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleATFailed001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleATFailed001 End";
}

/**
 * @tc.name: HandleNameConflict001
 * @tc.desc: Verify the HandleNameConflict function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, HandleNameConflict001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNameConflict001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry;
        DriveKit::DKError error_;
        error_.serverErrorCode = static_cast<int32_t>(DriveKit::DKServerErrorCode::ACCESS_DENIED);
        DriveKit::DKErrorDetail errorDetail2;
        errorDetail2.errorCode = NAME_CONFLICT;
        error_.errorDetails.push_back(errorDetail2);
        entry.second.SetDKError(error_);
        int32_t result = fileDataHandler->OnRecordFailed(entry);
        EXPECT_EQ(result, static_cast<int32_t>(fileDataHandler->HandleNameConflict()));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleNameConflict001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNameConflict001 End";
}

/**
 * @tc.name: HandleNameInvalid001
 * @tc.desc: Verify the HandleNameInvalid function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, HandleNameInvalid001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleNameInvalid001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry;
        DriveKit::DKError error_;
        error_.serverErrorCode = static_cast<int32_t>(DriveKit::DKServerErrorCode::ATOMIC_ERROR);
        DriveKit::DKErrorDetail errorDetail2;
        errorDetail2.errorCode = INVALID_FILE;
        error_.errorDetails.push_back(errorDetail2);
        entry.second.SetDKError(error_);
        int32_t result = fileDataHandler->OnRecordFailed(entry);
        EXPECT_EQ(result, static_cast<int32_t>(fileDataHandler->HandleNameInvalid()));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleNameInvalid001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNameInvalid001 End";
}

/**
 * @tc.name: OnRecordFailed001
 * @tc.desc: Verify the OnRecordFailed function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnRecordFailed001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRecordFailed001";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry;
        DriveKit::DKError error_;
        error_.serverErrorCode = static_cast<int32_t>(DriveKit::DKServerErrorCode::ATOMIC_ERROR);
        entry.second.SetDKError(error_);
        int32_t ret = fileDataHandler->OnRecordFailed(entry);
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleNameInvalid ERROR";
    }
    GTEST_LOG_(INFO) << "HandleNameInvalid End";
}

/**
 * @tc.name: OnRecordFailed002
 * @tc.desc: Verify the OnRecordFailed002 function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnRecordFailed002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnRecordFailed002";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry;
        DriveKit::DKError error_;
        error_.serverErrorCode = static_cast<int32_t>(DriveKit::DKServerErrorCode::ATOMIC_ERROR);
        DriveKit::DKErrorDetail errorDetail2;
        errorDetail2.errorCode = AT_FAILED;
        error_.errorDetails.push_back(errorDetail2);
        entry.second.SetDKError(error_);
        int32_t ret = fileDataHandler->OnRecordFailed(entry);
        EXPECT_EQ(E_STOP, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnRecordFailed002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnRecordFailed002 End";
}

/**
 * @tc.name: OnCreateRecordSuccess001
 * @tc.desc: Verify the OnCreateRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnCreateRecordSuccess001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalTimeMap001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
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
    GTEST_LOG_(INFO) << "GetLocalTimeMap002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        DriveKit::DKRecordFieldMap prop;
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
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
    GTEST_LOG_(INFO) << "GetLocalTimeMap003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        EXPECT_CALL(*rdb, Update(_, _, _, _, _))
            .WillOnce(Return(0))
            .WillOnce(Return(1))
            .WillOnce(Return(0))
            .WillOnce(Return(0));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        std::string path = "hh";
        DriveKit::DKRecordFieldMap prop;
        localMap[path] = std::make_pair(1111, 1111);
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("hh")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnCreateRecordSuccess(entry, localMap);
        EXPECT_EQ(E_OK, ret);
        ret = fileDataHandler->OnCreateRecordSuccess(entry, localMap);
        EXPECT_EQ(ret, 1);
        localMap[path] = std::make_pair(11, 1111);
        ret = fileDataHandler->OnCreateRecordSuccess(entry, localMap);
        EXPECT_EQ(E_OK, ret);
        localMap[path] = std::make_pair(1111, 11);
        ret = fileDataHandler->OnCreateRecordSuccess(entry, localMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateRecordSuccess003 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess003 End";
}

/**
 * @tc.name: OnModifyRecordSuccess001
 * @tc.desc: Verify the OnModifyRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyRecordSuccess001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyRecordSuccess001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnModifyRecordSuccess(entry, localMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyRecordSuccess001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyRecordSuccesss001 End";
}

/**
 * @tc.name: OnModifyRecordSuccess002
 * @tc.desc: Verify the OnModifyRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyRecordSuccesss002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalTimeMap002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        std::string cloudId = "hh";
        localMap[cloudId] = std::make_pair(1111, 1111);
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnModifyRecordSuccess(entry, localMap);
        EXPECT_EQ(E_OK, ret);
        ret = fileDataHandler->OnModifyRecordSuccess(entry, localMap);
        EXPECT_EQ(1, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyRecordSuccesss002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyRecordSuccesss002 End";
}

/**
 * @tc.name: OnModifyRecordSuccess003
 * @tc.desc: Verify the OnModifyRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyRecordSuccesss003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyRecordSuccesss003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        std::string cloudId = "hh";
        localMap[cloudId] = std::make_pair(1111, 11);
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnModifyRecordSuccess(entry, localMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyRecordSuccesss003 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyRecordSuccesss003 End";
}

/**
 * @tc.name: OnModifyRecordSuccess004
 * @tc.desc: Verify the OnModifyRecordSuccess function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyRecordSuccess004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyRecordSuccess004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<std::string, std::pair<std::int64_t, std::int64_t>> localMap;
        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        std::string cloudId = "hh";
        localMap[cloudId] = std::make_pair(1111, 11);
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1111)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnModifyRecordSuccess(entry, localMap);
        EXPECT_EQ(1, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyRecordSuccess004 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyRecordSuccess004 End";
}

/**
 * @tc.name: IsSkipSyncOKTest
 * @tc.desc: Verify the IsSkipSync function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnDeleteRecordSuccess001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDeleteRecordSuccess001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Delete(_, _, _, _)).WillOnce(Return(0)).WillOnce(Return(1));
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "1";
        std::pair<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> entry(recordId, operResult);
        int32_t ret = fileDataHandler->OnDeleteRecordSuccess(entry);
        EXPECT_EQ(0, ret);
        ret = fileDataHandler->OnDeleteRecordSuccess(entry);
        EXPECT_EQ(1, ret);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr))).WillOnce(Return(ByMove(nullptr)));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        DriveKit::DKError error_;
        error_.isLocalError = true;
        error_.serverErrorCode = static_cast<int32_t>(DriveKit::DKServerErrorCode::ATOMIC_ERROR);
        DriveKit::DKErrorDetail errorDetail2;
        errorDetail2.errorCode = AT_FAILED;
        error_.errorDetails.push_back(errorDetail2);
        operResult.SetDKError(error_);
        myMap.insert(std::make_pair(recordId, operResult));
        int32_t ret = fileDataHandler->OnModifyFdirtyRecords(myMap);
        EXPECT_EQ(E_STOP, ret);
        myMap.clear();
        error_.isLocalError = false;
        operResult.SetDKError(error_);
        myMap.insert(std::make_pair(recordId, operResult));
        ret = fileDataHandler->OnModifyFdirtyRecords(myMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyFdirtyRecords001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords001 End";
}

/**
 * @tc.name: OnModifyMdirtyRecords001
 * @tc.desc: Verify the OnModifyMdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, OnModifyMdirtyRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr))).WillOnce(Return(ByMove(nullptr)));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        DriveKit::DKError error_;
        error_.isLocalError = true;
        error_.serverErrorCode = static_cast<int32_t>(DriveKit::DKServerErrorCode::ATOMIC_ERROR);
        DriveKit::DKErrorDetail errorDetail2;
        errorDetail2.errorCode = AT_FAILED;
        error_.errorDetails.push_back(errorDetail2);
        operResult.SetDKError(error_);
        myMap.insert(std::make_pair(recordId, operResult));
        int32_t ret = fileDataHandler->OnModifyMdirtyRecords(myMap);
        EXPECT_EQ(E_STOP, ret);
        myMap.clear();
        error_.isLocalError = false;
        operResult.SetDKError(error_);
        myMap.insert(std::make_pair(recordId, operResult));
        ret = fileDataHandler->OnModifyMdirtyRecords(myMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyMdirtyRecords001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords001 End";
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;
        EXPECT_CALL(*rdb, Delete(_, _, _, _)).WillOnce(Return(0));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        DriveKit::DKError error_;
        error_.isLocalError = true;
        error_.serverErrorCode = static_cast<int32_t>(DriveKit::DKServerErrorCode::ATOMIC_ERROR);
        DriveKit::DKErrorDetail errorDetail2;
        errorDetail2.errorCode = AT_FAILED;
        error_.errorDetails.push_back(errorDetail2);
        operResult.SetDKError(error_);
        myMap.insert(std::make_pair(recordId, operResult));
        int32_t ret = fileDataHandler->OnDeleteRecords(myMap);
        EXPECT_EQ(E_STOP, ret);
        myMap.clear();
        error_.isLocalError = false;
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        DriveKit::DKError error_;
        error_.isLocalError = true;
        error_.serverErrorCode = static_cast<int32_t>(DriveKit::DKServerErrorCode::ATOMIC_ERROR);
        DriveKit::DKErrorDetail errorDetail2;
        errorDetail2.errorCode = AT_FAILED;
        error_.errorDetails.push_back(errorDetail2);
        operResult.SetDKError(error_);
        myMap.insert(std::make_pair(recordId, operResult));
        int32_t ret = fileDataHandler->OnCreateRecords(myMap);
        EXPECT_EQ(E_STOP, ret);
        myMap.clear();
        DriveKit::DKRecordFieldMap prop;
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        ret = fileDataHandler->OnCreateRecords(myMap);
        EXPECT_EQ(E_OK, ret);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;
        EXPECT_CALL(*rdb, Query(_, _))
            .WillOnce(Return(ByMove(nullptr)))
            .WillOnce(Return(ByMove(nullptr)));
        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(0));
        DriveKit::DKRecordData data;
        DriveKit::DKRecord record;
        DriveKit::DKRecordOperResult operResult;
        DriveKit::DKRecordId recordId = "hh";
        myMap.clear();
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("hh")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_PROPERTIES), prop));
        record.SetRecordData(data);
        operResult.SetDKRecord(std::move(record));
        auto ret = fileDataHandler->OnCreateRecords(myMap);
        EXPECT_EQ(E_OK, ret);
        myMap.clear();
        DriveKit::DKError error_;
        error_.isLocalError = false;
        operResult.SetDKError(error_);
        myMap.insert(std::make_pair(recordId, operResult));
        ret = fileDataHandler->OnCreateRecords(myMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateRecords002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateRecords002 End";
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        const shared_ptr<vector<DKRecord>> records = make_shared<vector<DKRecord>>();
        OnFetchParams onFetchParams;
        int32_t ret = fileDataHandler->OnFetchRecords(records, onFetchParams);
        EXPECT_EQ(E_OK, ret);
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
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        const shared_ptr<vector<DKRecord>> records = make_shared<vector<DKRecord>>();
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
        const int rowCount = 3;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));

        const shared_ptr<vector<DKRecord>> records = make_shared<vector<DKRecord>>();
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        DKRecord record;
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        DKRecord record;
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(1));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(0));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        int32_t ret = fileDataHandler->DeleteDentryFile();
        EXPECT_NE(E_OK, ret);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        string thmbDir = "/test/not/exits";
        string assetPath;
        string cloudId;
        int32_t ret = fileDataHandler->CleanNotDirtyData(thmbDir, assetPath, cloudId);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotDirtyData001 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotDirtyData001 End";
}

/**
 * @tc.name: CleanCloudRecord001
 * @tc.desc: Verify the CleanCloudRecord function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, CleanCloudRecord001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCloudRecord001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(0));

        int action = 0;
        string filePath;
        int32_t ret = fileDataHandler->CleanCloudRecord(*rset, action, filePath);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanCloudRecord001 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanCloudRecord001 End";
}

/**
 * @tc.name: CleanCloudRecord002
 * @tc.desc: Verify the CleanCloudRecord function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, CleanCloudRecord002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCloudRecord002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(1), Return(0)));

        int action = 0;
        string filePath;
        int32_t ret = fileDataHandler->CleanCloudRecord(*rset, action, filePath);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanCloudRecord002 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanCloudRecord002 End";
}

/**
 * @tc.name: Clean001
 * @tc.desc: Verify the Clean function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, Clean001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Clean001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        int action = 0;
        int32_t ret = fileDataHandler->Clean(action);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Clean001 ERROR";
    }

    GTEST_LOG_(INFO) << "Clean001 End";
}

/**
 * @tc.name: Clean002
 * @tc.desc: Verify the Clean function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(FileDataHandlerTest, Clean002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Clean002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        const int count = 1;
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(count), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(count)
            .WillRepeatedly(Return(0));

        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));

        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));

        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int action = 0;
        int32_t ret = fileDataHandler->Clean(action);
        EXPECT_NE(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Clean002 ERROR";
    }

    GTEST_LOG_(INFO) << "Clean002 End";
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        string filePath = "/test/pareantPath/thumbPpath";
        int action = 1;
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        int32_t ret = fileDataHandler->CleanPureCloudRecord(*rset, action, filePath);
        EXPECT_NE(E_OK, ret);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        string filePath = "/test/pareantPath/thumbPpath";
        int action = 1;
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(1));

        int32_t ret = fileDataHandler->CleanNotPureCloudRecord(*rset, action, filePath);
        EXPECT_EQ(E_RDB, ret);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        string filePath = "/test/pareantPath/thumbPpath";
        int action = 0;
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));

        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(1));

        int32_t ret = fileDataHandler->CleanNotPureCloudRecord(*rset, action, filePath);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        string filePath = "/test/pareantPath/thumbPpath";
        int action = 0;
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));

        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(0));

        int32_t ret = fileDataHandler->CleanNotPureCloudRecord(*rset, action, filePath);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        string filePath = "/test/pareantPath/thumbPpath";
        int action = 1;
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));

        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(1));

        int32_t ret = fileDataHandler->CleanNotPureCloudRecord(*rset, action, filePath);
        EXPECT_NE(E_OK, ret);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        string filePath = "/test/pareantPath/thumbPpath";
        int action = 1;
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));

        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(2), Return(0)));

        EXPECT_CALL(*rdb, Update(_, _, _, _, _)).WillOnce(Return(1));

        int32_t ret = fileDataHandler->CleanNotPureCloudRecord(*rset, action, filePath);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        std::vector<DKRecord> records;
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(rowCount)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        int32_t ret = fileDataHandler->GetCreatedRecords(records);
        EXPECT_EQ(E_OK, ret);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        std::vector<DKRecord> records;
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(rowCount)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        std::vector<DKRecord> records;
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        int32_t ret = fileDataHandler->GetMetaModifiedRecords(records);
        EXPECT_EQ(E_RDB, ret);
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
        const int rowCount = 3;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        std::vector<DKRecord> records;
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(rowCount)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        int32_t ret = fileDataHandler->GetMetaModifiedRecords(records);
        EXPECT_EQ(E_OK, ret);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        int changedRows;
        NativeRdb::ValuesBucket values;
        int32_t ret = fileDataHandler->Update(changedRows, values);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        int deletedRows;
        int32_t ret = fileDataHandler->Delete(deletedRows);
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
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
