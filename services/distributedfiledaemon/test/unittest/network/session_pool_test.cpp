/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"
#include "network/session_pool.h"
#include "network/softbus/softbus_session.h"
#include "network/kernel_talker.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int USER_ID = 100;
constexpr int TEST_SESSION_ID = 10;

static void GetSessionProcess(NotifyParam &param)
{
};

static void CloseSessionForOneDevice(const std::string &cid)
{
};

class SessionPoolTest : public testing::Test {
public:
    static void SetUpTestCase(void) {};
    static void TearDownTestCase(void) {};
    void SetUp() {};
    void TearDown() {};
};

/**
 * @tc.name: SessionPoolTest_HoldSession_0100
 * @tc.desc: Verify the HoldSession function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_HoldSession_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_HoldSession_0100 start";
    auto session = make_shared<SoftbusSession>(TEST_SESSION_ID);
    shared_ptr<MountPoint> smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    shared_ptr<KernelTalker> kernelTalker = make_shared<KernelTalker>(wmp, GetSessionProcess, CloseSessionForOneDevice);
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    int res = true;
    try {
        pool->HoldSession(session);
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
 * @tc.require: SR000H0387
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_ReleaseSession_Fd_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Fd_0100 start";
    shared_ptr<MountPoint> smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    shared_ptr<KernelTalker> kernelTalker = make_shared<KernelTalker>(wmp, GetSessionProcess, CloseSessionForOneDevice);
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    int res = true;
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
 * @tc.name: SessionPoolTest_ReleaseSession_Cid_0100
 * @tc.desc: Verify the ReleaseSession by Cid function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(SessionPoolTest, SessionPoolTest_ReleaseSession_Cid_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseSession_Cid_0100 start";
    shared_ptr<MountPoint> smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    shared_ptr<KernelTalker> kernelTalker = make_shared<KernelTalker>(wmp, GetSessionProcess, CloseSessionForOneDevice);
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    int res = true;
    try {
        pool->ReleaseSession("testSession");
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
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
    shared_ptr<MountPoint> smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, "account"));
    weak_ptr<MountPoint> wmp = smp;
    shared_ptr<KernelTalker> kernelTalker = make_shared<KernelTalker>(wmp, GetSessionProcess, CloseSessionForOneDevice);
    shared_ptr<SessionPool> pool = make_shared<SessionPool>(kernelTalker);

    int res = true;
    try {
        pool->ReleaseAllSession();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }

    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SessionPoolTest_ReleaseAllSession_0100 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
