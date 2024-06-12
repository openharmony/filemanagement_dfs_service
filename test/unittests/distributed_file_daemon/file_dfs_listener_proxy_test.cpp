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
#include "file_dfs_listener_proxy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dfs_error.h"
#include "file_dfs_listener_mock.h"
#include "message_parcel_mock.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class FileDfsListenerProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<FileDfsListenerProxy> proxy_ = nullptr;
    static inline sptr<FileDfsListenerMock> mock_ = nullptr;
    static inline shared_ptr<MessageParcelMock> messageParcelMock_ = nullptr;
};

void FileDfsListenerProxyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mock_ = sptr(new FileDfsListenerMock());
    proxy_ = make_shared<FileDfsListenerProxy>(mock_);
    messageParcelMock_ = make_shared<MessageParcelMock>();
    MessageParcelMock::messageParcel = messageParcelMock_;
}

void FileDfsListenerProxyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    mock_ = nullptr;
    proxy_ = nullptr;
    MessageParcelMock::messageParcel = nullptr;
    messageParcelMock_ = nullptr;
}

void FileDfsListenerProxyTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileDfsListenerProxyTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: FileDfsListener_OnStatus_0100
 * @tc.desc: The execution of the OnStatus failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileDfsListenerProxyTest, FileDfsListener_OnStatus_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileDfsListener_OnStatus_0100 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(false));
    proxy_->OnStatus("test", 0);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(false));
    proxy_->OnStatus("test", 0);

    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    proxy_->OnStatus("test", 0);
    GTEST_LOG_(INFO) << "FileDfsListener_OnStatus_0100 End";
}

/**
 * @tc.name: FileTransListener_OnStatus_0200
 * @tc.desc: The execution of the OnStatus remote failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileDfsListenerProxyTest, FileTransListener_OnStatus_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListener_OnStatus_0200 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(false));
    auto testProxy = make_shared<FileDfsListenerProxy>(nullptr);
    testProxy->OnStatus("test", 0);
    GTEST_LOG_(INFO) << "FileTransListener_OnStatus_0200 End";
}

/**
 * @tc.name: FileTransListener_OnStatus_0300
 * @tc.desc: The execution of the OnStatus SendRequest failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileDfsListenerProxyTest, FileTransListener_OnStatus_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListener_OnStatus_0300 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    proxy_->OnStatus("test", 0);
    GTEST_LOG_(INFO) << "FileTransListener_OnStatus_0300 End";
}

/**
 * @tc.name: FileTransListener_OnStatus_0400
 * @tc.desc: The execution of the OnStatus succeeded.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileDfsListenerProxyTest, FileTransListener_OnStatus_0400, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileTransListener_OnStatus_0400 Start";
    EXPECT_CALL(*messageParcelMock_, WriteInterfaceToken(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteString(_)).WillOnce(Return(true));
    EXPECT_CALL(*messageParcelMock_, WriteInt32(_)).WillOnce(Return(true));
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    proxy_->OnStatus("test", 0);
    GTEST_LOG_(INFO) << "FileTransListener_OnStatus_0400 End";
}
}