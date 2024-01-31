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

 /**
 * @tc.name: QueryAndDeleteMap001
 * @tc.desc: Verify the QueryAndDeleteMap001 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */

#include <memory>
HWTEST_F(FileDataHandlerTest, QueryAndDeleteMap001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAndDeleteMap001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(nullptr));

        int32_t fileId = 0;
        const set<int> cloudMapIds;
        fileDataHandler->QueryAndDeleteMap(fileId, cloudMapIds);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryAndDeleteMap001 ERROR";
    }
    GTEST_LOG_(INFO) << "QueryAndDeleteMap001 End";
}

/**
 * @tc.name: QueryAndDeleteMap002
 * @tc.desc: Verify the QueryAndDeleteMap002 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, QueryAndDeleteMap002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAndDeleteMap002 Begin";
    try {
        const int32_t rowCount = -1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(1)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t fileId = 0;
        const set<int> cloudMapIds;
        fileDataHandler->QueryAndDeleteMap(fileId, cloudMapIds);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryAndDeleteMap002 ERROR";
    }
    GTEST_LOG_(INFO) << "QueryAndDeleteMap002 End";
}

/**
 * @tc.name: QueryAndDeleteMap003
 * @tc.desc: Verify the QueryAndDeleteMap003 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, QueryAndDeleteMap003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAndDeleteMap003 Begin";
    try {
        const int32_t rowCount = -1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t fileId = 0;
        const set<int> cloudMapIds;
        fileDataHandler->QueryAndDeleteMap(fileId, cloudMapIds);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryAndDeleteMap003 ERROR";
    }
    GTEST_LOG_(INFO) << "QueryAndDeleteMap003 End";
}

/**
 * @tc.name: QueryAndDeleteMap004
 * @tc.desc: Verify the QueryAndDeleteMap004 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, QueryAndDeleteMap004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAndDeleteMap004 Begin";
    try {
        const int32_t rowCount = 0;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t fileId = 0;
        const set<int> cloudMapIds;
        fileDataHandler->QueryAndDeleteMap(fileId, cloudMapIds);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryAndDeleteMap004 ERROR";
    }
    GTEST_LOG_(INFO) << "QueryAndDeleteMap004 End";
}

/**
 * @tc.name: QueryAndDeleteMap005
 * @tc.desc: Verify the QueryAndDeleteMap005 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, QueryAndDeleteMap005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAndDeleteMap005 Begin";
    try {
        const int32_t rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t fileId = 0;
        const set<int> cloudMapIds;
        fileDataHandler->QueryAndDeleteMap(fileId, cloudMapIds);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryAndDeleteMap005 ERROR";
    }
    GTEST_LOG_(INFO) << "QueryAndDeleteMap005 End";
}

/**
 * @tc.name: QueryAndDeleteMap006
 * @tc.desc: Verify the QueryAndDeleteMap006 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, QueryAndDeleteMap006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAndDeleteMap006 Begin";
    try {
        const int32_t rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t fileId = 0;
        const set<int> cloudMapIds;
        fileDataHandler->QueryAndDeleteMap(fileId, cloudMapIds);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryAndDeleteMap006 ERROR";
    }
    GTEST_LOG_(INFO) << "QueryAndDeleteMap006 End";
}

/**
 * @tc.name: QueryAndDeleteMap007
 * @tc.desc: Verify the QueryAndDeleteMap007 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, QueryAndDeleteMap007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAndDeleteMap007 Begin";
    try {
        const int32_t rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t fileId = 0;
        const set<int> cloudMapIds;
        fileDataHandler->QueryAndDeleteMap(fileId, cloudMapIds);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryAndDeleteMap007 ERROR";
    }
    GTEST_LOG_(INFO) << "QueryAndDeleteMap007 End";
}

/**
 * @tc.name: QueryAndDeleteMap008
 * @tc.desc: Verify the QueryAndDeleteMap008 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, QueryAndDeleteMap008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAndDeleteMap008 Begin";
    try {
        const int32_t rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillOnce(Return(0)).WillOnce(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, Delete(_, _, _, A<const vector<string> &>())).WillOnce(Return(0));

        int32_t fileId = 0;
        const set<int> cloudMapIds;
        fileDataHandler->QueryAndDeleteMap(fileId, cloudMapIds);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryAndDeleteMap008 ERROR";
    }
    GTEST_LOG_(INFO) << "QueryAndDeleteMap008 End";
}

/**
 * @tc.name: QueryAndDeleteMap009
 * @tc.desc: Verify the QueryAndDeleteMap009 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, QueryAndDeleteMap009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAndDeleteMap009 Begin";
    try {
        const int32_t rowCount = 1;
        const int32_t localDirty = static_cast<int32_t>(Media::DirtyTypes::TYPE_SYNCED);
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _))
            .WillOnce(Return(0))
            .WillOnce(DoAll(SetArgReferee<1>(localDirty), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, Delete(_, _, _, A<const vector<string> &>())).WillOnce(Return(0));

        int32_t fileId = 0;
        const set<int> cloudMapIds;
        fileDataHandler->QueryAndDeleteMap(fileId, cloudMapIds);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryAndDeleteMap009 ERROR";
    }
    GTEST_LOG_(INFO) << "QueryAndDeleteMap009 End";
}

/**
 * @tc.name: QueryAndDeleteMap010
 * @tc.desc: Verify the QueryAndDeleteMap010 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, QueryAndDeleteMap010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAndDeleteMap010 Begin";
    try {
        const int32_t rowCount = 1;
        const int32_t localDirty = static_cast<int32_t>(Media::DirtyTypes::TYPE_SYNCED);
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(1), Return(0)))
            .WillOnce(DoAll(SetArgReferee<1>(localDirty), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, Delete(_, _, _, A<const vector<string> &>())).WillRepeatedly(Return(-1));

        int32_t fileId = 0;
        std::set<int> cloudMapIds;
        cloudMapIds.insert(1);
        fileDataHandler->QueryAndDeleteMap(fileId, cloudMapIds);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryAndDeleteMap010 ERROR";
    }
    GTEST_LOG_(INFO) << "QueryAndDeleteMap010 End";
}

/**
 * @tc.name: QueryAndDeleteMap011
 * @tc.desc: Verify the QueryAndDeleteMap011 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, QueryAndDeleteMap011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryAndDeleteMap011 Begin";
    try {
        const int32_t rowCount = 1;
        const int32_t localDirty = static_cast<int32_t>(Media::DirtyTypes::TYPE_SYNCED);
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(1), Return(0)))
            .WillOnce(DoAll(SetArgReferee<1>(localDirty), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, Delete(_, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        int32_t fileId = 0;
        std::set<int> cloudMapIds;
        cloudMapIds.insert(1);
        fileDataHandler->QueryAndDeleteMap(fileId, cloudMapIds);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " QueryAndDeleteMap011 ERROR";
    }
    GTEST_LOG_(INFO) << "QueryAndDeleteMap011 End";
}

/**
 * @tc.name: BatchInsertAssetMaps001
 * @tc.desc: Verify the BatchInsertAssetMaps001 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, BatchInsertAssetMaps001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchInsertAssetMaps001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(nullptr));

        OnFetchParams params;
        int32_t ret = fileDataHandler->BatchInsertAssetMaps(params);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchInsertAssetMaps001 ERROR";
    }
    GTEST_LOG_(INFO) << "BatchInsertAssetMaps001 End";
}

/**
 * @tc.name: BatchInsertAssetMaps002
 * @tc.desc: Verify the BatchInsertAssetMaps002 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, BatchInsertAssetMaps002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchInsertAssetMaps002 Begin";
    try {
        const int32_t rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        OnFetchParams params;
        int32_t ret = fileDataHandler->BatchInsertAssetMaps(params);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchInsertAssetMaps002 ERROR";
    }
    GTEST_LOG_(INFO) << "BatchInsertAssetMaps002 End";
}

/**
 * @tc.name: BatchInsertAssetMaps003
 * @tc.desc: Verify the BatchInsertAssetMaps003 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, BatchInsertAssetMaps003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchInsertAssetMaps003 Begin";
    try {
        const int32_t rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)));
        EXPECT_CALL(*rset, GoToRow(_)).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        std::set<int> albumIds;
        OnFetchParams params;
        params.recordAlbumMaps.insert(make_pair("1", std::move(albumIds)));
        int32_t ret = fileDataHandler->BatchInsertAssetMaps(params);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchInsertAssetMaps003 ERROR";
    }
    GTEST_LOG_(INFO) << "BatchInsertAssetMaps003 End";
}

/**
 * @tc.name: BatchInsertAssetMaps004
 * @tc.desc: Verify the BatchInsertAssetMaps004 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, BatchInsertAssetMaps004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchInsertAssetMaps004 Begin";
    try {
        const int32_t rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)));
        EXPECT_CALL(*rset, GoToRow(_)).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, Insert(_, _, _)).WillOnce(Return(1));

        std::set<int> albumIds;
        albumIds.insert(1);
        OnFetchParams params;
        params.recordAlbumMaps.insert(make_pair("1", std::move(albumIds)));
        int32_t ret = fileDataHandler->BatchInsertAssetMaps(params);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchInsertAssetMaps004 ERROR";
    }
    GTEST_LOG_(INFO) << "BatchInsertAssetMaps004 End";
}

/**
 * @tc.name: BatchInsertAssetMaps005
 * @tc.desc: Verify the BatchInsertAssetMaps005 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, BatchInsertAssetMaps005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchInsertAssetMaps005 Begin";
    try {
        const int32_t rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)));
        EXPECT_CALL(*rset, GoToRow(_)).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, Insert(_, _, _)).WillOnce(Return(0));

        std::set<int> albumIds;
        albumIds.insert(1);
        OnFetchParams params;
        params.recordAlbumMaps.insert(make_pair("1", std::move(albumIds)));
        int32_t ret = fileDataHandler->BatchInsertAssetMaps(params);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchInsertAssetMaps005 ERROR";
    }
    GTEST_LOG_(INFO) << "BatchInsertAssetMaps005 End";
}

/**
 * @tc.name: CleanPureCloudRecord002
 * @tc.desc: Verify the CleanPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanPureCloudRecord002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanPureCloudRecord002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(nullptr));

        int32_t ret = fileDataHandler->CleanPureCloudRecord();
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanPureCloudRecord002 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanPureCloudRecord002 End";
}

/**
 * @tc.name: CleanPureCloudRecord003
 * @tc.desc: Verify the CleanPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanPureCloudRecord003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanPureCloudRecord003 Begin";
    try {
        const int32_t rowCount = 0;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(1)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t ret = fileDataHandler->CleanPureCloudRecord();
        EXPECT_EQ(1, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanPureCloudRecord003 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanPureCloudRecord003 End";
}

/**
 * @tc.name: CleanPureCloudRecord004
 * @tc.desc: Verify the CleanPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanPureCloudRecord004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanPureCloudRecord004 Begin";
    try {
        const int32_t rowCount = -1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t ret = fileDataHandler->CleanPureCloudRecord();
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanPureCloudRecord004 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanPureCloudRecord004 End";
}

/**
 * @tc.name: CleanPureCloudRecord005
 * @tc.desc: Verify the CleanPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanPureCloudRecord005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanPureCloudRecord005 Begin";
    try {
        const int32_t rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset)))).WillOnce(Return(nullptr));

        int32_t ret = fileDataHandler->CleanPureCloudRecord();
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanPureCloudRecord005 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanPureCloudRecord005 End";
}

/**
 * @tc.name: CleanPureCloudRecord006
 * @tc.desc: Verify the CleanPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanPureCloudRecord006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanPureCloudRecord006 Begin";
    try {
        const int32_t rowCount = 1;
        string filePath = "/test/pareantPath/thumbPpath";
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _))
            .WillOnce(DoAll(SetArgReferee<1>("1"), Return(1)))
            .WillOnce(DoAll(SetArgReferee<1>(filePath), Return(1)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t ret = fileDataHandler->CleanPureCloudRecord();
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanPureCloudRecord006 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanPureCloudRecord006 End";
}

/**
 * @tc.name: CleanPureCloudRecord007
 * @tc.desc: Verify the CleanPureCloudRecord function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanPureCloudRecord007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanPureCloudRecord007 Begin";
    try {
        const int32_t rowCount = 0;
        string filePath = "/test/pareantPath/thumbPpath";
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _))
            .WillOnce(DoAll(SetArgReferee<1>("1"), Return(1)))
            .WillOnce(DoAll(SetArgReferee<1>(filePath), Return(0)));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t ret = fileDataHandler->CleanPureCloudRecord();
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanPureCloudRecord007 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanPureCloudRecord007 End";
}

/**
 * @tc.name: CleanNotDirtyData002
 * @tc.desc: Verify the CleanNotDirtyData function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanNotDirtyData002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotDirtyData002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(nullptr));

        int32_t ret = fileDataHandler->CleanNotDirtyData();
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotDirtyData002 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotDirtyData002 End";
}

/**
 * @tc.name: CleanNotDirtyData003
 * @tc.desc: Verify the CleanNotDirtyData function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanNotDirtyData003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotDirtyData003 Begin";
    try {
        const int32_t rowCount = 0;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(1)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t ret = fileDataHandler->CleanNotDirtyData();
        EXPECT_EQ(1, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotDirtyData003 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotDirtyData003 End";
}

/**
 * @tc.name: CleanNotDirtyData004
 * @tc.desc: Verify the CleanNotDirtyData function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanNotDirtyData004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotDirtyData004 Begin";
    try {
        const int32_t rowCount = -1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t ret = fileDataHandler->CleanNotDirtyData();
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotDirtyData004 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotDirtyData004 End";
}

/**
 * @tc.name: CleanNotDirtyData005
 * @tc.desc: Verify the CleanNotDirtyData function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanNotDirtyData005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotDirtyData005 Begin";
    try {
        const int32_t rowCount = 0;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t ret = fileDataHandler->CleanNotDirtyData();
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotDirtyData005 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotDirtyData005 End";
}

/**
 * @tc.name: CleanNotDirtyData006
 * @tc.desc: Verify the CleanNotDirtyData function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanNotDirtyData006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotDirtyData006 Begin";
    try {
        const int32_t rowCount = 0;
        string filePath = "/test/pareantPath/thumbPpath";
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _))
            .WillOnce(DoAll(SetArgReferee<1>("1"), Return(1)))
            .WillOnce(DoAll(SetArgReferee<1>(filePath), Return(1)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t ret = fileDataHandler->CleanNotDirtyData();
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotDirtyData006 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotDirtyData006 End";
}

/**
 * @tc.name: CleanNotDirtyData007
 * @tc.desc: Verify the CleanNotDirtyData function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, CleanNotDirtyData007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanNotDirtyData007 Begin";
    try {
        const int32_t rowCount = 0;
        string filePath = "/test/pareantPath/thumbPpath";
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _))
            .WillOnce(DoAll(SetArgReferee<1>("1"), Return(1)))
            .WillOnce(DoAll(SetArgReferee<1>(filePath), Return(0)));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        int32_t ret = fileDataHandler->CleanNotDirtyData();
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotDirtyData007 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotDirtyData007 End";
}
