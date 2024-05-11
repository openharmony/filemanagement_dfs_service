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

#include "album_data_handler.h"
#include "dfs_error.h"
#include "gallery_file_const.h"
#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"
#include "rdb_store_mock.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;
using namespace DriveKit;
const int USER_ID = 100;
const std::string BUND_NAME = "com.ohos.photos";
class AlbumDataHandlerMock : public AlbumDataHandler {
public:
    explicit AlbumDataHandlerMock(int32_t userId, const std::string &bundleName, std::shared_ptr<RdbStoreMock> rdb)
        : AlbumDataHandler(userId, bundleName, rdb, make_shared<bool>(false))
    {
    }
    MOCK_METHOD3(GetDownloadAsset, int32_t(string cloudId,
                                           vector<DriveKit::DKDownloadAsset> &outAssetsToDownload,
                                           std::shared_ptr<DentryContext> dentryContext));
    MOCK_METHOD2(OnDownloadSuccess, int32_t(const DriveKit::DKDownloadAsset &asset,
                                            std::shared_ptr<DriveKit::DKContext> context));
    MOCK_METHOD1(OnDownloadAssets,
                 int32_t(const std::map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> &resultMap));
    MOCK_METHOD1(OnDownloadAssets, int32_t(const DKDownloadAsset &asset));
    MOCK_METHOD1(GetAssetsToDownload, int32_t(std::vector<DriveKit::DKDownloadAsset> &outAssetsToDownload));
    MOCK_METHOD1(Clean, int32_t(const int action));
};
class AlbumDataHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AlbumDataHandlerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void AlbumDataHandlerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void AlbumDataHandlerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AlbumDataHandlerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: OnFetchRecords001
 * @tc.desc: Verify the OnFetchRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(AlbumDataHandlerTest, OnFetchRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchRecords001 Begin";
    try {
        auto rdb = make_shared<RdbStoreMock>();
        auto albumDataHandlerMock = make_shared<AlbumDataHandlerMock>(USER_ID, BUND_NAME, rdb);
        shared_ptr<vector<DKRecord>> records = make_shared<vector<DKRecord>>();
        OnFetchParams onFetchParams;
        int res = albumDataHandlerMock->OnFetchRecords(records, onFetchParams);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnFetchRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "OnFetchRecords001 End";
}

/**
 * @tc.name: GetFetchCondition001
 * @tc.desc: Verify the GetFetchCondition function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(AlbumDataHandlerTest, GetFetchCondition001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFetchCondition001 Begin";
    try {
        FetchCondition cond;
        auto rdb = make_shared<RdbStoreMock>();
        auto albumDataHandlerMock = make_shared<AlbumDataHandlerMock>(USER_ID, BUND_NAME, rdb);
        albumDataHandlerMock->GetFetchCondition(cond);
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
HWTEST_F(AlbumDataHandlerTest, GetRetryRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRetryRecords001 Begin";
    try {
        auto rdb = make_shared<RdbStoreMock>();
        auto albumDataHandlerMock = make_shared<AlbumDataHandlerMock>(USER_ID, BUND_NAME, rdb);
        vector<DriveKit::DKRecordId> records;
        int32_t ret = albumDataHandlerMock->GetRetryRecords(records);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetRetryRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetRetryRecords001 End";
}

/**
 * @tc.name: GetCreatedRecords001
 * @tc.desc: Verify the GetCreatedRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(AlbumDataHandlerTest, GetCreatedRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreatedRecords001 Begin";
    try {
        auto rdb = make_shared<RdbStoreMock>();
        auto albumDataHandlerMock = make_shared<AlbumDataHandlerMock>(USER_ID, BUND_NAME, rdb);
        vector<DriveKit::DKRecord> records;
        int32_t ret = albumDataHandlerMock->GetCreatedRecords(records);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetCreatedRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetCreatedRecords001 End";
}

/**
 * @tc.name: GetDeletedRecords001
 * @tc.desc: Verify the GetDeletedRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(AlbumDataHandlerTest, GetDeletedRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeletedRecords001 Begin";
    try {
        auto rdb = make_shared<RdbStoreMock>();
        auto albumDataHandlerMock = make_shared<AlbumDataHandlerMock>(USER_ID, BUND_NAME, rdb);
        vector<DriveKit::DKRecord> records;
        int32_t ret = albumDataHandlerMock->GetDeletedRecords(records);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetDeletedRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetDeletedRecords001 End";
}

/**
 * @tc.name: GetMetaModifiedRecords001
 * @tc.desc: Verify the GetMetaModifiedRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(AlbumDataHandlerTest, GetMetaModifiedRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMetaModifiedRecords001 Begin";
    try {
        auto rdb = make_shared<RdbStoreMock>();
        auto albumDataHandlerMock = make_shared<AlbumDataHandlerMock>(USER_ID, BUND_NAME, rdb);
        vector<DriveKit::DKRecord> records;
        int32_t ret = albumDataHandlerMock->GetMetaModifiedRecords(records);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetMetaModifiedRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetMetaModifiedRecords001 End";
}

/**
 * @tc.name: OnCreateRecords001
 * @tc.desc: Verify the OnCreateRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(AlbumDataHandlerTest, OnCreateRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecords001 Begin";
    try {
        auto rdb = make_shared<RdbStoreMock>();
        auto albumDataHandlerMock = make_shared<AlbumDataHandlerMock>(USER_ID, BUND_NAME, rdb);
        map<DKRecordId, DKRecordOperResult> map;
        int32_t ret = albumDataHandlerMock->OnCreateRecords(map);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnCreateRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "OnCreateRecords001 End";
}

/**
 * @tc.name: OnDeleteRecords001
 * @tc.desc: Verify the OnDeleteRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(AlbumDataHandlerTest, OnDeleteRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDeleteRecords001 Begin";
    try {
        auto rdb = make_shared<RdbStoreMock>();
        auto albumDataHandlerMock = make_shared<AlbumDataHandlerMock>(USER_ID, BUND_NAME, rdb);
        map<DKRecordId, DKRecordOperResult> map;
        int32_t ret = albumDataHandlerMock->OnDeleteRecords(map);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnDeleteRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "OnDeleteRecords001 End";
}

/**
 * @tc.name: OnModifyMdirtyRecords001
 * @tc.desc: Verify the OnModifyMdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(AlbumDataHandlerTest, OnModifyMdirtyRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords001 Begin";
    try {
        auto rdb = make_shared<RdbStoreMock>();
        auto albumDataHandlerMock = make_shared<AlbumDataHandlerMock>(USER_ID, BUND_NAME, rdb);
        map<DKRecordId, DKRecordOperResult> map;
        int32_t ret = albumDataHandlerMock->OnModifyMdirtyRecords(map);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyMdirtyRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords001 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test