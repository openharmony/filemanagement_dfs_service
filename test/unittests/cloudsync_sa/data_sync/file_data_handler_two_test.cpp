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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        int32_t mediaType = static_cast<int>(MediaType::MEDIA_TYPE_FILE);
        int32_t ret = fileDataHandler->GetAssetUniqueId(mediaType);
        EXPECT_EQ(ret, MediaType::MEDIA_TYPE_FILE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetAssetUniqueId001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAssetUniqueId001 End";
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DriveKit::DKDownloadAsset asset;
        fileDataHandler->OnDownloadAssets(asset);
        EXPECT_TRUE(false);
    } catch (...) {
        EXPECT_TRUE(true);
        GTEST_LOG_(INFO) << " OnDownloadAssets002 ERROR";
    }
    GTEST_LOG_(INFO) << "OnDownloadAssets002 End";
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
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
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
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

/**
 * @tc.name: BindAlbumChanges002
 * @tc.desc: Verify the BindAlbumChanges function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, BindAlbumChanges002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BindAlbumChanges002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(nullptr)));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::MediaColumn::MEDIA_ID, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        std::vector<DriveKit::DKRecord> records;
        records.push_back(record);
        int32_t ret = fileDataHandler->BindAlbumChanges(records);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BindAlbumChanges002 ERROR";
    }
    GTEST_LOG_(INFO) << "BindAlbumChanges002 End";
}

/**
 * @tc.name: BindAlbumChanges003
 * @tc.desc: Verify the BindAlbumChanges function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, BindAlbumChanges003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BindAlbumChanges003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
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
        int32_t ret = fileDataHandler->BindAlbumChanges(records);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BindAlbumChanges003 ERROR";
    }
    GTEST_LOG_(INFO) << "BindAlbumChanges003 End";
}

/**
 * @tc.name: BindAlbumChanges004
 * @tc.desc: Verify the BindAlbumChanges function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, BindAlbumChanges004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BindAlbumChanges004 Begin";
    try {
        int type = static_cast<int32_t>(Media::DirtyType::TYPE_NEW);
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(1), Return(0)))
            .WillOnce(DoAll(SetArgReferee<1>(type), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset)))).WillOnce(Return(ByMove(nullptr)));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::MediaColumn::MEDIA_ID, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        std::vector<DriveKit::DKRecord> records;
        records.push_back(record);
        int32_t ret = fileDataHandler->BindAlbumChanges(records);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BindAlbumChanges004 ERROR";
    }
    GTEST_LOG_(INFO) << "BindAlbumChanges004 End";
}

/**
 * @tc.name: BindAlbumChanges005
 * @tc.desc: Verify the BindAlbumChanges function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, BindAlbumChanges005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BindAlbumChanges005 Begin";
    try {
        int type = static_cast<int32_t>(Media::DirtyType::TYPE_DELETED);
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0)).WillOnce(Return(1));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetInt(_, _))
            .WillOnce(DoAll(SetArgReferee<1>(2), Return(0)))
            .WillOnce(DoAll(SetArgReferee<1>(type), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _))
            .WillOnce(Return(ByMove(std::move(rset))))
            .WillOnce(Return(ByMove(nullptr)));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::MediaColumn::MEDIA_ID, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        std::vector<DriveKit::DKRecord> records;
        records.push_back(record);
        int32_t ret = fileDataHandler->BindAlbumChanges(records);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BindAlbumChanges005 ERROR";
    }
    GTEST_LOG_(INFO) << "BindAlbumChanges005 End";
}

/**
 * @tc.name: ConflictHandler001
 * @tc.desc: Verify the ConflictHandler function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, ConflictHandler001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictHandler001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillOnce(Return(0)).WillOnce(Return(0));

        DKRecord record;
        int64_t isize = 0;
        bool modifyPathflag = false;
        int32_t ret = fileDataHandler->ConflictHandler(*rset, record, isize, modifyPathflag);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictHandler001 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictHandler001 End";
}

/**
 * @tc.name: ConflictHandler002
 * @tc.desc: Verify the ConflictHandler function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, ConflictHandler002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictHandler002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(Return(1));

        DKRecord record;
        int64_t isize = 0;
        bool modifyPathflag = false;
        int32_t ret = fileDataHandler->ConflictHandler(*rset, record, isize, modifyPathflag);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictHandler002 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictHandler002 End";
}

/**
 * @tc.name: ConflictHandler003
 * @tc.desc: Verify the ConflictHandler function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, ConflictHandler003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictHandler003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)));
        EXPECT_CALL(*rset, GetLong(_, _)).WillOnce(Return(1));

        DKRecord record;
        int64_t isize = 0;
        bool modifyPathflag = false;
        int32_t ret = fileDataHandler->ConflictHandler(*rset, record, isize, modifyPathflag);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictHandler003 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictHandler003 End";
}

/**
 * @tc.name: ConflictHandler004
 * @tc.desc: Verify the ConflictHandler function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, ConflictHandler004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictHandler004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)));
        EXPECT_CALL(*rset, GetLong(_, _)).WillOnce(Return(0)).WillOnce(Return(1));

        DKRecord record;
        int64_t isize = 0;
        bool modifyPathflag = false;
        int32_t ret = fileDataHandler->ConflictHandler(*rset, record, isize, modifyPathflag);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictHandler004 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictHandler004 End";
}

/**
 * @tc.name: ConflictHandler005
 * @tc.desc: Verify the ConflictHandler function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, ConflictHandler005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictHandler005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)));
        EXPECT_CALL(*rset, GetLong(_, _)).WillOnce(Return(0)).WillOnce(Return(0));

        DKRecord record;
        int64_t isize = 1;
        bool modifyPathflag = false;
        int32_t ret = fileDataHandler->ConflictHandler(*rset, record, isize, modifyPathflag);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictHandler005 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictHandler005 End";
}

/**
 * @tc.name: ConflictHandler006
 * @tc.desc: Verify the ConflictHandler function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, ConflictHandler006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConflictHandler006 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<ResultSetMock> rset = std::make_unique<ResultSetMock>();
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>("1"), Return(0)));
        EXPECT_CALL(*rset, GetLong(_, _)).WillOnce(Return(0)).WillOnce(DoAll(SetArgReferee<1>(100), Return(0)));

        DKRecord record;
        int64_t isize = 0;
        bool modifyPathflag = false;
        int32_t ret = fileDataHandler->ConflictHandler(*rset, record, isize, modifyPathflag);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ConflictHandler006 ERROR";
    }
    GTEST_LOG_(INFO) << "ConflictHandler006 End";
}

/**
 * @tc.name: GetConflictData001
 * @tc.desc: Verify the GetConflictData function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, GetConflictData001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConflictData001 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_RELATIVE_PATH, DriveKit::DKRecordField("relativePath")));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string fullPath = "";
        int64_t isize = 0;
        int64_t imetaModified = 0;
        std::string relativePath = "";
        int32_t ret = fileDataHandler->GetConflictData(record, fullPath, isize, imetaModified, relativePath);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetConflictData001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetConflictData001 End";
}

/**
 * @tc.name: GetConflictData002
 * @tc.desc: Verify the GetConflictData function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, GetConflictData002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConflictData002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DriveKit::DKRecord record;
        string fullPath = "";
        int64_t isize = 0;
        int64_t imetaModified = 0;
        std::string relativePath = "";
        int32_t ret = fileDataHandler->GetConflictData(record, fullPath, isize, imetaModified, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetConflictData002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetConflictData002 End";
}

/**
 * @tc.name: GetConflictData003
 * @tc.desc: Verify the GetConflictData function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, GetConflictData003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConflictData003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string fullPath = "";
        int64_t isize = 0;
        int64_t imetaModified = 0;
        std::string relativePath = "";
        int32_t ret = fileDataHandler->GetConflictData(record, fullPath, isize, imetaModified, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetConflictData003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetConflictData003 End";
}

/**
 * @tc.name: GetConflictData004
 * @tc.desc: Verify the GetConflictData function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, GetConflictData004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConflictData004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string fullPath = "";
        int64_t isize = 0;
        int64_t imetaModified = 0;
        std::string relativePath = "";
        int32_t ret = fileDataHandler->GetConflictData(record, fullPath, isize, imetaModified, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetConflictData004 ERROR";
    }
    GTEST_LOG_(INFO) << "GetConflictData004 End";
}

/**
 * @tc.name: GetConflictData005
 * @tc.desc: Verify the GetConflictData function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, GetConflictData005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConflictData005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField(0)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string fullPath = "";
        int64_t isize = 0;
        int64_t imetaModified = 0;
        std::string relativePath = "";
        int32_t ret = fileDataHandler->GetConflictData(record, fullPath, isize, imetaModified, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetConflictData005 ERROR";
    }
    GTEST_LOG_(INFO) << "GetConflictData005 End";
}

/**
 * @tc.name: GetConflictData006
 * @tc.desc: Verify the GetConflictData function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, GetConflictData006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConflictData006 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string fullPath = "";
        int64_t isize = 0;
        int64_t imetaModified = 0;
        std::string relativePath = "";
        int32_t ret = fileDataHandler->GetConflictData(record, fullPath, isize, imetaModified, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetConflictData006 ERROR";
    }
    GTEST_LOG_(INFO) << "GetConflictData006 End";
}

/**
 * @tc.name: GetConflictData007
 * @tc.desc: Verify the GetConflictData function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, GetConflictData007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConflictData007 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_RELATIVE_PATH, DriveKit::DKRecordField(0)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string fullPath = "";
        int64_t isize = 0;
        int64_t imetaModified = 0;
        std::string relativePath = "";
        int32_t ret = fileDataHandler->GetConflictData(record, fullPath, isize, imetaModified, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetConflictData007 ERROR";
    }
    GTEST_LOG_(INFO) << "GetConflictData007 End";
}

/**
 * @tc.name: GetConflictData008
 * @tc.desc: Verify the GetConflictData function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, GetConflictData008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConflictData008 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_RELATIVE_PATH, DriveKit::DKRecordField("relativePath")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string fullPath = "";
        int64_t isize = 0;
        int64_t imetaModified = 0;
        std::string relativePath = "";
        int32_t ret = fileDataHandler->GetConflictData(record, fullPath, isize, imetaModified, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetConflictData008 ERROR";
    }
    GTEST_LOG_(INFO) << "GetConflictData008 End";
}

/**
 * @tc.name: GetConflictData009
 * @tc.desc: Verify the GetConflictData function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, GetConflictData009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetConflictData009 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_RELATIVE_PATH, DriveKit::DKRecordField("relativePath")));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField("abc")));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        string fullPath = "";
        int64_t isize = 0;
        int64_t imetaModified = 0;
        std::string relativePath = "";
        int32_t ret = fileDataHandler->GetConflictData(record, fullPath, isize, imetaModified, relativePath);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetConflictData009 ERROR";
    }
    GTEST_LOG_(INFO) << "GetConflictData009 End";
}

/**
 * @tc.name: PullRecordConflict001
 * @tc.desc: Verify the PullRecordConflict function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, PullRecordConflict001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordConflict001 Begin";
    try {
        const int rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_RELATIVE_PATH, DriveKit::DKRecordField("relativePath")));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        data.insert(std::make_pair(DriveKit::DKFieldKey(Media::PhotoColumn::MEDIA_SIZE), 0));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        bool comflag = false;
        int32_t ret = fileDataHandler->PullRecordConflict(record, comflag);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " PullRecordConflict001 ERROR";
    }
    GTEST_LOG_(INFO) << "PullRecordConflict001 End";
}

/**
 * @tc.name: PullRecordConflict002
 * @tc.desc: Verify the PullRecordConflict function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, PullRecordConflict002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordConflict002 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));

        DriveKit::DKRecord record;
        bool comflag = false;
        int32_t ret = fileDataHandler->PullRecordConflict(record, comflag);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " PullRecordConflict002 ERROR";
    }
    GTEST_LOG_(INFO) << "PullRecordConflict002 End";
}

/**
 * @tc.name: PullRecordConflict003
 * @tc.desc: Verify the PullRecordConflict function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, PullRecordConflict003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordConflict003 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(nullptr));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_RELATIVE_PATH, DriveKit::DKRecordField("relativePath")));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        data.insert(std::make_pair(DriveKit::DKFieldKey(Media::PhotoColumn::MEDIA_SIZE), 0));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        bool comflag = false;
        int32_t ret = fileDataHandler->PullRecordConflict(record, comflag);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " PullRecordConflict003 ERROR";
    }
    GTEST_LOG_(INFO) << "PullRecordConflict003 End";
}

/**
 * @tc.name: PullRecordConflict004
 * @tc.desc: Verify the PullRecordConflict function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, PullRecordConflict004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordConflict004 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_RELATIVE_PATH, DriveKit::DKRecordField("relativePath")));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        data.insert(std::make_pair(DriveKit::DKFieldKey(Media::PhotoColumn::MEDIA_SIZE), 0));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        bool comflag = false;
        int32_t ret = fileDataHandler->PullRecordConflict(record, comflag);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " PullRecordConflict004 ERROR";
    }
    GTEST_LOG_(INFO) << "PullRecordConflict004 End";
}

/**
 * @tc.name: PullRecordConflict005
 * @tc.desc: Verify the PullRecordConflict function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, PullRecordConflict005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordConflict005 Begin";
    try {
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(0), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_RELATIVE_PATH, DriveKit::DKRecordField("relativePath")));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        data.insert(std::make_pair(DriveKit::DKFieldKey(Media::PhotoColumn::MEDIA_SIZE), 0));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        bool comflag = false;
        int32_t ret = fileDataHandler->PullRecordConflict(record, comflag);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " PullRecordConflict005 ERROR";
    }
    GTEST_LOG_(INFO) << "PullRecordConflict005 End";
}

/**
 * @tc.name: PullRecordConflict006
 * @tc.desc: Verify the PullRecordConflict function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, PullRecordConflict006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordConflict006 Begin";
    try {
        const int rowCount = 2;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_RELATIVE_PATH, DriveKit::DKRecordField("relativePath")));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        data.insert(std::make_pair(DriveKit::DKFieldKey(Media::PhotoColumn::MEDIA_SIZE), 0));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        bool comflag = false;
        int32_t ret = fileDataHandler->PullRecordConflict(record, comflag);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " PullRecordConflict006 ERROR";
    }
    GTEST_LOG_(INFO) << "PullRecordConflict006 End";
}

/**
 * @tc.name: PullRecordConflict007
 * @tc.desc: Verify the PullRecordConflict function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, PullRecordConflict007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordConflict007 Begin";
    try {
        const int rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_RELATIVE_PATH, DriveKit::DKRecordField("relativePath")));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        data.insert(std::make_pair(DriveKit::DKFieldKey(Media::PhotoColumn::MEDIA_SIZE), 0));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        bool comflag = false;
        int32_t ret = fileDataHandler->PullRecordConflict(record, comflag);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " PullRecordConflict007 ERROR";
    }
    GTEST_LOG_(INFO) << "PullRecordConflict007 End";
}

/**
 * @tc.name: PullRecordConflict008
 * @tc.desc: Verify the PullRecordConflict function
 * @tc.type: FUNC
 * @tc.require: issueI7Y6UO
 */
