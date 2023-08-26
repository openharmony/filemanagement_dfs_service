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
 * @tc.name: CompensateFilePath001
 * @tc.desc: Verify the CompensateFilePath function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, CompensateFilePath001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompensateFilePath001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("123")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->CompensateFilePath(record);
        EXPECT_NE(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CompensateFilePath001 ERROR";
    }
    GTEST_LOG_(INFO) << "CompensateFilePath001 End";
}

/**
 * @tc.name: CompensateFilePath002
 * @tc.desc: Verify the CompensateFilePath function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, CompensateFilePath002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompensateFilePath002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecord record;
        int32_t ret = fileDataHandler->CompensateFilePath(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CompensateFilePath002 ERROR";
    }
    GTEST_LOG_(INFO) << "CompensateFilePath002 End";
}

/**
 * @tc.name: CompensateFilePath003
 * @tc.desc: Verify the CompensateFilePath function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, CompensateFilePath003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompensateFilePath003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_SIZE, DriveKit::DKRecordField("123")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->CompensateFilePath(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CompensateFilePath003 ERROR";
    }
    GTEST_LOG_(INFO) << "CompensateFilePath003 End";
}

/**
 * @tc.name: CompensateFilePath004
 * @tc.desc: Verify the CompensateFilePath function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, CompensateFilePath004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CompensateFilePath004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_DATE_MODIFIED, DriveKit::DKRecordField("123")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->CompensateFilePath(record);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CompensateFilePath004 ERROR";
    }
    GTEST_LOG_(INFO) << "CompensateFilePath004 End";
}

/**
 * @tc.name: GetMediaType001
 * @tc.desc: Verify the GetMediaType function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, GetMediaType001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMediaType001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecordData data;
        data.insert(std::make_pair(FILE_FILETYPE, DriveKit::DKRecordField(FILE_TYPE_IMAGE)));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t mediaType = 0;
        int32_t ret = fileDataHandler->GetMediaType(record, mediaType);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetMediaType001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetMediaType001 End";
}

/**
 * @tc.name: GetMediaType002
 * @tc.desc: Verify the GetMediaType function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, GetMediaType002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMediaType002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecordData data;
        data.insert(std::make_pair(FILE_FILETYPE, DriveKit::DKRecordField(MEDIA_TYPE_VIDEO)));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t mediaType = 0;
        int32_t ret = fileDataHandler->GetMediaType(record, mediaType);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetMediaType002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetMediaType002 End";
}

/**
 * @tc.name: GetMediaType003
 * @tc.desc: Verify the GetMediaType function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, GetMediaType003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMediaType003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t mediaType = 0;
        DriveKit::DKRecord record;
        int32_t ret = fileDataHandler->GetMediaType(record, mediaType);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetMediaType003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetMediaType003 End";
}

/**
 * @tc.name: GetMediaType004
 * @tc.desc: Verify the GetMediaType function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, GetMediaType004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMediaType004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecordData data;
        data.insert(std::make_pair(FILE_FILETYPE, DriveKit::DKRecordField("abc")));
        int32_t mediaType = 0;
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        int32_t ret = fileDataHandler->GetMediaType(record, mediaType);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetMediaType004 ERROR";
    }
    GTEST_LOG_(INFO) << "GetMediaType004 End";
}

/**
 * @tc.name: GetAssetUniqueId001
 * @tc.desc: Verify the GetAssetUniqueId function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, GetAssetUniqueId001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAssetUniqueId001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));

        int32_t mediaType = static_cast<int>(MediaType::MEDIA_TYPE_IMAGE);
        int32_t ret = fileDataHandler->GetAssetUniqueId(mediaType);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAssetUniqueId001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAssetUniqueId001 End";
}

/**
 * @tc.name: GetAssetUniqueId002
 * @tc.desc: Verify the GetAssetUniqueId function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, GetAssetUniqueId002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAssetUniqueId002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));

        int32_t mediaType = static_cast<int>(MediaType::MEDIA_TYPE_VIDEO);
        int32_t ret = fileDataHandler->GetAssetUniqueId(mediaType);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAssetUniqueId002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAssetUniqueId002 End";
}

/**
 * @tc.name: GetAssetUniqueId003
 * @tc.desc: Verify the GetAssetUniqueId function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, GetAssetUniqueId003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAssetUniqueId003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t mediaType = static_cast<int>(MediaType::MEDIA_TYPE_FILE);
        int32_t ret = fileDataHandler->GetAssetUniqueId(mediaType);
        EXPECT_EQ(ret, MediaType::MEDIA_TYPE_FILE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAssetUniqueId003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAssetUniqueId003 End";
}

/**
 * @tc.name: GetAssetUniqueId004
 * @tc.desc: Verify the GetAssetUniqueId function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, GetAssetUniqueId004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAssetUniqueId004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();

        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));

        int32_t mediaType = static_cast<int>(MediaType::MEDIA_TYPE_IMAGE);
        int32_t ret = fileDataHandler->GetAssetUniqueId(mediaType);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAssetUniqueId004 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAssetUniqueId004 End";
}

/**
 * @tc.name: CreateAssetPathById001
 * @tc.desc: Verify the CreateAssetPathById function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, CreateAssetPathById001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAssetPathById001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t uniqueId = 0;
        std::string filePath = "";
        DriveKit::DKRecord record;
        int32_t mediaType = static_cast<int>(MediaType::MEDIA_TYPE_IMAGE);
        int32_t ret = fileDataHandler->CreateAssetPathById(record, uniqueId, mediaType, filePath);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CreateAssetPathById001 ERROR";
    }
    GTEST_LOG_(INFO) << "CreateAssetPathById001 End";
}

/**
 * @tc.name: CreateAssetPathById002
 * @tc.desc: Verify the CreateAssetPathById function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, CreateAssetPathById002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAssetPathById002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t uniqueId = -1;
        std::string filePath = "";
        DriveKit::DKRecord record;
        int32_t mediaType = static_cast<int>(MediaType::MEDIA_TYPE_IMAGE);
        int32_t ret = fileDataHandler->CreateAssetPathById(record, uniqueId, mediaType, filePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CreateAssetPathById002 ERROR";
    }
    GTEST_LOG_(INFO) << "CreateAssetPathById002 End";
}

/**
 * @tc.name: CreateAssetPathById003
 * @tc.desc: Verify the CreateAssetPathById function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, CreateAssetPathById003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAssetPathById003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t uniqueId = 0;
        std::string filePath = "";
        DriveKit::DKRecord record;
        int32_t mediaType = static_cast<int>(MediaType::MEDIA_TYPE_FILE);
        int32_t ret = fileDataHandler->CreateAssetPathById(record, uniqueId, mediaType, filePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CreateAssetPathById003 ERROR";
    }
    GTEST_LOG_(INFO) << "CreateAssetPathById003 End";
}

/**
 * @tc.name: CreateAssetBucket001
 * @tc.desc: Verify the CreateAssetBucket function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, CreateAssetBucket001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAssetBucket001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t uniqueId = 0;
        int32_t bucketNum = 0;
        int32_t ret = fileDataHandler->CreateAssetBucket(uniqueId, bucketNum);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CreateAssetBucket001 ERROR";
    }
    GTEST_LOG_(INFO) << "CreateAssetBucket001 End";
}

/**
 * @tc.name: CreateAssetBucket002
 * @tc.desc: Verify the CreateAssetBucket function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, CreateAssetBucket002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAssetBucket002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t uniqueId = -1;
        int32_t bucketNum = 0;
        int32_t ret = fileDataHandler->CreateAssetBucket(uniqueId, bucketNum);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CreateAssetBucket002 ERROR";
    }
    GTEST_LOG_(INFO) << "CreateAssetBucket002 End";
}

/**
 * @tc.name: CreateAssetBucket003
 * @tc.desc: Verify the CreateAssetBucket function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, CreateAssetBucket003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAssetBucket003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t uniqueId = 1;
        int32_t bucketNum = 0;
        int32_t ret = fileDataHandler->CreateAssetBucket(uniqueId, bucketNum);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CreateAssetBucket003 ERROR";
    }
    GTEST_LOG_(INFO) << "CreateAssetBucket003 End";
}

/**
 * @tc.name: OnDownloadAssets001
 * @tc.desc: Verify the OnDownloadAssets function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, OnDownloadAssets001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadAssets001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));

        DriveKit::DKDownloadAsset asset;
        asset.fieldKey = "thumbnail";
        fileDataHandler->OnDownloadAssets(asset);
        EXPECT_TRUE(false);
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << " OnDownloadAssets001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnDownloadAssets001 End";
}

/**
 * @tc.name: OnDownloadAssets002
 * @tc.desc: Verify the OnDownloadAssets function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, OnDownloadAssets002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadAssets002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(1));

        DriveKit::DKDownloadAsset asset;
        asset.fieldKey = "thumbnail";
        fileDataHandler->OnDownloadAssets(asset);
        EXPECT_TRUE(false);
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << " OnDownloadAssets002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnDownloadAssets002 End";
}

/**
 * @tc.name: OnDownloadAssets003
 * @tc.desc: Verify the OnDownloadAssets function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, OnDownloadAssets003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadAssets003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKDownloadAsset asset;
        fileDataHandler->OnDownloadAssets(asset);
        EXPECT_TRUE(false);
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << " OnDownloadAssets003 ERROR";
    }
    GTEST_LOG_(INFO) << "OnDownloadAssets003 End";
}

/**
 * @tc.name: OnDownloadAssets004
 * @tc.desc: Verify the OnDownloadAssets function
 * @tc.type: FUNC
 * @tc.require: issueI7VEA4
 */
HWTEST_F(FileDataHandlerTest, OnDownloadAssets004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadAssets004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillOnce(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr))).WillOnce(Return(ByMove(nullptr)));
        DriveKit::DKDownloadAsset asset1;
        DriveKit::DKDownloadAsset asset2;
        asset2.fieldKey = "thumbnail";
        DKDownloadResult result1;
        DKDownloadResult result2;
        DKError dkError;
        dkError.SetServerError(static_cast<int>(DKServerErrorCode::ACCESS_DENIED));
        result2.SetDKError(dkError);
        map<DKDownloadAsset, DKDownloadResult> resultMap;
        resultMap[asset1] = result1;
        resultMap[asset2] = result2;
        int32_t ret = fileDataHandler->OnDownloadAssets(resultMap);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnDownloadAssets004 ERROR";
    }
    GTEST_LOG_(INFO) << "OnDownloadAssets004 End";
}
