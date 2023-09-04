/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd.
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
 * @tc.name: UpdateLocalAlbumMap001
 * @tc.desc: Verify the UpdateLocalAlbumMap function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, UpdateLocalAlbumMap001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateLocalAlbumMap001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(0));

        string cloudId = "";
        int32_t ret = fileDataHandler->UpdateLocalAlbumMap(cloudId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " UpdateLocalAlbumMap001 ERROR";
    }
    GTEST_LOG_(INFO) << "UpdateLocalAlbumMap001 End";
}

/**
 * @tc.name: UpdateLocalAlbumMap002
 * @tc.desc: Verify the UpdateLocalAlbumMap function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, UpdateLocalAlbumMap002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateLocalAlbumMap002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillRepeatedly(Return(1));

        string cloudId = "";
        int32_t ret = fileDataHandler->UpdateLocalAlbumMap(cloudId);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " UpdateLocalAlbumMap002 ERROR";
    }
    GTEST_LOG_(INFO) << "UpdateLocalAlbumMap002 End";
}

/**
 * @tc.name: UpdateLocalAlbumMap003
 * @tc.desc: Verify the UpdateLocalAlbumMap function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, UpdateLocalAlbumMap003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateLocalAlbumMap003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillOnce(Return(0)).WillRepeatedly(Return(1));

        string cloudId = "";
        int32_t ret = fileDataHandler->UpdateLocalAlbumMap(cloudId);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " UpdateLocalAlbumMap003 ERROR";
    }
    GTEST_LOG_(INFO) << "UpdateLocalAlbumMap003 End";
}

/**
 * @tc.name: GetAlbumCloudIds001
 * @tc.desc: Verify the GetAlbumCloudIds function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumCloudIds001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumCloudIds001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        vector<int32_t> localIds;
        vector<string> cloudIds;
        localIds.push_back(1);
        int32_t ret = fileDataHandler->GetAlbumCloudIds(localIds, cloudIds);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumCloudIds001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumCloudIds001 End";
}

/**
 * @tc.name: GetAlbumCloudIds002
 * @tc.desc: Verify the GetAlbumCloudIds function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumCloudIds002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumCloudIds002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));

        vector<int32_t> localIds;
        vector<string> cloudIds;
        localIds.push_back(1);
        int32_t ret = fileDataHandler->GetAlbumCloudIds(localIds, cloudIds);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumCloudIds002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumCloudIds002 End";
}

/**
 * @tc.name: GetAlbumCloudIds003
 * @tc.desc: Verify the GetAlbumCloudIds function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumCloudIds003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumCloudIds003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        vector<int32_t> localIds;
        vector<string> cloudIds;
        localIds.push_back(1);
        int32_t ret = fileDataHandler->GetAlbumCloudIds(localIds, cloudIds);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumCloudIds003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumCloudIds003 End";
}

/**
 * @tc.name: GetAlbumCloudIds004
 * @tc.desc: Verify the GetAlbumCloudIds function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumCloudIds004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumCloudIds004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        vector<int32_t> localIds;
        vector<string> cloudIds;
        localIds.push_back(1);
        int32_t ret = fileDataHandler->GetAlbumCloudIds(localIds, cloudIds);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumCloudIds004 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumCloudIds004 End";
}

/**
 * @tc.name: GetAlbumCloudIds005
 * @tc.desc: Verify the GetAlbumCloudIds function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumCloudIds005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumCloudIds005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        vector<int32_t> localIds;
        vector<string> cloudIds;
        localIds.push_back(1);
        int32_t ret = fileDataHandler->GetAlbumCloudIds(localIds, cloudIds);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumCloudIds005 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumCloudIds005 End";
}

/**
 * @tc.name: GetAlbumIdsFromResultSet001
 * @tc.desc: Verify the GetAlbumIdsFromResultSet function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumIdsFromResultSet001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::shared_ptr<ResultSetMock> rset = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillOnce(DoAll(SetArgReferee<1>(1), Return(0)));

        vector<int32_t> ids;
        int32_t ret = fileDataHandler->GetAlbumIdsFromResultSet(rset, ids);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumIdsFromResultSet001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet001 End";
}

/**
 * @tc.name: GetAlbumIdsFromResultSet002
 * @tc.desc: Verify the GetAlbumIdsFromResultSet function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumIdsFromResultSet002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::shared_ptr<ResultSetMock> rset = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillOnce(Return(1));

        vector<int32_t> ids;
        int32_t ret = fileDataHandler->GetAlbumIdsFromResultSet(rset, ids);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumIdsFromResultSet002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet002 End";
}

/**
 * @tc.name: GetAlbumIdsFromResultSet003
 * @tc.desc: Verify the GetAlbumIdsFromResultSet function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumIdsFromResultSet003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet003 Begin";
    try {
        int type = static_cast<int32_t>(Media::DirtyType::TYPE_NEW);
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::shared_ptr<ResultSetMock> rset = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(1), Return(0)))
            .WillOnce(DoAll(SetArgReferee<1>(type), Return(0)));

        std::vector<int32_t> add;
        std::vector<int32_t> rm;
        int32_t ret = fileDataHandler->GetAlbumIdsFromResultSet(rset, add, rm);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumIdsFromResultSet003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet003 End";
}

/**
 * @tc.name: GetAlbumIdsFromResultSet004
 * @tc.desc: Verify the GetAlbumIdsFromResultSet function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumIdsFromResultSet004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::shared_ptr<ResultSetMock> rset = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillOnce(Return(1));

        std::vector<int32_t> add;
        std::vector<int32_t> rm;
        int32_t ret = fileDataHandler->GetAlbumIdsFromResultSet(rset, add, rm);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumIdsFromResultSet004 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet004 End";
}

/**
 * @tc.name: GetAlbumIdsFromResultSet005
 * @tc.desc: Verify the GetAlbumIdsFromResultSet function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumIdsFromResultSet005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::shared_ptr<ResultSetMock> rset = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillOnce(Return(0)).WillOnce(Return(1));

        std::vector<int32_t> add;
        std::vector<int32_t> rm;
        int32_t ret = fileDataHandler->GetAlbumIdsFromResultSet(rset, add, rm);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumIdsFromResultSet005 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet005 End";
}

/**
 * @tc.name: GetAlbumIdsFromResultSet006
 * @tc.desc: Verify the GetAlbumIdsFromResultSet function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumIdsFromResultSet006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet006 Begin";
    try {
        int type = static_cast<int32_t>(Media::DirtyType::TYPE_DELETED);
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::shared_ptr<ResultSetMock> rset = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(1), Return(0)))
            .WillOnce(DoAll(SetArgReferee<1>(type), Return(0)));

        std::vector<int32_t> add;
        std::vector<int32_t> rm;
        int32_t ret = fileDataHandler->GetAlbumIdsFromResultSet(rset, add, rm);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumIdsFromResultSet006 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet006 End";
}

/**
 * @tc.name: GetAlbumIdsFromResultSet007
 * @tc.desc: Verify the GetAlbumIdsFromResultSet function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetAlbumIdsFromResultSet007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet007 Begin";
    try {
        int type = static_cast<int32_t>(Media::DirtyType::TYPE_SYNCED);
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::shared_ptr<ResultSetMock> rset = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(1), Return(0)))
            .WillOnce(DoAll(SetArgReferee<1>(type), Return(0)));

        std::vector<int32_t> add;
        std::vector<int32_t> rm;
        int32_t ret = fileDataHandler->GetAlbumIdsFromResultSet(rset, add, rm);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAlbumIdsFromResultSet007 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAlbumIdsFromResultSet007 End";
}

/**
 * @tc.name: ConflictRenameThumb001
 * @tc.desc: Verify the ConflictRenameThumb function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, ConflictRenameThumb001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictRenameThumb001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();

        string fullPath = ROOT_MEDIA_DIR + "Photo/test.png";
        string tmpPath;
        string newPath;
        string ret = fileDataHandler->ConflictRenameThumb(*rset, fullPath, tmpPath, newPath);
        GTEST_LOG_(INFO) << ret;
        GTEST_LOG_(INFO) << tmpPath;
        GTEST_LOG_(INFO) << newPath;
        EXPECT_EQ(ret, "test_1.png");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictRenameThumb001 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictRenameThumb001 End";
}

/**
 * @tc.name: ConflictRenameThumb002
 * @tc.desc: Verify the ConflictRenameThumb function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, ConflictRenameThumb002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictRenameThumb002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();

        string fullPath = "";
        string tmpPath;
        string newPath;
        string ret = fileDataHandler->ConflictRenameThumb(*rset, fullPath, tmpPath, newPath);
        GTEST_LOG_(INFO) << ret;
        EXPECT_EQ(ret, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictRenameThumb002 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictRenameThumb002 End";
}

/**
 * @tc.name: ConflictRenamePath001
 * @tc.desc: Verify the ConflictRenamePath function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, ConflictRenamePath001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictRenamePath001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();

        string fullPath = ROOT_MEDIA_DIR + "Photo/test.png";
        string rdbPath;
        string localPath;
        string newLocalPath;
        int32_t ret = fileDataHandler->ConflictRenamePath(*rset, fullPath, rdbPath, localPath, newLocalPath);
        GTEST_LOG_(INFO) << rdbPath;
        GTEST_LOG_(INFO) << localPath;
        GTEST_LOG_(INFO) << newLocalPath;
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictRenamePath001 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictRenamePath001 End";
}

/**
 * @tc.name: ConflictRenamePath002
 * @tc.desc: Verify the ConflictRenamePath function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, ConflictRenamePath002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictRenamePath002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();

        string fullPath = "";
        string rdbPath;
        string localPath;
        string newLocalPath;
        int32_t ret = fileDataHandler->ConflictRenamePath(*rset, fullPath, rdbPath, localPath, newLocalPath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictRenamePath002 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictRenamePath002 End";
}

/**
 * @tc.name: ConflictRename001
 * @tc.desc: Verify the ConflictRename function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, ConflictRename001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictRename001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        string fullPath = ROOT_MEDIA_DIR + "Photo/test.png";
        string relativePath = "";
        int32_t ret = fileDataHandler->ConflictRename(*rset, fullPath, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictRename001 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictRename001 End";
}

/**
 * @tc.name: ConflictRename002
 * @tc.desc: Verify the ConflictRename function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, ConflictRename002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictRename002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        string fullPath = "";
        string relativePath = "";
        int32_t ret = fileDataHandler->ConflictRename(*rset, fullPath, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictRename002 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictRename002 End";
}

/**
 * @tc.name: ConflictRename003
 * @tc.desc: Verify the ConflictRename function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, ConflictRename003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictRename003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        string fullPath = ROOT_MEDIA_DIR + "Photo/test.png";
        string relativePath = "relativeDir/";
        int32_t ret = fileDataHandler->ConflictRename(*rset, fullPath, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictRename003 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictRename003 End";
}

/**
 * @tc.name: ConflictRename004
 * @tc.desc: Verify the ConflictRename function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, ConflictRename004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictRename004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(1));

        string fullPath = ROOT_MEDIA_DIR + "Photo/test.png";
        string relativePath = "";
        int32_t ret = fileDataHandler->ConflictRename(*rset, fullPath, relativePath);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictRename004 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictRename004 End";
}

/**
 * @tc.name: ConflictDataMerge001
 * @tc.desc: Verify the ConflictDataMerge function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, ConflictDataMerge001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictDataMerge001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        DKRecord record;
        record.SetRecordId("1");
        string fullPath = "";
        bool upflag = true;
        int32_t ret = fileDataHandler->ConflictDataMerge(record, fullPath, upflag);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictDataMerge001 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictDataMerge001 End";
}

/**
 * @tc.name: ConflictDataMerge002
 * @tc.desc: Verify the ConflictDataMerge function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, ConflictDataMerge002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictDataMerge002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        DKRecord record;
        record.SetRecordId("1");
        string fullPath = "";
        bool upflag = false;
        int32_t ret = fileDataHandler->ConflictDataMerge(record, fullPath, upflag);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictDataMerge002 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictDataMerge002 End";
}

/**
 * @tc.name: ConflictDataMerge003
 * @tc.desc: Verify the ConflictDataMerge function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, ConflictDataMerge003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictDataMerge003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(1));

        DKRecord record;
        record.SetRecordId("1");
        string fullPath = "";
        bool upflag = true;
        int32_t ret = fileDataHandler->ConflictDataMerge(record, fullPath, upflag);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictDataMerge003 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictDataMerge003 End";
}

/**
 * @tc.name: OnModifyMdirtyRecords001
 * @tc.desc: Verify the OnModifyMdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: issueI7XWAM
 */
