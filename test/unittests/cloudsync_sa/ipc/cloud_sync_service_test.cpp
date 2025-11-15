/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "battersrvclient_mock.h"
#include "cloud_file_kit.h"
#include "cloud_sync_common.h"
#include "cloud_sync_service.h"
#include "dfsu_access_token_helper_mock.h"
#include "i_cloud_download_callback_mock.h"
#include "service_callback_mock.h"
#include "system_ability_manager_client_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace CloudFile;
using ChargeState = PowerMgr::BatteryChargeState;
using PluggedType = PowerMgr::BatteryPluggedType;
constexpr int32_t SA_ID = 5204;

class CloudFileKitMock : public CloudFile::CloudFileKit {
public:
    MOCK_METHOD3(GetCloudUserInfo, int32_t(const std::string &bundleName, const int32_t userId,
                CloudFile::CloudUserInfo &userInfo));
    MOCK_METHOD3(GetAppSwitchStatus, int32_t(const std::string &bundleName, const int32_t userId, bool &switchStatus));
    MOCK_METHOD3(GetAppConfigParams, int32_t(const int32_t userId,
                const std::string &bundleName, std::map<std::string, std::string> &param));
    MOCK_METHOD2(GetCloudAssetsDownloader, std::shared_ptr<CloudAssetsDownloader>(const int32_t userId,
                const std::string &bundleName));
};

class CloudSyncServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<SystemAbilityManagerClientMock> saMgrClient_ = nullptr;
    static inline sptr<ISystemAbilityManagerMock> saMgr_ = nullptr;
    static inline shared_ptr<CloudDiskServiceAccessTokenMock> dfsuAccessToken_ = nullptr;
    static inline sptr<CloudSyncService> servicePtr_ = nullptr;
    static inline shared_ptr<BatterySrvClientMock> dfsBatterySrvClient_ = nullptr;
};

void CloudSyncServiceTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
    servicePtr_ = sptr(new CloudSyncService(SA_ID));
    saMgr_ = sptr(new ISystemAbilityManagerMock());
    saMgrClient_ = make_shared<SystemAbilityManagerClientMock>();
    ISystemAbilityManagerClient::smc = saMgrClient_;
    servicePtr_->dataSyncManager_ = make_shared<CloudFile::DataSyncManager>();
    dfsuAccessToken_ = make_shared<CloudDiskServiceAccessTokenMock>();
    CloudDiskServiceAccessTokenMock::dfsuAccessToken = dfsuAccessToken_;
    dfsBatterySrvClient_ = make_shared<BatterySrvClientMock>();
    BatterySrvClientMock::dfsBatterySrvClient = dfsBatterySrvClient_;
}

void CloudSyncServiceTest::TearDownTestCase(void)
{
    CloudDiskServiceAccessTokenMock::dfsuAccessToken = nullptr;
    ISystemAbilityManagerClient::smc = nullptr;
    dfsuAccessToken_ = nullptr;
    saMgrClient_ = nullptr;
    servicePtr_ = nullptr;
    saMgr_ = nullptr;
    BatterySrvClientMock::dfsBatterySrvClient = nullptr;
    dfsBatterySrvClient_ = nullptr;
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
 * @tc.name: ChangeAppSwitchTest001
 * @tc.desc: Verify the ChangeAppSwitch function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, ChangeAppSwitchTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeAppSwitchTest001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        std::string accountId = "";
        bool status = false;
        std::string bundleName = "";

        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(1));

        int32_t ret = servicePtr_->ChangeAppSwitch(accountId, bundleName, status);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "ChangeAppSwitchTest001 failed";
    }
    GTEST_LOG_(INFO) << "ChangeAppSwitchTest001 end";
}

/**
 * @tc.name: ChangeAppSwitchTest002
 * @tc.desc: Verify the ChangeAppSwitch function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, ChangeAppSwitchTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeAppSwitchTest002 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        std::string accountId = "";
        bool status = false;
        std::string bundleName = "";

        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(1));

        int32_t ret = servicePtr_->ChangeAppSwitch(accountId, bundleName, status);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "ChangeAppSwitchTest002 failed";
    }
    GTEST_LOG_(INFO) << "ChangeAppSwitchTest002 end";
}

/**
 * @tc.name: ChangeAppSwitchTest003
 * @tc.desc: Verify the ChangeAppSwitch function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, ChangeAppSwitchTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeAppSwitchTest003 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        std::string accountId = "";
        bool status = false;
        std::string bundleName = "";

        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(1));

        int32_t ret = servicePtr_->ChangeAppSwitch(accountId, bundleName, status);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "ChangeAppSwitchTest003 failed";
    }
    GTEST_LOG_(INFO) << "ChangeAppSwitchTest003 end";
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->UnRegisterCallbackInner(bundleName);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->RegisterCallbackInner(callback, bundleName);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->RegisterCallbackInner(callback, bundleName);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->TriggerSyncInner(bundleName, userId);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->StopSyncInner(bundleName);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->CleanCacheInner(uri);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->EnableCloud(accountId, switchData);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->Clean(accountId, cleanOptions);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "Clean FAILED";
    }
    GTEST_LOG_(INFO) << "Clean end";
}


/**
 * @tc.name:StopDownloadFileTest002
 * @tc.desc:Verify the StopDownloadFile function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, StopDownloadFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDownloadFile start";
    try {
        int64_t downloadId = 0;
        bool needClean = false;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetPid()).WillOnce(Return(101));
        int ret = servicePtr_->StopDownloadFile(downloadId, needClean);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopDownloadFileTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "StopDownloadFileTest002 end";
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
        int64_t downloadId = 0;
        bool needClean = false;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->StopDownloadFile(downloadId, needClean);
        EXPECT_EQ(E_PERMISSION_DENIED, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopDownloadFile FAILED";
    }
    GTEST_LOG_(INFO) << "StopDownloadFile end";
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->UploadAsset(userId, request, result);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->DownloadFile(userId, bundleName, assetInfoObj);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->DownloadAsset(taskId, userId, bundleName, networkId, assetInfoObj);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->DownloadAsset(taskId, userId, bundleName, networkId, assetInfoObj);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->RegisterDownloadAssetCallback(callback);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->RegisterDownloadAssetCallback(callback);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->DeleteAsset(userId, uri);
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
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        int ret = servicePtr_->DeleteAsset(userId, uri);
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
        std::string ret = servicePtr_->GetHmdfsPath(uri, userId);
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
        std::string ret = servicePtr_->GetHmdfsPath(uri, userId);
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
        std::string ret = servicePtr_->GetHmdfsPath(uri, userId);
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
        std::string ret = servicePtr_->GetHmdfsPath(uri, userId);
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
        servicePtr_->OnStop();
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
        servicePtr_->OnAddSystemAbility(systemAbilityId, deviceId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnAddSystemAbility FAILED";
    }
    GTEST_LOG_(INFO) << "OnAddSystemAbility end";
}

/**
 * @tc.name: OnAddSystemAbilityTest002
 * @tc.desc: Verify the OnAddSystemAbility function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, OnAddSystemAbilityTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAddSystemAbilityTest002 start";
    try {
        int32_t MEMORY_MANAGER_SA_ID = 1909;
        int32_t systemAbilityId = MEMORY_MANAGER_SA_ID;
        std::string deviceId = "";
        servicePtr_->OnAddSystemAbility(systemAbilityId, deviceId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnAddSystemAbilityTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "OnAddSystemAbilityTest002 end";
}

/**
 * @tc.name: LoadRemoteSATest001
 * @tc.desc: Verify the LoadRemoteSA function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, LoadRemoteSATest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadRemoteSATest001 start";
    try {
        std::string deviceId = "";
        int ret = servicePtr_->LoadRemoteSA(deviceId);
        EXPECT_EQ(ret, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadRemoteSATest001 FAILED";
    }
    GTEST_LOG_(INFO) << "LoadRemoteSATest001 end";
}

/**
 * @tc.name: LoadRemoteSATest002
 * @tc.desc: Verify the LoadRemoteSA function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, LoadRemoteSATest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadRemoteSATest002 start";
    try {
        std::string deviceId = "abc123";
        EXPECT_CALL(*saMgrClient_, GetSystemAbilityManager()).WillOnce(Return(nullptr));
        int ret = servicePtr_->LoadRemoteSA(deviceId);
        EXPECT_EQ(ret, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadRemoteSATest002 FAILED";
    }
    GTEST_LOG_(INFO) << "LoadRemoteSATest002 end";
}

/**
 * @tc.name: LoadRemoteSATest003
 * @tc.desc: Verify the LoadRemoteSA function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, LoadRemoteSATest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadRemoteSATest003 start";
    try {
        std::string deviceId = "abc123";
        sptr<CloudDownloadCallbackMock> downloadCallback = sptr(new CloudDownloadCallbackMock());
        servicePtr_->remoteObjectMap_.insert(std::make_pair(deviceId, downloadCallback));
        int ret = servicePtr_->LoadRemoteSA(deviceId);
        servicePtr_->remoteObjectMap_.clear();
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadRemoteSATest003 FAILED";
    }
    GTEST_LOG_(INFO) << "LoadRemoteSATest003 end";
}

/**
 * @tc.name: LoadRemoteSATest004
 * @tc.desc: Verify the LoadRemoteSA function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, LoadRemoteSATest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadRemoteSATest004 start";
    try {
        std::string deviceId = "abcd1234";
        EXPECT_CALL(*saMgrClient_, GetSystemAbilityManager()).WillOnce(Return(saMgr_));
        sptr<CloudDownloadCallbackMock> downloadCallback = sptr(new CloudDownloadCallbackMock());
        EXPECT_CALL(*saMgr_, CheckSystemAbility(An<int32_t>(), An<const string &>()))
            .WillOnce(Return(downloadCallback));
        servicePtr_->remoteObjectMap_.clear();
        int ret = servicePtr_->LoadRemoteSA(deviceId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadRemoteSATest004 FAILED";
    }
    GTEST_LOG_(INFO) << "LoadRemoteSATest004 end";
}

/**
 * @tc.name: LoadRemoteSATest005
 * @tc.desc: Verify the LoadRemoteSA function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, LoadRemoteSATest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadRemoteSATest005 start";
    try {
        std::string deviceId = "abcde12345";
        EXPECT_CALL(*saMgrClient_, GetSystemAbilityManager()).WillOnce(Return(saMgr_));
        EXPECT_CALL(*saMgr_, CheckSystemAbility(An<int32_t>(), An<const string &>())).WillOnce(Return(nullptr));
        int ret = servicePtr_->LoadRemoteSA(deviceId);
        servicePtr_->remoteObjectMap_.clear();
        EXPECT_EQ(ret, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadRemoteSATest005 FAILED";
    }
    GTEST_LOG_(INFO) << "LoadRemoteSATest005 end";
}

/**
 * @tc.name: LoadRemoteSATest006
 * @tc.desc: Verify the LoadRemoteSA function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, LoadRemoteSATest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadRemoteSATest006 start";
    try {
        std::string deviceId = "abcdef123456";
        EXPECT_CALL(*saMgrClient_, GetSystemAbilityManager()).WillOnce(Return(saMgr_));
        servicePtr_->remoteObjectMap_.insert(std::make_pair(deviceId, nullptr));
        EXPECT_CALL(*saMgr_, CheckSystemAbility(An<int32_t>(), An<const string &>())).WillOnce(Return(nullptr));
        int ret = servicePtr_->LoadRemoteSA(deviceId);
        servicePtr_->remoteObjectMap_.clear();
        EXPECT_EQ(ret, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadRemoteSATest006 FAILED";
    }
    GTEST_LOG_(INFO) << "LoadRemoteSATest006 end";
}

/**
 * @tc.name: LoadRemoteSATest007
 * @tc.desc: Verify the LoadRemoteSA function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, LoadRemoteSATest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadRemoteSATest007 start";
    try {
        std::string deviceId = "abcdefg1234567";
        EXPECT_CALL(*saMgrClient_, GetSystemAbilityManager()).WillOnce(Return(saMgr_)).WillOnce(Return(saMgr_));
        servicePtr_->remoteObjectMap_.insert(std::make_pair(deviceId, nullptr));
        sptr<CloudDownloadCallbackMock> downloadCallback = sptr(new CloudDownloadCallbackMock());
        EXPECT_CALL(*saMgr_, CheckSystemAbility(An<int32_t>(), An<const string &>()))
            .WillOnce(Return(downloadCallback));
        int ret = servicePtr_->LoadRemoteSA(deviceId);
        servicePtr_->remoteObjectMap_.clear();
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadRemoteSATest007 FAILED";
    }
    GTEST_LOG_(INFO) << "LoadRemoteSATest007 end";
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
        EXPECT_NE(servicePtr_, nullptr);
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        servicePtr_->SetDeathRecipient(callback);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetDeathRecipientTest001 failed";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipientTest001 end";
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
        EXPECT_NE(servicePtr_, nullptr);
        SystemAbilityOnDemandReason startReason;
        startReason.reasonName_ = "load";
        servicePtr_->HandleStartReason(startReason);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleStartReason failed";
    }
    GTEST_LOG_(INFO) << "HandleStartReasonTest001 end";
}

/**
 * @tc.name: HandleStartReasonTest002
 * @tc.desc: Verify the HandleStartReason function.
 * @tc.type: FUNC
 * @tc.require: IB3SLT
 */
HWTEST_F(CloudSyncServiceTest, HandleStartReasonTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStartReasonTest002 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        EXPECT_CALL(*saMgrClient_, GetSystemAbilityManager()).WillRepeatedly(Return(nullptr));
        SystemAbilityOnDemandReason startReason;
        startReason.reasonName_ = "usual.event.SCREEN_OFF";
        servicePtr_->HandleStartReason(startReason);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleStartReasonTest002 failed";
    }
    GTEST_LOG_(INFO) << "HandleStartReasonTest002 end";
}

/**
 * @tc.name: HandleStartReasonTest003
 * @tc.desc: Verify the HandleStartReason function.
 * @tc.type: FUNC
 * @tc.require: IB3SLT
 */
HWTEST_F(CloudSyncServiceTest, HandleStartReasonTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleStartReasonTest003 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        EXPECT_CALL(*saMgrClient_, GetSystemAbilityManager()).WillRepeatedly(Return(nullptr));
        SystemAbilityOnDemandReason startReason;
        startReason.reasonName_ = "usual.event.POWER_CONNECTED";
        servicePtr_->HandleStartReason(startReason);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleStartReasonTest003 failed";
    }
    GTEST_LOG_(INFO) << "HandleStartReasonTest003 end";
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
        EXPECT_NE(servicePtr_, nullptr);
        servicePtr_->Init();
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
        EXPECT_NE(servicePtr_, nullptr);
        BundleNameUserInfo userInfo;
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVAL_ARG));
        int32_t ret = servicePtr_->GetBundleNameUserInfo(userInfo);
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
        EXPECT_NE(servicePtr_, nullptr);
        std::vector<std::string> uriVec = {""};
        BundleNameUserInfo userInfo;
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetPid()).WillOnce(Return(101));
        servicePtr_->GetBundleNameUserInfo(uriVec, userInfo);
        EXPECT_EQ(userInfo.userId, 100);
        EXPECT_EQ(userInfo.pid, 101);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBundleNameUserInfoTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetBundleNameUserInfoTest002 end";
}

/**
 * @tc.name: StartDowngradeTest002
 * @tc.desc: Verify the StartDowngrade function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncServiceTest, StartDowngradeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDowngradeTest002 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        std::string bundleName = "com.ohos.aaa";
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        sptr<DowngradeDlCallbackMock> callback = sptr(new DowngradeDlCallbackMock());
        auto ret = servicePtr_->StartDowngrade(bundleName, callback);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartDowngradeTest002 failed";
    }
    GTEST_LOG_(INFO) << "StartDowngradeTest002 end";
}

/**
 * @tc.name: StopDowngradeTest001
 * @tc.desc: Verify the StopDowngrade function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncServiceTest, StopDowngradeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDowngradeTest001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        std::string bundleName = "com.ohos.aaa";
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        auto ret = servicePtr_->StopDowngrade(bundleName);
        EXPECT_NE(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopDowngradeTest001 failed";
    }
    GTEST_LOG_(INFO) << "StopDowngradeTest001 end";
}

/**
 * @tc.name: StopDowngradeTest002
 * @tc.desc: Verify the StopDowngrade function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncServiceTest, StopDowngradeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDowngradeTest002 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        std::string bundleName = "com.ohos.aaa";
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        auto ret = servicePtr_->StopDowngrade(bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopDowngradeTest002 failed";
    }
    GTEST_LOG_(INFO) << "StopDowngradeTest002 end";
}

/**
 * @tc.name: GetCloudFileInfoTest001
 * @tc.desc: Verify the GetCloudFileInfo function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncServiceTest, GetCloudFileInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudFileInfoTest001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        std::string bundleName = "com.ohos.aaa";
        CloudFileInfo cloudFileInfo;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        auto ret = servicePtr_->GetCloudFileInfo(bundleName, cloudFileInfo);
        EXPECT_NE(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCloudFileInfoTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetCloudFileInfoTest001 end";
}

/**
 * @tc.name: GetCloudFileInfoTest002
 * @tc.desc: Verify the GetCloudFileInfo function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncServiceTest, GetCloudFileInfoTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudFileInfoTest002 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        std::string bundleName = "com.ohos.aaa";
        CloudFileInfo cloudFileInfo;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        auto ret = servicePtr_->GetCloudFileInfo(bundleName, cloudFileInfo);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCloudFileInfoTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetCloudFileInfoTest002 end";
}

/**
 * @tc.name: GetBundlesLocalFilePresentStatusTest001
 * @tc.desc: Verify the GetBundlesLocalFilePresentStatus function when permission denied.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncServiceTest, GetBundlesLocalFilePresentStatusTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        std::vector<std::string> bundleNames = {"com.ohos.aaa"};
        std::vector<CloudSync::LocalFilePresentStatus> localFilePresentStatusList;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        auto ret = servicePtr_->GetBundlesLocalFilePresentStatus(bundleNames, localFilePresentStatusList);
        EXPECT_NE(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest001 end";
}

/**
 * @tc.name: GetBundlesLocalFilePresentStatusTest002
 * @tc.desc: Verify the GetBundlesLocalFilePresentStatus function when permission allowed and service returns OK.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncServiceTest, GetBundlesLocalFilePresentStatusTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest002 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        std::vector<std::string> bundleNames = {"com.ohos.aaa"};
        std::vector<CloudSync::LocalFilePresentStatus> localFilePresentStatusList;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(100));
        auto ret = servicePtr_->GetBundlesLocalFilePresentStatus(bundleNames, localFilePresentStatusList);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetBundlesLocalFilePresentStatusTest002 end";
}

/**
 * @tc.name: GetHistoryVersionList001
 * @tc.desc: Verify the GetHistoryVersionList function.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncServiceTest, GetHistoryVersionList001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHistoryVersionList001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        string uri;
        int32_t numLimit = 0;
        vector<HistoryVersion> historyVersionList;
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVAL_ARG));
        auto ret = servicePtr_->GetHistoryVersionList(uri, numLimit, historyVersionList);
        EXPECT_NE(ret, E_OK);

        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetPid()).WillOnce(Return(101));
        ret = servicePtr_->GetHistoryVersionList(uri, numLimit, historyVersionList);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetHistoryVersionList001 failed";
    }
    GTEST_LOG_(INFO) << "GetHistoryVersionList001 end";
}

/**
 * @tc.name: DownloadHistoryVersion001
 * @tc.desc: Verify the DownloadHistoryVersion function.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncServiceTest, DownloadHistoryVersion001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadHistoryVersion001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        string uri;
        int64_t downloadId = 0;
        uint64_t versionId = 0;
        sptr<CloudDownloadCallbackMock> callback = sptr(new CloudDownloadCallbackMock());
        string versionUri;
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVAL_ARG));
        auto ret = servicePtr_->DownloadHistoryVersion(uri, downloadId, versionId, callback, versionUri);
        EXPECT_NE(ret, E_OK);

        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetPid()).WillOnce(Return(101));
        ret = servicePtr_->DownloadHistoryVersion(uri, downloadId, versionId, callback, versionUri);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DownloadHistoryVersion001 failed";
    }
    GTEST_LOG_(INFO) << "DownloadHistoryVersion001 end";
}

/**
 * @tc.name: ReplaceFileWithHistoryVersion001
 * @tc.desc: Verify the ReplaceFileWithHistoryVersion function.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncServiceTest, ReplaceFileWithHistoryVersion001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReplaceFileWithHistoryVersion001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        string oriUri;
        string uri;
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVAL_ARG));
        auto ret = servicePtr_->ReplaceFileWithHistoryVersion(oriUri, uri);
        EXPECT_NE(ret, E_OK);

        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetPid()).WillOnce(Return(101));
        ret = servicePtr_->ReplaceFileWithHistoryVersion(oriUri, uri);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReplaceFileWithHistoryVersion001 failed";
    }
    GTEST_LOG_(INFO) << "ReplaceFileWithHistoryVersion001 end";
}

/**
 * @tc.name: IsFileConflict001
 * @tc.desc: Verify the IsFileConflict function.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncServiceTest, IsFileConflict001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsFileConflict001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        string uri;
        bool isConflict;
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVAL_ARG));
        auto ret = servicePtr_->IsFileConflict(uri, isConflict);
        EXPECT_NE(ret, E_OK);

        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetPid()).WillOnce(Return(101));
        ret = servicePtr_->IsFileConflict(uri, isConflict);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsFileConflict001 failed";
    }
    GTEST_LOG_(INFO) << "IsFileConflict001 end";
}

/**
 * @tc.name: ClearFileConflict001
 * @tc.desc: Verify the ClearFileConflict function.
 * @tc.type: FUNC
 * @tc.require: ICGORT
 */
HWTEST_F(CloudSyncServiceTest, ClearFileConflict001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearFileConflict001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        string uri;
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_INVAL_ARG));
        auto ret = servicePtr_->ClearFileConflict(uri);
        EXPECT_NE(ret, E_OK);

        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetPid()).WillOnce(Return(101));
        ret = servicePtr_->ClearFileConflict(uri);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ClearFileConflict001 failed";
    }
    GTEST_LOG_(INFO) << "ClearFileConflict001 end";
}

/**
 * @tc.name: CleanFileCacheInner001
 * @tc.desc: Verify the CleanFileCacheInner001 function.
 * @tc.type: FUNC
 * @tc.require: ICK6VD
 */
HWTEST_F(CloudSyncServiceTest, CleanFileCacheInner001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanFileCacheInner001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        string uri;
        EXPECT_CALL(*dfsuAccessToken_, CheckUriPermission(_)).WillOnce(Return(false));
        auto ret = servicePtr_->CleanFileCacheInner(uri);
        EXPECT_EQ(ret, E_ILLEGAL_URI);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "CleanFileCacheInner001 failed";
    }
    GTEST_LOG_(INFO) << "CleanFileCacheInner001 end";
}

/**
 * @tc.name: CleanFileCacheInner002
 * @tc.desc: Verify the CleanFileCacheInner002 function.
 * @tc.type: FUNC
 * @tc.require: ICK6VD
 */
HWTEST_F(CloudSyncServiceTest, CleanFileCacheInner002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanFileCacheInner002 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        string uri;
        EXPECT_CALL(*dfsuAccessToken_, CheckUriPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_ILLEGAL_URI));
        auto ret = servicePtr_->CleanFileCacheInner(uri);
        EXPECT_EQ(ret, E_SERVICE_INNER_ERROR);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "CleanFileCacheInner002 failed";
    }
    GTEST_LOG_(INFO) << "CleanFileCacheInner002 end";
}

/**
 * @tc.name: CleanFileCacheInner003
 * @tc.desc: Verify the CleanFileCacheInner003 function.
 * @tc.type: FUNC
 * @tc.require: ICK6VD
 */
HWTEST_F(CloudSyncServiceTest, CleanFileCacheInner003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanFileCacheInner003 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        string uri;
        EXPECT_CALL(*dfsuAccessToken_, CheckUriPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        auto ret = servicePtr_->CleanFileCacheInner(uri);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "CleanFileCacheInner003 failed";
    }
    GTEST_LOG_(INFO) << "CleanFileCacheInner003 end";
}

/**
 * @tc.name: StartFileCacheTest001
 * @tc.desc: Verify the StartFileCache function.
 * @tc.type: FUNC
 * @tc.require: ICK6VD
 */
HWTEST_F(CloudSyncServiceTest, StartFileCacheTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFileCacheTest001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        string uri = "file://media/xxx";
        int64_t downloadId = 0;
        int32_t fieldkey = 0;
        int32_t timeout = -1;
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(-1));
        auto ret = servicePtr_->StartFileCache({uri}, downloadId, fieldkey, callback, timeout);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "StartFileCacheTest001 failed";
    }
    GTEST_LOG_(INFO) << "StartFileCacheTest001 end";
}

/**
 * @tc.name: StartFileCacheTest002
 * @tc.desc: Verify the StartFileCache function.
 * @tc.type: FUNC
 * @tc.require: ICK6VD
 */
HWTEST_F(CloudSyncServiceTest, StartFileCacheTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFileCacheTest002 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        std::vector<std::string> uris{"file://media/xxx"};
        int64_t downloadId = 0;
        int32_t fieldkey = 0;
        int32_t timeout = -1;
        sptr<CloudDownloadCallbackMock> downloadCallback = sptr(new CloudDownloadCallbackMock());
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetPid()).WillOnce(Return(101));
        auto ret = servicePtr_->StartFileCache(uris, downloadId, fieldkey, downloadCallback, timeout);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "StartFileCacheTest002 failed";
    }
    GTEST_LOG_(INFO) << "StartFileCacheTest002 end";
}

/**
 * @tc.name: StartDownloadFileTest001
 * @tc.desc: Verify the StartDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: ICK6VD
 */
HWTEST_F(CloudSyncServiceTest, StartDownloadFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDownloadFileTest001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        string uri;
        int64_t downloadId = 0;
        sptr<CloudDownloadCallbackMock> downloadCallback = sptr(new CloudDownloadCallbackMock());
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetPid()).WillOnce(Return(101));
        auto ret = servicePtr_->StartDownloadFile(uri, downloadCallback, downloadId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "StartDownloadFileTest001 failed";
    }
    GTEST_LOG_(INFO) << "StartDownloadFileTest001 end";
}

/**
 * @tc.name: StartDownloadFileTest002
 * @tc.desc: Verify the StartDownloadFile function.
 * @tc.type: FUNC
 * @tc.require: ICK6VD
 */
