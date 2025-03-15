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
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
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
namespace fs = std::filesystem;
using namespace testing::ext;
using namespace testing;
using namespace std;
constexpr int32_t MAX_DENTRY_FILE_SIZE = 500;
constexpr int32_t MAX_FILE_CACHE_NUM = 400;
static const int32_t CLEAN_FILE_MAX_SIZE = 200;

class CloudSyncManagerImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<CloudSyncManagerImpl> managePtr_ = nullptr;
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
    managePtr_ = make_shared<CloudSyncManagerImpl>();
    std::cout << "SetUp" << std::endl;
}

void CloudSyncManagerImplTest::TearDown(void)
{
    managePtr_ = nullptr;
    std::cout << "TearDown" << std::endl;
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
        auto callback = nullptr;
        int32_t res = CloudSyncManagerImpl::GetInstance().RegisterCallback(callback);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterCallbackTest End";
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
        shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncCallbackDerived>();
        int32_t res = CloudSyncManagerImpl::GetInstance().RegisterCallback(callback);
        EXPECT_EQ(res, E_OK);
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
        string bundleName = "com.ohos.photos";
        int32_t res = CloudSyncManagerImpl::GetInstance().UnRegisterCallback(bundleName);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnRegisterCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "UnRegisterCallbackTest End";
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
        int32_t res = CloudSyncManagerImpl::GetInstance().StartSync(bundleName);
        EXPECT_EQ(res, E_OK);
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
    GTEST_LOG_(INFO) << "StartSyncTest Start";
    try {
        bool forceFlag = true;
        string bundleName = "com.ohos.photos";
        int32_t res = CloudSyncManagerImpl::GetInstance().StartSync(forceFlag, nullptr);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncTest End";
}

/**
 * @tc.name: StartSyncTest003
 * @tc.desc: Verify the StartSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, StartSyncTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncTest Start";
    try {
        bool forceFlag = true;
        shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncCallbackDerived>();
        string bundleName = "com.ohos.photos";
        int32_t res = CloudSyncManagerImpl::GetInstance().StartSync(forceFlag, callback);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncTest End";
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
        int32_t res = CloudSyncManagerImpl::GetInstance().GetSyncTime(syncTime, bundleName);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncTimeTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetSyncTimeTest End";
}

/**
 * @tc.name: TriggerSyncTest001
 * @tc.desc: Verify the TriggerSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerImplTest, TriggerSyncTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TriggerSyncTest Start";
    try {
        string bundleName = "";
        int32_t userId = 0;
        int32_t res = CloudSyncManagerImpl::GetInstance().TriggerSync(bundleName, userId);
        EXPECT_EQ(res, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "TriggerSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "TriggerSyncTest End";
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
        int32_t res = CloudSyncManagerImpl::GetInstance().TriggerSync(bundleName, userId);
        EXPECT_EQ(res, E_OK);
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
        int res = CloudSyncManagerImpl::GetInstance().StopSync(bundleName);
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
        auto res = CloudSyncManagerImpl::GetInstance().NotifyEventChange(userId, eventId, extraData);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "NotifyEventChangeTest FAILED";
    }
    GTEST_LOG_(INFO) << "NotifyEventChangeTest End";
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
        auto res = CloudSyncManagerImpl::GetInstance().StartFileCache(uri);
        EXPECT_EQ(res, E_OK);
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
    GTEST_LOG_(INFO) << "SetDeathRecipientTest Start";
    try {
        auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
        CloudSyncManagerImpl::GetInstance().SetDeathRecipient(CloudSyncServiceProxy->AsObject());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetDeathRecipientTest FAILED";
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
        auto res = CloudSyncManagerImpl::GetInstance().CleanCache(uri);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanCacheTest FAILED";
    }
    GTEST_LOG_(INFO) << "CleanCacheTest End";
}

HWTEST_F(CloudSyncManagerImplTest, BatchCleanFileTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BatchCleanFileTest1 Start";
    try {
        CleanFileInfo cleanFileInfo;
        std::vector<CleanFileInfo> fileInfo;
        fileInfo.emplace_back(cleanFileInfo);
        std::vector<std::string> failCloudId;
        auto res = CloudSyncManagerImpl::GetInstance().BatchCleanFile(fileInfo, failCloudId);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "BatchCleanFileTest1 FAILED";
    }
    GTEST_LOG_(INFO) << "BatchCleanFileTest1 End";
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

HWTEST_F(CloudSyncManagerImplTest, ResetCursorTest, TestSize.Level1)
{
    string bundleName = "com.ohos.photos";
    int res = CloudSyncManagerImpl::GetInstance().ResetCursor(bundleName);
    EXPECT_EQ(res, E_OK);
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

HWTEST_F(CloudSyncManagerImplTest, StartFileCacheTest003, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"uri"};
    int64_t downloadId = 0;
    int32_t res = CloudSyncManagerImpl::GetInstance().StartFileCache(uriVec, downloadId);
    EXPECT_EQ(res, E_OK);
}

HWTEST_F(CloudSyncManagerImplTest, StopFileCacheTest, TestSize.Level1)
{
    int64_t downloadId = 0;
    bool needClean = true;
    int32_t res = CloudSyncManagerImpl::GetInstance().StopFileCache(downloadId, needClean);
    EXPECT_EQ(res, E_OK);
}

HWTEST_F(CloudSyncManagerImplTest, ResetProxyCallbackTest, TestSize.Level1)
{
    uint32_t retryCount = 3;
    string bundleName = "testBundle";
    auto res = CloudSyncManagerImpl::GetInstance().ResetProxyCallback(retryCount, bundleName);
    EXPECT_EQ(res, true);
}

HWTEST_F(CloudSyncManagerImplTest, DownloadThumbTest, TestSize.Level1)
{
    int32_t res = CloudSyncManagerImpl::GetInstance().DownloadThumb();
    EXPECT_EQ(res, E_OK);
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
        int32_t result = CloudSyncManagerImpl::GetInstance().BatchDentryFileInsert(fileInfo, failCloudId);

        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " BatchDentryFileInsertTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "BatchDentryFileInsertTest002 End";
}

 /*
  * @tc.name: OnAddSystemAbilityTest
  * @tc.desc: Verify the OnAddSystemAbility function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncManagerImplTest, OnAddSystemAbilityTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAddSystemAbilityTest001 Start";
    try {
        string bundleName = "testbundleName";
        CloudSyncManagerImpl::SystemAbilityStatusChange statusChange(bundleName);
        int32_t systemAbilityId = 1;
        std::string deviceId = "testDeviceId";

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
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncManagerImplTest, CleanGalleryDentryFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest001 Start";
    try {
        managePtr_->CleanGalleryDentryFile();

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

        managePtr_->CleanGalleryDentryFile();

        EXPECT_FALSE(fs::exists("/storage/media/cloud/files/Photo"));
        EXPECT_FALSE(fs::exists("/storage/media/cloud/files/.thumbs/Photo"));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanGalleryDentryFileTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest002 End";
}

 /*
  * @tc.name: ResetProxyCallback
  * @tc.desc: Verify the ResetProxyCallback function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncManagerImplTest, ResetProxyCallbackTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest002 Start";
    try {
        CloudSyncManagerImpl cloudSyncManagerImpl;

        EXPECT_TRUE(cloudSyncManagerImpl.ResetProxyCallback(1, "testBundle"));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanGalleryDentryFileTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanGalleryDentryFileTest002 End";
}

 /*
  * @tc.name: ResetProxyCallback
  * @tc.desc: Verify the ResetProxyCallback function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncManagerImplTest, ResetProxyCallbackTest2, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResetProxyCallbackTest2 Start";
    try {
        CloudSyncManagerImpl cloudSyncManagerImpl;
        CloudSyncServiceProxy::GetInstance();

        EXPECT_TRUE(cloudSyncManagerImpl.ResetProxyCallback(1, "testBundle"));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ResetProxyCallbackTest2 FAILED";
    }
    GTEST_LOG_(INFO) << "ResetProxyCallbackTest2 End";
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
        CloudSyncManagerImpl cloudSyncManagerImpl;
        cloudSyncManagerImpl.SubscribeListener("testBundleName");

        EXPECT_EQ(cloudSyncManagerImpl.listener_, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SubscribeListenerTest1 FAILED";
    }
    GTEST_LOG_(INFO) << "SubscribeListenerTest1 End";
}

 /*
  * @tc.name: StopFileCache
  * @tc.desc: Verify the StopFileCache function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncManagerImplTest, StopFileCacheTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopFileCacheTest1 Start";
    try {
        CloudSyncManagerImpl &cloudSyncManagerImpl = CloudSyncManagerImpl::GetInstance();
        int64_t downloadId = 1;
        bool needClean = true;
        int32_t timeout = 10;

        int32_t result = cloudSyncManagerImpl.StopFileCache(downloadId, needClean, timeout);

        EXPECT_EQ(result, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopFileCacheTest1 FAILED";
    }
    GTEST_LOG_(INFO) << "StopFileCacheTest1 End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS