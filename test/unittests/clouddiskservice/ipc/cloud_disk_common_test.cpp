/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "cloud_disk_common.h"
#include "message_parcel_mock.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace OHOS::Storage::DistributedFile;

namespace {
constexpr uint64_t TEST_NEXT_USN = 1;
constexpr int32_t TEST_IS_EOF = 1;
constexpr int32_t TEST_CHANGE_DATA_SIZE = 1;
constexpr uint64_t TEST_UPDATE_SEQUENCE_NUMBER = 2;
constexpr uint64_t TEST_FILE_SIZE = 1024;
constexpr uint64_t TEST_MTIME = 123456;
constexpr uint64_t TEST_TIME_STAMP = 654321;
const std::string TEST_FILE_ID = "fileId";
const std::string TEST_PARENT_FILE_ID = "parentFileId";
const std::string TEST_RELATIVE_PATH = "dir/file.txt";
} // namespace

class CloudDiskCommonTest : public testing::Test {
public:
    void SetUp() override
    {
        messageParcelMock_ = std::make_shared<MessageParcelMock>();
        DfsMessageParcel::messageParcel = messageParcelMock_;
    }

    void TearDown() override
    {
        Mock::VerifyAndClearExpectations(messageParcelMock_.get());
        DfsMessageParcel::messageParcel = nullptr;
        messageParcelMock_ = nullptr;
    }

    std::shared_ptr<MessageParcelMock> messageParcelMock_;
};

