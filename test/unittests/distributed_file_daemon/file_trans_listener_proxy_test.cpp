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
#include "file_trans_listener_proxy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "file_trans_listener_mock.h"
#include "message_parcel_mock.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class FileTransListenerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<FileTransListenerProxy> proxy_ = nullptr;
    static inline sptr<FileTransListenerMock> mock_ = nullptr;
    static inline shared_ptr<MessageParcelMock> messageParcelMock_ = nullptr;
};

void FileTransListenerProxyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mock_ = sptr(new FileTransListenerMock());
    proxy_ = make_shared<FileTransListenerProxy>(mock_);
    messageParcelMock_ = make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void FileTransListenerProxyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    mock_ = nullptr;
    proxy_ = nullptr;
    MessageParcelMock::messageParcel = nullptr;
    messageParcelMock_ = nullptr;
}

void FileTransListenerProxyTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileTransListenerProxyTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FileTransListener_OnFileReceive_0100
 * @tc.desc: Verify the OnFileReceive function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileTransListenerProxyTest, FileTransListener_OnFileReceive_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListener_OnFileReceive_0100 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->OnFileReceive(0, 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(_)).WillOnce(Return(false));
    ret = proxy_->OnFileReceive(0, 0);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(_)).WillOnce(Return(true)).WillOnce(Return(false));
    ret = proxy_->OnFileReceive(0, 0);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(_)).WillOnce(Return(true)).WillOnce(Return(true));
    auto testProxy = make_shared<FileTransListenerProxy>(nullptr);
    ret = testProxy->OnFileReceive(0, 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "FileTransListener_OnFileReceive_0100 End";
}

/**
 * @tc.name: FileTransListener_OnFileReceive_0200
 * @tc.desc: Verify the OnFileReceive function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileTransListenerProxyTest, FileTransListener_OnFileReceive_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListener_OnFileReceive_0200 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_INVAL_ARG));
    int32_t ret = proxy_->OnFileReceive(0, 0);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteUint64(_)).WillOnce(Return(true)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->OnFileReceive(0, 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "FileTransListener_OnFileReceive_0200 End";
}

/**
 * @tc.name: FileTransListener_OnFailed_0100
 * @tc.desc: Verify the OnFailed function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileTransListenerProxyTest, FileTransListener_OnFailed_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListener_OnFailed_0100 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->OnFailed("test", 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->OnFailed("test", 0);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    ret = proxy_->OnFailed("test", 0);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    auto testProxy = make_shared<FileTransListenerProxy>(nullptr);
    ret = testProxy->OnFailed("test", 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);

    GTEST_LOG_(INFO) << "FileTransListener_OnFailed_0100 End";
}

/**
 * @tc.name: FileTransListener_OnFailed_0200
 * @tc.desc: Verify the OnFailed function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileTransListenerProxyTest, FileTransListener_OnFailed_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListener_OnFailed_0200 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_INVAL_ARG));
    auto ret = proxy_->OnFailed("test", 0);
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->OnFailed("test", 0);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "FileTransListener_OnFailed_0200 End";
}

/**
 * @tc.name: FileTransListener_OnFinished_0100
 * @tc.desc: Verify the OnFinished function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileTransListenerProxyTest, FileTransListener_OnFinished_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListener_OnFinished_0100 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    auto ret = proxy_->OnFinished("test");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    ret = proxy_->OnFinished("test");
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    auto testProxy = make_shared<FileTransListenerProxy>(nullptr);
    ret = testProxy->OnFinished("test");
    EXPECT_EQ(ret, E_BROKEN_IPC);

    GTEST_LOG_(INFO) << "FileTransListener_OnFinished_0100 End";
}

/**
 * @tc.name: FileTransListener_OnFinished_0200
 * @tc.desc: Verify the OnFinished function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileTransListenerProxyTest, FileTransListener_OnFinished_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListener_OnFinished_0200 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*messageParcelMock_, ReadInt32()).WillOnce(Return(E_INVAL_ARG));
    auto ret = proxy_->OnFinished("test");
    EXPECT_EQ(ret, E_INVAL_ARG);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    ret = proxy_->OnFinished("test");
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "FileTransListener_OnFinished_0200 End";
}
}