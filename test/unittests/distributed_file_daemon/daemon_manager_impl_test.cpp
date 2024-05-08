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

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/stat.h>

#include "dfs_error.h"
#include "distributed_file_daemon_manager_impl.h"
#include "distributed_file_daemon_proxy.h"
#include "ipc/hmdfs_info.h"
#include "i_daemon_mock.h"
#include "utils_log.h"

namespace OHOS::Storage::DistributedFile {
sptr<IDaemon> DistributedFileDaemonProxy::GetInstance()
{
    daemonProxy_ = iface_cast<IDaemon>(sptr(new DaemonServiceMock()));
    return daemonProxy_;
}
namespace Test {
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

class DistributedDaemonManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DistributedFileDaemonManagerImpl> distributedDaemonManagerImpl_;
};

void DistributedDaemonManagerImplTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DistributedDaemonManagerImplTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DistributedDaemonManagerImplTest::SetUp(void)
{
    if (distributedDaemonManagerImpl_ == nullptr) {
        distributedDaemonManagerImpl_ = std::make_shared<DistributedFileDaemonManagerImpl>();
        ASSERT_TRUE(distributedDaemonManagerImpl_ != nullptr) << "CallbackManager failed";
    }
    GTEST_LOG_(INFO) << "SetUp";
}

void DistributedDaemonManagerImplTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, GetInstanceTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest Start";
    try {
        DistributedFileDaemonManagerImpl::GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest  ERROR";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest End";
}

/**
 * @tc.name: OpenP2PConnectionTest
 * @tc.desc: Verify the OpenP2PConnection function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, OpenP2PConnectionTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenP2PConnectionTest Start";
    try {
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->OpenP2PConnection(deviceInfo);
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenP2PConnectionTest  ERROR";
    }
    GTEST_LOG_(INFO) << "OpenP2PConnectionTest End";
}

/**
 * @tc.name: CloseP2PConnectionTest
 * @tc.desc: Verify the CloseP2PConnection function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, CloseP2PConnectionTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloseP2PConnectionTest Start";
    try {
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->CloseP2PConnection(deviceInfo);
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CloseP2PConnectionTest  ERROR";
    }
    GTEST_LOG_(INFO) << "CloseP2PConnectionTest End";
}

/**
 * @tc.name: PrepareSessionTest
 * @tc.desc: Verify the PrepareSession function
 * @tc.type: FUNC
 * @tc.require: I7M6L1
 */
HWTEST_F(DistributedDaemonManagerImplTest, PrepareSessionTest, TestSize.Level1)
{
    const std::string srcUri = "file://docs/storage/Users/currentUser/Documents?networkId=xxxxx";
    const std::string dstUri = "file://docs/storage/Users/currentUser/Documents";
    const std::string srcDeviceId = "testSrcDeviceId";
    const sptr<IRemoteObject> listener = sptr(new DaemonServiceMock());
    const std::string copyPath = "tmpDir";
    const std::string sessionName = "DistributedDevice0";
    HmdfsInfo fileInfo = {
        .copyPath = copyPath,
        .dirExistFlag = false,
        .sessionName = sessionName,
    };
    GTEST_LOG_(INFO) << "PrepareSessionTest Start";
    try {
        auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
        EXPECT_NE(distributedFileDaemonProxy, nullptr);
        auto res = distributedDaemonManagerImpl_->PrepareSession(srcUri, dstUri, srcDeviceId, listener, fileInfo);
        EXPECT_NE(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PrepareSessionTest  ERROR";
    }
    GTEST_LOG_(INFO) << "PrepareSessionTest End";
}
} // namespace Test
} // namespace OHOS::Storage::DistributedFile