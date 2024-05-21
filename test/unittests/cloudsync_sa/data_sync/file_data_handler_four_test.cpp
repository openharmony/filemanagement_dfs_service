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
        EXPECT_CALL(*rdb, Insert(_, _, _)).WillRepeatedly(Return(1));

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
        EXPECT_CALL(*rdb, Query(_, _)).WillRepeatedly(Return(nullptr));

        int32_t ret = fileDataHandler->CleanPureCloudRecord();
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanPureCloudRecord002 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanPureCloudRecord002 End";
}

/**
 * @tc.name: GetFilePathAndId001
 * @tc.desc: Verify the GetFilePathAndId function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetFilePathAndId001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathAndId001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(nullptr));
        NativeRdb::AbsRdbPredicates cleanPredicates = NativeRdb::AbsRdbPredicates("");
        vector<ValueObject> deleteFileId;
        shared_ptr<vector<string>> filePaths = make_shared<vector<string>>();
        int32_t ret = fileDataHandler->GetFilePathAndId(cleanPredicates, deleteFileId, *filePaths);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFilePathAndId001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetFilePathAndId001 End";
}

/**
 * @tc.name: GetFilePathAndId002
 * @tc.desc: Verify the GetFilePathAndId function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetFilePathAndId002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathAndId002 Begin";
    try {
        const int32_t rowCount = 0;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(1)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        NativeRdb::AbsRdbPredicates cleanPredicates = NativeRdb::AbsRdbPredicates("");
        vector<ValueObject> deleteFileId;
        shared_ptr<vector<string>> filePaths = make_shared<vector<string>>();
        int32_t ret = fileDataHandler->GetFilePathAndId(cleanPredicates, deleteFileId, *filePaths);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFilePathAndId00 ERROR";
    }

    GTEST_LOG_(INFO) << "GetFilePathAndId002 End";
}

/**
 * @tc.name: GetFilePathAndId003
 * @tc.desc: Verify the GetFilePathAndId function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetFilePathAndId003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathAndId003 Begin";
    try {
        const int32_t rowCount = -1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        NativeRdb::AbsRdbPredicates cleanPredicates = NativeRdb::AbsRdbPredicates("");
        vector<ValueObject> deleteFileId;
        shared_ptr<vector<string>> filePaths = make_shared<vector<string>>();
        int32_t ret = fileDataHandler->GetFilePathAndId(cleanPredicates, deleteFileId, *filePaths);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFilePathAndId003 ERROR";
    }

    GTEST_LOG_(INFO) << "GetFilePathAndId003 End";
}

/**
 * @tc.name: GetFilePathAndId004
 * @tc.desc: Verify the GetFilePathAndId function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetFilePathAndId004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathAndId004 Begin";
    try {
        const int32_t rowCount = 1;
        string filePath = "/test/pareantPath/thumbPpath";
        auto rdb = std::make_shared<RdbStoreMock>();
        auto rset = std::make_unique<AbsSharedResultSetMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(1)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        NativeRdb::AbsRdbPredicates cleanPredicates = NativeRdb::AbsRdbPredicates("");
        vector<ValueObject> deleteFileId;
        shared_ptr<vector<string>> filePaths = make_shared<vector<string>>();
        int32_t ret = fileDataHandler->GetFilePathAndId(cleanPredicates, deleteFileId, *filePaths);
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFilePathAndId004 ERROR";
    }

    GTEST_LOG_(INFO) << "GetFilePathAndId004 End";
}

/**
 * @tc.name: GetFilePathAndId005
 * @tc.desc: Verify the GetFilePathAndId function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetFilePathAndId005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathAndId005 Begin";
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
            .WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)))
            .WillOnce(DoAll(SetArgReferee<1>(filePath), Return(1)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        NativeRdb::AbsRdbPredicates cleanPredicates = NativeRdb::AbsRdbPredicates("");
        vector<ValueObject> deleteFileId;
        shared_ptr<vector<string>> filePaths = make_shared<vector<string>>();
        int32_t ret = fileDataHandler->GetFilePathAndId(cleanPredicates, deleteFileId, *filePaths);
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFilePathAndId005 ERROR";
    }

    GTEST_LOG_(INFO) << "GetFilePathAndId005 End";
}

/**
 * @tc.name: GetFilePathAndId006
 * @tc.desc: Verify the GetFilePathAndId function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetFilePathAndId006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathAndId006 Begin";
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
            .WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)))
            .WillOnce(DoAll(SetArgReferee<1>(filePath), Return(0)));
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        NativeRdb::AbsRdbPredicates cleanPredicates = NativeRdb::AbsRdbPredicates("");
        vector<ValueObject> deleteFileId;
        shared_ptr<vector<string>> filePaths = make_shared<vector<string>>();
        int32_t ret = fileDataHandler->GetFilePathAndId(cleanPredicates, deleteFileId, *filePaths);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFilePathAndId006 ERROR";
    }

    GTEST_LOG_(INFO) << "GetFilePathAndId006 End";
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
        EXPECT_CALL(*rdb, Query(_, _)).WillRepeatedly(Return(nullptr));

        int32_t ret = fileDataHandler->CleanNotDirtyData();
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanNotDirtyData002 ERROR";
    }

    GTEST_LOG_(INFO) << "CleanNotDirtyData002 End";
}

/**
 * @tc.name: OptimizeStorageTest
 * @tc.desc: Verify the OptimizeStorage function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, OptimizeStorageTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OptimizeStorage Begin";
    try {
        const int32_t agingDays = 30;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        int32_t ret = fileDataHandler-> OptimizeStorage(agingDays);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OptimizeStorage ERROR";
    }
    GTEST_LOG_(INFO) << "OptimizeStorage End";
}

/**
 * @tc.name: GetAgingFileTest001
 * @tc.desc: Verify the GetAgingFile function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetAgingFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAgingFile Begin";
    try {
        int64_t agingTime = 360;
        int32_t rowCount = 0;

        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        auto results = fileDataHandler->GetAgingFile(agingTime, rowCount);
        EXPECT_TRUE(results == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAgingFile ERROR";
    }
    GTEST_LOG_(INFO) << "GetAgingFile End";
}

/**
 * @tc.name: GetAgingFileTest002
 * @tc.desc: Verify the GetAgingFile function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetAgingFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAgingFile Begin";
    try {
        int rowCount = 1;
        int64_t agingTime = 360;

        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        auto results = fileDataHandler->GetAgingFile(agingTime, rowCount);
        EXPECT_TRUE(results != nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAgingFile ERROR";
    }
    GTEST_LOG_(INFO) << "GetAgingFile End";
}

/**
 * @tc.name: GetAgingFileTest003
 * @tc.desc: Verify the GetAgingFile function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetAgingFileTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAgingFile Begin";
    try {
        int rowCount = 1;
        int64_t agingTime = 360;

        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        auto results = fileDataHandler->GetAgingFile(agingTime, rowCount);
        EXPECT_TRUE(results == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAgingFile ERROR";
    }
    GTEST_LOG_(INFO) << "GetAgingFile End";
}

/**
 * @tc.name: UpdateAgingFileTest001
 * @tc.desc: Verify the UpdateAgingFile function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, UpdateAgingFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAgingFile Begin";
    try {
        string cloudId = "";

        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));
        int32_t ret = fileDataHandler->UpdateAgingFile(cloudId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateAgingFile ERROR";
    }
    GTEST_LOG_(INFO) << "UpdateAgingFile End";
}

/**
 * @tc.name: UpdateAgingFileTest002
 * @tc.desc: Verify the UpdateAgingFile function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, UpdateAgingFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateAgingFile Begin";
    try {
        string cloudId = "";

        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(1));;
        int32_t ret = fileDataHandler->UpdateAgingFile(cloudId);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateAgingFile ERROR";
    }
    GTEST_LOG_(INFO) << "UpdateAgingFile End";
}

/**
 * @tc.name: FileAgingDeleteTest001
 * @tc.desc: Verify the FileAgingDelete function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, FileAgingDeleteTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileAgingDelete Begin";
    try {
        int64_t agingTime = 360;
        int64_t deleteSize = 1000;

        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        int32_t ret = fileDataHandler->FileAgingDelete(agingTime, deleteSize);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FileAgingDelete ERROR";
    }
    GTEST_LOG_(INFO) << "FileAgingDelete End";
}

/**
 * @tc.name: FileAgingDeleteTest002
 * @tc.desc: Verify the FileAgingDelete function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, FileAgingDeleteTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileAgingDelete Begin";
    try {
        const int32_t rowCount = 1;
        int64_t agingTime = 360;
        int64_t deleteSize = 1000;

        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        int32_t ret = fileDataHandler->FileAgingDelete(agingTime, deleteSize);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FileAgingDelete ERROR";
    }
    GTEST_LOG_(INFO) << "FileAgingDelete End";
}

/**
 * @tc.name: FileAgingDeleteTest003
 * @tc.desc: Verify the FileAgingDelete function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, FileAgingDeleteTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileAgingDelete Begin";
    try {
        const int32_t rowCount = 3;
        int64_t agingTime = 360;
        int64_t deleteSize = 1000;

        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(rowCount)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        int32_t ret = fileDataHandler->FileAgingDelete(agingTime, deleteSize);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FileAgingDelete ERROR";
    }
    GTEST_LOG_(INFO) << "FileAgingDelete End";
}

/**
 * @tc.name: FileAgingDeleteTest004
 * @tc.desc: Verify the FileAgingDelete function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, FileAgingDeleteTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileAgingDelete Begin";
    try {
        const int32_t rowCount = 3;
        int64_t agingTime = 360;
        int64_t deleteSize = 1000;

        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow())
            .Times(rowCount)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        int32_t ret = fileDataHandler->FileAgingDelete(agingTime, deleteSize);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FileAgingDelete ERROR";
    }
    GTEST_LOG_(INFO) << "FileAgingDelete End";
}

/**
 * @tc.name: GetThumbToDownloadTest001
 * @tc.desc: Verify the GetThumbToDownload function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetThumbToDownloadTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetThumbToDownload Begin";
    try {
        std::vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));
        int32_t ret = fileDataHandler->GetThumbToDownload(outAssetsToDownload);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetThumbToDownload ERROR";
    }
    GTEST_LOG_(INFO) << "GetThumbToDownload End";
}

/**
 * @tc.name: GetThumbToDownloadTest002
 * @tc.desc: Verify the GetThumbToDownload function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetThumbToDownloadTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetThumbToDownload Begin";
    try {
        std::vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        int32_t ret = fileDataHandler->GetThumbToDownload(outAssetsToDownload);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetThumbToDownload ERROR";
    }
    GTEST_LOG_(INFO) << "GetThumbToDownload End";
}

/**
 * @tc.name: GetThumbToDownloadTest003
 * @tc.desc: Verify the GetThumbToDownload function
 * @tc.type: FUNC
 * @tc.require: issuesI8R3H3
 */