/**
 * @tc.name: ChangesResultReadFromParcelTest001
 * @tc.desc: Verify ChangesResult ReadFromParcel fails when nextUsn read fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, ChangesResultReadFromParcelTest001, TestSize.Level1)
{
    MessageParcel parcel;
    ChangesResult result;

    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(Return(false));

    EXPECT_FALSE(result.ReadFromParcel(parcel));
    EXPECT_TRUE(result.changesData.empty());
}

/**
 * @tc.name: ChangesResultReadFromParcelTest002
 * @tc.desc: Verify ChangesResult ReadFromParcel fails when isEof read fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, ChangesResultReadFromParcelTest002, TestSize.Level1)
{
    MessageParcel parcel;
    ChangesResult result;

    InSequence sequence;
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_NEXT_USN), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));

    EXPECT_FALSE(result.ReadFromParcel(parcel));
    EXPECT_EQ(result.nextUsn, TEST_NEXT_USN);
    EXPECT_TRUE(result.changesData.empty());
}

/**
 * @tc.name: ChangesResultReadFromParcelTest003
 * @tc.desc: Verify ChangesResult ReadFromParcel fails when changesData size read fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, ChangesResultReadFromParcelTest003, TestSize.Level1)
{
    MessageParcel parcel;
    ChangesResult result;

    InSequence sequence;
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_NEXT_USN), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_IS_EOF), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));

    EXPECT_FALSE(result.ReadFromParcel(parcel));
    EXPECT_EQ(result.nextUsn, TEST_NEXT_USN);
    EXPECT_EQ(result.isEof, static_cast<bool>(TEST_IS_EOF));
    EXPECT_TRUE(result.changesData.empty());
}

/**
 * @tc.name: ChangesResultReadFromParcelTest004
 * @tc.desc: Verify ChangesResult ReadFromParcel succeeds when changesData is empty
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, ChangesResultReadFromParcelTest004, TestSize.Level1)
{
    MessageParcel parcel;
    ChangesResult result;
    constexpr int32_t emptyChangeDataSize = 0;

    InSequence sequence;
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_NEXT_USN), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_IS_EOF), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(emptyChangeDataSize), Return(true)));

    EXPECT_TRUE(result.ReadFromParcel(parcel));
    EXPECT_EQ(result.nextUsn, TEST_NEXT_USN);
    EXPECT_EQ(result.isEof, static_cast<bool>(TEST_IS_EOF));
    EXPECT_TRUE(result.changesData.empty());
}

/**
 * @tc.name: ChangesResultReadFromParcelTest005
 * @tc.desc: Verify ChangesResult ReadFromParcel succeeds with one ChangeData item
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, ChangesResultReadFromParcelTest005, TestSize.Level1)
{
    MessageParcel parcel;
    ChangesResult result;
    constexpr uint8_t operationType = static_cast<uint8_t>(OperationType::CLOSE_WRITE);

    InSequence sequence;
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_NEXT_USN), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_IS_EOF), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_CHANGE_DATA_SIZE), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_UPDATE_SEQUENCE_NUMBER), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_FILE_ID), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_PARENT_FILE_ID), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_RELATIVE_PATH), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint8(_))
        .WillOnce(DoAll(SetArgReferee<0>(operationType), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_FILE_SIZE), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_MTIME), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_TIME_STAMP), Return(true)));

    EXPECT_TRUE(result.ReadFromParcel(parcel));
    ASSERT_EQ(result.changesData.size(), TEST_CHANGE_DATA_SIZE);
    EXPECT_EQ(result.nextUsn, TEST_NEXT_USN);
    EXPECT_EQ(result.isEof, static_cast<bool>(TEST_IS_EOF));
    EXPECT_EQ(result.changesData[0].updateSequenceNumber, TEST_UPDATE_SEQUENCE_NUMBER);
    EXPECT_EQ(result.changesData[0].fileId, TEST_FILE_ID);
    EXPECT_EQ(result.changesData[0].parentFileId, TEST_PARENT_FILE_ID);
    EXPECT_EQ(result.changesData[0].relativePath, TEST_RELATIVE_PATH);
    EXPECT_EQ(result.changesData[0].operationType, OperationType::CLOSE_WRITE);
    EXPECT_EQ(result.changesData[0].size, TEST_FILE_SIZE);
    EXPECT_EQ(result.changesData[0].mtime, TEST_MTIME);
    EXPECT_EQ(result.changesData[0].timeStamp, TEST_TIME_STAMP);
}

/**
 * @tc.name: ChangesResultReadFromParcelTest006
 * @tc.desc: Verify ChangesResult ReadFromParcel skips failed ChangeData and continues
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, ChangesResultReadFromParcelTest006, TestSize.Level1)
{
    MessageParcel parcel;
    ChangesResult result;
    constexpr int32_t changeDataSize = 2;
    constexpr uint8_t operationType = static_cast<uint8_t>(OperationType::CLOSE_WRITE);

    InSequence sequence;
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_NEXT_USN), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_IS_EOF), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(changeDataSize), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_UPDATE_SEQUENCE_NUMBER), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_UPDATE_SEQUENCE_NUMBER), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_FILE_ID), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_PARENT_FILE_ID), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_RELATIVE_PATH), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint8(_))
        .WillOnce(DoAll(SetArgReferee<0>(operationType), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_FILE_SIZE), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_MTIME), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_TIME_STAMP), Return(true)));

    EXPECT_TRUE(result.ReadFromParcel(parcel));
    ASSERT_EQ(result.changesData.size(), TEST_CHANGE_DATA_SIZE);
    EXPECT_EQ(result.changesData[0].fileId, TEST_FILE_ID);
    EXPECT_EQ(result.changesData[0].operationType, OperationType::CLOSE_WRITE);
}

/**
 * @tc.name: PlaceholderCustomInfoParcelTest001
 * @tc.desc: Verify non-empty placeholder custom info parcel round trip
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, PlaceholderCustomInfoParcelTest001, TestSize.Level1)
{
    MessageParcel parcel;
    PlaceholderCustomInfo info;
    info.data = {0x00, 0x7F, 0x80, 0xFF};

    EXPECT_CALL(*messageParcelMock_, WriteUInt8Vector(info.data)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_))
        .WillOnce(DoAll(SetArgPointee<0>(info.data), Return(true)));

    EXPECT_TRUE(info.Marshalling(parcel));
    std::unique_ptr<PlaceholderCustomInfo> result(PlaceholderCustomInfo::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->data, info.data);
}

/**
 * @tc.name: PlaceholderCustomInfoParcelTest002
 * @tc.desc: Verify empty placeholder custom info parcel round trip
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, PlaceholderCustomInfoParcelTest002, TestSize.Level1)
{
    MessageParcel parcel;
    PlaceholderCustomInfo info;

    EXPECT_CALL(*messageParcelMock_, WriteUInt8Vector(info.data)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_))
        .WillOnce(DoAll(SetArgPointee<0>(info.data), Return(true)));

    EXPECT_TRUE(info.Marshalling(parcel));
    std::unique_ptr<PlaceholderCustomInfo> result(PlaceholderCustomInfo::Unmarshalling(parcel));
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->data.empty());
}

/**
 * @tc.name: PlaceholderCustomInfoParcelTest003
 * @tc.desc: Verify placeholder custom info marshalling failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, PlaceholderCustomInfoParcelTest003, TestSize.Level1)
{
    MessageParcel parcel;
    PlaceholderCustomInfo info;
    info.data = {1};

    EXPECT_CALL(*messageParcelMock_, WriteUInt8Vector(info.data)).WillOnce(Return(false));
    EXPECT_FALSE(info.Marshalling(parcel));
}

/**
 * @tc.name: PlaceholderCustomInfoParcelTest004
 * @tc.desc: Verify placeholder custom info unmarshalling failure
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, PlaceholderCustomInfoParcelTest004, TestSize.Level1)
{
    MessageParcel parcel;

    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Return(false));
    std::unique_ptr<PlaceholderCustomInfo> result(PlaceholderCustomInfo::Unmarshalling(parcel));
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name: FetchDataCallbackParcelTest001
 * @tc.desc: Verify fetch-data callback request priority marshalling and unmarshalling.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, FetchDataCallbackParcelTest001, TestSize.Level1)
{
    MessageParcel parcel;
    std::string syncFolder = "/sync";
    std::string filePath = "dir/file.txt";
    CloudDiskCallbackReqHead reqHead{{syncFolder.data(), syncFolder.length()}, CloudDiskCallbackType::FETCH_DATA,
                                     {nullptr, 0}};
    CloudDiskFetchDataRequest request{{filePath.data(), filePath.length()}, CLOUD_DISK_HYDRATE_PRIORITY_HIGH};
    CloudDiskCallbackContext context{};
    context.fetchData = &request;

    EXPECT_CALL(*messageParcelMock_, WriteString(syncFolder)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA)))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUInt8Vector(std::vector<uint8_t>{})).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(filePath)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(static_cast<int32_t>(CLOUD_DISK_HYDRATE_PRIORITY_HIGH)))
        .WillOnce(Return(true));
    EXPECT_TRUE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(filePath), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA)), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(static_cast<int32_t>(CLOUD_DISK_HYDRATE_PRIORITY_HIGH)), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Invoke([](std::vector<uint8_t> *data) {
        data->clear();
        return true;
    }));
    CloudDiskCallbackReqHead decodedHead{};
    CloudDiskCallbackContext decodedContext{};
    CallbackParcelStorage storage;
    ASSERT_TRUE(ReadCallbackParcel(parcel, decodedHead, decodedContext, storage));
    ASSERT_NE(decodedContext.fetchData, nullptr);
    EXPECT_EQ(decodedHead.callbackType, CloudDiskCallbackType::FETCH_DATA);
    EXPECT_EQ(std::string(decodedContext.fetchData->filePath.value, decodedContext.fetchData->filePath.length),
              filePath);
    EXPECT_EQ(decodedContext.fetchData->priority, CLOUD_DISK_HYDRATE_PRIORITY_HIGH);
}

/**
 * @tc.name: DehydrateCallbackParcelTest001
 * @tc.desc: Verify dehydrate callback request marshalling and unmarshalling.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, DehydrateCallbackParcelTest001, TestSize.Level1)
{
    MessageParcel parcel;
    std::string syncFolder = "/sync";
    std::string filePath = "dir/file.txt";
    CloudDiskCallbackReqHead reqHead{{syncFolder.data(), syncFolder.length()}, CloudDiskCallbackType::DEHYDRATE,
                                     {nullptr, 0}};
    CloudDiskDehydrateInfo info{{filePath.data(), filePath.length()}, false};
    CloudDiskCallbackContext context{};
    context.dehydrateData = &info;

    EXPECT_CALL(*messageParcelMock_, WriteString(syncFolder)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(static_cast<int32_t>(CloudDiskCallbackType::DEHYDRATE)))
        .WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUInt8Vector(std::vector<uint8_t>{})).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(filePath)).WillOnce(Return(true));
    EXPECT_TRUE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(filePath), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::DEHYDRATE)), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Invoke([](std::vector<uint8_t> *data) {
        data->clear();
        return true;
    }));
    CloudDiskCallbackReqHead decodedHead{};
    CloudDiskCallbackContext decodedContext{};
    CallbackParcelStorage storage;
    ASSERT_TRUE(ReadCallbackParcel(parcel, decodedHead, decodedContext, storage));
    ASSERT_NE(decodedContext.dehydrateData, nullptr);
    EXPECT_EQ(decodedHead.callbackType, CloudDiskCallbackType::DEHYDRATE);
    EXPECT_EQ(std::string(decodedContext.dehydrateData->filePath.value,
                          decodedContext.dehydrateData->filePath.length), filePath);
    EXPECT_FALSE(decodedContext.dehydrateData->allow);
}

/**
 * @tc.name: DehydrateCallbackReplyTest001
 * @tc.desc: Verify dehydrate authorization is written to and read from the callback reply.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, DehydrateCallbackReplyTest001, TestSize.Level1)
{
    MessageParcel parcel;
    CloudDiskDehydrateInfo info{{nullptr, 0}, true};
    CloudDiskCallbackContext context{};
    context.dehydrateData = &info;

    EXPECT_CALL(*messageParcelMock_, WriteBool(true)).WillOnce(Return(true));
    EXPECT_TRUE(WriteCallbackReply(parcel, CloudDiskCallbackType::DEHYDRATE, context, 0));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(DoAll(SetArgReferee<0>(false), Return(true)));
    EXPECT_TRUE(ReadCallbackReply(parcel, CloudDiskCallbackType::DEHYDRATE, context));
    EXPECT_FALSE(info.allow);
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
