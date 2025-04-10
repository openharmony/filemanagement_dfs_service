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
    if (g_servicePtr_ == nullptr) {
        int32_t saId = 5204;
        g_servicePtr_ = std::make_shared<CloudSyncService>(saId);
        ASSERT_TRUE(g_servicePtr_ != nullptr) << "SystemAbility failed";
    }
    std::cout << "SetUpTestCase" << std::endl;
    g_servicePtr_->dataSyncManager_ = make_shared<CloudFile::DataSyncManager>();
}

void CloudSyncServiceTest::TearDownTestCase(void)
{
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
 * @tc.name: OnStartTest001
 * @tc.desc: Verify the OnStart function.
 * @tc.type: FUNC
 * @tc.require: IB3SLT
 */
HWTEST_F(CloudSyncServiceTest, OnStartTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStartTest001 start";
    try {
        EXPECT_NE(g_servicePtr_, nullptr);
        SystemAbilityOnDemandReason startReason;
        g_servicePtr_->OnStart(startReason);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStartTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnStartTest001 end";
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
 * @tc.name: PreInitTest001
 * @tc.desc: Verify the PreInit function.
 * @tc.type: FUNC
 * @tc.require: IB3SLT
 */
HWTEST_F(CloudSyncServiceTest, PreInitTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PreInitTest001 start";
    try {
        EXPECT_NE(g_servicePtr_, nullptr);
        g_servicePtr_->PreInit();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PreInitTest001 failed";
    }
    GTEST_LOG_(INFO) << "PreInitTest001 end";
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
