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

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>

#include "cloud_sync_manager_impl.cpp"
#include "dfs_error.h"
#include "icloud_sync_service.h"
#include "iservice_registry.h"
#include "meta_file.h"
#include "os_account_manager_mock.h"
#include "service_callback_mock.h"
#include "service_proxy.h"
#include "service_proxy_mock.h"
#include "system_ability_manager_client_mock.h"


namespace OHOS {
namespace FileManagement::CloudSync {
namespace fs = std::filesystem;
using namespace testing::ext;
using namespace testing;
using namespace std;

class CloudSyncManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<MockServiceProxy> proxy_ = nullptr;
    sptr<CloudSyncServiceMock> serviceProxy_ = nullptr;
    std::shared_ptr<SystemAbilityManagerClientMock> saMgrClient_{nullptr};
    sptr<ISystemAbilityManagerMock> saMgr_{nullptr};
    std::shared_ptr<OsAccountManagerMethodMock> OsAccountMethodMock_ = nullptr;
};

class CloudSyncCallbackDerived : public CloudSyncCallback {
public:
    void OnSyncStateChanged(CloudSyncState state, ErrorType error)
    {
        std::cout << "OnSyncStateChanged" << std::endl;
    }
};

class CloudDownloadCallbackDerived : public CloudDownloadCallback {
public:
    void OnDownloadProcess(const DownloadProgressObj &progress)
    {
        std::cout << "OnDownloadProcess" << std::endl;
    }
};

class DowngradeDlCallbackDerived : public DowngradeDlCallback {
public:
    void OnDownloadProcess(const DowngradeProgress &progress)
    {
        std::cout << "OnDownloadProcess" << std::endl;
    }
};

void CloudSyncManagerImplTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncManagerImplTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncManagerImplTest::SetUp(void)
{
    saMgrClient_ = make_shared<SystemAbilityManagerClientMock>();
    proxy_ = std::make_shared<MockServiceProxy>();
    IserviceProxy::proxy_ = proxy_;
    serviceProxy_ = sptr(new CloudSyncServiceMock());
    saMgr_ = sptr(new ISystemAbilityManagerMock());
    ISystemAbilityManagerClient::smc = saMgrClient_;
    OsAccountMethodMock_ = make_shared<OsAccountManagerMethodMock>();
    OsAccountManagerMethod::osMethod_ = OsAccountMethodMock_;
    CloudSyncManagerImpl::GetInstance().isFirstCall_.test_and_set();
    std::cout << "SetUp" << std::endl;
}

void CloudSyncManagerImplTest::TearDown(void)
{
    ISystemAbilityManagerClient::smc = nullptr;
    IserviceProxy::proxy_ = nullptr;
    saMgrClient_ = nullptr;
    proxy_ = nullptr;
    serviceProxy_ = nullptr;
    saMgr_ = nullptr;
    OsAccountMethodMock_ = nullptr;
    OsAccountManagerMethod::osMethod_ = nullptr;
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: RegisterCallbackTest002
 * @tc.desc: Verify the RegisterCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, RegisterCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCallbackTest Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b670";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudSyncCallbackDerived>();
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, RegisterCallbackInner(_, _, _)).WillOnce(Return(E_PERMISSION_DENIED));
        int32_t res = CloudSyncManagerImpl::GetInstance().RegisterCallback(callbackInfo);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterCallbackTest End";
}

/*
 * @tc.name: UnRegisterCallbackTest001
 * @tc.desc: Verify the UnRegisterCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, UnRegisterCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterCallbackTest Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b660";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudSyncCallbackDerived>();
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, UnRegisterCallbackInner(_, _)).WillOnce(Return(E_OK));
        int32_t res = CloudSyncManagerImpl::GetInstance().UnRegisterCallback(callbackInfo);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "UnRegisterCallbackTest End";
}

/*
 * @tc.name: UnRegisterCallbackTest002
 * @tc.desc: Verify the UnRegisterCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, UnRegisterCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterCallbackTest002 Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = nullptr;
        int32_t res = CloudSyncManagerImpl::GetInstance().UnRegisterCallback(callbackInfo);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterCallbackTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "UnRegisterCallbackTest002 End";
}

/*
 * @tc.name: UnRegisterFileSyncCallbackTest001
 * @tc.desc: Verify the UnRegisterCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, UnRegisterFileSyncCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterFileSyncCallbackTest001 Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudSyncCallbackDerived>();
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, UnRegisterFileSyncCallbackInner(_, _)).WillOnce(Return(E_OK));
        int32_t res = CloudSyncManagerImpl::GetInstance().UnRegisterFileSyncCallback(callbackInfo);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterFileSyncCallbackTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "UnRegisterFileSyncCallbackTest001 End";
}

/*
 * @tc.name: UnRegisterFileSyncCallbackTest002
 * @tc.desc: Verify the UnRegisterCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, UnRegisterFileSyncCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterFileSyncCallbackTest002 Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = nullptr;
        int32_t res = CloudSyncManagerImpl::GetInstance().UnRegisterFileSyncCallback(callbackInfo);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterFileSyncCallbackTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "UnRegisterFileSyncCallbackTest002 End";
}

/**
 * @tc.name: StartSyncTest001
 * @tc.desc: Verify the StartSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartSyncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncTest Start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountLocalIdFromProcess)
            .WillOnce(DoAll(SetArgReferee<0>(userId), Return(E_OK)));
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, StartSyncInner(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        int32_t res = CloudSyncManagerImpl::GetInstance().StartSync(bundleName);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncTest End";
}

/**
 * @tc.name: StartSyncTest002
 * @tc.desc: Verify the StartSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartSyncTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncTest002 Start";
    try {
        bool forceFlag = true;
        shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncCallbackDerived>();
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountLocalIdFromProcess)
            .WillOnce(DoAll(SetArgReferee<0>(userId), Return(E_OK)));
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, RegisterCallbackInner(_, _, _)).WillOnce(Return(E_OK));
        CloudSyncManagerImpl::GetInstance().isFirstCall_.clear();
        EXPECT_CALL(*serviceProxy_, StartSyncInner(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        int32_t res = CloudSyncManagerImpl::GetInstance().StartSync(forceFlag, callback);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncTest002 End";
}

/**
 * @tc.name: StartSyncTest003
 * @tc.desc: Verify the StartSync function.
 * @tc.type: FUNC
 * @tc.require: 2544
 */
