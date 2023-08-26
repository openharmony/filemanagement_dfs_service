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

/**
 * @tc.name: GetFileExtension001
 * @tc.desc: Verify the GetFileExtension function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetFileExtension001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileExtension001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecordData data;
        data.insert(std::make_pair(FILE_FILE_NAME, DriveKit::DKRecordField("test.txt")));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string ret = fileDataHandler->GetFileExtension(record);
        EXPECT_EQ(ret, "txt");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFileExtension001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetFileExtension001 End";
}

/**
 * @tc.name: GetFileExtension002
 * @tc.desc: Verify the GetFileExtension function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetFileExtension002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileExtension002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecord record;
        string ret = fileDataHandler->GetFileExtension(record);
        EXPECT_EQ(ret, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFileExtension002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetFileExtension002 End";
}

/**
 * @tc.name: GetFileExtension003
 * @tc.desc: Verify the GetFileExtension function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetFileExtension003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileExtension003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecordData data;
        data.insert(std::make_pair(FILE_FILE_NAME, DriveKit::DKRecordField(1)));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string ret = fileDataHandler->GetFileExtension(record);
        EXPECT_EQ(ret, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFileExtension003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetFileExtension003 End";
}

/**
 * @tc.name: GetFileExtension004
 * @tc.desc: Verify the GetFileExtension function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetFileExtension004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileExtension004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecordData data;
        data.insert(std::make_pair(FILE_FILE_NAME, DriveKit::DKRecordField("")));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string ret = fileDataHandler->GetFileExtension(record);
        EXPECT_EQ(ret, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFileExtension004 ERROR";
    }
    GTEST_LOG_(INFO) << "GetFileExtension004 End";
}

/**
 * @tc.name: GetFileExtension005
 * @tc.desc: Verify the GetFileExtension function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, GetFileExtension005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileExtension005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        DriveKit::DKRecordData data;
        data.insert(std::make_pair(FILE_FILE_NAME, DriveKit::DKRecordField("text")));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string ret = fileDataHandler->GetFileExtension(record);
        EXPECT_EQ(ret, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFileExtension005 ERROR";
    }
    GTEST_LOG_(INFO) << "GetFileExtension005 End";
}

/**
 * @tc.name: CreateAssetRealName001
 * @tc.desc: Verify the CreateAssetRealName function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, CreateAssetRealName001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAssetRealName001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t fileId = 1;
        int32_t mediaType = static_cast<int32_t>(MediaType::MEDIA_TYPE_IMAGE);
        string extension = "txt";
        string name = "";
        int32_t ret = fileDataHandler->CreateAssetRealName(fileId, mediaType, extension, name);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CreateAssetRealName001 ERROR";
    }
    GTEST_LOG_(INFO) << "CreateAssetRealName001 End";
}

/**
 * @tc.name: CreateAssetRealName002
 * @tc.desc: Verify the CreateAssetRealName function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, CreateAssetRealName002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAssetRealName002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t fileId = ASSET_MAX_COMPLEMENT_ID + 1;
        int32_t mediaType = static_cast<int32_t>(MediaType::MEDIA_TYPE_IMAGE);
        string extension = "txt";
        string name = "";
        int32_t ret = fileDataHandler->CreateAssetRealName(fileId, mediaType, extension, name);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CreateAssetRealName002 ERROR";
    }
    GTEST_LOG_(INFO) << "CreateAssetRealName002 End";
}

/**
 * @tc.name: CreateAssetRealName003
 * @tc.desc: Verify the CreateAssetRealName function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, CreateAssetRealName003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAssetRealName003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t fileId = 1;
        int32_t mediaType = static_cast<int32_t>(MediaType::MEDIA_TYPE_VIDEO);
        string extension = "txt";
        string name = "";
        int32_t ret = fileDataHandler->CreateAssetRealName(fileId, mediaType, extension, name);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CreateAssetRealName003 ERROR";
    }
    GTEST_LOG_(INFO) << "CreateAssetRealName003 End";
}

/**
 * @tc.name: CreateAssetRealName004
 * @tc.desc: Verify the CreateAssetRealName function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, CreateAssetRealName004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAssetRealName004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);

        int32_t fileId = 1;
        int32_t mediaType = static_cast<int32_t>(MediaType::MEDIA_TYPE_AUDIO);
        string extension = "txt";
        string name = "";
        int32_t ret = fileDataHandler->CreateAssetRealName(fileId, mediaType, extension, name);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CreateAssetRealName004 ERROR";
    }
    GTEST_LOG_(INFO) << "CreateAssetRealName004 End";
}

/**
 * @tc.name: BindAlbums001
 * @tc.desc: Verify the BindAlbums function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, BindAlbums001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BindAlbums001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset1 = std::make_unique<AbsSharedResultSetMock>();
        std::unique_ptr<AbsSharedResultSetMock> rset2 = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset1, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset2, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset1, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset2, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset1, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset1, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(1), Return(0)));
        EXPECT_CALL(*rset2, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _))
            .WillOnce(Return(ByMove(std::move(rset1))))
            .WillOnce(Return(ByMove(std::move(rset2))));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::MediaColumn::MEDIA_ID, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        std::vector<DriveKit::DKRecord> records;
        records.push_back(record);
        int32_t ret = fileDataHandler->BindAlbums(records);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BindAlbums001 ERROR";
    }
    GTEST_LOG_(INFO) << "BindAlbums001 End";
}

/**
 * @tc.name: BindAlbums002
 * @tc.desc: Verify the BindAlbums function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, BindAlbums002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BindAlbums002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::MediaColumn::MEDIA_ID, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        std::vector<DriveKit::DKRecord> records;
        records.push_back(record);
        int32_t ret = fileDataHandler->BindAlbums(records);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BindAlbums002 ERROR";
    }
    GTEST_LOG_(INFO) << "BindAlbums002 End";
}

/**
 * @tc.name: BindAlbums003
 * @tc.desc: Verify the BindAlbums function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, BindAlbums003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BindAlbums003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::MediaColumn::MEDIA_ID, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        std::vector<DriveKit::DKRecord> records;
        records.push_back(record);
        int32_t ret = fileDataHandler->BindAlbums(records);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BindAlbums003 ERROR";
    }
    GTEST_LOG_(INFO) << "BindAlbums003 End";
}

/**
 * @tc.name: BindAlbums004
 * @tc.desc: Verify the BindAlbums function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, BindAlbums004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BindAlbums004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(1), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset)))).WillOnce(Return(ByMove(nullptr)));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::MediaColumn::MEDIA_ID, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        std::vector<DriveKit::DKRecord> records;
        records.push_back(record);
        int32_t ret = fileDataHandler->BindAlbums(records);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BindAlbums004 ERROR";
    }
    GTEST_LOG_(INFO) << "BindAlbums004 End";
}

/**
 * @tc.name: BindAlbumChanges001
 * @tc.desc: Verify the BindAlbumChanges function
 * @tc.type: FUNC
 * @tc.require: issueI7VPAH
 */
HWTEST_F(FileDataHandlerTest, BindAlbumChanges001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BindAlbumChanges001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb);
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::MediaColumn::MEDIA_ID, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        std::vector<DriveKit::DKRecord> records;
        records.push_back(record);
        int32_t ret = fileDataHandler->BindAlbumChanges(records);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BindAlbumChanges001 ERROR";
    }
    GTEST_LOG_(INFO) << "BindAlbumChanges001 End";
}
