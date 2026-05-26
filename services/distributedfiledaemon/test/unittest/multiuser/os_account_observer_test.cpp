/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <iostream>
#include <memory>

#include "gtest/gtest.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "device_manager_agent_mock.h"
#include "multiuser/os_account_observer.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing;
using namespace testing::ext;
using Want = OHOS::AAFwk::Want;
constexpr int32_t USER_ID = 101;
constexpr int32_t INVALID_USER_ID = -1;
std::shared_ptr<OsAccountObserver> g_subScriber = nullptr;
std::shared_ptr<DeviceManagerAgentMock> g_deviceManagerAgentMock = nullptr;

class OsAccountObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void OsAccountObserverTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
    g_deviceManagerAgentMock = std::make_shared<DeviceManagerAgentMock>();
    IDeviceManagerAgentMock::iDeviceManagerAgentMock_ = g_deviceManagerAgentMock;

    if (g_subScriber == nullptr) {
        EventFwk::MatchingSkills matchingSkills;
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
        matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
        EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
        g_subScriber = std::make_shared<OsAccountObserver>(subscribeInfo);
        if (g_subScriber == nullptr) {
            LOGE("subscriber is nullptr");
        }
    }
}

void OsAccountObserverTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
    g_subScriber = nullptr;
    g_deviceManagerAgentMock = nullptr;
    IDeviceManagerAgentMock::iDeviceManagerAgentMock_ = nullptr;
}

void OsAccountObserverTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void OsAccountObserverTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name: OsAccountObserverTest_OnReceiveEvent_0100
 * @tc.desc: Verify to OnReceiveEvent function.
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(OsAccountObserverTest, OsAccountObserverTest_OnReceiveEvent_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OsAccountObserverTest_OnReceiveEvent_0100 start";
    bool res = true;
    try {
        if (g_subScriber != nullptr) {
            Want want;
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
            int32_t code = USER_ID;
            std::string data(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
            EventFwk::CommonEventData eventData(want, code, data);
            g_subScriber->OnReceiveEvent(eventData);
            EXPECT_EQ(g_subScriber->curUsrId_, USER_ID);

            g_subScriber->curUsrId_ = USER_ID;
            g_subScriber->OnReceiveEvent(eventData);
            EXPECT_EQ(g_subScriber->curUsrId_, USER_ID);

            g_subScriber->needAddUserId_.store(-1); // -1: default value
            std::string data1(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
            want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
            EventFwk::CommonEventData eventData1(want, code, data1);
            g_subScriber->OnReceiveEvent(eventData1);
            EXPECT_EQ(g_subScriber->needAddUserId_.load(), -1); // -1: default value

            g_subScriber->needAddUserId_.store(USER_ID);
            g_subScriber->OnReceiveEvent(eventData1);
            EXPECT_EQ(g_subScriber->needAddUserId_.load(), -1); // -1: default value
        }
    } catch (const std::exception &e) {
        res = false;
        LOGE("OsAccountObserverTest_OnReceiveEvent_0100 : %{public}s", e.what());
    }
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "OsAccountObserverTest_OnReceiveEvent_0100 end";
}

/**
 * @tc.name: OsAccountObserverTest_Constructor_001
 * @tc.desc: Verify constructor
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(OsAccountObserverTest, OsAccountObserverTest_Constructor_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OsAccountObserverTest_Constructor_001 start";
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_SWITCHED);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_USER_UNLOCKED);
    EventFwk::CommonEventSubscribeInfo subscribeInfo(matchingSkills);
    auto observer = std::make_shared<OsAccountObserver>(subscribeInfo);
    ASSERT_NE(observer, nullptr);
    GTEST_LOG_(INFO) << "OsAccountObserverTest_Constructor_001 end";
}

/**
 * @tc.name: OsAccountObserverTest_GetCurrentUserId_001
 * @tc.desc: Verify GetCurrentUserId success
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(OsAccountObserverTest, OsAccountObserverTest_GetCurrentUserId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OsAccountObserverTest_GetCurrentUserId_001 start";
    if (g_subScriber != nullptr) {
        auto userId = g_subScriber->GetCurrentUserId();
        EXPECT_NE(userId, INVALID_USER_ID);
    }
    GTEST_LOG_(INFO) << "OsAccountObserverTest_GetCurrentUserId_001 end";
}

/**
 * @tc.name: OsAccountObserverTest_AddMountPointInfo_001
 * @tc.desc: Verify AddMountPointInfo success
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(OsAccountObserverTest, OsAccountObserverTest_AddMountPointInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OsAccountObserverTest_AddMountPointInfo_001 start";
    if (g_subScriber != nullptr) {
        try {
            g_subScriber->AddMountPointInfo(USER_ID, "test_path");
            EXPECT_TRUE(true);
        } catch (...) {
            EXPECT_TRUE(false);
        }
    }
    GTEST_LOG_(INFO) << "OsAccountObserverTest_AddMountPointInfo_001 end";
}

/**
 * @tc.name: OsAccountObserverTest_RemoveMountPointInfo_001
 * @tc.desc: Verify RemoveMountPointInfo success
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(OsAccountObserverTest, OsAccountObserverTest_RemoveMountPointInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OsAccountObserverTest_RemoveMountPointInfo_001 start";
    if (g_subScriber != nullptr) {
        try {
            g_subScriber->RemoveMountPointInfo(USER_ID);
            EXPECT_TRUE(true);
        } catch (...) {
            EXPECT_TRUE(false);
        }
    }
    GTEST_LOG_(INFO) << "OsAccountObserverTest_RemoveMountPointInfo_001 end";
}

/**
 * @tc.name: OsAccountObserverTest_OnEventUserSwitched_001
 * @tc.desc: Verify OnEventUserSwitched normal switch
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(OsAccountObserverTest, OsAccountObserverTest_OnEventUserSwitched_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OsAccountObserverTest_OnEventUserSwitched_001 start";
    if (g_subScriber != nullptr) {
        EXPECT_NO_FATAL_FAILURE(g_subScriber->OnEventUserSwitched(USER_ID));
    }
    GTEST_LOG_(INFO) << "OsAccountObserverTest_OnEventUserSwitched_001 end";
}

/**
 * @tc.name: OsAccountObserverTest_OnEventUserUnlocked_001
 * @tc.desc: Verify OnEventUserUnlocked normal unlock
 * @tc.type: FUNC
 * @tc.require: SR000H0387
 */
HWTEST_F(OsAccountObserverTest, OsAccountObserverTest_OnEventUserUnlocked_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OsAccountObserverTest_OnEventUserUnlocked_001 start";
    if (g_subScriber != nullptr) {
        g_subScriber->needAddUserId_.store(USER_ID);
        EXPECT_NO_FATAL_FAILURE(g_subScriber->OnEventUserUnlocked(USER_ID));
    }
    GTEST_LOG_(INFO) << "OsAccountObserverTest_OnEventUserUnlocked_001 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS