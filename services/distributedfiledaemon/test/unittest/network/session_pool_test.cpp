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
constexpr int TEST_SESSION_ID_TWO = 20;
constexpr uint8_t LINK_TYPE_P2P = 2;
constexpr uint8_t LINK_TYPE_AP = 1;
constexpr int32_t MOUNT_DFS_COUNT_ONE = 1;

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
 * @tc.name: SessionPoolTest_OccupySession_0100
 * @tc.desc: Verify the OccupySession by Cid function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_OccupySession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_OccupySession_0100 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    bool res = true;
    try {
        pool->OccupySession(TEST_SESSION_ID, 1);
    } catch (const exception &e) {
        res = false;
        GTEST_LOG_(INFO) << e.what();
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SessionPoolTest_OccupySession_0100 end";
}

/**
 * @tc.name: SessionPoolTest_FindSession_0100
 * @tc.desc: Verify the FindSession by Cid function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_FindSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_FindSession_0100 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    bool res = true;
    try {
        pool->occupySession_.erase(TEST_SESSION_ID);
        pool->occupySession_.insert(make_pair(TEST_SESSION_ID, LINK_TYPE_AP));
        bool flag = pool->FindSession(TEST_SESSION_ID);
        EXPECT_EQ(flag, true);
        pool->occupySession_.erase(TEST_SESSION_ID);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SessionPoolTest_FindSession_0100 end";
}

/**
 * @tc.name: SessionPoolTest_FindSession_0200
 * @tc.desc: Verify the FindSession by Cid function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_FindSession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_FindSession_0200 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    bool res = true;
    try {
        pool->occupySession_.erase(TEST_SESSION_ID_TWO);
        bool flag = pool->FindSession(TEST_SESSION_ID_TWO);
        EXPECT_EQ(flag, false);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SessionPoolTest_FindSession_0200 end";
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
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
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
 * @tc.name: SessionPoolTest_HoldSession_0200
 * @tc.desc: Verify the HoldSession function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_HoldSession_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_HoldSession_0200 start";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID,  "testNetWork");
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    pool->occupySession_.insert(make_pair(TEST_SESSION_ID, LINK_TYPE_P2P));
    pool->occupySession_.erase(TEST_SESSION_ID);
    pool->occupySession_.insert(make_pair(TEST_SESSION_ID, LINK_TYPE_P2P));
    pool->deviceConnectCount_["testdevid_2"] = 1;
    bool res = true;
    try {
        auto size = pool->usrSpaceSessionPool_.size();
        pool->HoldSession(session, "Server");
        EXPECT_EQ(pool->usrSpaceSessionPool_.size(), size);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SessionPoolTest_HoldSession_0200 end";
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
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    bool res = true;
    try {
        pool->ReleaseSession(1);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Fd_0100 end";
}

/**
 * @tc.name: SessionPoolTest_ReleaseSession_Fd_0200
 * @tc.desc: Verify the ReleaseSession by Fd function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_ReleaseSession_Fd_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Fd_0200 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID,  peerDeviceId);
    session->socketFd_ = 1;
    pool->usrSpaceSessionPool_.push_back(session);

    std::string peerDeviceId2 = "f6d4c0864707aefte7a78f09473aa122ff57ed9";
    auto session2 = make_shared<SoftbusSession>(TEST_SESSION_ID_TWO,  peerDeviceId2);
    session2->socketFd_ = 1;
    pool->usrSpaceSessionPool_.push_back(session2);

    auto size = pool->usrSpaceSessionPool_.size();
    pool->occupySession_.insert(make_pair(TEST_SESSION_ID_TWO, LINK_TYPE_P2P));
    pool->deviceIdByCid_[peerDeviceId2] = "deviceId1";
    string key = "deviceId1_" + to_string(LINK_TYPE_P2P);
    pool->deviceConnectCount_[key] = MOUNT_DFS_COUNT_ONE + 1;
    EXPECT_EQ(pool->ReleaseSession(1), LINK_TYPE_P2P);
    EXPECT_EQ(pool->usrSpaceSessionPool_.size(), size - 1);

    auto iter = pool->occupySession_.find(TEST_SESSION_ID_TWO);
    if (iter == pool->occupySession_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }

    pool->deviceConnectCount_["test"] = 0;
    pool->ReleaseSession(-2);
    EXPECT_EQ(pool->deviceConnectCount_.size(), 0);

    pool->usrSpaceSessionPool_.clear();
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Fd_0200 end";
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
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    bool res = true;
    try {
        pool->ReleaseSession("testSession", 1);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Cid_0100 end";
}

/**
 * @tc.name: SessionPoolTest_ReleaseSession_Cid_0200
 * @tc.desc: Verify the ReleaseSession by Cid function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_ReleaseSession_Cid_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Cid_0200 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID,  peerDeviceId);
    pool->usrSpaceSessionPool_.push_back(session);

    bool res = true;
    try {
        pool->ReleaseSession("testSession", 1);
        pool->ReleaseSession(peerDeviceId, 1);
        pool->occupySession_.insert(make_pair(TEST_SESSION_ID, LINK_TYPE_P2P));
        pool->ReleaseSession(peerDeviceId, 1);
        auto iter = pool->occupySession_.find(TEST_SESSION_ID);
        if (iter == pool->occupySession_.end()) {
            EXPECT_TRUE(false);
        } else {
            EXPECT_TRUE(true);
        }
        pool->occupySession_.erase(TEST_SESSION_ID);
        pool->occupySession_.insert(make_pair(TEST_SESSION_ID, 0));
        pool->ReleaseSession(peerDeviceId, 1);
        iter = pool->occupySession_.find(TEST_SESSION_ID);
        if (iter == pool->occupySession_.end()) {
            EXPECT_TRUE(true);
        } else {
            EXPECT_TRUE(false);
        }
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Cid_0200 end";
}

/**
 * @tc.name: SessionPoolTest_ReleaseSession_Cid_0300
 * @tc.desc: Verify the ReleaseSession by Cid function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_ReleaseSession_Cid_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Cid_0300 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID,  peerDeviceId);
    pool->usrSpaceSessionPool_.push_back(session);

    pool->deviceIdByCid_[peerDeviceId] = "deviceId2";
    string key = "deviceId2_2";
    pool->deviceConnectCount_[key] = MOUNT_DFS_COUNT_ONE + 1;

    bool res = true;
    try {
        pool->occupySession_.erase(TEST_SESSION_ID);
        pool->occupySession_.insert(make_pair(TEST_SESSION_ID, 0));
        pool->ReleaseSession(peerDeviceId, LINK_TYPE_P2P);
        auto iter = pool->occupySession_.find(TEST_SESSION_ID);
        if (iter == pool->occupySession_.end()) {
            EXPECT_TRUE(true);
        } else {
            EXPECT_TRUE(false);
        }
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Cid_0300 end";
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
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

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
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
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
 * @tc.name: SessionPoolTest_GetDeviceIdByCid_0100
 * @tc.desc: Verify the GetDeviceIdByCid function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_GetDeviceIdByCid_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_GetDeviceIdByCid_0100 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);
    pool->deviceIdByCid_["cid"] = "deviceId";
    EXPECT_EQ(pool->GetDeviceIdByCid("cid"), "deviceId");
    pool->deviceIdByCid_.erase("cid");

    EXPECT_EQ(pool->GetDeviceIdByCid(""), "");

    EXPECT_EQ(pool->GetDeviceIdByCid("cid"), "");

    EXPECT_EQ(pool->GetDeviceIdByCid("testNetWork"), "testdevid");
    GTEST_LOG_(INFO) << "SessionPoolTest_GetDeviceIdByCid_0100 end";
}

/**
 * @tc.name: SessionPoolTest_DeviceDisconnectCountOnly_0100
 * @tc.desc: Verify the DeviceDisconnectCountOnly function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_DeviceDisconnectCountOnly_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_DeviceDisconnectCountOnly_0100 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    EXPECT_EQ(pool->DeviceDisconnectCountOnly("", LINK_TYPE_AP, false), false);

    EXPECT_EQ(pool->DeviceDisconnectCountOnly("cid", LINK_TYPE_P2P, false), false);

    pool->deviceIdByCid_["cid"] = "deviceId";
    EXPECT_EQ(pool->DeviceDisconnectCountOnly("cid", LINK_TYPE_P2P, false), false);
    auto itCid = pool->deviceIdByCid_.find("cid");
    if (itCid == pool->deviceIdByCid_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }

    pool->deviceIdByCid_["cid"] = "deviceId";
    string key = "deviceId_2";
    pool->deviceConnectCount_[key] = 2;

    EXPECT_EQ(pool->DeviceDisconnectCountOnly("cid", LINK_TYPE_P2P, true), false);
    itCid = pool->deviceIdByCid_.find("cid");
    if (itCid == pool->deviceIdByCid_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }

    auto itCount = pool->deviceConnectCount_.find(key);
    if (itCount == pool->deviceConnectCount_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SessionPoolTest_DeviceDisconnectCountOnly_0100 end";
}

/**
 * @tc.name: SessionPoolTest_DeviceDisconnectCountOnly_0200
 * @tc.desc: Verify the DeviceDisconnectCountOnly function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_DeviceDisconnectCountOnly_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_DeviceDisconnectCountOnly_0200 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    pool->deviceIdByCid_["cid"] = "deviceId";
    string key = "deviceId_2";
    pool->deviceConnectCount_[key] = MOUNT_DFS_COUNT_ONE + 1;

    EXPECT_EQ(pool->DeviceDisconnectCountOnly("cid", LINK_TYPE_P2P, false), true);
    auto itCid = pool->deviceIdByCid_.find("cid");
    if (itCid == pool->deviceIdByCid_.end()) {
        EXPECT_TRUE(false);
    } else {
        EXPECT_TRUE(true);
    }

    EXPECT_EQ(pool->deviceConnectCount_[key], MOUNT_DFS_COUNT_ONE);
    EXPECT_EQ(pool->DeviceDisconnectCountOnly("cid", LINK_TYPE_P2P, false), false);
    itCid = pool->deviceIdByCid_.find("cid");
    if (itCid == pool->deviceIdByCid_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }

    auto itCount = pool->deviceConnectCount_.find(key);
    if (itCount == pool->deviceConnectCount_.end()) {
        EXPECT_TRUE(true);
    } else {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SessionPoolTest_DeviceDisconnectCountOnly_0200 end";
}

/**
 * @tc.name: SessionPoolTest_DeviceConnectCountOnly_0100
 * @tc.desc: Verify the DeviceConnectCountOnly function.
 * @tc.type: FUNC
 * @tc.require: IA4TFG
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_DeviceConnectCountOnly_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_DeviceConnectCountOnly_0100 start";
    auto smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    auto kernelTalker = std::make_shared<KernelTalker>(wmp, [](NotifyParam &param) {}, [](int32_t fd) {});
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);
    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID,  peerDeviceId);
    EXPECT_EQ(pool->DeviceConnectCountOnly(session), false);

    auto session2 = make_shared<SoftbusSession>(TEST_SESSION_ID,  "");
    EXPECT_EQ(pool->DeviceConnectCountOnly(session2), false);

    auto session3 = make_shared<SoftbusSession>(TEST_SESSION_ID,  "testNetWork");
    pool->occupySession_.insert(make_pair(TEST_SESSION_ID, LINK_TYPE_AP));
    EXPECT_EQ(pool->DeviceConnectCountOnly(session3), false);

    pool->occupySession_.erase(TEST_SESSION_ID);
    pool->occupySession_.insert(make_pair(TEST_SESSION_ID, LINK_TYPE_P2P));
    pool->deviceConnectCount_["testdevid_2"] = 0;
    EXPECT_EQ(pool->DeviceConnectCountOnly(session3), false);
    EXPECT_EQ(pool->deviceConnectCount_["testdevid_2"], 1);
    
    EXPECT_EQ(pool->DeviceConnectCountOnly(session3), true);
    EXPECT_EQ(pool->deviceConnectCount_["testdevid_2"], 2);
    GTEST_LOG_(INFO) << "SessionPoolTest_DeviceConnectCountOnly_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
