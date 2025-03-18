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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_status.h"
#include "common_event_manager.h"
#include "common_event_support.h"
#include "package_status_listener.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace CloudFile;
using Want = OHOS::AAFwk::Want;

constexpr int32_t USER_ID = 101;

class PackageStatusListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void PackageStatusListenerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void PackageStatusListenerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void PackageStatusListenerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void PackageStatusListenerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: StopTest
 * @tc.desc: Verify the Stop function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(PackageStatusListenerTest, StopTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopTest Start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        packageStatusListener->Start();
        packageStatusListener->Stop();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopTest FAILED";
    }
    GTEST_LOG_(INFO) << "StopTest End";
}

/**
 * @tc.name: OnReceiveEventTest
 * @tc.desc: Verify the OnReceiveEvent function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(PackageStatusListenerTest, OnReceiveEventTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnReceiveEventTest001 Start";
    try {
        Want want;
        want.SetAction(EventFwk::CommonEventSupport::COMMON_EVENT_PACKAGE_REMOVED);
        EventFwk::CommonEventData eventData(want);
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        auto subscriber = std::make_shared<PackageStatusSubscriber>(EventFwk::CommonEventSubscribeInfo(),
            packageStatusListener);
        subscriber->OnReceiveEvent(eventData);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnReceiveEventTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "OnReceiveEventTest001 End";
}

/**
 * @tc.name: RemovedCleanTest001
 * @tc.desc: Verify the RemovedClean function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, RemovedCleanTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest001 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "Test";
        const int32_t userId = USER_ID;
        packageStatusListener->RemovedClean(bundleName, userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RemovedCleanTest001 failed";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest001 end";
}

/**
 * @tc.name: RemovedCleanTest002
 * @tc.desc: Verify the RemovedClean function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, RemovedCleanTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest002 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "";
        const int32_t userId = USER_ID;
        packageStatusListener->RemovedClean(bundleName, userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RemovedCleanTest002 failed";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest002 end";
}

/**
 * @tc.name: RemovedCleanTest003
 * @tc.desc: Verify the RemovedClean function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, RemovedCleanTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest003 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "Test";
        const int32_t userId = -1;
        packageStatusListener->RemovedClean(bundleName, userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RemovedCleanTest003 failed";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest003 end";
}

/**
 * @tc.name: RemovedCleanTest004
 * @tc.desc: Verify the RemovedClean function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, RemovedCleanTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest004 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "";
        const int32_t userId = -1;
        packageStatusListener->RemovedClean(bundleName, userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RemovedCleanTest004 failed";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest004 end";
}

/**
 * @tc.name: RemovedCleanTest005
 * @tc.desc: Verify the RemovedClean function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, RemovedCleanTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest005 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "Test";
        const int32_t userId = 0;
        packageStatusListener->RemovedClean(bundleName, userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RemovedCleanTest005 failed";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest005 end";
}

/**
 * @tc.name: RemovedCleanTest006
 * @tc.desc: Verify the RemovedClean function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, RemovedCleanTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest006 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "";
        const int32_t userId = 0;
        packageStatusListener->RemovedClean(bundleName, userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RemovedCleanTest006 failed";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest006 end";
}

/**
 * @tc.name: IsCloudSyncOnTest001
 * @tc.desc: Verify the IsCloudSyncOn function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, IsCloudSyncOnTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest001 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "Test";
        const int32_t userId = USER_ID;
        packageStatusListener->IsCloudSyncOn(userId, bundleName);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsCloudSyncOnTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest001 end";
}

/**
 * @tc.name: IsCloudSyncOnTest002
 * @tc.desc: Verify the IsCloudSyncOn function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, IsCloudSyncOnTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest002 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "";
        const int32_t userId = USER_ID;
        packageStatusListener->IsCloudSyncOn(userId, bundleName);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsCloudSyncOnTest002 failed";
    }
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest002 end";
}

/**
 * @tc.name: IsCloudSyncOnTest003
 * @tc.desc: Verify the IsCloudSyncOn function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, IsCloudSyncOnTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest003 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "Test";
        const int32_t userId = -1;
        packageStatusListener->IsCloudSyncOn(userId, bundleName);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsCloudSyncOnTest003 failed";
    }
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest003 end";
}

/**
 * @tc.name: IsCloudSyncOnTest004
 * @tc.desc: Verify the IsCloudSyncOn function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, IsCloudSyncOnTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest004 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "";
        const int32_t userId = -1;
        packageStatusListener->IsCloudSyncOn(userId, bundleName);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsCloudSyncOnTest004 failed";
    }
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest004 end";
}

/**
 * @tc.name: IsCloudSyncOnTest005
 * @tc.desc: Verify the IsCloudSyncOn function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, IsCloudSyncOnTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest005 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "Test";
        const int32_t userId = 0;
        packageStatusListener->IsCloudSyncOn(userId, bundleName);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsCloudSyncOnTest005 failed";
    }
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest005 end";
}

/**
 * @tc.name: IsCloudSyncOnTest006
 * @tc.desc: Verify the IsCloudSyncOn function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(PackageStatusListenerTest, IsCloudSyncOnTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest006 start";
    try {
        auto dataSyncManager = std::make_shared<DataSyncManager>();
        auto packageStatusListener = std::make_shared<PackageStatusListener>(dataSyncManager);
        const std::string bundleName = "";
        const int32_t userId = 0;
        packageStatusListener->IsCloudSyncOn(userId, bundleName);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsCloudSyncOnTest006 failed";
    }
    GTEST_LOG_(INFO) << "IsCloudSyncOnTest006 end";
}
} // namespace OHOS::FileManagement::CloudSync::Test
