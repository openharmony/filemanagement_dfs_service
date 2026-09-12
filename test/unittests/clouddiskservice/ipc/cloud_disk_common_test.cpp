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

#include <limits>
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
constexpr int32_t UNSUPPORTED_CALLBACK_TYPE = 3;
static_assert(static_cast<int32_t>(CloudDiskCallbackType::DEHYDRATE) == 2);
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

void ExpectCallbackHeader(MessageParcelMock &mock,
                          const std::string &syncFolder,
                          CloudDiskCallbackType callbackType,
                          const std::vector<uint8_t> &reqKey)
{
    EXPECT_CALL(mock, WriteString(syncFolder)).WillOnce(Return(true));
    EXPECT_CALL(mock, WriteInt32(static_cast<int32_t>(callbackType))).WillOnce(Return(true));
    EXPECT_CALL(mock, WriteUInt8Vector(reqKey)).WillOnce(Return(true));
}

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
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_NEXT_USN), Return(true)));
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
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_NEXT_USN), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_IS_EOF), Return(true)));
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
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_NEXT_USN), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_IS_EOF), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(emptyChangeDataSize), Return(true)));

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
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_NEXT_USN), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_IS_EOF), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_CHANGE_DATA_SIZE), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_UPDATE_SEQUENCE_NUMBER), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_FILE_ID), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_PARENT_FILE_ID), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_RELATIVE_PATH), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint8(_)).WillOnce(DoAll(SetArgReferee<0>(operationType), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_FILE_SIZE), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_MTIME), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_TIME_STAMP), Return(true)));

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
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_NEXT_USN), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_IS_EOF), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(changeDataSize), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_UPDATE_SEQUENCE_NUMBER), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_UPDATE_SEQUENCE_NUMBER), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_FILE_ID), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(TEST_PARENT_FILE_ID), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_RELATIVE_PATH), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint8(_)).WillOnce(DoAll(SetArgReferee<0>(operationType), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_FILE_SIZE), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_MTIME), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(DoAll(SetArgReferee<0>(TEST_TIME_STAMP), Return(true)));

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
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(DoAll(SetArgPointee<0>(info.data), Return(true)));

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
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(DoAll(SetArgPointee<0>(info.data), Return(true)));

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
    CloudDiskCallbackReqHead reqHead{
        {syncFolder.data(), syncFolder.length()}, CloudDiskCallbackType::FETCH_DATA, {nullptr, 0}};
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
    CloudDiskCallbackReqHead reqHead{
        {syncFolder.data(), syncFolder.length()}, CloudDiskCallbackType::DEHYDRATE, {nullptr, 0}};
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
    EXPECT_EQ(std::string(decodedContext.dehydrateData->filePath.value, decodedContext.dehydrateData->filePath.length),
              filePath);
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
    EXPECT_TRUE(WriteCallbackReply(parcel, CloudDiskCallbackType::DEHYDRATE, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(DoAll(SetArgReferee<0>(false), Return(true)));
    EXPECT_TRUE(ReadCallbackReply(parcel, CloudDiskCallbackType::DEHYDRATE, context));
    EXPECT_FALSE(info.allow);
}

/**
 * @tc.name: UnsupportedCallbackTypeParcelTest001
 * @tc.desc: Verify an unsupported callback type value is rejected during unmarshalling.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, UnsupportedCallbackTypeParcelTest001, TestSize.Level1)
{
    MessageParcel parcel;
    std::string syncFolder = "/sync";
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(UNSUPPORTED_CALLBACK_TYPE), Return(true)));

    CloudDiskCallbackReqHead reqHead{};
    CloudDiskCallbackContext context{};
    CallbackParcelStorage storage;
    EXPECT_FALSE(ReadCallbackParcel(parcel, reqHead, context, storage));
}

/**
 * @tc.name: WriteCallbackParcel_001
 * @tc.desc: Verify callback request header validation and parcel write failures.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, WriteCallbackParcel_001, TestSize.Level2)
{
    MessageParcel parcel;
    std::string syncFolder = "/sync";
    CloudDiskCallbackReqHead reqHead{{nullptr, 1}, CloudDiskCallbackType::CANCEL_FETCH_DATA, {nullptr, 0}};
    CloudDiskCallbackContext context{};
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));

    reqHead.syncFolderPath = {syncFolder.data(), syncFolder.size()};
    EXPECT_CALL(*messageParcelMock_, WriteString(syncFolder)).WillOnce(Return(false));
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    EXPECT_CALL(*messageParcelMock_, WriteString(syncFolder)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(static_cast<int32_t>(reqHead.callbackType))).WillOnce(Return(false));
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    reqHead.reqKey = {nullptr, 1};
    EXPECT_CALL(*messageParcelMock_, WriteString(syncFolder)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(static_cast<int32_t>(reqHead.callbackType))).WillOnce(Return(true));
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    reqHead.reqKey = {nullptr, 0};
    EXPECT_CALL(*messageParcelMock_, WriteString(syncFolder)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(static_cast<int32_t>(reqHead.callbackType))).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUInt8Vector(std::vector<uint8_t>{})).WillOnce(Return(false));
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));
}

/**
 * @tc.name: WriteCallbackParcel_003
 * @tc.desc: Reject a data-buffer size that cannot be represented by size_t on 32-bit targets.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, WriteCallbackParcel_003, TestSize.Level2)
{
    if (sizeof(size_t) >= sizeof(uint64_t)) {
        GTEST_SKIP() << "The uint64_t-to-size_t overflow branch is specific to 32-bit targets";
    }

    MessageParcel parcel;
    std::string syncFolder = "/sync";
    uint8_t reqKey = 1;
    CloudDiskCallbackReqHead reqHead{{syncFolder.data(), syncFolder.size()},
                                     CloudDiskCallbackType::CANCEL_FETCH_DATA,
                                     {&reqKey, static_cast<uint64_t>(std::numeric_limits<size_t>::max()) + 1}};
    CloudDiskCallbackContext context{};
    EXPECT_CALL(*messageParcelMock_, WriteString(syncFolder)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(static_cast<int32_t>(reqHead.callbackType))).WillOnce(Return(true));
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));
}

/**
 * @tc.name: WriteCallbackParcel_002
 * @tc.desc: Verify null callback contexts and every callback-specific write branch.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, WriteCallbackParcel_002, TestSize.Level2)
{
    MessageParcel parcel;
    std::string syncFolder = "/sync";
    std::string filePath = "file.txt";
    std::vector<uint8_t> reqKey{1, 2};
    CloudDiskCallbackReqHead reqHead{
        {syncFolder.data(), syncFolder.size()}, CloudDiskCallbackType::FETCH_DATA, {reqKey.data(), reqKey.size()}};
    CloudDiskCallbackContext context{};
    ExpectCallbackHeader(*messageParcelMock_, syncFolder, reqHead.callbackType, reqKey);
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    CloudDiskFetchDataRequest fetchRequest{{nullptr, 1}, CLOUD_DISK_HYDRATE_PRIORITY_NORMAL};
    context.fetchData = &fetchRequest;
    ExpectCallbackHeader(*messageParcelMock_, syncFolder, reqHead.callbackType, reqKey);
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    fetchRequest.filePath = {filePath.data(), filePath.size()};
    ExpectCallbackHeader(*messageParcelMock_, syncFolder, reqHead.callbackType, reqKey);
    EXPECT_CALL(*messageParcelMock_, WriteString(filePath)).WillOnce(Return(false));
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    ExpectCallbackHeader(*messageParcelMock_, syncFolder, reqHead.callbackType, reqKey);
    EXPECT_CALL(*messageParcelMock_, WriteString(filePath)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(static_cast<int32_t>(fetchRequest.priority))).WillOnce(Return(false));
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    reqHead.callbackType = CloudDiskCallbackType::CANCEL_FETCH_DATA;
    context.cancelFetchData = nullptr;
    ExpectCallbackHeader(*messageParcelMock_, syncFolder, reqHead.callbackType, reqKey);
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    CloudDiskPathInfo pathInfo{filePath.data(), filePath.size()};
    context.cancelFetchData = &pathInfo;
    ExpectCallbackHeader(*messageParcelMock_, syncFolder, reqHead.callbackType, reqKey);
    EXPECT_CALL(*messageParcelMock_, WriteString(filePath)).WillOnce(Return(true));
    EXPECT_TRUE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    reqHead.callbackType = CloudDiskCallbackType::DEHYDRATE;
    context.dehydrateData = nullptr;
    ExpectCallbackHeader(*messageParcelMock_, syncFolder, reqHead.callbackType, reqKey);
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    reqHead.callbackType = static_cast<CloudDiskCallbackType>(UNSUPPORTED_CALLBACK_TYPE);
    ExpectCallbackHeader(*messageParcelMock_, syncFolder, reqHead.callbackType, reqKey);
    EXPECT_FALSE(WriteCallbackParcel(parcel, reqHead, context));
}

/**
 * @tc.name: ReadCallbackParcel_001
 * @tc.desc: Verify each callback request header read failure is rejected.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, ReadCallbackParcel_001, TestSize.Level2)
{
    MessageParcel parcel;
    std::string syncFolder = "/sync";
    CloudDiskCallbackReqHead reqHead{};
    CloudDiskCallbackContext context{};
    CallbackParcelStorage storage;

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    EXPECT_FALSE(ReadCallbackParcel(parcel, reqHead, context, storage));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    EXPECT_FALSE(ReadCallbackParcel(parcel, reqHead, context, storage));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(
            DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA)), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Return(false));
    EXPECT_FALSE(ReadCallbackParcel(parcel, reqHead, context, storage));

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)))
        .WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(
            DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA)), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Return(true));
    EXPECT_FALSE(ReadCallbackParcel(parcel, reqHead, context, storage));
}

/**
 * @tc.name: ReadCallbackParcel_002
 * @tc.desc: Verify cancel decoding and fetch priority validation boundaries.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, ReadCallbackParcel_002, TestSize.Level2)
{
    MessageParcel parcel;
    std::string syncFolder = "/sync";
    std::string emptyPath;
    std::vector<uint8_t> reqKey{4, 5};
    CloudDiskCallbackReqHead reqHead{};
    CloudDiskCallbackContext context{};
    CallbackParcelStorage storage;

    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(emptyPath), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(
            DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA)), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(DoAll(SetArgPointee<0>(reqKey), Return(true)));
    ASSERT_TRUE(ReadCallbackParcel(parcel, reqHead, context, storage));
    ASSERT_NE(context.cancelFetchData, nullptr);
    EXPECT_EQ(context.cancelFetchData->value, nullptr);
    EXPECT_EQ(context.cancelFetchData->length, 0U);
    ASSERT_NE(reqHead.reqKey.data, nullptr);
    EXPECT_EQ(reqHead.reqKey.dataSize, reqKey.size());

    for (int32_t priority : {-1, static_cast<int32_t>(CLOUD_DISK_HYDRATE_PRIORITY_HIGH) + 1}) {
        Mock::VerifyAndClearExpectations(messageParcelMock_.get());
        std::string filePath = "file.txt";
        EXPECT_CALL(*messageParcelMock_, ReadString(_))
            .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)))
            .WillOnce(DoAll(SetArgReferee<0>(filePath), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
            .WillOnce(DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA)), Return(true)))
            .WillOnce(DoAll(SetArgReferee<0>(priority), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Return(true));
        CallbackParcelStorage invalidStorage;
        EXPECT_FALSE(ReadCallbackParcel(parcel, reqHead, context, invalidStorage));
    }

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    std::string filePath = "file.txt";
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(filePath), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA)), Return(true)))
        .WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Return(true));
    CallbackParcelStorage invalidStorage;
    EXPECT_FALSE(ReadCallbackParcel(parcel, reqHead, context, invalidStorage));
}

/**
 * @tc.name: CallbackReply_001
 * @tc.desc: Verify reply handling for one-way, dehydrate, null-context and invalid callback types.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskCommonTest, CallbackReply_001, TestSize.Level2)
{
    MessageParcel parcel;
    CloudDiskCallbackContext context{};
    EXPECT_TRUE(WriteCallbackReply(parcel, CloudDiskCallbackType::FETCH_DATA, context));
    EXPECT_TRUE(ReadCallbackReply(parcel, CloudDiskCallbackType::FETCH_DATA, context));
    EXPECT_TRUE(WriteCallbackReply(parcel, CloudDiskCallbackType::CANCEL_FETCH_DATA, context));
    EXPECT_TRUE(ReadCallbackReply(parcel, CloudDiskCallbackType::CANCEL_FETCH_DATA, context));
    EXPECT_FALSE(WriteCallbackReply(parcel, CloudDiskCallbackType::DEHYDRATE, context));
    EXPECT_FALSE(ReadCallbackReply(parcel, CloudDiskCallbackType::DEHYDRATE, context));
    EXPECT_FALSE(WriteCallbackReply(parcel, static_cast<CloudDiskCallbackType>(UNSUPPORTED_CALLBACK_TYPE), context));
    EXPECT_FALSE(ReadCallbackReply(parcel, static_cast<CloudDiskCallbackType>(UNSUPPORTED_CALLBACK_TYPE), context));

    CloudDiskDehydrateInfo info{{nullptr, 0}, true};
    context.dehydrateData = &info;
    EXPECT_CALL(*messageParcelMock_, WriteBool(true)).WillOnce(Return(false));
    EXPECT_FALSE(WriteCallbackReply(parcel, CloudDiskCallbackType::DEHYDRATE, context));
    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    EXPECT_CALL(*messageParcelMock_, ReadBool(_)).WillOnce(Return(false));
    EXPECT_FALSE(ReadCallbackReply(parcel, CloudDiskCallbackType::DEHYDRATE, context));
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
