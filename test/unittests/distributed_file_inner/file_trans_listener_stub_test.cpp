/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <file_trans_listener_stub.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dfs_error.h"
#include "file_trans_listener_interface_code.h"
#include "message_parcel_mock.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class MockFileTransListenerStub final : public FileTransListenerStub {
public:
    MOCK_METHOD2(OnFileReceive, int32_t(uint64_t totalBytes, uint64_t processedBytes));
    MOCK_METHOD2(OnFailed, int32_t(const std::string &sessionName, int32_t errorCode));
    MOCK_METHOD1(OnFinished, int32_t(const std::string &sessionName));
};

class FileTransListenerStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<MockFileTransListenerStub> mockStub_ = nullptr;
    static inline shared_ptr<MessageParcelMock> messageParcelMock_ = nullptr;
};

void FileTransListenerStubTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mockStub_ = make_shared<MockFileTransListenerStub>();
    messageParcelMock_ = make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void FileTransListenerStubTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    mockStub_ = nullptr;
    MessageParcelMock::messageParcel = nullptr;
    messageParcelMock_ = nullptr;
}

void FileTransListenerStubTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileTransListenerStubTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FileTransListenerStub_OnRemoteRequest_0100
 * @tc.desc: The execution of the OnRemoteRequest failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileTransListenerStubTest, FileTransListenerStub_OnRemoteRequest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListenerStub_OnRemoteRequest_0100 Start";
    uint32_t code = static_cast<uint32_t>(FileTransListenerInterfaceCode::FILE_TRANS_LISTENER_ON_PROGRESS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(u"test"));
    auto ret = mockStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, FileTransListenerStub::File_Trans_Listener_DESCRIPTOR_IS_EMPTY);

    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(IFileTransListener::GetDescriptor()));
    uint32_t errCode = 65535;
    ret = mockStub_->OnRemoteRequest(errCode, data, reply, option);
    EXPECT_EQ(ret, 305);

    mockStub_->opToInterfaceMap_.insert(make_pair(errCode, nullptr));
    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(IFileTransListener::GetDescriptor()));
    ret = mockStub_->OnRemoteRequest(errCode, data, reply, option);
    EXPECT_EQ(ret, 305);
    mockStub_->opToInterfaceMap_.erase(errCode);
    GTEST_LOG_(INFO) << "FileTransListenerStub_OnRemoteRequest_0100 End";
}

/**
 * @tc.name: FileTransListenerStub_OnRemoteRequest_0200
 * @tc.desc: The execution of the OnRemoteRequest success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileTransListenerStubTest, FileTransListenerStub_OnRemoteRequest_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListenerStub_OnRemoteRequest_0200 Start";
    uint32_t code = static_cast<uint32_t>(FileTransListenerInterfaceCode::FILE_TRANS_LISTENER_ON_PROGRESS);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    EXPECT_CALL(*messageParcelMock_, ReadInterfaceToken()).WillOnce(Return(IFileTransListener::GetDescriptor()));
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mockStub_, OnFileReceive(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    auto ret = mockStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FileTransListenerStub_OnRemoteRequest_0200 End";
}

/**
 * @tc.name: FileTransListenerStub_HandleOnFileReceive_0100
 * @tc.desc: verify HandleOnFileReceive.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileTransListenerStubTest, FileTransListenerStub_HandleOnFileReceive_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListenerStub_HandleOnFileReceive_0100 Start";
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(Return(false));
    auto ret = mockStub_->HandleOnFileReceive(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = mockStub_->HandleOnFileReceive(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mockStub_, OnFileReceive(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
    ret = mockStub_->HandleOnFileReceive(data, reply);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, ReadUint64(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mockStub_, OnFileReceive(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = mockStub_->HandleOnFileReceive(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FileTransListenerStub_HandleOnFileReceive_0100 End";
}

/**
 * @tc.name: FileTransListenerStub_HandleOnFailed_0100
 * @tc.desc: verify HandleOnFileReceive.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileTransListenerStubTest, FileTransListenerStub_HandleOnFailed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListenerStub_HandleOnFailed_0100 Start";
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = mockStub_->HandleOnFailed(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);
    
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
    ret = mockStub_->HandleOnFailed(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mockStub_, OnFailed(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
    ret = mockStub_->HandleOnFailed(data, reply);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mockStub_, OnFailed(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = mockStub_->HandleOnFailed(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FileTransListenerStub_HandleOnFailed_0100 End";
}

/**
 * @tc.name: FileTransListenerStub_HandleOnFinished_0100
 * @tc.desc: verify HandleOnFinished.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileTransListenerStubTest, FileTransListenerStub_HandleOnFinished_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListenerStub_HandleOnFinished_0100 Start";
    MessageParcel data;
    MessageParcel reply;
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(false));
    auto ret = mockStub_->HandleOnFinished(data, reply);
    EXPECT_EQ(ret, E_INVAL_ARG);
    
    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mockStub_, OnFinished(_)).WillOnce(Return(E_PERMISSION_DENIED));
    ret = mockStub_->HandleOnFinished(data, reply);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, ReadString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mockStub_, OnFinished(_)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    ret = mockStub_->HandleOnFinished(data, reply);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FileTransListenerStub_HandleOnFinished_0100 End";
}
}