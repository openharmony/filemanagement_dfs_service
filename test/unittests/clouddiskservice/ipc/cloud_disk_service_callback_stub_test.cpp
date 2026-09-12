/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "cloud_disk_service_callback_stub.h"
#include "cloud_disk_service_callback_table_client.h"
#include "cloud_disk_service_callback_table_stub.h"
#include "cloud_disk_service_error.h"
#include "dfs_error.h"
#include "message_parcel_mock.h"

namespace OHOS {
namespace FileManagement::CloudDiskService {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::Storage::DistributedFile;

class MockCloudDiskServiceCallback : public CloudDiskServiceCallbackStub {
public:
    MOCK_METHOD2(OnChangeData, void(const std::string &sandboxPath, const std::vector<ChangeData> &changeData));
};

class MockCloudDiskServiceCallbackTable : public CloudDiskServiceCallbackTableStub {
public:
    MOCK_METHOD2(OnCallback, void(const CloudDiskCallbackReqHead &reqHead, CloudDiskCallbackContext &reqContext));
};

class CloudDiskServiceCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<MockCloudDiskServiceCallback> callback_;
    std::shared_ptr<MessageParcelMock> messageParcelMock_;
};

void CloudDiskServiceCallbackStubTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDiskServiceCallbackStubTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDiskServiceCallbackStubTest::SetUp(void)
{
    callback_ = std::make_shared<MockCloudDiskServiceCallback>();
    messageParcelMock_ = std::make_shared<MessageParcelMock>();
    DfsMessageParcel::messageParcel = messageParcelMock_;
    std::cout << "SetUp" << std::endl;
}

void CloudDiskServiceCallbackStubTest::TearDown(void)
{
    DfsMessageParcel::messageParcel = nullptr;
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: HandleOnChangeDataTest001
 * @tc.desc: Verify HandleOnChangeData function - ReadString(sandboxPath) failed
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, HandleOnChangeDataTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest001 Start";
    try {

        EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
        
        MessageParcel data;
        MessageParcel reply;
        int ret = callback_->HandleOnChangeData(data, reply);
        EXPECT_EQ(ret, E_INVALID_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnChangeDataTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest001 End";
}

/**
 * @tc.name: HandleOnChangeDataTest002
 * @tc.desc: Verify HandleOnChangeData function - ReadInt32(size) failed
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, HandleOnChangeDataTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest002 Start";
    try {

        EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>("path"), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
        
        MessageParcel data;
        MessageParcel reply;
        int ret = callback_->HandleOnChangeData(data, reply);
        EXPECT_EQ(ret, E_INVALID_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnChangeDataTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest002 End";
}

/**
 * @tc.name: HandleOnChangeDataTest003
 * @tc.desc: Verify HandleOnChangeData function - all reads succeed
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, HandleOnChangeDataTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest003 Start";
    try {
        EXPECT_CALL(*callback_, OnChangeData(_, _)).WillOnce(Return());
        
        EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>("path"), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(0), Return(true)));
        
        MessageParcel data;
        MessageParcel reply;
        int ret = callback_->HandleOnChangeData(data, reply);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleOnChangeDataTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnChangeDataTest003 End";
}

/**
 * @tc.name: HandleOnCallbackTest001
 * @tc.desc: Verify callback table stub rejects an invalid callback type.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, HandleOnCallbackTest001, TestSize.Level1)
{
    auto callbackTable = std::make_shared<MockCloudDiskServiceCallbackTable>();
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(DoAll(SetArgReferee<0>("/sync"), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(DoAll(SetArgReferee<0>(99), Return(true)));

    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(callbackTable->HandleOnCallback(data, reply), E_INVALID_ARG);
}

/**
 * @tc.name: HandleOnCallbackTest002
 * @tc.desc: Verify callback table stub decodes and dispatches a fetch-data callback.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, HandleOnCallbackTest002, TestSize.Level1)
{
    auto callbackTable = std::make_shared<MockCloudDiskServiceCallbackTable>();
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>("/sync"), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>("dir/file.txt"), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA)), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(static_cast<int32_t>(CLOUD_DISK_HYDRATE_PRIORITY_HIGH)), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Invoke([](std::vector<uint8_t> *value) {
        *value = {1, 2, 3};
        return true;
    }));
    EXPECT_CALL(*callbackTable, OnCallback(_, _))
        .WillOnce(Invoke([](const CloudDiskCallbackReqHead &reqHead, CloudDiskCallbackContext &context) {
            EXPECT_EQ(reqHead.callbackType, CloudDiskCallbackType::FETCH_DATA);
            ASSERT_NE(context.fetchData, nullptr);
            EXPECT_EQ(std::string(context.fetchData->filePath.value, context.fetchData->filePath.length),
                      "dir/file.txt");
            EXPECT_EQ(context.fetchData->priority, CLOUD_DISK_HYDRATE_PRIORITY_HIGH);
        }));

    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(callbackTable->HandleOnCallback(data, reply), E_OK);
}

/**
 * @tc.name: OnRemoteRequest_001
 * @tc.desc: Verify callback-table stub rejects bad tokens and unknown or empty transactions.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, OnRemoteRequest_001, TestSize.Level2)
{
    auto callbackTable = std::make_shared<MockCloudDiskServiceCallbackTable>();
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(u"invalid"));
    EXPECT_EQ(
        callbackTable->OnRemoteRequest(ICloudDiskServiceCallbackTable::SERVICE_CMD_ON_CALLBACK, data, reply, option),
        E_INVALID_ARG);

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(callbackTable->GetDescriptor()));
    EXPECT_NE(callbackTable->OnRemoteRequest(99, data, reply, option), E_OK);

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    callbackTable->opToInterfaceMap_[ICloudDiskServiceCallbackTable::SERVICE_CMD_ON_CALLBACK] = nullptr;
    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(callbackTable->GetDescriptor()));
    EXPECT_NE(
        callbackTable->OnRemoteRequest(ICloudDiskServiceCallbackTable::SERVICE_CMD_ON_CALLBACK, data, reply, option),
        E_OK);
}

/**
 * @tc.name: HandleOnCallback_003
 * @tc.desc: Verify cancel is one-way and dehydrate writes the callback decision.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, HandleOnCallback_003, TestSize.Level1)
{
    auto callbackTable = std::make_shared<MockCloudDiskServiceCallbackTable>();
    std::string syncFolder = "/sync";
    std::string filePath = "file.txt";
    MessageParcel data;
    MessageParcel reply;

    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(filePath), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(
            DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA)), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Return(true));
    EXPECT_CALL(*callbackTable, OnCallback(_, _))
        .WillOnce(Invoke([&filePath](const CloudDiskCallbackReqHead &head, CloudDiskCallbackContext &context) {
            EXPECT_EQ(head.callbackType, CloudDiskCallbackType::CANCEL_FETCH_DATA);
            ASSERT_NE(context.cancelFetchData, nullptr);
            EXPECT_EQ(std::string(context.cancelFetchData->value, context.cancelFetchData->length), filePath);
        }));
    EXPECT_EQ(callbackTable->HandleOnCallback(data, reply), E_OK);

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    Mock::VerifyAndClearExpectations(callbackTable.get());
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(filePath), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::DEHYDRATE)), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteBool(true)).WillOnce(Return(true));
    EXPECT_CALL(*callbackTable, OnCallback(_, _))
        .WillOnce(Invoke([](const CloudDiskCallbackReqHead &, CloudDiskCallbackContext &context) {
            ASSERT_NE(context.dehydrateData, nullptr);
            context.dehydrateData->allow = true;
        }));
    EXPECT_EQ(callbackTable->HandleOnCallback(data, reply), E_OK);

    Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    Mock::VerifyAndClearExpectations(callbackTable.get());
    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(filePath), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::DEHYDRATE)), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteBool(false)).WillOnce(Return(false));
    EXPECT_CALL(*callbackTable, OnCallback(_, _)).Times(1);
    EXPECT_EQ(callbackTable->HandleOnCallback(data, reply), E_INVALID_ARG);
}

/**
 * @tc.name: HandleOnCallback_004
 * @tc.desc: Verify fetch-data callback rejects unreadable and out-of-range priorities.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, HandleOnCallback_004, TestSize.Level2)
{
    auto callbackTable = std::make_shared<MockCloudDiskServiceCallbackTable>();
    std::string syncFolder = "/sync";
    std::string filePath = "file.txt";
    MessageParcel data;
    MessageParcel reply;
    for (int32_t priority : {-1, static_cast<int32_t>(CLOUD_DISK_HYDRATE_PRIORITY_HIGH) + 1}) {
        EXPECT_CALL(*messageParcelMock_, ReadString(_))
            .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)))
            .WillOnce(DoAll(SetArgReferee<0>(filePath), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
            .WillOnce(DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA)), Return(true)))
            .WillOnce(DoAll(SetArgReferee<0>(priority), Return(true)));
        EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Return(true));
        EXPECT_EQ(callbackTable->HandleOnCallback(data, reply), E_INVALID_ARG);
        Mock::VerifyAndClearExpectations(messageParcelMock_.get());
    }

    EXPECT_CALL(*messageParcelMock_, ReadString(_))
        .WillOnce(DoAll(SetArgReferee<0>(syncFolder), Return(true)))
        .WillOnce(DoAll(SetArgReferee<0>(filePath), Return(true)));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
        .WillOnce(DoAll(SetArgReferee<0>(static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA)), Return(true)))
        .WillOnce(Return(false));
    EXPECT_CALL(*messageParcelMock_, ReadUInt8Vector(_)).WillOnce(Return(true));
    EXPECT_EQ(callbackTable->HandleOnCallback(data, reply), E_INVALID_ARG);
}

/**
 * @tc.name: CallbackTableClient_001
 * @tc.desc: Verify callback-table client handles null, active, inactive, and reactivated delegates.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceCallbackStubTest, CallbackTableClient_001, TestSize.Level1)
{
    CloudDiskCallbackReqHead head{};
    CloudDiskCallbackContext context{};
    auto nullClient = std::make_shared<CloudDiskServiceCallbackTableClient>(nullptr);
    nullClient->OnCallback(head, context);

    auto callbackTable = std::make_shared<MockCloudDiskServiceCallbackTable>();
    auto client = std::make_shared<CloudDiskServiceCallbackTableClient>(callbackTable);
    EXPECT_CALL(*callbackTable, OnCallback(_, _)).Times(1);
    client->OnCallback(head, context);

    Mock::VerifyAndClearExpectations(callbackTable.get());
    client->SetActive(false);
    EXPECT_CALL(*callbackTable, OnCallback(_, _)).Times(0);
    client->OnCallback(head, context);

    Mock::VerifyAndClearExpectations(callbackTable.get());
    client->SetActive(true);
    EXPECT_CALL(*callbackTable, OnCallback(_, _)).Times(1);
    client->OnCallback(head, context);
}

} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS
