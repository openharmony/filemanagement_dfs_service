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
#include "dfsu_access_token_helper_mock.h"
#include "service_callback_mock.h"
#include "utils_log.h"
#include "i_cloud_download_callback_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudSyncServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<DfsuAccessTokenMock> dfsuAccessToken_ = nullptr;
    static inline std::shared_ptr<CloudSyncService> g_servicePtr_ = nullptr;
};

void CloudSyncServiceTest::SetUpTestCase(void)
{
    if (g_servicePtr_ == nullptr) {
        int32_t saId = 5204;
        g_servicePtr_ = std::make_shared<CloudSyncService>(saId);
        ASSERT_TRUE(g_servicePtr_ != nullptr) << "SystemAbility failed";
    }
    std::cout << "SetUpTestCase" << std::endl;
    g_servicePtr_->dataSyncManager_ = make_shared<CloudFile::DataSyncManager>();
    dfsuAccessToken_ = make_shared<DfsuAccessTokenMock>();
    DfsuAccessTokenMock::dfsuAccessToken = dfsuAccessToken_;
}

void CloudSyncServiceTest::TearDownTestCase(void)
{
    DfsuAccessTokenMock::dfsuAccessToken.reset();
    g_servicePtr_ = nullptr;
    dfsuAccessToken_ = nullptr;
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncServiceTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudSyncServiceTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
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
        std::string bundleName = "";
        int ret = g_servicePtr_->UnRegisterCallbackInner(bundleName);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterCallbackInner FAILED";
    }
    GTEST_LOG_(INFO) << "UnRegisterCallbackInner End";
}
 
/**
 * @tc.name:RegisterCallbackInnerTest001
 * @tc.desc:Verify the RegisterCallbackInner function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, RegisterCallbackInnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCallbackInner start";
    try {
        std::string bundleName = "";
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        int ret = g_servicePtr_->RegisterCallbackInner(callback, bundleName);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
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
        std::string bundleName = "com.ohos.photos";
        sptr<CloudSyncCallbackMock> callback = nullptr;
        int ret = g_servicePtr_->RegisterCallbackInner(callback, bundleName);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterCallbackInner error branch FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterCallbackInner error branch end";
}
 
/**
 * @tc.name:TriggerSyncInnerTest
 * @tc.desc:Verify the TriggerSyncInner function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, TriggerSyncInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TriggerSyncInner start";
    try {
        std::string bundleName = "com.ohos.photos";
        int32_t userId = 0;
        int ret = g_servicePtr_->TriggerSyncInner(bundleName, userId);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TriggerSyncInner FAILED";
    }
    GTEST_LOG_(INFO) << "TriggerSyncInner end";
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
        std::string bundleName = "";
        int ret = g_servicePtr_->StopSyncInner(bundleName);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopSyncInner FAILED";
    }
    GTEST_LOG_(INFO) << "StopSyncInner End";
}
 
/**
 * @tc.name:CleanCacheInnerTest
 * @tc.desc:Verify the CleanCacheInner function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, CleanCacheInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCacheInner Start";
    try {
        std::string uri = "";
        int ret = g_servicePtr_->CleanCacheInner(uri);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanCacheInner FAILED";
    }
    GTEST_LOG_(INFO) << "CleanCacheInner End";
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
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
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
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "Clean FAILED";
    }
    GTEST_LOG_(INFO) << "Clean end";
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
        std::string path;
        bool needClean = false;
        int ret = g_servicePtr_->StopDownloadFile(path, needClean);
        EXPECT_EQ(E_PERMISSION_DENIED, ret);
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
        sptr<CloudDownloadCallbackMock> downloadCallback = sptr(new CloudDownloadCallbackMock());
        int ret = g_servicePtr_->RegisterDownloadFileCallback(downloadCallback);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
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
        int ret = g_servicePtr_->UnregisterDownloadFileCallback();
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
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
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
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
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadFileTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "DownloadFileTest001 end";
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
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
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
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
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
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
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
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback error branch FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterDownloadAssetCallback error branch end";
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
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
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
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DeleteAssetTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "DeleteAssetTest002 end";
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
        std::string uri = "file://com.hmos.notepad/data/storage/el2/distributedfiles/dir/1...txt";
        int32_t userId = 100;
        std::string ret = g_servicePtr_->GetHmdfsPath(uri, userId);
        std::string out = "/mnt/hmdfs/100/account/device_view/local/data/com.hmos.notepad/dir/1...txt";
        EXPECT_EQ(ret, out);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetHmdfsPathTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "GetHmdfsPathTest003 end";
}

/**
 * @tc.name:GetHmdfsPathTest004
 * @tc.desc:Verify the GetHmdfsPath function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, GetHmdfsPathTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHmdfsPathTest004 start";
    try {
        std::string uri = "file://com.hmos.notepad/data/storage/el2/distributedfiles/../../dir/1.txt";
        int32_t userId = 100;
        std::string ret = g_servicePtr_->GetHmdfsPath(uri, userId);
        EXPECT_EQ(ret, "");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetHmdfsPathTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "GetHmdfsPathTest004 end";
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
 * @tc.name: OnAddSystemAbilityTest
 * @tc.desc: Verify the OnAddSystemAbility function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, OnAddSystemAbilityTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAddSystemAbility start";
    try {
        int32_t systemAbilityId = 100;
        std::string deviceId = "";
        g_servicePtr_->OnAddSystemAbility(systemAbilityId, deviceId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnAddSystemAbility FAILED";
    }
    GTEST_LOG_(INFO) << "OnAddSystemAbility end";
}

/**
 * @tc.name: LoadRemoteSATest
 * @tc.desc: Verify the LoadRemoteSA function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, LoadRemoteSATest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadRemoteSA start";
    try {
        std::string deviceId = "";
        int ret = g_servicePtr_->LoadRemoteSA(deviceId);
        EXPECT_EQ(ret, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadRemoteSA FAILED";
    }
    GTEST_LOG_(INFO) << "LoadRemoteSA end";
}

/**
 * @tc.name:SetDeathRecipientTest001
 * @tc.desc:Verify the SetDeathRecipient function.
 * @tc.type:FUNC
 * @tc.require: IB3SLT
 */
HWTEST_F(CloudSyncServiceTest, SetDeathRecipientTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeathRecipientTest001 start";
    try {
        EXPECT_NE(g_servicePtr_, nullptr);
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        g_servicePtr_->SetDeathRecipient(callback);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetDeathRecipientTest001 failed";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipientTest001 end";
}

/**
 * @tc.name: OnActiveTest001
 * @tc.desc: Verify the OnActive function.
 * @tc.type: FUNC
 * @tc.require: IB3SLT
 */
HWTEST_F(CloudSyncServiceTest, OnActiveTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnActiveTest001 start";
    try {
        EXPECT_NE(g_servicePtr_, nullptr);
        SystemAbilityOnDemandReason startReason;
        g_servicePtr_->OnActive(startReason);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnActiveTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnActiveTest001 end";
}

/**
 * @tc.name: HandleStartReasonTest001
 * @tc.desc: Verify the HandleStartReason function.
 * @tc.type: FUNC
 * @tc.require: IB3SLT
 */
HWTEST_F(CloudSyncServiceTest, HandleStartReasonTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStartReasonTest001 start";
    try {
        EXPECT_NE(g_servicePtr_, nullptr);
        SystemAbilityOnDemandReason startReason;
        g_servicePtr_->HandleStartReason(startReason);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleStartReason failed";
    }
    GTEST_LOG_(INFO) << "HandleStartReasonTest001 end";
}

/**
 * @tc.name: InitTest001
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: IB3SLT
 */
HWTEST_F(CloudSyncServiceTest, InitTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitTest001 start";
    try {
        EXPECT_NE(g_servicePtr_, nullptr);
        g_servicePtr_->Init();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitTest001 failed";
    }
    GTEST_LOG_(INFO) << "InitTest001 end";
}

/**
 * @tc.name: GetBundleNameUserInfoTest001
 * @tc.desc: Verify the GetBundleNameUserInfo function.
 * @tc.type: FUNC
 * @tc.require: IB3SLT
 */
HWTEST_F(CloudSyncServiceTest, GetBundleNameUserInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleNameUserInfoTest001 start";
    try {
        EXPECT_NE(g_servicePtr_, nullptr);
        BundleNameUserInfo userInfo;
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVAL_ARG));
        int32_t ret = g_servicePtr_->GetBundleNameUserInfo(userInfo);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBundleNameUserInfoTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetBundleNameUserInfoTest001 end";
}

/**
 * @tc.name: GetBundleNameUserInfoTest002
 * @tc.desc: Verify the GetBundleNameUserInfo function.
 * @tc.type: FUNC
 * @tc.require: IB3SLT
 */
HWTEST_F(CloudSyncServiceTest, GetBundleNameUserInfoTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleNameUserInfoTest002 start";
    try {
        EXPECT_NE(g_servicePtr_, nullptr);
        std::vector<std::string> uriVec = {""};
        BundleNameUserInfo userInfo;
        g_servicePtr_->GetBundleNameUserInfo(uriVec, userInfo);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBundleNameUserInfoTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetBundleNameUserInfoTest002 end";
}

/**
 * @tc.name: OnLoadSACompleteForRemoteTest001
 * @tc.desc: Verify the OnLoadSACompleteForRemote function.
 * @tc.type: FUNC
 * @tc.require: IB3SLT
 */
HWTEST_F(CloudSyncServiceTest, OnLoadSACompleteForRemoteTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnLoadSACompleteForRemoteTest001 start";
    try {
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        CloudSyncService::LoadRemoteSACallback callBack;
        callBack.OnLoadSACompleteForRemote("test", 0, callback);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnLoadSACompleteForRemoteTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnLoadSACompleteForRemoteTest001 end";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