HWTEST_F(FileDataHandlerTest, OnModifyMdirtyRecords001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));

        std::map<DriveKit::DKRecordId, DriveKit::DKRecordOperResult> myMap;
        int32_t ret = fileDataHandler->OnModifyMdirtyRecords(myMap);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyMdirtyRecords001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords001 End";
}

/**
 * @tc.name: OnModifyMdirtyRecords002
 * @tc.desc: Verify the OnModifyMdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: issueI7XWAM
 */
HWTEST_F(FileDataHandlerTest, OnModifyMdirtyRecords002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
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
        int32_t ret = fileDataHandler->OnModifyMdirtyRecords(myMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyMdirtyRecords002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords002 End";
}

/**
 * @tc.name: OnModifyMdirtyRecords003
 * @tc.desc: Verify the OnModifyMdirtyRecords function
 * @tc.type: FUNC
 * @tc.require: issueI7XWAM
 */
HWTEST_F(FileDataHandlerTest, OnModifyMdirtyRecords003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
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
        int32_t ret = fileDataHandler->OnModifyMdirtyRecords(myMap);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnModifyMdirtyRecords003 ERROR";
    }
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords003 End";
}

/**
 * @tc.name: AddCloudThumbs001
 * @tc.desc: Verify the AddCloudThumbs function
 * @tc.type: FUNC
 * @tc.require: issueI7XWAM
 */
