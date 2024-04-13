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

#include <memory>
#include <unistd.h>

#include "gtest/gtest.h"
#include "network/kernel_talker.h"
#include "network/softbus/softbus_session.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int TEST_SESSION_ID = 10;
constexpr int USER_ID = 100;
static const string SESSION_CID = "testSession";
static const string SAME_ACCOUNT = "account";
shared_ptr<MountPoint> g_smp;
weak_ptr<MountPoint> g_wmp;
shared_ptr<KernelTalker> g_talker;

enum Notify {
    NOTIFY_GET_SESSION = 0,
    NOTIFY_OFFLINE,
    NOTIFY_NONE,
    NOTIFY_CNT,
};

class KernelTalkerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
};

void KernelTalkerTest::SetUpTestCase(void)
{
    g_smp = make_shared<MountPoint>(Utils::DfsuMountArgumentDescriptors::Alpha(USER_ID, SAME_ACCOUNT));
    g_wmp = g_smp;
    g_talker = std::make_shared<KernelTalker>(g_wmp, [](NotifyParam &param) {}, [](const std::string &cid) {});
};

void KernelTalkerTest::TearDownTestCase(void)
{
    g_talker = nullptr;
    g_smp = nullptr;
};

/**
 * @tc.name: KernelTalkerTest_SinkSessionTokernel_0100
 * @tc.desc: Verify the SinkSessionTokernel function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_SinkSessionTokernel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkSessionTokernel_0100 start";
    bool res = true;
    std::string peerDeviceId = "f6d4c0864707aefte7a78f09473aa122ff57fc8";
    std::shared_ptr<SoftbusSession> session = make_shared<SoftbusSession>(TEST_SESSION_ID, peerDeviceId);
    try {
        g_talker->SinkSessionTokernel(session, "Server");
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkSessionTokernel_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_SinkDevslTokernel_0100
 * @tc.desc: Verify the SinkDevslTokernel function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_SinkDevslTokernel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkDevslTokernel_0100 start";
    bool res = true;
    try {
        g_talker->SinkDevslTokernel(SESSION_CID, 1);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkDevslTokernel_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_SinkOfflineCmdToKernel_0100
 * @tc.desc: Verify the SinkOfflineCmdToKernel function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_SinkOfflineCmdToKernel_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkOfflineCmdToKernel_0100 start";
    bool res = true;
    try {
        g_talker->SinkOfflineCmdToKernel("testSession");
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_SinkOfflineCmdToKernel_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_CreatePollThread_0100
 * @tc.desc: Verify the CreatePollThread function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_CreatePollThread_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_CreatePollThread_0100 start";
    bool res = true;
    try {
        g_talker->CreatePollThread();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_CreatePollThread_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_WaitForPollThreadExited_0100
 * @tc.desc: Verify the WaitForPollThreadExited function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_WaitForPollThreadExited_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_WaitForPollThreadExited_0100 start";
    int res = true;
    try {
        g_talker->WaitForPollThreadExited();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_WaitForPollThreadExited_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_PollRun_0100
 * @tc.desc: Verify the PollRun function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_PollRun_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_PollRun_0100 start";
    int res = true;
    try {
        g_talker->PollRun();
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_PollRun_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_HandleAllNotify_0100
 * @tc.desc: Verify the HandleAllNotify function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_HandleAllNotify_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_HandleAllNotify_0100 start";
    bool res = true;
    int fd = -1;
    try {
        g_talker->HandleAllNotify(fd);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_HandleAllNotify_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_NotifyHandler_0100
 * @tc.desc: Verify the NotifyHandler function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_NotifyHandler_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_NotifyHandler_0100 start";
    bool res = true;
    NotifyParam param;
    param.notify = NOTIFY_GET_SESSION;
    try {
        g_talker->NotifyHandler(param);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_NotifyHandler_0100 end";
}

/**
 * @tc.name: KernelTalkerTest_NotifyHandler_0200
 * @tc.desc: Verify the NotifyHandler function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_NotifyHandler_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_NotifyHandler_0200 start";
    bool res = true;
    NotifyParam param;
    param.notify = NOTIFY_OFFLINE;
    try {
        g_talker->NotifyHandler(param);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_NotifyHandler_0200 end";
}

/**
 * @tc.name: KernelTalkerTest_NotifyHandler_0300
 * @tc.desc: Verify the NotifyHandler function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(KernelTalkerTest, KernelTalkerTest_NotifyHandler_0300, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "KernelTalkerTest_NotifyHandler_0300 start";
    bool res = true;
    NotifyParam param;
    param.notify = NOTIFY_NONE;
    try {
        g_talker->NotifyHandler(param);
    } catch (const exception &e) {
        res = false;
        LOGE("%{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "KernelTalkerTest_NotifyHandler_0300 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
