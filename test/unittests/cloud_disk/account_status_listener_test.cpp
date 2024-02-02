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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "account_status_listener.h"
#include "common_event_manager.h"
#include "common_event_support.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using Want = OHOS::AAFwk::Want;

constexpr int32_t USER_ID = 101;

class AccountStatusListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<AccountStatusListener> accountStatusListener_ = nullptr;
    static inline shared_ptr<AccountStatusSubscriber> accountStatusSubscriber_ = nullptr;
    static inline shared_ptr<EventFwk::CommonEventSubscribeInfo> subscribeInfo_ = nullptr;
};

void AccountStatusListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGIN);
    matchingSkills.AddEvent(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
    subscribeInfo_ = make_shared<EventFwk::CommonEventSubscribeInfo>(matchingSkills);
    accountStatusSubscriber_ = make_shared<AccountStatusSubscriber>(*subscribeInfo_);
    accountStatusListener_ = make_shared<AccountStatusListener>();
}

void AccountStatusListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void AccountStatusListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AccountStatusListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: OnReceiveEventTest001
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: issuesI90JZZ
 */
HWTEST_F(AccountStatusListenerTest, OnReceiveEventTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest001 Start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGIN);
        string data(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGIN);
        EventFwk::CommonEventData eventData(want, USER_ID, data);
        accountStatusSubscriber_->OnReceiveEvent(eventData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest001 End";
}

/**
 * @tc.name: OnReceiveEventTest002
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: issuesI90JZZ
 */
HWTEST_F(AccountStatusListenerTest, OnReceiveEventTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest002 Start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
        string data(EventFwk::CommonEventSupport::COMMON_EVENT_HWID_LOGOUT);
        EventFwk::CommonEventData eventData(want, USER_ID, data);
        accountStatusSubscriber_->OnReceiveEvent(eventData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnReceiveEventTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest002 End";
}

/**
 * @tc.name: StopTest001
 * @tc.desc: Verify the Stop function
 * @tc.type: FUNC
 * @tc.require: issuesI90JZZ
 */
HWTEST_F(AccountStatusListenerTest, StopTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest001 Start";
    try {
        accountStatusListener_->commonEventSubscriber_ = std::make_shared<AccountStatusSubscriber>(*subscribeInfo_);
        accountStatusListener_->Stop();
        EXPECT_TRUE(accountStatusListener_->commonEventSubscriber_ == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "StopTest001 End";
}

/**
 * @tc.name: StopTest002
 * @tc.desc: Verify the Stop function
 * @tc.type: FUNC
 * @tc.require: issuesI90JZZ
 */
HWTEST_F(AccountStatusListenerTest, StopTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest002 Start";
    try {
        accountStatusListener_->commonEventSubscriber_ = nullptr;
        accountStatusListener_->Stop();
        EXPECT_TRUE(accountStatusListener_->commonEventSubscriber_ == nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "StopTest002 End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test