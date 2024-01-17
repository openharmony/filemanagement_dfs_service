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

#include "cloud_sync_manager_impl.h"
#include "cloud_sync_service_proxy.h"
#include "dfs_error.h"
#include "i_cloud_sync_service_mock.h"
#include "iservice_registry.h"
#include "service_callback_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

class CloudSyncManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<CloudSyncManagerImpl> managePtr_;
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
    void OnDownloadProcess(const DownloadProgressObj& progress)
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
    std::cout << "SetUp" << std::endl;
}

void CloudSyncManagerImplTest::TearDown(void)
{
    managePtr_ = nullptr;
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: RegisterCallbackTest
 * @tc.desc: Verify the RegisterCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, RegisterCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCallbackTest Start";
    try {
        shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncCallbackDerived>();
        auto res = CloudSyncManagerImpl::GetInstance().RegisterCallback(callback);
        EXPECT_EQ(res, E_OK);
        callback = nullptr;
        res = CloudSyncManagerImpl::GetInstance().RegisterCallback(callback);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RegisterCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterCallbackTest End";
}

/*
 * @tc.name: UnRegisterCallbackTest
 * @tc.desc: Verify the UnRegisterCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, UnRegisterCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterCallbackTest Start";
    try {
        auto res = CloudSyncManagerImpl::GetInstance().UnRegisterCallback();
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " UnRegisterCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "UnRegisterCallbackTest End";
}

/**
 * @tc.name: StartSyncTest
 * @tc.desc: Verify the StartSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartSyncTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncTest Start";
    try {
        bool forceFlag = false;
        shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncCallbackDerived>();
        auto res = CloudSyncManagerImpl::GetInstance().StartSync();
        EXPECT_EQ(res, E_OK);
        res = CloudSyncManagerImpl::GetInstance().StartSync(forceFlag, callback);
        EXPECT_EQ(res, E_OK);
        forceFlag = true;
        res = CloudSyncManagerImpl::GetInstance().StartSync(forceFlag, callback);
        EXPECT_EQ(res, E_OK);
        forceFlag = true;
        res = CloudSyncManagerImpl::GetInstance().StartSync(forceFlag, nullptr);
        EXPECT_EQ(res, E_INVAL_ARG);
        forceFlag = false;
        res = CloudSyncManagerImpl::GetInstance().StartSync(forceFlag, nullptr);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StartSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncTest End";
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
        int res = CloudSyncManagerImpl::GetInstance().StopSync();
        EXPECT_EQ(res, E_OK);
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
        auto res = CloudSyncManagerImpl::GetInstance().ChangeAppSwitch(accoutId, bundleName, status);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ChangeAppSwitchTest FAILED";
    }
    GTEST_LOG_(INFO) << "ChangeAppSwitchTest End";
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
        auto res = CloudSyncManagerImpl::GetInstance().NotifyDataChange(accoutId, bundleName);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " NotifyDataChangeTest FAILED";
    }
    GTEST_LOG_(INFO) << "NotifyDataChangeTest End";
}

/*
 * @tc.name: StartDownloadFileTest
 * @tc.desc: Verify the StartDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartDownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string uri = "uri";
        auto res = CloudSyncManagerImpl::GetInstance().StartDownloadFile(uri);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StartDownloadFileTest FAILED";
    }
    GTEST_LOG_(INFO) << "StartDownloadFileTest End";
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
        std::string uri = "uri";
        auto res = CloudSyncManagerImpl::GetInstance().StopDownloadFile(uri);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopDownloadFileTest FAILED";
    }
    GTEST_LOG_(INFO) << "StopDownloadFileTest End";
}

/*
 * @tc.name: RegisterDownloadFileCallbackTest
 * @tc.desc: Verify the UnregisterDownloadFileCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, RegisterDownloadFileCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallbackTest Start";
    try {
        shared_ptr<CloudDownloadCallback> downloadCallback = make_shared<CloudDownloadCallbackDerived>();
        auto res = CloudSyncManagerImpl::GetInstance().RegisterDownloadFileCallback(downloadCallback);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RegisterDownloadFileCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallbackTest End";
}

/*
 * @tc.name: UnregisterDownloadFileCallbackTest
 * @tc.desc: Verify the UnregisterDownloadFileCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, UnregisterDownloadFileCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallbackTest Start";
    try {
        auto res = CloudSyncManagerImpl::GetInstance().UnregisterDownloadFileCallback();
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " UnregisterDownloadFileCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallbackTest End";
}

/*
 * @tc.name: SetDeathRecipientTest
 * @tc.desc: Verify the SetDeathRecipient function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, SetDeathRecipientTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
        CloudSyncManagerImpl::GetInstance().SetDeathRecipient(CloudSyncServiceProxy->AsObject());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetDeathRecipientTest FAILED";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipientTest End";
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
        auto res = CloudSyncManagerImpl::GetInstance().EnableCloud(accoutId, switchData);
        EXPECT_EQ(res, E_OK);
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
        auto res = CloudSyncManagerImpl::GetInstance().DisableCloud(accoutId);
        EXPECT_EQ(res, E_OK);
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
        auto res = CloudSyncManagerImpl::GetInstance().Clean(accoutId, cleanOptions);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanTest FAILED";
    }
    GTEST_LOG_(INFO) << "CleanTest End";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
