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

#include <gtest/gtest.h>
#include <memory>
#define protect public
#include "cloud_sync_service_proxy.h"
#include "dfs_error.h"
#include "i_cloud_download_callback_mock.h"
#include "i_cloud_sync_service_mock.h"
#include "service_callback_mock.h"

#include "peer_holder.h"
#include "if_system_ability_manager.h"
#include "system_ability_manager_client_mock.h"
#include "if_system_ability_manager_mock.h"
#include "cloud_sync_service_proxy_lite.cpp"

#undef protect
namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

static const int64_t FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID = 5204;

class CloudSyncServiceProxyLiteTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<CloudSyncServiceProxy> proxy_ = nullptr;
    sptr<CloudSyncServiceMock> mock_ = nullptr;
    sptr<CloudSyncCallbackMock> remote_ = nullptr;
    sptr<CloudDownloadCallbackMock> download_ = nullptr;
    static inline shared_ptr<SystemAbilityManagerClientMock> smc_ = nullptr;
    static inline shared_ptr<ISystemAbilityManagerMock> samgr = make_shared<ISystemAbilityManagerMock>();
};

void CloudSyncServiceProxyLiteTest::SetUpTestCase(void)
{
    smc_ = make_shared<SystemAbilityManagerClientMock>();
    SystemAbilityManagerClientMock::smc = smc_;
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = sptr<ISystemAbilityManager>(samgr.get());
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncServiceProxyLiteTest::TearDownTestCase(void)
{
    SystemAbilityManagerClientMock::smc = nullptr;
    smc_ = nullptr;
    samgr = nullptr;
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = nullptr;
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncServiceProxyLiteTest::SetUp(void)
{
    mock_ = sptr(new CloudSyncServiceMock());
    proxy_ = make_shared<CloudSyncServiceProxy>(mock_);
    remote_ = sptr(new CloudSyncCallbackMock());
    download_ = sptr(new CloudDownloadCallbackMock());
    std::cout << "SetUp" << std::endl;
}

void CloudSyncServiceProxyLiteTest::TearDown(void)
{
    proxy_ = nullptr;
    mock_ = nullptr;
    remote_ = nullptr;
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: TriggerSyncInnerInner001
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyLiteTest, TriggerSyncInner001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TriggerSyncInner Start";
    string bundleName = "com.ohos.photos";
    int32_t userId = 100;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(-1));

    int result = proxy_->TriggerSyncInner(bundleName, userId);
    EXPECT_EQ(result, E_BROKEN_IPC);
    GTEST_LOG_(INFO) << "TriggerSyncInner End";
}

/**
 * @tc.name: TriggerSyncInner002
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyLiteTest, TriggerSyncInner002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TriggerSyncInner Start";
    string bundleName = "com.ohos.photos";
    int32_t userId = 100;
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));

    int result = proxy_->TriggerSyncInner(bundleName, userId);
    EXPECT_EQ(result, E_OK);
    GTEST_LOG_(INFO) << "TriggerSyncInner End";
}

/**
 * @tc.name: OnLoadSystemAbilityTest001
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyLiteTest, OnLoadSystemAbilityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnLoadSystemAbilityTest001 Start";

    sptr<IRemoteObject> remoteObject = sptr(new CloudSyncServiceMock());
    int32_t systemAbilityId = FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID;
    sptr<CloudSyncServiceProxy::ServiceProxyLoadCallback> callback =
        new CloudSyncServiceProxy::ServiceProxyLoadCallback();
    callback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    callback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    EXPECT_TRUE(callback->isLoadSuccess_.load());

    GTEST_LOG_(INFO) << "OnLoadSystemAbilityTest001 End";
}

/**
 * @tc.name: OnLoadSystemAbilityTest001
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyLiteTest, GetInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest001 Start";
    try {
        auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
        EXPECT_NE(CloudSyncServiceProxy, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest001 End";
}

/**
 * @tc.name: GetInstanceTest002
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyLiteTest, GetInstanceTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest002 Start";
    EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(nullptr));
    shared_ptr<CloudSyncServiceProxy> proxy_ = make_shared<CloudSyncServiceProxy>(nullptr);
    proxy_->serviceProxy_ = nullptr;
    auto ret = proxy_->GetInstance();
    EXPECT_TRUE(ret == nullptr);
    GTEST_LOG_(INFO) << "GetInstanceTest002 End";
}

/**
 * @tc.name: GetInstanceTest003
 * @tc.desc: Verify the TriggerSyncInner function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceProxyLiteTest, GetInstanceTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest003 Start";
    try {
        auto sysAbilityManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
        EXPECT_CALL(*sysAbilityManager, LoadSystemAbility(An<int32_t>(), An<const sptr<ISystemAbilityLoadCallback>&>()))
            .WillOnce(Return(E_PERMISSION));
        shared_ptr<CloudSyncServiceProxy> proxy_ = make_shared<CloudSyncServiceProxy>(nullptr);
        proxy_->serviceProxy_ = nullptr;
        auto ret = proxy_->GetInstance();
        EXPECT_TRUE(ret == nullptr);
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
HWTEST_F(CloudSyncServiceProxyLiteTest, GetInstanceTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest004 Start";
    try {
        auto sysAbilityManager = sptr<ISystemAbilityManagerMock>(new ISystemAbilityManagerMock());
        EXPECT_CALL(*smc_, GetSystemAbilityManager()).WillOnce(Return(sysAbilityManager));
        EXPECT_CALL(*sysAbilityManager, LoadSystemAbility(An<int32_t>(), An<const sptr<ISystemAbilityLoadCallback>&>()))
            .WillOnce(Return(ERR_OK));
        shared_ptr<CloudSyncServiceProxy> proxy_ = make_shared<CloudSyncServiceProxy>(nullptr);
        proxy_->serviceProxy_ = nullptr;
        auto ret = proxy_->GetInstance();
        EXPECT_TRUE(ret == nullptr);
    } catch (...) {
        GTEST_LOG_(ERROR) << "GetInstanceTest004 occurs an exception.";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest004 End";
}

} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS