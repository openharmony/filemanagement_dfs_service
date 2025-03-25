/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include <memory>
#include <unistd.h>

#include "device_manager_impl.h"
#include "dm_constants.h"

#include "network/kernel_talker.h"
#include "network/session_pool.h"
#include "network/softbus/softbus_session.h"
#include "network/kernel_talker.h"
#include "utils_log.h"

namespace {
    bool g_mockGetTrustedDeviceList = true;
}

namespace OHOS {
namespace DistributedHardware {
int32_t DeviceManagerImpl::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
                                                std::vector<DmDeviceInfo> &deviceList)
{
    if (!g_mockGetTrustedDeviceList) {
        return ERR_DM_INPUT_PARA_INVALID;
    }

    DmDeviceInfo deviceInfo{
        .deviceId = "testdevid",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
        .networkId = "testNetWork",
    };
    deviceList.push_back(deviceInfo);
    return DM_OK;
}
}
}

namespace OHOS {
namespace Storage {
namespace DistributedFile {
void KernelTalker::SinkSessionTokernel(std::shared_ptr<BaseSession> session, const std::string backStage)
{
    return;
}

void KernelTalker::SinkOfflineCmdToKernel(std::string cid)
{
    return;
}

void KernelTalker::CreatePollThread()
{
    return;
}

void KernelTalker::WaitForPollThreadExited()
{
    return;
}
}
}
}

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

constexpr int USER_ID = 100;
constexpr int TEST_SESSION_ID = 10;
class SessionPoolTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SessionPoolTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SessionPoolTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SessionPoolTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void SessionPoolTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SessionPoolTest_HoldSession_0100
 * @tc.desc: Verify the HoldSession function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_HoldSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_HoldSession_0100 start";
    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID,  peerDeviceId);
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](const std::string &) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);
    bool res = true;
    try {
        auto size = pool->usrSpaceSessionPool_.size();
        pool->HoldSession(session, "Server");
        EXPECT_EQ(pool->usrSpaceSessionPool_.size(), size + 1);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SessionPoolTest_HoldSession_0100 end";
}

/**
 * @tc.name: SessionPoolTest_ReleaseSession_Fd_0100
 * @tc.desc: Verify the ReleaseSession by Fd function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_ReleaseSession_Fd_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Fd_0100 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](const std::string &) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);
    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID,  peerDeviceId);
    pool->usrSpaceSessionPool_.push_back(session);
    pool->ReleaseSession(0);
    EXPECT_EQ(pool->usrSpaceSessionPool_.size(), 1); // 1: session size

    pool->ReleaseSession(-1); // -1: session fd
    EXPECT_EQ(pool->usrSpaceSessionPool_.size(), 0); // 0: session size
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Fd_0100 end";
}

/**
 * @tc.name: SessionPoolTest_ReleaseSession_Cid_0100
 * @tc.desc: Verify the ReleaseSession by Cid function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_ReleaseSession_Cid_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Cid_0100 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](const std::string &) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID,  peerDeviceId);
    session->SetFromServer(true);
    pool->usrSpaceSessionPool_.push_back(session);

    std::string peerDeviceId1 = "f6d4c0864707aefte7a78f09473aa122ff57fc7";
    auto session1 = make_shared<SoftbusSession>(TEST_SESSION_ID, peerDeviceId1);
    session1->SetFromServer(false);
    pool->usrSpaceSessionPool_.push_back(session1);

    auto session2 = make_shared<SoftbusSession>(TEST_SESSION_ID,  peerDeviceId);
    session2->SetFromServer(false);
    pool->usrSpaceSessionPool_.push_back(session2);
    bool ifReleaseService = false;
    size_t len = 3; // 3: session size;

    pool->ReleaseSession("test", ifReleaseService);
    EXPECT_EQ(pool->usrSpaceSessionPool_.size(), len);

    pool->ReleaseSession(peerDeviceId, ifReleaseService);
    EXPECT_EQ(pool->usrSpaceSessionPool_.size(), len - 1); // 1: remove session

    pool->ReleaseSession(peerDeviceId1, ifReleaseService);
    EXPECT_EQ(pool->usrSpaceSessionPool_.size(), len - 2); // 2: remove session1

    ifReleaseService = true;
    pool->ReleaseSession(peerDeviceId, ifReleaseService);
    EXPECT_EQ(pool->usrSpaceSessionPool_.size(), 0); // 1: remove one
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Cid_0100 end";
}

/**
 * @tc.name: SessionPoolTest_ReleaseAllSession_0100
 * @tc.desc: Verify the ReleaseAllSession function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_ReleaseAllSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseAllSession_0100 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](const std::string &) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);
    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID,  peerDeviceId);
    pool->usrSpaceSessionPool_.push_back(session);

    bool res = true;
    try {
        pool->ReleaseAllSession();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseAllSession_0100 end";
}

/**
 * @tc.name: SessionPoolTest_AddSessionToPool_0100
 * @tc.desc: Verify the AddSessionToPool function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_AddSessionToPool_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_AddSessionToPool_0100 start";
    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID, peerDeviceId);
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](const std::string &) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    bool res = true;
    try {
        pool->AddSessionToPool(session);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SessionPoolTest_AddSessionToPool_0100 end";
}

/**
 * @tc.name: SessionPoolTest_CheckIfGetSession_0100
 * @tc.desc: Verify the CheckIfGetSession function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_CheckIfGetSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_CheckIfGetSession_0100 start";
    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID, peerDeviceId);
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](const std::string &) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);
    int32_t fd = -1; // -1: fd
    bool ret = pool->CheckIfGetSession(fd);
    EXPECT_EQ(ret, false);

    pool->usrSpaceSessionPool_.push_back(session);
    ret = pool->CheckIfGetSession(fd);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "SessionPoolTest_CheckIfGetSession_0100 end";
}

/**
 * @tc.name: SessionPoolTest_SinkOffLine_0100
 * @tc.desc: Verify the SinkOffLine function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_SinkOffLine_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_SinkOffLine_0100 start";
    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID, peerDeviceId);
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](const std::string &) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);
    pool->usrSpaceSessionPool_.push_back(session);
    pool->SinkOffline(peerDeviceId);
    pool->SinkOffline("test");
    GTEST_LOG_(INFO) << "SessionPoolTest_SinkOffLine_0100 end";
}

/**
 * @tc.name: SessionPoolTest_FindSocketId_0100
 * @tc.desc: Verify the FindSocketId function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_FindSocketId_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_FindSocketId_0100 start";
    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID, peerDeviceId);
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](const std::string &) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);
    pool->usrSpaceSessionPool_.push_back(session);
    bool ret = pool->FindSocketId(-1); // -1: session id
    EXPECT_EQ(ret, false);

    ret = pool->FindSocketId(TEST_SESSION_ID);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "SessionPoolTest_FindSocketId_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
