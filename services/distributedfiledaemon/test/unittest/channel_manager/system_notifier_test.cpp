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

#include "channel_manager.h"
#include "control_cmd_parser.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "system_notifier.h"
#include "dfs_error.h"
#include "device_manager.h"
#include "utils_log.h"

namespace OHOS {
bool LoadStringFromFile(const std::string& filePath, std::string& content)
{
    return true;
}
}

namespace OHOS {
namespace Storage {
namespace DistributedFile {

int32_t ChannelManager::SendRequest(const std::string &networkId,
                                    ControlCmd &request,
                                    ControlCmd &response,
                                    bool needResponse)
{
    return OHOS::FileManagement::ERR_OK;
}

int32_t ChannelManager::NotifyClient(const std::string &networkId, const ControlCmd &request)
{
    return OHOS::FileManagement::ERR_OK;
}

namespace Test {
using namespace testing;
using namespace testing::ext;
using OHOS::FileManagement::ERR_BAD_VALUE;
using OHOS::FileManagement::ERR_OK;

class SystemNotifierTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    static inline std::string testNetworkId = "test-network-id";
    static inline int32_t testNotificationId = 1001;
};

void SystemNotifierTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SystemNotifierTest SetUpTestCase";
}

void SystemNotifierTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "SystemNotifierTest TearDownTestCase";
}

void SystemNotifierTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void SystemNotifierTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SystemNotifierTest_CreateNotification_001
 * @tc.desc: Verify CreateNotification creates notification successfully.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(SystemNotifierTest, CreateNotification_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateNotification_001 start";

    auto &notifier = SystemNotifier::GetInstance();
    int32_t ret = notifier.CreateNotification(testNetworkId);
    EXPECT_EQ(ret, ERR_OK);

    int32_t ret2 = notifier.CreateNotification(testNetworkId);
    EXPECT_EQ(ret2, ERR_OK);
    notifier.notificationMap_.clear();

    GTEST_LOG_(INFO) << "CreateNotification_001 end";
}

/**
 * @tc.name: SystemNotifierTest_CancelNotifyByNetworkId_001
 * @tc.desc: Verify CancelNotifyByNetworkId cancels notification successfully.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(SystemNotifierTest, CancelNotifyByNetworkId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelNotifyByNetworkId_001 start";

    auto &notifier = SystemNotifier::GetInstance();
    notifier.CreateNotification(testNetworkId);

    int32_t ret = notifier.DestroyNotifyByNetworkId(testNetworkId);
    EXPECT_EQ(ret, ERR_OK);

    GTEST_LOG_(INFO) << "CancelNotifyByNetworkId_001 end";
}

/**
 * @tc.name: SystemNotifierTest_CancelNotifyByNetworkId_002
 * @tc.desc: Verify CancelNotifyByNetworkId handles non-existent networkId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(SystemNotifierTest, CancelNotifyByNetworkId_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelNotifyByNetworkId_002 start";

    auto &notifier = SystemNotifier::GetInstance();
    int32_t ret = notifier.DestroyNotifyByNetworkId("non-existent-network-id");
    EXPECT_EQ(ret, ERR_BAD_VALUE);

    GTEST_LOG_(INFO) << "CancelNotifyByNetworkId_002 end";
}

/**
 * @tc.name: SystemNotifierTest_NotificationLifecycle_001
 * @tc.desc: Verify complete notification lifecycle.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(SystemNotifierTest, NotificationLifecycle_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotificationLifecycle_001 start";

    auto &notifier = SystemNotifier::GetInstance();
    int32_t ret = notifier.CreateNotification(testNetworkId);
    EXPECT_EQ(ret, ERR_OK);

    ret = notifier.DestroyNotifyByNetworkId(testNetworkId);
    EXPECT_EQ(ret, ERR_OK);

    GTEST_LOG_(INFO) << "NotificationLifecycle_001 end";
}

/**
 * @tc.name: SystemNotifierTest_MultipleNotifications_001
 * @tc.desc: Verify handling of multiple notifications.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(SystemNotifierTest, MultipleNotifications_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MultipleNotifications_001 start";

    std::string networkId1 = "network-id-1";
    std::string networkId2 = "network-id-2";

    auto &notifier = SystemNotifier::GetInstance();
    int32_t ret = notifier.CreateNotification(networkId1);
    EXPECT_EQ(ret, ERR_OK);
    ret = notifier.CreateNotification(networkId2);
    EXPECT_EQ(ret, ERR_OK);

    ret = notifier.DestroyNotifyByNetworkId(networkId2);
    EXPECT_EQ(ret, ERR_OK);
    ret = notifier.DestroyNotifyByNetworkId(networkId1);
    EXPECT_EQ(ret, ERR_OK);

    GTEST_LOG_(INFO) << "MultipleNotifications_001 end";
}

}  // namespace Test
}  // namespace DistributedFile
}  // namespace Storage
}  // namespace OHOS