HWTEST_F(FileDataHandlerTest, GetThumbToDownloadTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetThumbToDownload Begin";
    try {
        const int32_t rowCount = 3;
        std::vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GoToNextRow())
            .Times(rowCount)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        int32_t ret = fileDataHandler->GetThumbToDownload(outAssetsToDownload);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetThumbToDownload ERROR";
    }
    GTEST_LOG_(INFO) << "GetThumbToDownload End";
}

/**
 * @tc.name: BatchInsertAssetAnalysisMaps001
 * @tc.desc: Verify the BatchInsertAssetAnalysisMaps001 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, BatchInsertAssetAnalysisMaps001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchInsertAssetAnalysisMaps001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(nullptr));

        OnFetchParams params;
        int32_t ret = fileDataHandler->BatchInsertAssetAnalysisMaps(params);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchInsertAssetAnalysisMaps001 ERROR";
    }
    GTEST_LOG_(INFO) << "BatchInsertAssetAnalysisMaps001 End";
}

/**
 * @tc.name: BatchInsertAssetAnalysisMaps002
 * @tc.desc: Verify the BatchInsertAssetAnalysisMaps002 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, BatchInsertAssetAnalysisMaps002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchInsertAssetAnalysisMaps002 Begin";
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
        int32_t ret = fileDataHandler->BatchInsertAssetAnalysisMaps(params);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchInsertAssetAnalysisMaps002 ERROR";
    }
    GTEST_LOG_(INFO) << "BatchInsertAssetAnalysisMaps002 End";
}

/**
 * @tc.name: BatchInsertAssetAnalysisMaps003
 * @tc.desc: Verify the BatchInsertAssetAnalysisMaps003 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, BatchInsertAssetAnalysisMaps003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchInsertAssetAnalysisMaps003 Begin";
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

        int albumIds = 1;
        OnFetchParams params;
        params.recordAnalysisAlbumMaps.insert(make_pair("1", std::move(albumIds)));
        int32_t ret = fileDataHandler->BatchInsertAssetAnalysisMaps(params);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchInsertAssetAnalysisMaps003 ERROR";
    }
    GTEST_LOG_(INFO) << "BatchInsertAssetAnalysisMaps003 End";
}

/**
 * @tc.name: BatchInsertAssetAnalysisMaps004
 * @tc.desc: Verify the BatchInsertAssetAnalysisMaps004 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, BatchInsertAssetAnalysisMaps004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchInsertAssetAnalysisMaps004 Begin";
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
        EXPECT_CALL(*rdb, Insert(_, _, _)).WillRepeatedly(Return(1));

        int albumIds = 1;
        OnFetchParams params;
        params.recordAnalysisAlbumMaps.insert(make_pair("1", std::move(albumIds)));
        int32_t ret = fileDataHandler->BatchInsertAssetAnalysisMaps(params);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchInsertAssetAnalysisMaps004 ERROR";
    }
    GTEST_LOG_(INFO) << "BatchInsertAssetAnalysisMaps004 End";
}

/**
 * @tc.name: BatchInsertAssetAnalysisMaps005
 * @tc.desc: Verify the BatchInsertAssetAnalysisMaps005 function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, BatchInsertAssetAnalysisMaps005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchInsertAssetAnalysisMaps005 Begin";
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

        int albumIds = 1;
        OnFetchParams params;
        params.recordAnalysisAlbumMaps.insert(make_pair("1", std::move(albumIds)));
        int32_t ret = fileDataHandler->BatchInsertAssetAnalysisMaps(params);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchInsertAssetAnalysisMaps005 ERROR";
    }
    GTEST_LOG_(INFO) << "BatchInsertAssetAnalysisMaps005 End";
}

/**
 * @tc.name: HandleShootingMode001
 * @tc.desc: Verify the HandleShootingMode function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, HandleShootingMode001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleShootingMode001 Begin";
    ValuesBucket valuebucket;
    DKRecord record;
    OnFetchParams params;
    auto rdb = std::make_shared<RdbStoreMock>();
    auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
    fileDataHandler->HandleShootingMode(record, valuebucket, params);
    EXPECT_EQ(params.recordAnalysisAlbumMaps.size(), 0);
    GTEST_LOG_(INFO) << "HandleShootingMode001 End";
}

/**
 * @tc.name: HandleShootingMode002
 * @tc.desc: Verify the HandleShootingMode function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, HandleShootingMode002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleShootingMode002 Begin";
    ValuesBucket valuebucket;
    valuebucket.PutString(Media::PhotoColumn::PHOTO_SHOOTING_MODE, "");
    DKRecord record;
    OnFetchParams params;
    auto rdb = std::make_shared<RdbStoreMock>();
    auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
    fileDataHandler->HandleShootingMode(record, valuebucket, params);
    EXPECT_EQ(params.recordAnalysisAlbumMaps.size(), 0);
    GTEST_LOG_(INFO) << "HandleShootingMode002 End";
}

/**
 * @tc.name: HandleShootingMode003
 * @tc.desc: Verify the HandleShootingMode function
 * @tc.type: FUNC
 * @tc.require: issueI8ARMY
 */
HWTEST_F(FileDataHandlerTest, HandleShootingMode003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleShootingMode003 Begin";
    ValuesBucket valuebucket;
    valuebucket.PutString(Media::PhotoColumn::PHOTO_SHOOTING_MODE, "1");
    DKRecord record;
    record.SetRecordId("10");
    OnFetchParams params;
    auto rdb = std::make_shared<RdbStoreMock>();
    auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
    fileDataHandler->HandleShootingMode(record, valuebucket, params);
    EXPECT_EQ(params.recordAnalysisAlbumMaps.size(), 1);
    GTEST_LOG_(INFO) << "HandleShootingMode003 End";
}