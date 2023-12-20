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

#include "cloud_sync_service.h"
#include "cloud_sync_common.h"
#include "dfs_error.h"
#include "service_callback_mock.h"
#include "utils_log.h"
#include "i_cloud_download_callback_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;

std::shared_ptr<CloudSyncService> g_servicePtr_;

class CloudSyncServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncServiceTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncServiceTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncServiceTest::SetUp(void)
{
    if (g_servicePtr_ == nullptr) {
        int32_t saId = 5204;
        g_servicePtr_ = std::make_shared<CloudSyncService>(saId);
        ASSERT_TRUE(g_servicePtr_ != nullptr) << "SystemAbility failed";
    }
    std::cout << "SetUp" << std::endl;
}

void CloudSyncServiceTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name:GetHmdfsPathTest001
 * @tc.desc:Verify the GetHmdfsPath function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, GetHmdfsPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHmdfsPathTest001 start";
    try {
        std::string uri = "";
        int32_t userId = 100;
        std::string ret = g_servicePtr_->GetHmdfsPath(uri, userId);
        EXPECT_EQ(ret, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetHmdfsPathTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetHmdfsPathTest001 end";
}

/**
 * @tc.name:GetHmdfsPathTest002
 * @tc.desc:Verify the GetHmdfsPath function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, GetHmdfsPathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHmdfsPathTest002 start";
    try {
        std::string uri = "invaild_uri";
        int32_t userId = 100;
        std::string ret = g_servicePtr_->GetHmdfsPath(uri, userId);
        EXPECT_EQ(ret, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetHmdfsPathTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GetHmdfsPathTest002 end";
}

/**
 * @tc.name:GetHmdfsPathTest003
 * @tc.desc:Verify the GetHmdfsPath function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, GetHmdfsPathTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHmdfsPathTest003 start";
    try {
        std::string uri = "file://com.hmos.notepad/data/storage/el2/distributedfiles/dir/1.txt";
        int32_t userId = 100;
        std::string ret = g_servicePtr_->GetHmdfsPath(uri, userId);
        std::string out = "/mnt/hmdfs/100/account/device_view/local/data/com.hmos.notepad/dir/1.txt";
        EXPECT_EQ(ret, out);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetHmdfsPathTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "GetHmdfsPathTest003 end";
}

/**
 * @tc.name:DeleteAssetTest001
 * @tc.desc:Verify the DeleteAsset function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, DeleteAssetTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAssetTest001 start";
    try {
        int32_t userId = 100;
        std::string uri = "";
        int ret = g_servicePtr_->DeleteAsset(userId, uri);
        EXPECT_EQ(ret, E_GET_PHYSICAL_PATH_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteAssetTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "DeleteAssetTest001 end";
}

/**
 * @tc.name:DeleteAssetTest002
 * @tc.desc:Verify the DeleteAsset function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, DeleteAssetTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteAssetTest002 start";
    try {
        int32_t userId = 100;
        std::string uri = "file://com.hmos.notepad/data/storage/el2/distributedfiles/dir/1.txt";
        int ret = g_servicePtr_->DeleteAsset(userId, uri);
        EXPECT_EQ(ret, E_DELETE_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteAssetTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "DeleteAssetTest002 end";
}

/**
 * @tc.name: OnStartTest
 * @tc.desc: Verify the OnStart function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, OnStartTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStart start";
    try {
        SystemAbilityOnDemandReason reason;
        g_servicePtr_->OnStart(reason);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStart FAILED";
    }
    GTEST_LOG_(INFO) << "OnStart end";
}

/**
 * @tc.name: OnStopTest
 * @tc.desc: Verify the OnStop function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, OnStopTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStop start";
    try {
        g_servicePtr_->OnStop();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStop FAILED";
    }
    GTEST_LOG_(INFO) << "OnStop end";
}

/**
 * @tc.name:RegisterCallbackInnerTest
 * @tc.desc:Verify the RegisterCallbackInner function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, RegisterCallbackInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCallbackInner start";
    try {
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        int ret = g_servicePtr_->RegisterCallbackInner(callback);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterCallbackInner FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterCallbackInner end";
}

/**
 * @tc.name:RegisterCallbackInnerTest002
 * @tc.desc:Verify the RegisterCallbackInner function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, RegisterCallbackInnerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCallbackInner error branch start";
    try {
        sptr<CloudSyncCallbackMock> callback = nullptr;
        int ret = g_servicePtr_->RegisterCallbackInner(callback);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterCallbackInner error branch FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterCallbackInner error branch end";
}

/**
 * @tc.name:StopSyncInnerTest
 * @tc.desc:Verify the StopSyncInner function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, StopSyncInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopSyncInner Start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t callerUserId = 100;
        auto dataSyncManager = g_servicePtr_->dataSyncManager_;
        int ret = dataSyncManager->TriggerStopSync(bundleName, callerUserId, SyncTriggerType::APP_TRIGGER);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopSyncInner FAILED";
    }
    GTEST_LOG_(INFO) << "StopSyncInner End";
}

/**
 * @tc.name:ChangeAppSwitchTest1
 * @tc.desc:Verify the ChangeAppSwitch function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, ChangeAppSwitchTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeAppSwitch Start";
    try {
        std::string accountId = "testId";
        std::string bundleName = "com.ohos.photos";
        int32_t callerUserId = 100;
        auto dataSyncManager = g_servicePtr_->dataSyncManager_;
        int ret = dataSyncManager->TriggerStopSync(bundleName, callerUserId, SyncTriggerType::CLOUD_TRIGGER);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeAppSwitch FAILED";
    }
    GTEST_LOG_(INFO) << "ChangeAppSwitch End";
}

/**
 * @tc.name:ChangeAppSwitchTest2
 * @tc.desc:Verify the ChangeAppSwitch function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, ChangeAppSwitchTest2, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeAppSwitch Start";
    try {
        std::string accountId = "testId";
        std::string bundleName = "com.ohos.photos";
        int32_t callerUserId = 100;
        auto dataSyncManager = g_servicePtr_->dataSyncManager_;
        int ret = dataSyncManager->TriggerStartSync(bundleName, callerUserId, false, SyncTriggerType::CLOUD_TRIGGER);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ChangeAppSwitch FAILED";
    }
    GTEST_LOG_(INFO) << "ChangeAppSwitch End";
}

/**
 * @tc.name:NotifyDataChangeTest
 * @tc.desc:Verify the NotifyDataChange function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, NotifyDataChangeTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChange Start";
    try {
        std::string accountId = "testId";
        string bundleName = "com.ohos.photos";
        int32_t callerUserId = 100;
        auto dataSyncManager = g_servicePtr_->dataSyncManager_;
        int ret = dataSyncManager->TriggerStartSync(bundleName, callerUserId, false, SyncTriggerType::CLOUD_TRIGGER);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NotifyDataChange FAILED";
    }
    GTEST_LOG_(INFO) << "NotifyDataChange End";
}

/**
 * @tc.name:UnRegisterCallbackInnerTest
 * @tc.desc:Verify the UnRegisterCallbackInner function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, UnRegisterCallbackInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnRegisterCallbackInner Start";
    try {
        int ret = g_servicePtr_->UnRegisterCallbackInner();
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterCallbackInner FAILED";
    }
    GTEST_LOG_(INFO) << "UnRegisterCallbackInner End";
}

/**
 * @tc.name:StartSyncInnerTest
 * @tc.desc:Verify the StartSyncInner function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, StartSyncInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncInner Start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t callerUserId = 100;
        bool forceFlag = true;
        auto dataSyncManager = g_servicePtr_->dataSyncManager_;
        int ret = dataSyncManager->TriggerStartSync(bundleName, callerUserId, forceFlag, SyncTriggerType::APP_TRIGGER);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncInner FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncInner End";
}

/**
 * @tc.name:EnableCloudTest
 * @tc.desc:Verify the EnableCloud function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, EnableCloudTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "EnableCloud start";
    try {
        std::string accountId = "testId";
        SwitchDataObj switchData;
        int ret = g_servicePtr_->EnableCloud(accountId, switchData);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "EnableCloud FAILED";
    }
    GTEST_LOG_(INFO) << "EnableCloud end";
}

/**
 * @tc.name:CleanTest
 * @tc.desc:Verify the Clean function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, CleanTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DisableCloud start";
    try {
        std::string accountId = "testId";
        CleanOptions cleanOptions;
        int ret = g_servicePtr_->Clean(accountId, cleanOptions);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "Clean FAILED";
    }
    GTEST_LOG_(INFO) << "Clean end";
}

/**
 * @tc.name:StartDownloadFileTest
 * @tc.desc:Verify the StartDownloadFile function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, StartDownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDownloadFile start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t callerUserId = 100;
        std::string path;
        auto dataSyncManager = g_servicePtr_->dataSyncManager_;
        int ret = dataSyncManager->StartDownloadFile(bundleName, callerUserId, path);
        EXPECT_TRUE(E_STOP == ret || ret == E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartDownloadFile FAILED";
    }
    GTEST_LOG_(INFO) << "StartDownloadFile end";
}

/**
 * @tc.name:StopDownloadFileTest
 * @tc.desc:Verify the StopDownloadFile function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, StopDownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDownloadFile start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t callerUserId = 100;
        std::string path;
        auto dataSyncManager = g_servicePtr_->dataSyncManager_;
        int ret = dataSyncManager->StopDownloadFile(bundleName, callerUserId, path);
        EXPECT_TRUE(E_OK == ret || ret == E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopDownloadFile FAILED";
    }
    GTEST_LOG_(INFO) << "StopDownloadFile end";
}

/**
 * @tc.name:RegisterDownloadFileCallbackTest
 * @tc.desc:Verify the RegisterDownloadFileCallback function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, RegisterDownloadFileCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallback start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t callerUserId = 100;
        sptr<CloudDownloadCallbackMock> downloadCallback = sptr(new CloudDownloadCallbackMock());
        auto dataSyncManager = g_servicePtr_->dataSyncManager_;
        int ret = dataSyncManager->RegisterDownloadFileCallback(bundleName, callerUserId, downloadCallback);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterDownloadFileCallback FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterDownloadFileCallback end";
}

/**
 * @tc.name:UnregisterDownloadFileCallbackTest
 * @tc.desc:Verify the UnregisterDownloadFileCallback function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, UnregisterDownloadFileCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallback start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t callerUserId = 100;
        auto dataSyncManager = g_servicePtr_->dataSyncManager_;
        int ret = dataSyncManager->UnregisterDownloadFileCallback(bundleName, callerUserId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnregisterDownloadFileCallback FAILED";
    }
    GTEST_LOG_(INFO) << "UnregisterDownloadFileCallback end";
}

/**
 * @tc.name:UploadAssetTest
 * @tc.desc:Verify the UploadAsset function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, UploadAssetTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UploadAsset start";
    try {
        int32_t userId = 100;
        std::string request = "testReq";
        std::string result = "expRes";
        int ret = g_servicePtr_->UploadAsset(userId, request, result);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UploadAsset FAILED";
    }
    GTEST_LOG_(INFO) << "UploadAsset end";
}

/**
 * @tc.name:DownloadFileTest001
 * @tc.desc:Verify the DownloadFile function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, DownloadFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFileTest001 start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.photos";
        std::string uri = "file://com.hmos.notepad/data/storage/el2/distributedfiles/dir/1.txt";
        AssetInfoObj assetInfoObj;
        assetInfoObj.uri = uri;
        int ret = g_servicePtr_->DownloadFile(userId, bundleName, assetInfoObj);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadFileTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadFileTest001 end";
}

/**
 * @tc.name:DownloadFileTest002
 * @tc.desc:Verify the DownloadFile function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, DownloadFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFileTest002 start";
    try {
        int32_t userId = 100;
        std::string bundleName = "com.ohos.photos";
        AssetInfoObj assetInfoObj;
        int ret = g_servicePtr_->DownloadFile(userId, bundleName, assetInfoObj);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadFileTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadFileTest002 end";
}

/**
 * @tc.name:DownloadAssetTest001
 * @tc.desc:Verify the DownloadAsset function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, DownloadAssetTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadAssetTest001 start";
    try {
        uint64_t taskId = 100;
        int32_t userId = 100;
        std::string bundleName = "com.ohos.photos";
        std::string networkId = "0.0.0.0";
        std::string uri = "file://com.hmos.notepad/data/storage/el2/distributedfiles/dir/1.txt";
        AssetInfoObj assetInfoObj;
        assetInfoObj.uri = uri;
        int ret = g_servicePtr_->DownloadAsset(taskId, userId, bundleName, networkId, assetInfoObj);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadAssetTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadAssetTest001 end";
}

/**
 * @tc.name:DownloadAssetTest002
 * @tc.desc:Verify the DownloadAsset function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, DownloadAssetTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadAssetTest002 start";
    try {
        uint64_t taskId = 100;
        int32_t userId = 100;
        std::string bundleName = "com.ohos.photos";
        std::string networkId = "edge2cloud";
        AssetInfoObj assetInfoObj;
        int ret = g_servicePtr_->DownloadAsset(taskId, userId, bundleName, networkId, assetInfoObj);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadAssetTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadAssetTest002 end";
}

/**
 * @tc.name:RegisterDownloadAssetCallbackTest
 * @tc.desc:Verify the RegisterDownloadAssetCallback function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, RegisterDownloadAssetCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback start";
    try {
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        int ret = g_servicePtr_->RegisterDownloadAssetCallback(callback);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback end";
}

/**
 * @tc.name:RegisterDownloadAssetCallbackTest002
 * @tc.desc:Verify theRegisterDownloadAssetCallback function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, RegisterDownloadAssetCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback error branch start";
    try {
        sptr<CloudSyncCallbackMock> callback = nullptr;
        int ret = g_servicePtr_->RegisterDownloadAssetCallback(callback);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback error branch FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback error branch end";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