HWTEST_F(FileDataHandlerTest, PullRecordConflict008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordConflict008 Begin";
    try {
        const int rowCount = 1;
        auto rdb = std::make_shared<RdbStoreMock>();
        auto fileDataHandler = make_shared<FileDataHandler>(USER_ID, BUND_NAME, rdb, std::make_shared<bool>(false));
        std::unique_ptr<AbsSharedResultSetMock> rset = std::make_unique<AbsSharedResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*rset, GoToNextRow()).WillOnce(Return(0));
        EXPECT_CALL(*rset, GetColumnIndex(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetString(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rset, GetLong(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*rdb, Query(_, _)).WillOnce(Return(ByMove(std::move(rset))));
        EXPECT_CALL(*rdb, Update(_, _, _, _, A<const vector<string> &>())).WillRepeatedly(Return(1));

        DKRecordData data;
        DriveKit::DKRecordFieldMap prop;
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_FILE_PATH, DriveKit::DKRecordField("fullPath")));
        prop.insert(std::make_pair(Media::PhotoColumn::MEDIA_RELATIVE_PATH, DriveKit::DKRecordField("relativePath")));
        prop.insert(std::make_pair(Media::PhotoColumn::PHOTO_META_DATE_MODIFIED, DriveKit::DKRecordField(1)));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_ATTRIBUTES), prop));
        data.insert(std::make_pair(DriveKit::DKFieldKey(FILE_SIZE), 1));
        data.insert(std::make_pair(DriveKit::DKFieldKey(Media::PhotoColumn::MEDIA_SIZE), 1));
        DriveKit::DKRecord record;
        record.SetRecordData(data);
        bool comflag = false;
        int32_t ret = fileDataHandler->PullRecordConflict(record, comflag);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " PullRecordConflict008 ERROR";
    }
    GTEST_LOG_(INFO) << "PullRecordConflict008 End";
}