HWTEST_F(FileDataHandlerTest, AddCloudThumbs001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCloudThumbs001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_THUMB_SIZE), 1));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_LCD_SIZE), 1));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->AddCloudThumbs(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AddCloudThumbs001 ERROR";
    }
    GTEST_LOG_(INFO) << "AddCloudThumbs001 End";
}

/**
 * @tc.name: AddCloudThumbs002
 * @tc.desc: Verify the AddCloudThumbs function
 * @tc.type: FUNC
 * @tc.require: issueI7XWAM
 */
HWTEST_F(FileDataHandlerTest, AddCloudThumbs002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCloudThumbs002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecord record;
        int32_t ret = fileDataHandler->AddCloudThumbs(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AddCloudThumbs002 ERROR";
    }
    GTEST_LOG_(INFO) << "AddCloudThumbs002 End";
}

/**
 * @tc.name: AddCloudThumbs003
 * @tc.desc: Verify the AddCloudThumbs function
 * @tc.type: FUNC
 * @tc.require: issueI7XWAM
 */
HWTEST_F(FileDataHandlerTest, AddCloudThumbs003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCloudThumbs003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->AddCloudThumbs(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AddCloudThumbs003 ERROR";
    }
    GTEST_LOG_(INFO) << "AddCloudThumbs003 End";
}

