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

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "system_notifier.h"
#include "notification_helper.h"
#include "image_source.h"
#include "dfs_error.h"
#include "device_manager.h"
#include "utils_log.h"
#include "file_ex.h"

namespace {
int g_publishNotification = OHOS::FileManagement::ERR_OK;
int g_cancelNotification = OHOS::FileManagement::ERR_OK;
std::string g_loadStringFromFile = "";
}  // namespace

namespace OHOS {
bool LoadStringFromFile(const std::string& filePath, std::string& content)
{
    content = g_loadStringFromFile;
    return true;
}
}

namespace OHOS {
namespace Notification {
ErrCode NotificationHelper::PublishNotification(const NotificationRequest &request, const std::string &instanceKey)
{
    return g_publishNotification;
}

ErrCode NotificationHelper::CancelNotification(int32_t notificationId, const std::string &instanceKey)
{
    return g_cancelNotification;
}
}  // namespace Notification
}  // namespace OHOS

namespace OHOS {
namespace Storage {
namespace DistributedFile {
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
    g_publishNotification = OHOS::FileManagement::ERR_OK;
    g_cancelNotification = OHOS::FileManagement::ERR_OK;
    g_loadStringFromFile = "{\"key-value\":[{\"key\":\"notification_title\",\"value\":\"文件桥\"},"
    "{\"key\":\"notification_text\",\"value\":\"可在 {} 上管理本机文件\"},"
    "{\"key\":\"capsule_title\",\"value\":\"已连接\"},"
    "{\"key\":\"button_name\",\"value\":\"断开\"}]}";
}

void SystemNotifierTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SystemNotifierTest_CreateLocalLiveView_001
 * @tc.desc: Verify CreateLocalLiveView creates notification successfully.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(SystemNotifierTest, CreateLocalLiveView_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateLocalLiveView_001 start";

    auto &notifier = SystemNotifier::GetInstance();
    int32_t ret = notifier.CreateLocalLiveView(testNetworkId);
    EXPECT_EQ(ret, ERR_OK);

    g_publishNotification = ERR_BAD_VALUE;
    int32_t ret2 = notifier.CreateLocalLiveView(testNetworkId);
    EXPECT_EQ(ret2, ERR_BAD_VALUE);

    GTEST_LOG_(INFO) << "CreateLocalLiveView_001 end";
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

    g_publishNotification = ERR_BAD_VALUE;
    int32_t ret2 = notifier.CreateNotification(testNetworkId);
    EXPECT_EQ(ret2, ERR_BAD_VALUE);

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
 * @tc.name: SystemNotifierTest_CancelNotifyByNotificationId_001
 * @tc.desc: Verify CancelNotifyByNotificationId cancels notification successfully.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(SystemNotifierTest, CancelNotifyByNotificationId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelNotifyByNotificationId_001 start";

    auto &notifier = SystemNotifier::GetInstance();
    int32_t res = notifier.CreateNotification(testNetworkId);
    EXPECT_EQ(res, ERR_OK);

    notifier.notificationMap_["network-1"] = 666;
    notifier.notificationMap_["network-2"] = 999;
    notifier.notificationMap_["network-3"] = 333;

    int32_t ret = notifier.DestroyNotifyByNotificationId(666);
    EXPECT_EQ(ret, ERR_OK);

    GTEST_LOG_(INFO) << "CancelNotifyByNotificationId_001 end";
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
 * @tc.name: SystemNotifierTest_CancelNotifyByNotificationId_002
 * @tc.desc: Verify CancelNotifyByNotificationId handles non-existent notificationId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(SystemNotifierTest, CancelNotifyByNotificationId_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CancelNotifyByNotificationId_002 start";

    g_cancelNotification = ERR_BAD_VALUE;
    auto &notifier = SystemNotifier::GetInstance();
    int32_t ret = notifier.DestroyNotifyByNotificationId(9999);  // Non-existent ID
    EXPECT_EQ(ret, ERR_OK);

    GTEST_LOG_(INFO) << "CancelNotifyByNotificationId_002 end";
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

/**
 * @tc.name: SystemNotifierTest_GetPixelMap_001
 * @tc.desc: Verify GetPixelMap.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(SystemNotifierTest, GetPixelMap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetPixelMap_001 start";

    auto &notifier = SystemNotifier::GetInstance();
    std::shared_ptr<Media::PixelMap> nullPixelMap = nullptr;
    auto ret = notifier.GetPixelMap("testPath", nullPixelMap);
    EXPECT_EQ(ret, false);

    std::shared_ptr<Media::PixelMap> tempPixelMap = std::make_shared<Media::PixelMap>();
    auto ret2 = notifier.GetPixelMap("testPath/testPath", tempPixelMap);
    EXPECT_EQ(ret2, false);

    auto ret3 = notifier.GetPixelMap(
        "/system/etc/dfs_service/resources/icon/capsule_icon.png", nullPixelMap);
    EXPECT_EQ(ret3, true);
    GTEST_LOG_(INFO) << "GetPixelMap_001 end";
}

/**
 * @tc.name: SystemNotifierTest_UpdateResourceMap_001
 * @tc.desc: Verify UpdateResourceMap.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(SystemNotifierTest, UpdateResourceMap_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateResourceMap_001 start";

    auto &notifier = SystemNotifier::GetInstance();
    g_loadStringFromFile = "";
    EXPECT_NO_THROW(notifier.UpdateResourceMap("/system/etc/dfs_service/resources/i18/xxx.json"));

    g_loadStringFromFile = "{\"key-value_no\":[{\"key\":\"notification_title\",\"value\":\"文件桥\"},"
    "{\"key\":\"notification_text\",\"value\":\"可在 {} 上管理本机文件\"},"
    "{\"key\":\"capsule_title\",\"value\":\"已连接\"},"
    "{\"key\":\"button_name\",\"value\":\"断开\"}]}";
    EXPECT_NO_THROW(notifier.UpdateResourceMap("/system/etc/dfs_service/resources/i18/xxx.json"));

    g_loadStringFromFile = "{\"key-value\":[{\"key_no\":\"notification_title\",\"value\":\"文件桥\"},"
    "{\"key\":\"notification_text\",\"value\":\"可在 {} 上管理本机文件\"},"
    "{\"key\":\"capsule_title\",\"value\":\"已连接\"},"
    "{\"key\":\"button_name\",\"value\":\"断开\"}]}";
    EXPECT_NO_THROW(notifier.UpdateResourceMap("/system/etc/dfs_service/resources/i18/xxx.json"));

    g_loadStringFromFile = "{\"key-value\":[{\"key\":\"notification_title\",\"value_no\":\"文件桥\"},"
    "{\"key\":\"notification_text\",\"value\":\"可在 {} 上管理本机文件\"},"
    "{\"key\":\"capsule_title\",\"value\":\"已连接\"},"
    "{\"key\":\"button_name\",\"value\":\"断开\"}]}";
    EXPECT_NO_THROW(notifier.UpdateResourceMap("/system/etc/dfs_service/resources/i18/xxx.json"));

    g_loadStringFromFile = "{\"key-value\":[{\"key\":\"notification_title\",\"value\":\"文件桥\"}]}";
    EXPECT_NO_THROW(notifier.UpdateResourceMap("/system/etc/dfs_service/resources/i18/xxx.json"));

    g_loadStringFromFile = "{\"key-value\":[{\"key\":notification_title,\"value\":文件桥}]}";
    EXPECT_NO_THROW(notifier.UpdateResourceMap("/system/etc/dfs_service/resources/i18/xxx.json"));

    GTEST_LOG_(INFO) << "UpdateResourceMap_001 end";
}

/**
 * @tc.name: SystemNotifierTest_GetKeyValue_001
 * @tc.desc: Verify getKeyValue.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(SystemNotifierTest, GetKeyValue_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetKeyValue_001 start";

    auto &notifier = SystemNotifier::GetInstance();
    EXPECT_EQ(notifier.GetKeyValue("no_such_key"), "");

    GTEST_LOG_(INFO) << "GetKeyValue_001 end";
}

}  // namespace Test
}  // namespace DistributedFile
}  // namespace Storage
}  // namespace OHOS