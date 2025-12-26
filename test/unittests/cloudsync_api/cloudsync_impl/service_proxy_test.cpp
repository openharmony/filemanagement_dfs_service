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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <sstream>

#include "dfs_error.h"
#include "i_cloud_download_callback_mock.h"
#include "i_cloud_sync_service_mock.h"
#include "peer_holder.h"
#include "service_proxy.h"
#include "system_ability_manager_client_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

static const int64_t FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID = 5204;
class ServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<SystemAbilityManagerClientMock> smc_ = nullptr;
};

void ServiceProxyTest::SetUpTestCase(void)
{
    smc_ = make_shared<SystemAbilityManagerClientMock>();
    SystemAbilityManagerClientMock::smc = smc_;
    std::cout << "SetUpTestCase" << std::endl;
}

void ServiceProxyTest::TearDownTestCase(void)
{
    smc_ = nullptr;
    std::cout << "TearDownTestCase" << std::endl;
}

void ServiceProxyTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void ServiceProxyTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: GetInstanceTest001
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(ServiceProxyTest, GetInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest001 Start";
    try {
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(nullptr));
        auto ServiceProxy = ServiceProxy::GetInstance();
        EXPECT_TRUE(ServiceProxy == nullptr);
    } catch (...) {
        GTEST_LOG_(ERROR) << "GetInstanceTest001 occurs an exception.";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest001 End";
}

/**
 * @tc.name: GetInstanceTest002
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(ServiceProxyTest, GetInstanceTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest002 Start";
    try {
        auto sysAbilityManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
        auto object = sptr<CloudSyncServiceMock>(new CloudSyncServiceMock());
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
        EXPECT_CALL(*sysAbilityManager, CheckSystemAbility(_)).WillOnce(Return(object));

        auto ServiceProxy = ServiceProxy::GetInstance();
        EXPECT_TRUE(ServiceProxy != nullptr);
    } catch (...) {
        GTEST_LOG_(ERROR) << "GetInstanceTest002 occurs an exception.";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest002 End";
}

/**
 * @tc.name: GetInstanceTest003
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(ServiceProxyTest, GetInstanceTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest003 Start";
    try {
        auto sysAbilityManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
        EXPECT_CALL(*sysAbilityManager, CheckSystemAbility(_)).WillOnce(Return(nullptr));
        EXPECT_CALL(*sysAbilityManager, LoadSystemAbility(An<int32_t>(), An<const sptr<ISystemAbilityLoadCallback>&>()))
            .WillOnce(Return(E_PERMISSION));

        auto ServiceProxy = ServiceProxy::GetInstance();
        EXPECT_TRUE(ServiceProxy == nullptr);
    } catch (...) {
        GTEST_LOG_(ERROR) << "GetInstanceTest003 occurs an exception.";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest003 End";
}

/**
 * @tc.name: GetInstanceTest004
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(ServiceProxyTest, GetInstanceTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest004 Start";
    try {
        auto sysAbilityManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
        EXPECT_CALL(*sysAbilityManager, CheckSystemAbility(_)).WillOnce(Return(nullptr));
        EXPECT_CALL(*sysAbilityManager, LoadSystemAbility(An<int32_t>(), An<const sptr<ISystemAbilityLoadCallback>&>()))
            .WillOnce(Return(ERR_OK));
        auto ServiceProxy = ServiceProxy::GetInstance();
        EXPECT_TRUE(ServiceProxy == nullptr);
    } catch (...) {
        GTEST_LOG_(ERROR) << "GetInstanceTest004 occurs an exception.";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest004 End";
}

/**
 * @tc.name: OnLoadSystemAbilityTest001
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(ServiceProxyTest, OnLoadSystemAbilityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnLoadSystemAbilityTest001 Start";
    try {
        sptr<IRemoteObject> remoteObject = nullptr;
        int32_t systemAbilityId = FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID;
        sptr<ServiceProxy::ServiceProxyLoadCallback> callback =
            new ServiceProxy::ServiceProxyLoadCallback();

        callback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);

        EXPECT_FALSE(callback->isLoadSuccess_.load());
    } catch (...) {
        GTEST_LOG_(ERROR) << "OnLoadSystemAbilityTest001 occurs an exception.";
    }
    GTEST_LOG_(INFO) << "OnLoadSystemAbilityTest001 End";
}

/**
 * @tc.name: OnLoadSystemAbilityTest002
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(ServiceProxyTest, OnLoadSystemAbilityTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnLoadSystemAbilityTest002 Start";
    try {
        sptr<IRemoteObject> remoteObject = sptr<CloudSyncServiceMock>(new CloudSyncServiceMock());
        int32_t systemAbilityId = FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID;

        sptr<ServiceProxy::ServiceProxyLoadCallback> callback =
            new ServiceProxy::ServiceProxyLoadCallback();
        callback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);

        EXPECT_TRUE(callback->isLoadSuccess_.load());
    } catch (...) {
        GTEST_LOG_(ERROR) << "OnLoadSystemAbilityTest002 occurs an exception.";
    }
    GTEST_LOG_(INFO) << "OnLoadSystemAbilityTest002 End";
}

}
}
}