/**
 * @tc.name: AddCloudThumbs004
 * @tc.desc: Verify the AddCloudThumbs function
 * @tc.type: FUNC
 * @tc.require: issueI7YE3V
 */
HWTEST_F(FileDataHandlerTest, AddCloudThumbs004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCloudThumbs004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->AddCloudThumbs(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AddCloudThumbs004 ERROR";
    }
    GTEST_LOG_(INFO) << "AddCloudThumbs004 End";
}

/**
 * @tc.name: AddCloudThumbs005
 * @tc.desc: Verify the AddCloudThumbs function
 * @tc.type: FUNC
 * @tc.require: issueI7YE3V
 */
HWTEST_F(FileDataHandlerTest, AddCloudThumbs005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCloudThumbs005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->AddCloudThumbs(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AddCloudThumbs005 ERROR";
    }
    GTEST_LOG_(INFO) << "AddCloudThumbs005 End";
}

/**
 * @tc.name: AddCloudThumbs006
 * @tc.desc: Verify the AddCloudThumbs function
 * @tc.type: FUNC
 * @tc.require: issueI7YE3V
 */
HWTEST_F(FileDataHandlerTest, AddCloudThumbs006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCloudThumbs006 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_THUMB_SIZE), "abc"));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->AddCloudThumbs(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AddCloudThumbs006 ERROR";
    }
    GTEST_LOG_(INFO) << "AddCloudThumbs006 End";
}

/**
 * @tc.name: AddCloudThumbs007
 * @tc.desc: Verify the AddCloudThumbs function
 * @tc.type: FUNC
 * @tc.require: issueI7YE3V
 */
