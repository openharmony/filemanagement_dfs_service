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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "common_event_manager.h"
#include "common_event_support.h"
#include "sync_rule/i_user_status_abserver.h"
#include "sync_rule/user_status_listener.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class UserStatusListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void UserStatusListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void UserStatusListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void UserStatusListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void UserStatusListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StopTest
 * @tc.desc: Verify the Stop function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(UserStatusListenerTest, StopTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest Start";
    try {
        auto observer = std::make_shared<IUserStatusObserver>();
        auto userStatusListener = std::make_shared<UserStatusListener>(observer);
        userStatusListener->Start();
        userStatusListener->Stop();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopTest FAILED";
    }
    GTEST_LOG_(INFO) << "StopTest End";
}

/**
 * @tc.name: NotifyUserUnlockedTest
 * @tc.desc: Verify the NotifyUserUnlocked function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(BatteryStatusListenerTest, NotifyUserUnlockedTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyUserUnlockedTest Start";
    try {
        auto observer = std::make_shared<IUserStatusObserver>();
        auto userStatusListener = std::make_shared<UserStatusListener>(observer);
        userStatusListener->NotifyUserUnlocked();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnStatusAbnormalTest FAILED";
    }
    GTEST_LOG_(INFO) << "OnStatusAbnormalTest End";
}
} // namespace OHOS::FileManagement::CloudSync::Test