HWTEST_F(CloudSyncServiceTest, StartDownloadFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDownloadFileTest002 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        string uri;
        int64_t downloadId = 0;
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetPid()).WillOnce(Return(101));
        auto ret = servicePtr_->StartDownloadFile(uri, callback, downloadId);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "StartDownloadFileTest002 failed";
    }
    GTEST_LOG_(INFO) << "StartDownloadFileTest002 end";
}

/**
 * @tc.name: StopFileCacheTest001
 * @tc.desc: Verify the StopFileCache function.
 * @tc.type: FUNC
 * @tc.require: ICK6VD
 */
HWTEST_F(CloudSyncServiceTest, StopFileCacheTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopFileCacheTest001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        int64_t downloadId = 0;
        bool needClean = false;
        int32_t timeout = -1;
        EXPECT_CALL(*dfsuAccessToken_, GetCallerBundleName(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*dfsuAccessToken_, GetUserId()).WillOnce(Return(0));
        EXPECT_CALL(*dfsuAccessToken_, GetPid()).WillOnce(Return(101));
        auto ret = servicePtr_->StopFileCache(downloadId, needClean, timeout);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "StopFileCacheTest001 failed";
    }
    GTEST_LOG_(INFO) << "StopFileCacheTest001 end";
}

/**
 * @tc.name: DownloadFilesTest001
 * @tc.desc: Verify the DownloadFiles function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, DownloadFilesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFilesTest001 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        int32_t userId = 100;
        const std::string bundleName = "com.ohos.test";
        std::string uri = "file://com.hmos.notepad/data/storage/el2/distributedfiles/dir/1.txt";
        AssetInfoObj assetInfoObj;
        assetInfoObj.uri = uri;
        std::vector<AssetInfoObj> assetInfoObjs;
        assetInfoObjs.push_back(assetInfoObj);
        std::vector<bool> assetResultMap;
        assetResultMap.push_back(true);
        int32_t connectTime = 1;

        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));

        CloudFile::CloudFileKit::instance_ = nullptr;

        auto ret = servicePtr_->DownloadFiles(userId, bundleName, assetInfoObjs, assetResultMap, connectTime);
        EXPECT_EQ(ret, E_NULLPTR);
        CloudFile::CloudFileKit::instance_ = new (std::nothrow) CloudFileKit();
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "DownloadFilesTest001 failed";
    }
    GTEST_LOG_(INFO) << "DownloadFilesTest001 end";
}

/**
 * @tc.name: DownloadFilesTest002
 * @tc.desc: Verify the DownloadFiles function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, DownloadFilesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFilesTest002 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        int32_t userId = 100;
        const std::string bundleName = "com.ohos.test";
        std::string uri = "file://com.hmos.notepad/data/storage/el2/distributedfiles/dir/1.txt";
        AssetInfoObj assetInfoObj;
        assetInfoObj.uri = uri;
        std::vector<AssetInfoObj> assetInfoObjs;
        assetInfoObjs.push_back(assetInfoObj);
        std::vector<bool> assetResultMap;
        assetResultMap.push_back(true);
        int32_t connectTime = 1;
        auto cloudFileKitMock = new (std::nothrow) CloudFileKitMock();
        CloudFile::CloudFileKit::instance_ = cloudFileKitMock;

        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*cloudFileKitMock, GetCloudAssetsDownloader(_, _)).WillOnce(Return(nullptr));

        auto ret = servicePtr_->DownloadFiles(userId, bundleName, assetInfoObjs, assetResultMap, connectTime);
        EXPECT_EQ(ret, E_NULLPTR);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "DownloadFilesTest002 failed";
    }
    GTEST_LOG_(INFO) << "DownloadFilesTest002 end";
}

/**
 * @tc.name: DownloadFilesTest003
 * @tc.desc: Verify the DownloadFiles function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, DownloadFilesTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DownloadFilesTest003 start";
    try {
        EXPECT_NE(servicePtr_, nullptr);
        int32_t userId = 100;
        const std::string bundleName = "com.ohos.test";
        std::string uri = "";
        AssetInfoObj assetInfoObj;
        assetInfoObj.uri = uri;
        std::vector<AssetInfoObj> assetInfoObjs;
        assetInfoObjs.push_back(assetInfoObj);
        std::vector<bool> assetResultMap;
        assetResultMap.push_back(true);
        int32_t connectTime = 1;
        auto cloudFileKitMock = new (std::nothrow) CloudFileKitMock();
        CloudFile::CloudFileKit::instance_ = cloudFileKitMock;

        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        EXPECT_CALL(*dfsuAccessToken_, IsSystemApp()).WillOnce(Return(true));
        EXPECT_CALL(*cloudFileKitMock, GetCloudAssetsDownloader(_, _)).WillOnce(
                        Return(make_shared<CloudAssetsDownloader>(userId, bundleName)));

        auto ret = servicePtr_->DownloadFiles(userId, bundleName, assetInfoObjs, assetResultMap, connectTime);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "DownloadFilesTest003 failed";
    }
    GTEST_LOG_(INFO) << "DownloadFilesTest003 end";
}

/**
 * @tc.name: RemovedCleanTest001
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, RemovedCleanTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest001 start";
    try {
        string bundleName = "test.bundle.name";
        int32_t userId = 1;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        auto ret = servicePtr_->RemovedClean(bundleName, userId);
        EXPECT_EQ(ret, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "RemovedCleanTest001 failed";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest001 end";
}

/**
 * @tc.name: RemovedCleanTest002
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, RemovedCleanTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest002 start";
    try {
        string bundleName = "";
        int32_t userId = 2;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        auto ret = servicePtr_->RemovedClean(bundleName, userId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "RemovedCleanTest002 failed";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest002 end";
}

/**
 * @tc.name: RemovedCleanTest003
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, RemovedCleanTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest003 start";
    try {
        string bundleName = "";
        int32_t userId = 1;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        auto ret = servicePtr_->RemovedClean(bundleName, userId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "RemovedCleanTest003 failed";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest003 end";
}

/**
 * @tc.name: RemovedCleanTest004
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, RemovedCleanTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest004 start";
    try {
        string bundleName = "";
        int32_t userId = 3;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        auto ret = servicePtr_->RemovedClean(bundleName, userId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "RemovedCleanTest004 failed";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest004 end";
}

/**
 * @tc.name: RemovedCleanTest005
 * @tc.desc: Verify the RemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, RemovedCleanTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemovedCleanTest005 start";
    try {
        string bundleName = "";
        int32_t userId = 4;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        auto ret = servicePtr_->RemovedClean(bundleName, userId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "RemovedCleanTest005 failed";
    }
    GTEST_LOG_(INFO) << "RemovedCleanTest005 end";
}

/**
 * @tc.name: HandleRemovedCleanTest001
 * @tc.desc: Verify the HandleRemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, HandleRemovedCleanTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRemovedCleanTest001 start";
    try {
        string bundleName = "test.bundle.name";
        int32_t userId = 1;
        auto ret = servicePtr_->HandleRemovedClean(bundleName, userId);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "HandleRemovedCleanTest001 failed";
    }
    GTEST_LOG_(INFO) << "HandleRemovedCleanTest001 end";
}

/**
 * @tc.name: HandleRemovedCleanTest002
 * @tc.desc: Verify the HandleRemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, HandleRemovedCleanTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRemovedCleanTest002 start";
    try {
        string bundleName = "";
        int32_t userId = 2;
        auto ret = servicePtr_->HandleRemovedClean(bundleName, userId);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "HandleRemovedCleanTest002 failed";
    }
    GTEST_LOG_(INFO) << "HandleRemovedCleanTest002 end";
}

/**
 * @tc.name: HandleRemovedCleanTest003
 * @tc.desc: Verify the HandleRemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, HandleRemovedCleanTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRemovedCleanTest003 start";
    try {
        string bundleName = "";
        int32_t userId = 1;
        auto ret = servicePtr_->HandleRemovedClean(bundleName, userId);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "HandleRemovedCleanTest003 failed";
    }
    GTEST_LOG_(INFO) << "HandleRemovedCleanTest003 end";
}

/**
 * @tc.name: HandleRemovedCleanTest004
 * @tc.desc: Verify the HandleRemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, HandleRemovedCleanTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRemovedCleanTest004 start";
    try {
        string bundleName = "";
        int32_t userId = 3;
        auto ret = servicePtr_->HandleRemovedClean(bundleName, userId);
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "HandleRemovedCleanTest004 failed";
    }
    GTEST_LOG_(INFO) << "HandleRemovedCleanTest004 end";
}

/**
 * @tc.name: HandleRemovedCleanTest005
 * @tc.desc: Verify the HandleRemovedClean function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudSyncServiceTest, HandleRemovedCleanTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleRemovedCleanTest005 start";
    try {
        string bundleName = "";
        int32_t userId = 4;
        auto ret = servicePtr_->HandleRemovedClean(bundleName, userId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "HandleRemovedCleanTest005 failed";
    }
    GTEST_LOG_(INFO) << "HandleRemovedCleanTest005 end";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS