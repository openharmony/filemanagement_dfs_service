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
#include "distributed_file_daemon_proxy.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dfs_error.h"
#include "file_dfs_listener_mock.h"
#include "i_daemon_mock.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class DistributedFileDaemonProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<DistributedFileDaemonProxy> proxy_ = nullptr;
    static inline sptr<DaemonServiceMock> mock_ = nullptr;
};

void DistributedFileDaemonProxyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    mock_ = sptr(new DaemonServiceMock());
    proxy_ = make_shared<DistributedFileDaemonProxy>(mock_);
}

void DistributedFileDaemonProxyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    mock_ = nullptr;
    proxy_ = nullptr;
}

void DistributedFileDaemonProxyTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DistributedFileDaemonProxyTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DistributedFileDaemon_OpenP2PConnection_0100
 * @tc.desc: The execution of the OpenP2PConnection failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_OpenP2PConnection_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnection_0100 Start";
    DistributedHardware::DmDeviceInfo deviceInfo;
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    auto ret = testProxy->OpenP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnection_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_OpenP2PConnection_0200
 * @tc.desc: The execution of the OpenP2PConnection failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_OpenP2PConnection_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnection_0200 Start";
    DistributedHardware::DmDeviceInfo deviceInfo;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    auto ret = proxy_->OpenP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnection_0200 End";
}

/**
 * @tc.name: DistributedFileDaemon_OpenP2PConnection_0300
 * @tc.desc: The execution of the OpenP2PConnection success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_OpenP2PConnection_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnection_0300 Start";
    DistributedHardware::DmDeviceInfo deviceInfo;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    auto ret = proxy_->OpenP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_OpenP2PConnection_0300 End";
}

/**
 * @tc.name: DistributedFileDaemon_CloseP2PConnection_0100
 * @tc.desc: The execution of the CloseP2PConnection failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_CloseP2PConnection_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnection_0100 Start";
    DistributedHardware::DmDeviceInfo deviceInfo;
    auto testProxy = make_shared<DistributedFileDaemonProxy>(nullptr);
    auto ret = testProxy->CloseP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnection_0100 End";
}

/**
 * @tc.name: DistributedFileDaemon_CloseP2PConnection_0200
 * @tc.desc: The execution of the CloseP2PConnection failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_CloseP2PConnection_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnection_0200 Start";
    DistributedHardware::DmDeviceInfo deviceInfo;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_INVAL_ARG));
    auto ret = proxy_->CloseP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnection_0200 End";
}

/**
 * @tc.name: DistributedFileDaemon_CloseP2PConnection_0300
 * @tc.desc: The execution of the CloseP2PConnection success.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DistributedFileDaemonProxyTest, DistributedFileDaemon_CloseP2PConnection_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnection_0300 Start";
    DistributedHardware::DmDeviceInfo deviceInfo;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).WillOnce(Return(E_OK));
    auto ret = proxy_->CloseP2PConnection(deviceInfo);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DistributedFileDaemon_CloseP2PConnection_0300 End";
}
}