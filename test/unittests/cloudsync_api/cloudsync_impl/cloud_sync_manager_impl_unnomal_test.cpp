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

#include "cloud_sync_manager_impl.h"
#include "dfs_error.h"
#include "service_proxy_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

class CloudSyncManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<CloudSyncManagerImpl> managePtr_;
    static inline std::shared_ptr<MockServiceProxy> proxy_ = nullptr;
    sptr<CloudSyncServiceMock> serviceProxy_ = nullptr;
};

class CloudSyncCallbackDerived : public CloudSyncCallback {
public:
    void OnSyncStateChanged(CloudSyncState state, ErrorType error)
    {
        std::cout << "OnSyncStateChanged" << std::endl;
    }
};

void CloudSyncManagerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
    proxy_ = std::make_shared<MockServiceProxy>();
    IserviceProxy::proxy_ = proxy_;
}

void CloudSyncManagerTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
    IserviceProxy::proxy_.reset();
    proxy_ = nullptr;
}

void CloudSyncManagerTest::SetUp(void)
{
    managePtr_ = make_shared<CloudSyncManagerImpl>();
    managePtr_->startSyncPending_ = false;
    serviceProxy_ = sptr(new CloudSyncServiceMock());
    testing::Mock::VerifyAndClear(proxy_.get());
    std::cout << "SetUp" << std::endl;
}

void CloudSyncManagerTest::TearDown(void)
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
HWTEST_F(CloudSyncManagerTest, RegisterCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterCallbackTest Start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudSyncCallbackDerived>();
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        auto res = managePtr_->RegisterCallback(callbackInfo);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RegisterCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterCallbackTest End";
}

