/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "appstate_observer.h"

#include <cstring>
#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "assistant.h"
#include "application_state_observer_stub.h"
#include "dfs_error.h"
#include "io_message_listener.h"
#include "singleton_mock.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class AppStateObserverTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<AppStateObserverManager> appStateObserverManager_ = nullptr;
    static inline shared_ptr<AppStateObserver> appStateObserver_ = nullptr;
    static inline shared_ptr<AssistantMock> insMock = nullptr;
};

void AppStateObserverTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    appStateObserverManager_ = make_shared<AppStateObserverManager>();
    appStateObserver_ = make_shared<AppStateObserver>();
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
}

void AppStateObserverTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    appStateObserverManager_ = nullptr;
    appStateObserver_ = nullptr;
    Assistant::ins = nullptr;
    insMock = nullptr;
}

void AppStateObserverTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AppStateObserverTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: OnForegroundApplicationChangedTest001
 * @tc.desc: Read IO data
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(AppStateObserverTest, OnForegroundApplicationChangedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnForegroundApplicationChangedTest001 Start";
    try {
        auto appStateData_= make_shared<AppExecFwk::AppStateData>();
        appStateData_->bundleName = "com.ohos.sceneboard";
        appStateObserver_->OnForegroundApplicationChanged(*appStateData_);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnForegroundApplicationChangedTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "OnForegroundApplicationChangedTest001 End";
}

/**
 * @tc.name: GetInstanceTest001
 * @tc.desc: Read IO data
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(AppStateObserverTest, GetInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest001 Start";
    try {
        shared_ptr<AppStateObserverManager> appStateObserverManager1_ = make_shared<AppStateObserverManager>();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest001 End";
}

/**
 * @tc.name: SubscribeAppStateTest001
 * @tc.desc: Read IO data
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(AppStateObserverTest, SubscribeAppStateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubscribeAppStateTest001 Start";
    try {
        vector<string> bundleNameList = {"com.example"};
        OHOS::DelayedSingleton<OHOS::AppExecFwk::AppMgrClient>::instance_ = nullptr;
        appStateObserverManager_->SubscribeAppState(bundleNameList);
        EXPECT_FALSE(false);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "SubscribeAppStateTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "SubscribeAppStateTest001 End";
}

/**
 * @tc.name: SubscribeAppStateTest002
 * @tc.desc: Read IO data
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(AppStateObserverTest, SubscribeAppStateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubscribeAppStateTest002 Start";
    try {
        vector<string> bundleNameList = {"com.example"};
        OHOS::DelayedSingleton<OHOS::AppExecFwk::AppMgrClient>::instance_ =
            make_shared<OHOS::AppExecFwk::AppMgrClient>();
        sptr<OHOS::AppExecFwk::ApplicationStateObserverStub> appStateObserver1_ =
            sptr<AppStateObserver>(new (std::nothrow) AppStateObserver());
        appStateObserverManager_->appStateObserver_ = appStateObserver1_;
        appStateObserverManager_->SubscribeAppState(bundleNameList);
        EXPECT_FALSE(false);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "SubscribeAppStateTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "SubscribeAppStateTest002 End";
}

/**
 * @tc.name: SubscribeAppStateTest003
 * @tc.desc: Read IO data
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(AppStateObserverTest, SubscribeAppStateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubscribeAppStateTest003 Start";
    try {
        vector<string> bundleNameList = {"com.example"};
        OHOS::DelayedSingleton<OHOS::AppExecFwk::AppMgrClient>::instance_ =
            make_shared<OHOS::AppExecFwk::AppMgrClient>();
        appStateObserverManager_->appStateObserver_ = nullptr;
        appStateObserverManager_->SubscribeAppState(bundleNameList);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SubscribeAppStateTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "SubscribeAppStateTest003 End";
}

/**
 * @tc.name: SubscribeAppStateTest004
 * @tc.desc: Read IO data
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(AppStateObserverTest, SubscribeAppStateTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubscribeAppStateTest004 Start";
    try {
        vector<string> bundleNameList = {""};
        OHOS::DelayedSingleton<OHOS::AppExecFwk::AppMgrClient>::instance_ =
            make_shared<OHOS::AppExecFwk::AppMgrClient>();
        appStateObserverManager_->appStateObserver_ = nullptr;
        appStateObserverManager_->SubscribeAppState(bundleNameList);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SubscribeAppStateTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "SubscribeAppStateTest004 End";
}

/**
 * @tc.name: UnSubscribeAppStateTest001
 * @tc.desc: Read IO data
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(AppStateObserverTest, UnSubscribeAppStateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnSubscribeAppStateTest001 Start";
    try {
        appStateObserverManager_->appStateObserver_ = nullptr;
        appStateObserverManager_->UnSubscribeAppState();
        EXPECT_FALSE(false);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "UnSubscribeAppStateTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "UnSubscribeAppStateTest001 End";
}

/**
 * @tc.name: UnSubscribeAppStateTest002
 * @tc.desc: Read IO data
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(AppStateObserverTest, UnSubscribeAppStateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnSubscribeAppStateTest002 Start";
    try {
        sptr<OHOS::AppExecFwk::ApplicationStateObserverStub> appStateObserver1_ =
            sptr<AppStateObserver>(new (std::nothrow) AppStateObserver());
        appStateObserverManager_->appStateObserver_ = appStateObserver1_;
        OHOS::DelayedSingleton<OHOS::AppExecFwk::AppMgrClient>::instance_ = nullptr;
        appStateObserverManager_->UnSubscribeAppState();
        EXPECT_FALSE(false);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "UnSubscribeAppStateTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "UnSubscribeAppStateTest002 End";
}

/**
 * @tc.name: UnSubscribeAppStateTest003
 * @tc.desc: Read IO data
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(AppStateObserverTest, UnSubscribeAppStateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnSubscribeAppStateTest003 Start";
    try {
        sptr<OHOS::AppExecFwk::ApplicationStateObserverStub> appStateObserver1_ =
            sptr<AppStateObserver>(new (std::nothrow) AppStateObserver());
        appStateObserverManager_->appStateObserver_ = appStateObserver1_;
        OHOS::DelayedSingleton<OHOS::AppExecFwk::AppMgrClient>::instance_ =
            make_shared<OHOS::AppExecFwk::AppMgrClient>();
        appStateObserverManager_->UnSubscribeAppState();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnSubscribeAppStateTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "UnSubscribeAppStateTest003 End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test