HWTEST_F(CloudSyncManagerImplTest, StartSyncTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncTest003 Start";
    try {
        bool forceFlag = true;
        shared_ptr<CloudSyncCallback> callback = nullptr;
        string bundleName = "com.ohos.photos";
        int32_t res = CloudSyncManagerImpl::GetInstance().StartSync(forceFlag, callback);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncTest003 End";
}

/**
 * @tc.name: StartSyncTest004
 * @tc.desc: Verify the StartSync function.
 * @tc.type: FUNC
 * @tc.require: 2544
 */
HWTEST_F(CloudSyncManagerImplTest, StartSyncTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncTest004 Start";
    try {
        bool forceFlag = true;
        shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncCallbackDerived>();
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountLocalIdFromProcess)
            .WillOnce(DoAll(SetArgReferee<0>(userId), Return(E_OK)));
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        int32_t res = CloudSyncManagerImpl::GetInstance().StartSync(forceFlag, callback);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncTest004 End";
}

/**
 * @tc.name: StartSyncTest005
 * @tc.desc: Verify the StartSync function.
 * @tc.type: FUNC
 * @tc.require: 2544
 */
HWTEST_F(CloudSyncManagerImplTest, StartSyncTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncTest005 Start";
    try {
        bool forceFlag = true;
        CloudSyncManagerImpl::GetInstance().isFirstCall_.clear();
        shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncCallbackDerived>();
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountLocalIdFromProcess)
            .WillOnce(DoAll(SetArgReferee<0>(userId), Return(E_OK)));
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, RegisterCallbackInner(_, _, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*serviceProxy_, StartSyncInner(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        EXPECT_CALL(*serviceProxy_, AsObject()).WillRepeatedly(Return(serviceProxy_->AsObject()));
        int32_t res = CloudSyncManagerImpl::GetInstance().StartSync(forceFlag, callback);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncTest005 End";
}

/**
 * @tc.name: StartSyncTest006
 * @tc.desc: Verify the StartSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartSyncTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncTest Start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 101;
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountLocalIdFromProcess)
            .WillOnce(DoAll(SetArgReferee<0>(userId), Return(E_OK)));
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountTypeFromProcess(_))
            .WillOnce(DoAll(SetArgReferee<0>(AccountSA::OsAccountType::PRIVATE), Return(E_OK)));
        int32_t res = CloudSyncManagerImpl::GetInstance().StartSync(bundleName);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncTest End";
}

/**
 * @tc.name: StartSyncTest007
 * @tc.desc: Verify the StartSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartSyncTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncTest Start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 101;
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountLocalIdFromProcess)
            .WillOnce(DoAll(SetArgReferee<0>(userId), Return(E_OK)));
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountTypeFromProcess(_))
            .WillOnce(DoAll(SetArgReferee<0>(AccountSA::OsAccountType::PRIVATE), Return(E_OK)));
        shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncCallbackDerived>();
        int32_t res = CloudSyncManagerImpl::GetInstance().StartSync(false, callback);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncTest End";
}

/**
 * @tc.name: StartFileSyncTest001
 * @tc.desc: Verify the StartFileSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartFileSyncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncTest Start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountLocalIdFromProcess)
            .WillOnce(DoAll(SetArgReferee<0>(userId), Return(E_OK)));
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, StartFileSyncInner(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        int32_t res = CloudSyncManagerImpl::GetInstance().StartFileSync(bundleName);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncTest End";
}

/**
 * @tc.name: StartFileSyncTest002
 * @tc.desc: Verify the StartFileSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartFileSyncTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFileSyncTest Start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 101;
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountLocalIdFromProcess)
            .WillOnce(DoAll(SetArgReferee<0>(userId), Return(E_OK)));
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountTypeFromProcess(_))
            .WillOnce(DoAll(SetArgReferee<0>(AccountSA::OsAccountType::PRIVATE), Return(E_OK)));
        int32_t res = CloudSyncManagerImpl::GetInstance().StartFileSync(bundleName);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFileSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "StartFileSyncTest End";
}

/**
 * @tc.name: GetSyncTimeTest001
 * @tc.desc: Verify the GetSyncTime function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, GetSyncTimeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncTimeTest Start";
    try {
        int64_t syncTime = 0;
        string bundleName = "com.ohos.photos";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, GetSyncTimeInner(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        int32_t res = CloudSyncManagerImpl::GetInstance().GetSyncTime(syncTime, bundleName);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncTimeTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetSyncTimeTest End";
}

/**
 * @tc.name: TriggerSyncTest002
 * @tc.desc: Verify the TriggerSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, TriggerSyncTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TriggerSyncTest Start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountLocalIdFromProcess)
            .WillOnce(DoAll(SetArgReferee<0>(userId), Return(E_OK)));
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, TriggerSyncInner(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        int32_t res = CloudSyncManagerImpl::GetInstance().TriggerSync(bundleName, userId);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TriggerSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "TriggerSyncTest End";
}

/*
 * @tc.name: StopSyncTest
 * @tc.desc: Verify the StopSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StopSyncTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopSyncTest Start";
    try {
        string bundleName = "com.ohos.photos";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, StopSyncInner(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        int res = CloudSyncManagerImpl::GetInstance().StopSync(bundleName);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "StopSyncTest End";
}

/*
 * @tc.name: ChangeAppSwitchTest
 * @tc.desc: Verify the ChangeAppSwitch function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, ChangeAppSwitchTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeAppSwitchTest Start";
    try {
        std::string accoutId = "accoutId";
        std::string bundleName = "bundleName";
        bool status = true;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, ChangeAppSwitch(_, _, _)).WillOnce(Return(E_INVAL_ARG));
        auto res = CloudSyncManagerImpl::GetInstance().ChangeAppSwitch(accoutId, bundleName, status);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ChangeAppSwitchTest FAILED";
    }
    GTEST_LOG_(INFO) << "ChangeAppSwitchTest End";
}

/*
 * @tc.name: NotifyEventChangeTest
 * @tc.desc: Verify the NotifyEventChange function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, NotifyEventChangeTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyEventChangeTest Start";
    try {
        int32_t userId = 100;
        std::string eventId = "eventId";
        std::string extraData = "extraData";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, NotifyEventChange(_, _, _)).WillOnce(Return(E_PERMISSION_DENIED));
        auto res = CloudSyncManagerImpl::GetInstance().NotifyEventChange(userId, eventId, extraData);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NotifyEventChangeTest FAILED";
    }
    GTEST_LOG_(INFO) << "NotifyEventChangeTest End";
}

/*
 * @tc.name: StartDownloadFileTest001
 * @tc.desc: Verify the StartDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartDownloadFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string uri = "uri";
        int64_t downloadId = 0;
        shared_ptr<CloudDownloadCallback> callback = make_shared<CloudDownloadCallbackDerived>();
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, StartDownloadFile).WillOnce(Return(E_PERMISSION_DENIED));
        auto res = CloudSyncManagerImpl::GetInstance().StartDownloadFile(uri, callback, downloadId);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StartDownloadFileTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "StartDownloadFileTest001 End";
}

/*
 * @tc.name: StartDownloadFileTest002
 * @tc.desc: Verify the StartDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartDownloadFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string uri = "uri";
        int64_t downloadId = 0;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        auto res = CloudSyncManagerImpl::GetInstance().StartDownloadFile(uri, nullptr, downloadId);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StartDownloadFileTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "StartDownloadFileTest002 End";
}

/*
 * @tc.name: StartFileCacheTest
 * @tc.desc: Verify the StartFileCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartFileCacheTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFileCacheTest Start";
    try {
        std::string uri = "uri";
        int64_t downloadId = 0;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, StartFileCache).WillOnce(Return(E_PERMISSION_DENIED));
        auto res = CloudSyncManagerImpl::GetInstance().StartFileCache({uri}, downloadId);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFileCacheTest FAILED";
    }
    GTEST_LOG_(INFO) << "StartFileCacheTest End";
}

/*
 * @tc.name: StopDownloadFileTest
 * @tc.desc: Verify the StopDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StopDownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        int64_t downloadId = 1;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, StopDownloadFile(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        auto res = CloudSyncManagerImpl::GetInstance().StopDownloadFile(downloadId, true);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopDownloadFileTest FAILED";
    }
    GTEST_LOG_(INFO) << "StopDownloadFileTest End";
}

/*
 * @tc.name: EnableCloudTest
 * @tc.desc: Verify the EnableCloud function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, EnableCloudTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string accoutId = "accoutId";
        SwitchDataObj switchData;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, EnableCloud(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        auto res = CloudSyncManagerImpl::GetInstance().EnableCloud(accoutId, switchData);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " EnableCloudTest FAILED";
    }
    GTEST_LOG_(INFO) << "EnableCloudTest End";
}

/*
 * @tc.name: DisableCloudTest
 * @tc.desc: Verify the DisableCloud function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, DisableCloudTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string accoutId = "accoutId";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, DisableCloud(_)).WillOnce(Return(E_PERMISSION_DENIED));
        auto res = CloudSyncManagerImpl::GetInstance().DisableCloud(accoutId);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " DisableCloudTest FAILED";
    }
    GTEST_LOG_(INFO) << "DisableCloudTest End";
}

/*
 * @tc.name: CleanTest
 * @tc.desc: Verify the Clean function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, CleanTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string accoutId = "accoutId";
        CleanOptions cleanOptions;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, Clean(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        auto res = CloudSyncManagerImpl::GetInstance().Clean(accoutId, cleanOptions);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanTest FAILED";
    }
    GTEST_LOG_(INFO) << "CleanTest End";
}

/*
 * @tc.name: CleanCacheTest
 * @tc.desc: Verify the CleanCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, CleanCacheTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCacheTest Start";
    try {
        string uri = "uri";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, CleanCacheInner(_)).WillOnce(Return(E_PERMISSION_DENIED));
        auto res = CloudSyncManagerImpl::GetInstance().CleanCache(uri);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanCacheTest FAILED";
    }
    GTEST_LOG_(INFO) << "CleanCacheTest End";
}

/*
 * @tc.name: CleanFileCacheTest001
 * @tc.desc: Verify the CleanFileCache function.
 * @tc.type: FUNC
 * @tc.require: ICK6VD
 */
HWTEST_F(CloudSyncManagerImplTest, CleanFileCacheTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanFileCacheTest001 Start";
    try {
        string uri = "uri";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, CleanFileCacheInner(_)).WillOnce(Return(E_OK));
        auto res = CloudSyncManagerImpl::GetInstance().CleanFileCache(uri);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanFileCacheTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanFileCacheTest001 End";
}

/*
 * @tc.name: CleanFileCacheTest002
 * @tc.desc: Verify the CleanFileCache function.
 * @tc.type: FUNC
 * @tc.require: ICK6VD
 */
HWTEST_F(CloudSyncManagerImplTest, CleanFileCacheTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanFileCacheTest002 Start";
    try {
        string uri = "uri";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, CleanFileCacheInner(_)).WillOnce(Return(E_INVAL_ARG));
        auto res = CloudSyncManagerImpl::GetInstance().CleanFileCache(uri);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanFileCacheTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanFileCacheTest002 End";
}

/*
 * @tc.name: CleanFileCacheTest003
 * @tc.desc: Verify the CleanFileCache function.
 * @tc.type: FUNC
 * @tc.require: ICK6VD
 */
HWTEST_F(CloudSyncManagerImplTest, CleanFileCacheTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanFileCacheTest003 Start";
    try {
        string uri = "uri";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImpl::GetInstance().CleanFileCache(uri);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanFileCacheTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanFileCacheTest003 End";
}

HWTEST_F(CloudSyncManagerImplTest, BatchCleanFileTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchCleanFileTest1 Start";
    try {
        CleanFileInfo cleanFileInfo;
        std::vector<CleanFileInfo> fileInfo;
        fileInfo.emplace_back(cleanFileInfo);
        std::vector<std::string> failCloudId;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, BatchCleanFile(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        auto res = CloudSyncManagerImpl::GetInstance().BatchCleanFile(fileInfo, failCloudId);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatchCleanFileTest1 FAILED";
    }
    GTEST_LOG_(INFO) << "BatchCleanFileTest1 End";
}

HWTEST_F(CloudSyncManagerImplTest, ResetCursorTest, TestSize.Level1)
{
    string bundleName = "com.ohos.photos";
    EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
    EXPECT_CALL(*serviceProxy_, ResetCursor(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
    int res = CloudSyncManagerImpl::GetInstance().ResetCursor(bundleName);
    EXPECT_EQ(res, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncManagerImplTest, ResetCursorTest002, TestSize.Level1)
{
    string bundleName = "com.ohos.photos";
    EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
    EXPECT_CALL(*serviceProxy_, ResetCursor(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
    int res = CloudSyncManagerImpl::GetInstance().ResetCursor(false, bundleName);
    EXPECT_EQ(res, E_PERMISSION_DENIED);
}

HWTEST_F(CloudSyncManagerImplTest, ResetCursorTest003, TestSize.Level1)
{
    string bundleName = "com.ohos.photos";
    EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
    EXPECT_CALL(*serviceProxy_, ResetCursor(_, _)).Times(0);
    int res = CloudSyncManagerImpl::GetInstance().ResetCursor(false, bundleName);
    EXPECT_EQ(res, E_SA_LOAD_FAILED);
}

HWTEST_F(CloudSyncManagerImplTest, DownloadThumbTest, TestSize.Level1)
{
    EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
    EXPECT_CALL(*serviceProxy_, DownloadThumb()).WillOnce(Return(E_PERMISSION_DENIED));
    int32_t res = CloudSyncManagerImpl::GetInstance().DownloadThumb();
    EXPECT_EQ(res, E_PERMISSION_DENIED);
}

/*
 * @tc.name: BatchDentryFileInsertTest
 * @tc.desc: Verify the BatchDentryFileInsert function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, BatchDentryFileInsertTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchDentryFileInsertTest002 Start";
    try {
        std::vector<DentryFileInfo> fileInfo(MAX_DENTRY_FILE_SIZE - 1);
        std::vector<std::string> failCloudId;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, BatchDentryFileInsert(_, _)).WillOnce(Return(E_PERMISSION_DENIED));
        int32_t result = CloudSyncManagerImpl::GetInstance().BatchDentryFileInsert(fileInfo, failCloudId);

        EXPECT_EQ(result, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchDentryFileInsertTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "BatchDentryFileInsertTest002 End";
}

/**
 * @tc.name: StopFileCache
 * @tc.desc: Verify the StopFileCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 **/
HWTEST_F(CloudSyncManagerImplTest, StopFileCacheTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopFileCacheTest1 Start";
    try {
        int64_t downloadId = 1;
        bool needClean = true;
        int32_t timeout = 10;

        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, StopFileCache(_, _, _)).WillOnce(Return(E_PERMISSION_DENIED));
        int32_t result = CloudSyncManagerImpl::GetInstance().StopFileCache(downloadId, needClean, timeout);

        EXPECT_EQ(result, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopFileCacheTest1 FAILED";
    }
    GTEST_LOG_(INFO) << "StopFileCacheTest1 End";
}

/**
 * @tc.name: RegisterCallbackTest001
 * @tc.desc: Verify the RegisterCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, RegisterCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCallbackTest Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = nullptr;
        int32_t res = CloudSyncManagerImpl::GetInstance().RegisterCallback(callbackInfo);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterCallbackTest End";
}

/**
 * @tc.name: RegisterFileSyncCallbackTest001
 * @tc.desc: Verify the invalid input parameters of the RegisterFileSyncCallback function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncManagerImplTest, RegisterFileSyncCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterFileSyncCallbackTest001 Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = nullptr;
        int32_t res = CloudSyncManagerImpl::GetInstance().RegisterFileSyncCallback(callbackInfo);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterFileSyncCallbackTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterFileSyncCallbackTest001 End";
}

/*
 * @tc.name: NotifyDataChangeTest
 * @tc.desc: Verify the NotifyDataChange function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, NotifyDataChangeTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string accoutId = "accoutId";
        std::string bundleName = "bundleName";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, NotifyDataChange(_, _)).WillOnce(Return(E_OK));
        auto res = CloudSyncManagerImpl::GetInstance().NotifyDataChange(accoutId, bundleName);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " NotifyDataChangeTest FAILED";
    }
    GTEST_LOG_(INFO) << "NotifyDataChangeTest End";
}

/*
 * @tc.name: SetDeathRecipientTest001
 * @tc.desc: Verify the SetDeathRecipient function.
 * @tc.type: FUNC
 * @tc.require: ICEU6Z
 */
HWTEST_F(CloudSyncManagerImplTest, SetDeathRecipientTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeathRecipientTest001 Start";
    try {
        CloudSyncManagerImpl::GetInstance().SetDeathRecipient(serviceProxy_->AsObject());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetDeathRecipientTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipientTest001 End";
}

/*
 * @tc.name: SetDeathRecipientTest002
 * @tc.desc: Verify the SetDeathRecipient function.
 * @tc.type: FUNC
 * @tc.require: ICEU6Z
 */
HWTEST_F(CloudSyncManagerImplTest, SetDeathRecipientTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeathRecipientTest002 Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = nullptr;
        CloudSyncCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        CloudSyncManagerImpl::GetInstance().isFirstCall_.clear();
        CloudSyncManagerImpl::GetInstance().SetDeathRecipient(serviceProxy_->AsObject());
        CloudSyncCallbackClientManager::GetInstance().RemoveCallback(callbackInfo);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetDeathRecipientTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipientTest002 End";
}

/*
 * @tc.name: SetDeathRecipientTest003
 * @tc.desc: Verify the SetDeathRecipient function.
 * @tc.type: FUNC
 * @tc.require: ICEU6Z
 */
HWTEST_F(CloudSyncManagerImplTest, SetDeathRecipientTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeathRecipientTest003 Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudSyncCallback>();
        CloudSyncCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        CloudSyncManagerImpl::GetInstance().isFirstCall_.clear();
        CloudSyncManagerImpl::GetInstance().SetDeathRecipient(serviceProxy_->AsObject());
        CloudSyncCallbackClientManager::GetInstance().RemoveCallback(callbackInfo);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetDeathRecipientTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipientTest003 End";
}

HWTEST_F(CloudSyncManagerImplTest, BatchCleanFileTest2, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchCleanFileTest2 Start";
    try {
        CleanFileInfo cleanFileInfo;
        std::vector<CleanFileInfo> fileInfo;
        for (int i = 0; i < CLEAN_FILE_MAX_SIZE + 1; i++) {
            cleanFileInfo.cloudId = to_string(i);
            fileInfo.emplace_back(cleanFileInfo);
        }
        std::vector<std::string> failCloudId;
        auto res = CloudSyncManagerImpl::GetInstance().BatchCleanFile(fileInfo, failCloudId);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatchCleanFileTest2 FAILED";
    }
    GTEST_LOG_(INFO) << "BatchCleanFileTest2 End";
}

HWTEST_F(CloudSyncManagerImplTest, StartFileCacheTest001, TestSize.Level1)
{
    std::vector<std::string> uriVec;
    int64_t downloadId = 0;
    int32_t res = CloudSyncManagerImpl::GetInstance().StartFileCache(uriVec, downloadId);
    EXPECT_EQ(res, E_INVAL_ARG);
}

HWTEST_F(CloudSyncManagerImplTest, StartFileCacheTest002, TestSize.Level1)
{
    std::vector<std::string> uriVec(MAX_FILE_CACHE_NUM + 1, "uri");
    int64_t downloadId = 0;
    int32_t res = CloudSyncManagerImpl::GetInstance().StartFileCache(uriVec, downloadId);
    EXPECT_EQ(res, E_EXCEED_MAX_SIZE);
}

/*
 * @tc.name: ResetProxyCallbackTest001
 * @tc.desc: proxy 为 空.
 * @tc.type: FUNC
 * @tc.require: 2544
 */
HWTEST_F(CloudSyncManagerImplTest, ResetProxyCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResetProxyCallbackTest001 Start";
    try {
        uint32_t retryCount = 3;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImpl::GetInstance().ResetProxyCallback(retryCount);
        EXPECT_EQ(res, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ResetProxyCallbackTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ResetProxyCallbackTest001 End";
}

/*
 * @tc.name: ResetProxyCallbackTest002
 * @tc.desc: proxy 不为空，但callback为空
 * @tc.type: FUNC
 * @tc.require: 2544
 */
HWTEST_F(CloudSyncManagerImplTest, ResetProxyCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResetProxyCallbackTest002 Start";
    try {
        uint32_t retryCount = 3;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        auto res = CloudSyncManagerImpl::GetInstance().ResetProxyCallback(retryCount);
        EXPECT_EQ(res, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ResetProxyCallbackTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "ResetProxyCallbackTest002 End";
}

/*
 * @tc.name: ResetProxyCallbackTest002
 * @tc.desc: proxy 不为空，但callback 不为空, RegisterCallbackInner返回异常
 * @tc.type: FUNC
 * @tc.require: 2544
 */
HWTEST_F(CloudSyncManagerImplTest, ResetProxyCallbackTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResetProxyCallbackTest003 Start";
    try {
        uint32_t retryCount = 3;
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudSyncCallback>();
        CloudSyncCallbackClientManager::GetInstance().callbackList_.clear();
        CloudSyncCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, RegisterFileSyncCallbackInner(_, _, _)).WillOnce(Return(E_PERMISSION_DENIED));
        auto res = CloudSyncManagerImpl::GetInstance().ResetProxyCallback(retryCount);
        EXPECT_EQ(res, true);
        CloudSyncCallbackClientManager::GetInstance().RemoveCallback(callbackInfo);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ResetProxyCallbackTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "ResetProxyCallbackTest003 End";
}

/*
 * @tc.name: ResetProxyCallbackTest004
 * @tc.desc: proxy 不为空，但callback 不为空，RegisterCallbackInner 返回正常
 * @tc.type: FUNC
 * @tc.require: 2544
 */
HWTEST_F(CloudSyncManagerImplTest, ResetProxyCallbackTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResetProxyCallbackTest004 Start";
    try {
        uint32_t retryCount = 3;
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudSyncCallback>();
        CloudSyncCallbackClientManager::GetInstance().callbackList_.clear();
        CloudSyncCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, RegisterFileSyncCallbackInner(_, _, _)).WillOnce(Return(E_OK));
        auto res = CloudSyncManagerImpl::GetInstance().ResetProxyCallback(retryCount);
        EXPECT_EQ(res, true);
        CloudSyncCallbackClientManager::GetInstance().RemoveCallback(callbackInfo);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ResetProxyCallbackTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "ResetProxyCallbackTest004 End";
}

/*
 * @tc.name: BatchDentryFileInsertTest
 * @tc.desc: Verify the BatchDentryFileInsert function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, BatchDentryFileInsertTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchDentryFileInsertTest001 Start";
    try {
        std::vector<DentryFileInfo> fileInfo(MAX_DENTRY_FILE_SIZE + 1);
        std::vector<std::string> failCloudId;
        int32_t result = CloudSyncManagerImpl::GetInstance().BatchDentryFileInsert(fileInfo, failCloudId);

        EXPECT_EQ(result, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchDentryFileInsertTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "BatchDentryFileInsertTest001 End";
}

/*
 * @tc.name: OnAddSystemAbilityTest
 * @tc.desc: Verify the OnAddSystemAbility function.
 * @tc.type: FUNC
 * @tc.require: ICEU6Z
 */
HWTEST_F(CloudSyncManagerImplTest, OnAddSystemAbilityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAddSystemAbilityTest001 Start";
    try {
        CloudSyncManagerImpl::SystemAbilityStatusChange statusChange;
        int32_t systemAbilityId = 1;
        std::string deviceId = "testDeviceId";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr))
            .WillOnce(Return(nullptr)).WillOnce(Return(nullptr));
        EXPECT_NO_THROW(statusChange.OnAddSystemAbility(systemAbilityId, deviceId));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnAddSystemAbilityTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "OnAddSystemAbilityTest001 End";
}

/*
 * @tc.name: CleanGalleryDentryFile
 * @tc.desc: Verify the CleanGalleryDentryFile function.
 * @tc.type: FUNC
 * @tc.require: ICEU6Z
 */
HWTEST_F(CloudSyncManagerImplTest, CleanGalleryDentryFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest001 Start";
    try {
        CloudSyncManagerImpl::GetInstance().CleanGalleryDentryFile();

        EXPECT_FALSE(fs::exists("/storage/media/cloud/files/Photo"));
        EXPECT_FALSE(fs::exists("/storage/media/cloud/files/.thumbs/Photo"));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanGalleryDentryFileTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest001 End";
}

/*
 * @tc.name: CleanGalleryDentryFile
 * @tc.desc: Verify the CleanGalleryDentryFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, CleanGalleryDentryFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest002 Start";
    try {
        fs::remove_all("/storage/media/cloud/files/Photo");
        fs::remove_all("/storage/media/cloud/files/.thumbs/Photo");

        CloudSyncManagerImpl::GetInstance().CleanGalleryDentryFile();

        EXPECT_FALSE(fs::exists("/storage/media/cloud/files/Photo"));
        EXPECT_FALSE(fs::exists("/storage/media/cloud/files/.thumbs/Photo"));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanGalleryDentryFileTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest002 End";
}

/*
 * @tc.name: CleanGalleryDentryFile
 * @tc.desc: Verify the CleanGalleryDentryFile function.
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncManagerImplTest, CleanGalleryDentryFileTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest003 Start";
    try {
        system("rm -rf /storage/media/cloud/files/Photo/1/666666.jpg");
        fs::create_directories("/storage/media/cloud/files/Photo/1");
        std::ofstream("/storage/media/cloud/files/Photo/1/666666.jpg");
        std::string testDir = "/storage/cloud/files/Photo/1/666666.jpg";
        CloudSyncManagerImpl::GetInstance().CleanGalleryDentryFile(testDir);
        bool isExists = fs::exists("/storage/media/cloud/files/Photo/1/666666.jpg");
        system("rm -rf /storage/media/cloud/files/Photo/1/666666.jpg");
        EXPECT_FALSE(isExists);
    } catch (...) {
        system("rm -rf /storage/media/cloud/files/Photo/1/666666.jpg");
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanGalleryDentryFileTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest003 End";
}

/*
 * @tc.name: CleanGalleryDentryFile
 * @tc.desc: Verify the CleanGalleryDentryFile function.
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncManagerImplTest, CleanGalleryDentryFileTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest004 Start";
    try {
        system("rm -rf /storage/media/cloud/files/Photo/1/666666.jpg");
        fs::create_directories("/storage/media/cloud/files/Photo/1");
        std::ofstream("/storage/media/cloud/files/Photo/1/666666.jpg");
        std::string testDir = "/storage/666666.jpg";
        CloudSyncManagerImpl::GetInstance().CleanGalleryDentryFile(testDir);
        bool isExists = fs::exists("/storage/media/cloud/files/Photo/1/666666.jpg");
        system("rm -rf /storage/media/cloud/files/Photo/1/666666.jpg");
        EXPECT_TRUE(isExists);
    } catch (...) {
        system("rm -rf /storage/media/cloud/files/Photo/1/666666.jpg");
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanGalleryDentryFileTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest004 End";
}

/*
 * @tc.name: SubscribeListener
 * @tc.desc: Verify the SubscribeListener function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, SubscribeListenerTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubscribeListenerTest1 Start";
    try {
        CloudSyncCallbackClientManager::GetInstance().callbackList_.clear();
        CloudSyncManagerImpl::GetInstance().listener_ = nullptr;
        EXPECT_CALL(*saMgrClient_, GetSystemAbilityManager()).WillOnce(Return(saMgr_));
        CloudSyncManagerImpl::GetInstance().SubscribeListener();
        EXPECT_EQ(CloudSyncManagerImpl::GetInstance().listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SubscribeListenerTest1 FAILED";
    }
    GTEST_LOG_(INFO) << "SubscribeListenerTest1 End";
}

/*
 * @tc.name: SubscribeListener002
 * @tc.desc: Verify the SubscribeListener function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, SubscribeListener002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubscribeListener002 Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudSyncCallback>();
        CloudSyncCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        CloudSyncManagerImpl::GetInstance().listener_ = nullptr;
        EXPECT_CALL(*saMgrClient_, GetSystemAbilityManager()).WillOnce(Return(saMgr_));
        EXPECT_CALL(*saMgr_, SubscribeSystemAbility(_, _)).WillOnce(Return(0));
        CloudSyncManagerImpl::GetInstance().SubscribeListener();
        EXPECT_NE(CloudSyncManagerImpl::GetInstance().listener_, nullptr);
        CloudSyncCallbackClientManager::GetInstance().RemoveCallback(callbackInfo);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SubscribeListener002 FAILED";
    }
    GTEST_LOG_(INFO) << "SubscribeListener002 End";
}

/*
 * @tc.name: SubscribeListener003
 * @tc.desc: Verify the SubscribeListener function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, SubscribeListener003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubscribeListener003 Start";
    try {
        CloudSyncManagerImpl::GetInstance().listener_ = new CloudSyncManagerImpl::SystemAbilityStatusChange();
        EXPECT_CALL(*saMgrClient_, GetSystemAbilityManager()).WillOnce(Return(saMgr_));
        CloudSyncManagerImpl::GetInstance().SubscribeListener();
        EXPECT_NE(CloudSyncManagerImpl::GetInstance().listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SubscribeListener003 FAILED";
    }
    GTEST_LOG_(INFO) << "SubscribeListener003 End";
}

/*
 * @tc.name: StartDowngradeTest001
 * @tc.desc: Verify the StartDowngrade function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartDowngradeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDowngradeTest001 Start";
    try {
        std::string bundleName = "com.ohos.aaa";
        shared_ptr<DowngradeDlCallback> downloadCallback = make_shared<DowngradeDlCallbackDerived>();
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, StartDowngrade(_, _)).WillOnce(Return(E_OK));
        auto res = CloudSyncManagerImpl::GetInstance().StartDowngrade(bundleName, downloadCallback);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StartDowngradeTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "StartDowngradeTest001 End";
}

/*
 * @tc.name: StartDowngradeTest002
 * @tc.desc: Verify the StartDowngrade function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartDowngradeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDowngradeTest002 Start";
    try {
        std::string bundleName = "";
        shared_ptr<DowngradeDlCallback> downloadCallback = make_shared<DowngradeDlCallbackDerived>();
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        auto res = CloudSyncManagerImpl::GetInstance().StartDowngrade(bundleName, downloadCallback);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StartDowngradeTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "StartDowngradeTest002 End";
}

/*
 * @tc.name: StartDowngradeTest003
 * @tc.desc: Verify the StartDowngrade function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartDowngradeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDowngradeTest003 Start";
    try {
        std::string bundleName = "com.ohos.aaa";
        shared_ptr<DowngradeDlCallback> downloadCallback = make_shared<DowngradeDlCallbackDerived>();
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImpl::GetInstance().StartDowngrade(bundleName, downloadCallback);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StartDowngradeTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "StartDowngradeTest003 End";
}

/*
 * @tc.name: StartDowngradeTest004
 * @tc.desc: Verify the StartDowngrade function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartDowngradeTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDowngradeTest004 Start";
    try {
        std::string bundleName = "com.ohos.aaa";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        auto res = CloudSyncManagerImpl::GetInstance().StartDowngrade(bundleName, nullptr);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StartDowngradeTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "StartDowngradeTest004 End";
}

/*
 * @tc.name: StopDowngradeTest001
 * @tc.desc: Verify the StopDowngrade function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StopDowngradeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDowngradeTest001 Start";
    try {
        std::string bundleName = "com.ohos.aaa";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, StopDowngrade(_)).WillOnce(Return(E_OK));
        auto res = CloudSyncManagerImpl::GetInstance().StopDowngrade(bundleName);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopDowngradeTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "StopDowngradeTest001 End";
}

/*
 * @tc.name: StopDowngradeTest002
 * @tc.desc: Verify the StopDowngrade function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StopDowngradeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDowngradeTest002 Start";
    try {
        std::string bundleName = "";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        auto res = CloudSyncManagerImpl::GetInstance().StopDowngrade(bundleName);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopDowngradeTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "StopDowngradeTest002 End";
}

/*
 * @tc.name: StopDowngradeTest003
 * @tc.desc: Verify the StopDowngrade function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StopDowngradeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDowngradeTest003 Start";
    try {
        std::string bundleName = "";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImpl::GetInstance().StopDowngrade(bundleName);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopDowngradeTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "StopDowngradeTest003 End";
}

/*
 * @tc.name: GetCloudFileInfoTest001
 * @tc.desc: Verify the GetCloudFileInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, GetCloudFileInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudFileInfoTest001 Start";
    try {
        std::string bundleName = "com.ohos.aaa";
        CloudFileInfo cloudFileInfo;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, GetCloudFileInfo(_, _)).WillOnce(Return(E_OK));
        auto res = CloudSyncManagerImpl::GetInstance().GetCloudFileInfo(bundleName, cloudFileInfo);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetCloudFileInfoTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetCloudFileInfoTest001 End";
}

/*
 * @tc.name: GetCloudFileInfoTest002
 * @tc.desc: Verify the StopDowngrade function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, GetCloudFileInfoTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudFileInfoTest002 Start";
    try {
        std::string bundleName = "";
        CloudFileInfo cloudFileInfo;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        auto res = CloudSyncManagerImpl::GetInstance().GetCloudFileInfo(bundleName, cloudFileInfo);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetCloudFileInfoTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GetCloudFileInfoTest002 End";
}

/*
 * @tc.name: GetCloudFileInfoTest003
 * @tc.desc: Verify the StopDowngrade function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, GetCloudFileInfoTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudFileInfoTest003 Start";
    try {
        std::string bundleName = "com.ohos.aaa";
        CloudFileInfo cloudFileInfo;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImpl::GetInstance().GetCloudFileInfo(bundleName, cloudFileInfo);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetCloudFileInfoTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "GetCloudFileInfoTest003 End";
}

/*
 * @tc.name: GetHistoryVersionList
 * @tc.desc: Verify the GetHistoryVersionList function, proxy is null.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncManagerImplTest, GetHistoryVersionListTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHistoryVersionListTest001 Start";
    try {
        string uri = "";
        int32_t numLimit = 0;
        vector<CloudSync::HistoryVersion> historyVersionList;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImpl::GetInstance().GetHistoryVersionList(uri, numLimit, historyVersionList);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);

        uri = "";
        numLimit = 1;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        res = CloudSyncManagerImpl::GetInstance().GetHistoryVersionList(uri, numLimit, historyVersionList);
        EXPECT_EQ(res, E_ILLEGAL_URI);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetHistoryVersionListTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetHistoryVersionListTest001 End";
}

/*
 * @tc.name: GetHistoryVersionList
 * @tc.desc: Verify the GetHistoryVersionList function, uri empty or limit below zero.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncManagerImplTest, GetHistoryVersionListTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHistoryVersionListTest002 Start";
    try {
        string uri = "path/file/1.txt";
        int32_t numLimit = 0;
        vector<CloudSync::HistoryVersion> historyVersionList;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        auto res = CloudSyncManagerImpl::GetInstance().GetHistoryVersionList(uri, numLimit, historyVersionList);
        EXPECT_EQ(res, E_INVAL_ARG);

        uri = "";
        numLimit = 0;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        res = CloudSyncManagerImpl::GetInstance().GetHistoryVersionList(uri, numLimit, historyVersionList);
        EXPECT_EQ(res, E_ILLEGAL_URI);

        uri = "path/file/1.txt";
        numLimit = 1;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, GetHistoryVersionList(_, _, _)).WillOnce(Return(E_OK));
        res = CloudSyncManagerImpl::GetInstance().GetHistoryVersionList(uri, numLimit, historyVersionList);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetHistoryVersionListTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GetHistoryVersionListTest002 End";
}

/*
 * @tc.name: DownloadHistoryVersion
 * @tc.desc: Verify the DownloadHistoryVersion function, proxy is null.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncManagerImplTest, DownloadHistoryVersionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadHistoryVersionTest001 Start";
    try {
        string uri = "";
        int64_t downloadId = 0;
        uint64_t versionId = 0;
        shared_ptr<CloudDownloadCallback> callback = nullptr;
        string versionUri;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImpl::GetInstance()
            .DownloadHistoryVersion(uri, downloadId, versionId, callback, versionUri);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);

        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        res = CloudSyncManagerImpl::GetInstance()
            .DownloadHistoryVersion(uri, downloadId, versionId, callback, versionUri);
        EXPECT_EQ(res, E_ILLEGAL_URI);

        uri = "path/file/1.txt";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        res = CloudSyncManagerImpl::GetInstance()
            .DownloadHistoryVersion(uri, downloadId, versionId, callback, versionUri);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " DownloadHistoryVersionTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadHistoryVersionTest001 End";
}

/*
 * @tc.name: DownloadHistoryVersion
 * @tc.desc: Verify the DownloadHistoryVersion function, uri or callback empty.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncManagerImplTest, DownloadHistoryVersionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadHistoryVersionTest002 Start";
    try {
        string uri = "";
        int64_t downloadId = 0;
        uint64_t versionId = 0;
        shared_ptr<CloudDownloadCallback> callback = make_shared<CloudDownloadCallbackDerived>();
        string versionUri;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        auto res = CloudSyncManagerImpl::GetInstance()
            .DownloadHistoryVersion(uri, downloadId, versionId, callback, versionUri);
        EXPECT_EQ(res, E_ILLEGAL_URI);

        uri = "path/file/1.txt";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, DownloadHistoryVersion(_, _, _, _, _)).WillOnce(Return(E_OK));
        res = CloudSyncManagerImpl::GetInstance()
            .DownloadHistoryVersion(uri, downloadId, versionId, callback, versionUri);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " DownloadHistoryVersionTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadHistoryVersionTest002 End";
}

/*
 * @tc.name: ReplaceFileWithHistoryVersion
 * @tc.desc: Verify the ReplaceFileWithHistoryVersion function, proxy is null.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncManagerImplTest, ReplaceFileWithHistoryVersionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReplaceFileWithHistoryVersionTest001 Start";
    try {
        string oriUri = "";
        string uri = "";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImpl::GetInstance().ReplaceFileWithHistoryVersion(oriUri, uri);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);

        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        res = CloudSyncManagerImpl::GetInstance().ReplaceFileWithHistoryVersion(oriUri, uri);
        EXPECT_EQ(res, E_ILLEGAL_URI);

        oriUri = "path/file/1.txt";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        res = CloudSyncManagerImpl::GetInstance().ReplaceFileWithHistoryVersion(oriUri, uri);
        EXPECT_EQ(res, E_ILLEGAL_URI);

        oriUri = "";
        uri = "path/file/1.txt";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        res = CloudSyncManagerImpl::GetInstance().ReplaceFileWithHistoryVersion(oriUri, uri);
        EXPECT_EQ(res, E_ILLEGAL_URI);

        oriUri = "path/file/1.txt";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, ReplaceFileWithHistoryVersion(_, _)).WillOnce(Return(E_OK));
        res = CloudSyncManagerImpl::GetInstance().ReplaceFileWithHistoryVersion(oriUri, uri);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ReplaceFileWithHistoryVersionTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ReplaceFileWithHistoryVersionTest001 End";
}

/*
 * @tc.name: IsFileConflict
 * @tc.desc: Verify the IsFileConflict function, proxy is null.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncManagerImplTest, IsFileConflictTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsFileConflictTest001 Start";
    try {
        string uri = "";
        bool isConflict;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImpl::GetInstance().IsFileConflict(uri, isConflict);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);

        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        res = CloudSyncManagerImpl::GetInstance().IsFileConflict(uri, isConflict);
        EXPECT_EQ(res, E_ILLEGAL_URI);

        uri = "path/file/1.txt";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, IsFileConflict(_, _)).WillOnce(Return(E_OK));
        res = CloudSyncManagerImpl::GetInstance().IsFileConflict(uri, isConflict);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsFileConflictTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "IsFileConflictTest001 End";
}

/*
 * @tc.name: ClearFileConflict
 * @tc.desc: Verify the ClearFileConflict function, proxy is null.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncManagerImplTest, ClearFileConflictTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearFileConflictTest001 Start";
    try {
        string uri = "";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImpl::GetInstance().ClearFileConflict(uri);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);

        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        res = CloudSyncManagerImpl::GetInstance().ClearFileConflict(uri);
        EXPECT_EQ(res, E_ILLEGAL_URI);

        uri = "path/file/1.txt";
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, ClearFileConflict(_)).WillOnce(Return(E_OK));
        res = CloudSyncManagerImpl::GetInstance().ClearFileConflict(uri);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ClearFileConflictTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ClearFileConflictTest001 End";
}

/*
 * @tc.name: GetThumbsPath
 * @tc.desc: Verify the GetThumbsPath function
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncManagerImplTest, GetThumbsPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetThumbsPathTest001 Start";
    try {
        string inputPath = "/storage/cloud/100/data";
        auto ret = GetThumbsPath(inputPath);
        EXPECT_EQ(ret, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetThumbsPathTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetThumbsPathTest001 End";
}

/*
 * @tc.name: GetMediaPath
 * @tc.desc: Verify the GetMediaPath function
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncManagerImplTest, GetMediaPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMediaPathTest001 Start";
    try {
        string inputPath = "/data/service/el2/100/hmdfs/account/files";
        auto ret = GetMediaPath(inputPath);
        EXPECT_EQ(ret, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetMediaPathTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetMediaPathTest001 End";
}

/*
 * @tc.name: GetBundlesLocalFilePresentStatusTest001
 * @tc.desc: Verify the GetBundlesLocalFilePresentStatus function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, GetBundlesLocalFilePresentStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest001 Start";
    try {
        std::vector<std::string> emptyBundles;
        std::vector<LocalFilePresentStatus> outList;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        auto res = CloudSyncManagerImpl::GetInstance().GetBundlesLocalFilePresentStatus(emptyBundles, outList);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetBundlesLocalFilePresentStatusTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest001 End";
}

/*
 * @tc.name: GetBundlesLocalFilePresentStatusTest002
 * @tc.desc: Verify the GetBundlesLocalFilePresentStatus function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, GetBundlesLocalFilePresentStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest002 Start";
    try {
        std::vector<std::string> emptyBundles;
        std::vector<LocalFilePresentStatus> outList;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        auto res = CloudSyncManagerImpl::GetInstance().GetBundlesLocalFilePresentStatus(emptyBundles, outList);
        EXPECT_EQ(res, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetBundlesLocalFilePresentStatusTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest002 End";
}

/*
 * @tc.name: GetBundlesLocalFilePresentStatusTest003
 * @tc.desc: Verify the GetBundlesLocalFilePresentStatus function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, GetBundlesLocalFilePresentStatusTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest003 Start";
    try {
        std::vector<std::string> NotEmptyBundles = {"com.ohos.test1", "com.ohos.test2"};
        std::vector<LocalFilePresentStatus> outList;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, GetBundlesLocalFilePresentStatus(_, _)).WillOnce(Return(E_OK));
        auto res = CloudSyncManagerImpl::GetInstance().GetBundlesLocalFilePresentStatus(NotEmptyBundles, outList);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetBundlesLocalFilePresentStatusTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest003 End";
}

/**
 * @tc.name: RegisterFileSyncCallbackTest002
 * @tc.desc: Verify the RegisterFileSyncCallback function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncManagerImplTest, RegisterFileSyncCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterFileSyncCallbackTest002 Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudSyncCallbackDerived>();
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, RegisterFileSyncCallbackInner(_, _, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*saMgrClient_, GetSystemAbilityManager()).WillOnce(Return(nullptr));
        int32_t res = CloudSyncManagerImpl::GetInstance().RegisterFileSyncCallback(callbackInfo);
        CloudSyncCallbackClientManager::GetInstance().RemoveCallback(callbackInfo);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterFileSyncCallbackTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterFileSyncCallbackTest002 End";
}

/**
 * @tc.name: GetDentryFileOccupyTest001
 * @tc.desc: Verify the GetDentryFileOccupy function.
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(CloudSyncManagerImplTest, GetDentryFileOccupyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDentryFileOccupyTest001 Start";
    try {
        int64_t num = 1;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        int32_t res = CloudSyncManagerImpl::GetInstance().GetDentryFileOccupy(num);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);

        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, GetDentryFileOccupy(_)).WillOnce(Return(E_OK));
        res = CloudSyncManagerImpl::GetInstance().GetDentryFileOccupy(num);
        EXPECT_EQ(res, E_OK);

        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, GetDentryFileOccupy(_)).WillOnce(Return(E_PERMISSION_DENIED));
        res = CloudSyncManagerImpl::GetInstance().GetDentryFileOccupy(num);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDentryFileOccupyTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetDentryFileOccupyTest001 End";
}

/**
 * @tc.name: IsFinishPullTest001
 * @tc.desc: Verify the IsFinishPull function.
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncManagerImplTest, IsFinishPullTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsFinishPullTest001 Start";
    try {
        bool finishFlag = false;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        int32_t res = CloudSyncManagerImpl::GetInstance().IsFinishPull(finishFlag);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);

        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, IsFinishPull(_)).WillOnce(Return(E_OK));
        res = CloudSyncManagerImpl::GetInstance().IsFinishPull(finishFlag);
        EXPECT_EQ(res, E_OK);

        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, IsFinishPull(_)).WillOnce(Return(E_PERMISSION_DENIED));
        res = CloudSyncManagerImpl::GetInstance().IsFinishPull(finishFlag);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsFinishPullTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "IsFinishPullTest001 End";
}

/**
 * @tc.name: GetAclXattrBatchTest001
 * @tc.desc: Verify the GetAclXattrBatch function.
 * @tc.type: FUNC
 * @tc.require: issue3881
 */
HWTEST_F(CloudSyncManagerImplTest, GetAclXattrBatchTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAclXattrBatchTest001 Start";
    try {
        bool isAccess = true;
        std::vector<std::string> filePaths = {"/storage/media/local/test/file1.txt"};
        std::vector<XattrResult> aclXattrResults;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(nullptr));
        int32_t res = CloudSyncManagerImpl::GetInstance().GetAclXattrBatch(isAccess, filePaths, aclXattrResults);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAclXattrBatchTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetAclXattrBatchTest001 End";
}

/**
 * @tc.name: GetAclXattrBatchTest002
 * @tc.desc: Verify the GetAclXattrBatch function.
 * @tc.type: FUNC
 * @tc.require: issue3881
 */
HWTEST_F(CloudSyncManagerImplTest, GetAclXattrBatchTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAclXattrBatchTest002 Start";
    try {
        bool isAccess = false;
        std::vector<std::string> filePaths = {"/storage/media/local/test/file1.txt"};
        std::vector<XattrResult> aclXattrResults;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, GetAclXattrBatch(_, _, _)).WillOnce(Return(E_OK));
        int32_t res = CloudSyncManagerImpl::GetInstance().GetAclXattrBatch(isAccess, filePaths, aclXattrResults);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAclXattrBatchTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GetAclXattrBatchTest002 End";
}

/**
 * @tc.name: GetAclXattrBatchTest003
 * @tc.desc: Verify the GetAclXattrBatch function.
 * @tc.type: FUNC
 * @tc.require: issue3881
 */
HWTEST_F(CloudSyncManagerImplTest, GetAclXattrBatchTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAclXattrBatchTest003 Start";
    try {
        bool isAccess = true;
        std::vector<std::string> filePaths =
            {"/storage/media/local/test/file1.txt", "/storage/media/local/test/file2.txt"};
        std::vector<XattrResult> aclXattrResults;
        EXPECT_CALL(*proxy_, GetInstance()).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, GetAclXattrBatch(_, _, _)).WillOnce(Return(E_PERMISSION_DENIED));
        int32_t res = CloudSyncManagerImpl::GetInstance().GetAclXattrBatch(isAccess, filePaths, aclXattrResults);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAclXattrBatchTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "GetAclXattrBatchTest003 End";
}
} // namespace FileManagement::CloudSync
} // namespace OHOS