/**
 * @tc.name: StartSyncTest
 * @tc.desc: Verify the StartSync function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncManagerTest, StartSyncTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncTest Start";
    try {
        bool forceFlag = false;
        shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncCallbackDerived>();
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        auto res = managePtr_->StartSync();
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        res = managePtr_->StartSync(forceFlag, callback);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
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
HWTEST_F(CloudSyncManagerTest, StopSyncTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopSyncTest Start";
    try {
        EXPECT_CALL(*proxy_, GetInstanceWithoutLoad(_)).WillOnce(Return(nullptr));
        int res = managePtr_->StopSync();
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
HWTEST_F(CloudSyncManagerTest, ChangeAppSwitchTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeAppSwitchTest Start";
    try {
        std::string accoutId = "accoutId";
        std::string bundleName = "bundleName";
        bool status = true;
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        auto res = managePtr_->ChangeAppSwitch(accoutId, bundleName, status);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
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
HWTEST_F(CloudSyncManagerTest, NotifyDataChangeTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string accoutId = "accoutId";
        std::string bundleName = "bundleName";
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        auto res = managePtr_->NotifyDataChange(accoutId, bundleName);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
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
HWTEST_F(CloudSyncManagerTest, StartDownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string uri = "uri";
        int64_t downloadId = 0;
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        auto res = managePtr_->StartDownloadFile(uri, nullptr, downloadId);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
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
HWTEST_F(CloudSyncManagerTest, StopDownloadFileTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        int64_t downloadId = 0;
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        auto res = managePtr_->StopDownloadFile(downloadId, true);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
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
HWTEST_F(CloudSyncManagerTest, EnableCloudTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string accoutId = "accoutId";
        SwitchDataObj switchData;
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        auto res = managePtr_->EnableCloud(accoutId, switchData);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
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
HWTEST_F(CloudSyncManagerTest, DisableCloudTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string accoutId = "accoutId";
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        auto res = managePtr_->DisableCloud(accoutId);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
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
HWTEST_F(CloudSyncManagerTest, CleanTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyDataChangeTest Start";
    try {
        std::string accoutId = "accoutId";
        CleanOptions cleanOptions;
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        auto res = managePtr_->Clean(accoutId, cleanOptions);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CleanTest FAILED";
    }
    GTEST_LOG_(INFO) << "CleanTest End";
}

/**
 * @tc.name: GetUploadListTest004
 * @tc.desc: Verify GetDownloadList function with nullptr.
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(CloudSyncManagerTest, GetUploadListTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUploadListTest004 Start";
    try {
        std::vector<std::string> uriVec = {"file://path1"};
        std::vector<CloudSync::UploadProgressObj> uploadList;
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        int32_t res = CloudSyncManagerImpl::GetInstance().GetUploadList(uriVec, uploadList);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetUploadListTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "GetUploadListTest004 End";
}

/**
 * @tc.name: GetDownloadListTest004
 * @tc.desc: Verify GetDownloadList function with nullptr.
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(CloudSyncManagerTest, GetDownloadListTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadListTest004 Start";
    try {
        std::vector<std::string> uriVec = {"file://path1"};
        std::vector<CloudSync::DownloadProgressObj> downloadList;
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        int32_t res = CloudSyncManagerImpl::GetInstance().GetDownloadList(uriVec, downloadList);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDownloadListTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "GetDownloadListTest004 End";
}

/**
 * @tc.name: PauseUploadTest001
 * @tc.desc: Verify PauseUpload function.
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(CloudSyncManagerTest, PauseUploadTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PauseUploadTest001 Start";
    try {
        std::string uri = "file://com.example.test/path1";
        
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, PauseUpload(_)).WillOnce(Return(E_OK));
        int32_t res = managePtr_->PauseUpload(uri);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PauseUploadTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "PauseUploadTest001 End";
}

/**
 * @tc.name: PauseUploadTest002
 * @tc.desc: Verify PauseUpload function with error.
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(CloudSyncManagerTest, PauseUploadTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PauseUploadTest002 Start";
    try {
        std::string uri = "file://com.example.test/path1";
        
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, PauseUpload(_)).WillOnce(Return(E_PERMISSION_DENIED));
        int32_t res = managePtr_->PauseUpload(uri);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PauseUploadTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "PauseUploadTest002 End";
}

/**
 * @tc.name: PauseUploadTest003
 * @tc.desc: Verify PauseUpload function with nullptr proxy.
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(CloudSyncManagerTest, PauseUploadTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PauseUploadTest003 Start";
    try {
        std::string uri = "file://com.example.test/path1";
        
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        int32_t res = managePtr_->PauseUpload(uri);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PauseUploadTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "PauseUploadTest003 End";
}

/**
 * @tc.name: ResumeUploadTest001
 * @tc.desc: Verify ResumeUpload function.
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(CloudSyncManagerTest, ResumeUploadTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResumeUploadTest001 Start";
    try {
        std::string uri = "file://com.example.test/path1";
        
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, ResumeUpload(_)).WillOnce(Return(E_OK));
        int32_t res = managePtr_->ResumeUpload(uri);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ResumeUploadTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ResumeUploadTest001 End";
}

/**
 * @tc.name: ResumeUploadTest002
 * @tc.desc: Verify ResumeUpload function with error.
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(CloudSyncManagerTest, ResumeUploadTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResumeUploadTest002 Start";
    try {
        std::string uri = "file://com.example.test/path1";
        
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(serviceProxy_));
        EXPECT_CALL(*serviceProxy_, ResumeUpload(_)).WillOnce(Return(E_PERMISSION_DENIED));
        int32_t res = managePtr_->ResumeUpload(uri);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ResumeUploadTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "ResumeUploadTest002 End";
}

/**
 * @tc.name: ResumeUploadTest003
 * @tc.desc: Verify ResumeUpload function with nullptr proxy.
 * @tc.type: FUNC
 * @tc.require: issueIC7I52
 */
HWTEST_F(CloudSyncManagerTest, ResumeUploadTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResumeUploadTest003 Start";
    try {
        std::string uri = "file://com.example.test/path1";
        
        EXPECT_CALL(*proxy_, GetInstance(_)).WillOnce(Return(nullptr));
        int32_t res = managePtr_->ResumeUpload(uri);
        EXPECT_EQ(res, E_SA_LOAD_FAILED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ResumeUploadTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "ResumeUploadTest003 End";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
