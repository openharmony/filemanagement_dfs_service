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

#include <gtest/gtest.h>
#include <memory>

#include "dfs_error.h"
#include "distributed_file_daemon_proxy.h"
#include "ipc/hmdfs_info.h"
#include "i_daemon_mock.h"
#include "utils_log.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

namespace {
    DistributedHardware::DmDeviceInfo deviceInfo = {
        .deviceId = "testdevid",
        .deviceName = "testdevname",
        .networkId = "testnetworkid",
    };
}

class DistributedDaemonProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<DistributedFileDaemonProxy> proxy_ = nullptr;
    sptr<DaemonServiceMock> mock_ = nullptr;
};

void DistributedDaemonProxyTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DistributedDaemonProxyTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DistributedDaemonProxyTest::SetUp(void)
{
    mock_ = sptr(new DaemonServiceMock());
    proxy_ = make_shared<DistributedFileDaemonProxy>(mock_);
    GTEST_LOG_(INFO) << "SetUp";
}

void DistributedDaemonProxyTest::TearDown(void)
{
    mock_ = nullptr;
    proxy_ = nullptr;
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ConnectDfsTest
 * @tc.desc: Verify the ConnectDfs function.
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonProxyTest, ConnectDfsTest, TestSize.Level0)
{
    std::string networkId = "ConnectDfsTest";
    GTEST_LOG_(INFO) << "ConnectDfsTest Start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    int ret = proxy_->ConnectDfs(networkId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "ConnectDfsTest End";
}

/**
 * @tc.name: DisconnectDfsTest
 * @tc.desc: Verify the DisconnectDfs function.
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonProxyTest, DisconnectDfsTest, TestSize.Level0)
{
    std::string networkId = "DisconnectDfsTest";
    GTEST_LOG_(INFO) << "DisconnectDfsTest Start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    int ret = proxy_->DisconnectDfs(networkId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "DisconnectDfsTest End";
}

/**
 * @tc.name: PrepareSessionTest
 * @tc.desc: Verify the PrepareSession function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonProxyTest, PrepareSessionTest, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "PrepareSessionTest Start";
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
    auto listener = sptr(new DaemonServiceMock());
    const std::string srcUri = "file://docs/storage/Users/currentUser/Documents?networkId=xxxxx";
    const std::string dstUri = "file://docs/storage/Users/currentUser/Documents";
    const std::string srcDeviceId = "testSrcDeviceId";
    const std::string copyPath = "tmpDir";
    HmdfsInfo fileInfo = {
        .copyPath = copyPath,
        .dirExistFlag = false,
    };
    int ret = proxy_->PrepareSession(srcUri, dstUri, srcDeviceId, listener, fileInfo);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "PrepareSessionTest End";
}
} // namespace OHOS::Storage::DistributedFile::Test