HWTEST_F(FileDataHandlerTest, AddCloudThumbs007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCloudThumbs007 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_THUMB_SIZE), 1));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->AddCloudThumbs(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AddCloudThumbs007 ERROR";
    }
    GTEST_LOG_(INFO) << "AddCloudThumbs007 End";
}

/**
 * @tc.name: AddCloudThumbs008
 * @tc.desc: Verify the AddCloudThumbs function
 * @tc.type: FUNC
 * @tc.require: issueI7YE3V
 */
HWTEST_F(FileDataHandlerTest, AddCloudThumbs008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCloudThumbs008 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_THUMB_SIZE), 1));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_LCD_SIZE), "abc"));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->AddCloudThumbs(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AddCloudThumbs008 ERROR";
    }
    GTEST_LOG_(INFO) << "AddCloudThumbs008 End";
}

/**
 * @tc.name: AddCloudThumbs009
 * @tc.desc: Verify the AddCloudThumbs function
 * @tc.type: FUNC
 * @tc.require: issueI7YE3V
 */
HWTEST_F(FileDataHandlerTest, AddCloudThumbs009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCloudThumbs009 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_THUMB_SIZE), 1));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_LCD_SIZE), 1));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->AddCloudThumbs(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AddCloudThumbs009 ERROR";
    }
    GTEST_LOG_(INFO) << "AddCloudThumbs009 End";
}

/**
 * @tc.name: ConflictDifferent001
 * @tc.desc: Verify the ConflictDifferent function
 * @tc.type: FUNC
 * @tc.require: issueI7YE3V
 */
HWTEST_F(FileDataHandlerTest, ConflictDifferent001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictDifferent001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        DKRecord record;
        string fullPath = ROOT_MEDIA_DIR + "Photo/test.png";
        string relativePath = "";
        int32_t ret = fileDataHandler->ConflictDifferent(*rset, record, fullPath, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictDifferent001 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictDifferent001 End";
}

/**
 * @tc.name: ConflictMerge001
 * @tc.desc: Verify the ConflictMerge function
 * @tc.type: FUNC
 * @tc.require: issueI7YE3V
 */
HWTEST_F(FileDataHandlerTest, ConflictMerge001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictMerge001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        DKRecord record;
        string fullPath = ROOT_MEDIA_DIR + "Photo/test.png";
        bool comflag = false;
        int64_t imetaModified = 0;
        int32_t ret = fileDataHandler->ConflictMerge(*rset, record, fullPath, comflag, imetaModified);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictMerge001 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictMerge001 End";
}

/**
 * @tc.name: ConflictMerge002
 * @tc.desc: Verify the ConflictMerge function
 * @tc.type: FUNC
 * @tc.require: issueI7YE3V
 */
HWTEST_F(FileDataHandlerTest, ConflictMerge002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictMerge002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillOnce(Return(1));

        DKRecord record;
        string fullPath = ROOT_MEDIA_DIR + "Photo/test.png";
        bool comflag = false;
        int64_t imetaModified = 0;
        int32_t ret = fileDataHandler->ConflictMerge(*rset, record, fullPath, comflag, imetaModified);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictMerge002 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictMerge002 End";
}

/**
 * @tc.name: ConflictMerge003
 * @tc.desc: Verify the ConflictMerge function
 * @tc.type: FUNC
 * @tc.require: issueI7YE3V
 */
HWTEST_F(FileDataHandlerTest, ConflictMerge003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictMerge003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(0));

        DKRecord record;
        string fullPath = ROOT_MEDIA_DIR + "Photo/test.png";
        bool comflag = false;
        int64_t imetaModified = -1;
        int32_t ret = fileDataHandler->ConflictMerge(*rset, record, fullPath, comflag, imetaModified);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictMerge003 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictMerge003 End";
}

/**
 * @tc.name: ConflictMerge004
 * @tc.desc: Verify the ConflictMerge function
 * @tc.type: FUNC
 * @tc.require: issueI7YE3V
 */
HWTEST_F(FileDataHandlerTest, ConflictMerge004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictMerge004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(1));

        DKRecord record;
        string fullPath = ROOT_MEDIA_DIR + "Photo/test.png";
        bool comflag = false;
        int64_t imetaModified = 0;
        int32_t ret = fileDataHandler->ConflictMerge(*rset, record, fullPath, comflag, imetaModified);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictMerge004 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictMerge004 End";
}
