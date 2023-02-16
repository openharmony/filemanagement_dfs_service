/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

#include "accesstoken_kit.h"
#include "token_setproc.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;
using namespace OHOS::Security::AccessToken;

class CloudSyncDerived : public CloudSyncCallback {
public:
    void OnSyncStateChanged(SyncType type, SyncPromptState state)
    {
        LOGI("type: %{public}d state: %{public}d", static_cast<int32_t>(type), static_cast<int32_t>(state));
    }
};

static HapInfoParams g_info = {.userID = 100,
                               .bundleName = "hdcd", // shell process name, avoid register callback more times
                               .instIndex = 0,
                               .appIDDesc = "ohos.permission_test.demo",
                               .isSystemApp = true};

static HapPolicyParams g_policy = {.apl = APL_NORMAL,
                                   .domain = "test.domain",
                                   .permList = {{.permissionName = "ohos.permission.MANAGE_MEDIA_RESOURCES",
                                                 .bundleName = "hdcd",
                                                 .grantMode = 1,
                                                 .availableLevel = APL_NORMAL,
                                                 .label = "label",
                                                 .labelId = 1,
                                                 .description = "test",
                                                 .descriptionId = 1}},
                                   .permStateList = {{.permissionName = "ohos.permission.MANAGE_MEDIA_RESOURCES",
                                                      .isGeneral = true,
                                                      .resDeviceID = {"local"},
                                                      .grantStatus = {PermissionState::PERMISSION_GRANTED},
                                                      .grantFlags = {1}}}};

class CloudSyncServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncServiceTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncServiceTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncServiceTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
    std::cout << "SetUp" << std::endl;
}

void CloudSyncServiceTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: StartSync_001
 * @tc.desc:  function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
/*
   E_OK = ERR_OK,
   E_SEVICE_DIED = CSS_ERR_OFFSET,
   E_INVAL_ARG,
   E_BROKEN_IPC,
   E_SA_LOAD_FAILED,
*/
HWTEST_F(CloudSyncServiceTest, StartSync_001, TestSize.Level1)
{
    LOGE("testcase run OK");
    shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncDerived>();
    SyncType type = SyncType(0);
    bool forceFlag = true;
    int ret = CloudSyncManager::GetInstance().StartSync(type, forceFlag, callback);
    EXPECT_EQ(ret, E_OK);
}

/**
 * @tc.name: StartSync_002
 * @tc.desc:  function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
/*
   E_OK = ERR_OK,
   E_SEVICE_DIED = CSS_ERR_OFFSET,
   E_INVAL_ARG,
   E_BROKEN_IPC,
   E_SA_LOAD_FAILED,
*/
HWTEST_F(CloudSyncServiceTest, StartSync_002, TestSize.Level1)
{
    LOGE("testcase run OK");

    uint64_t selfTokenId = GetSelfTokenID();
    LOGE("selfTokenId = %{public}llu", selfTokenId);
    AccessTokenKit::AllocHapToken(g_info, g_policy);
    AccessTokenIDEx tokenID = AccessTokenKit::GetHapTokenIDEx(g_info.userID, g_info.bundleName, g_info.instIndex);
    SetSelfTokenID(tokenID.tokenIDEx);

    shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncDerived>();
    SyncType type = SyncType(0);
    bool forceFlag = true;
    int ret = CloudSyncManager::GetInstance().StartSync(type, forceFlag, callback);
    EXPECT_EQ(ret, E_OK);

    SetSelfTokenID(selfTokenId);
    auto tokenId = AccessTokenKit::GetHapTokenID(g_info.userID, g_info.bundleName, g_info.instIndex);
    AccessTokenKit::DeleteToken(tokenId);
}

/**
 * @tc.name: StopSync_001
 * @tc.desc:  function.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
/*
   E_OK = ERR_OK,
   E_SEVICE_DIED = CSS_ERR_OFFSET,
   E_INVAL_ARG,
   E_BROKEN_IPC,
   E_SA_LOAD_FAILED,
*/
HWTEST_F(CloudSyncServiceTest, StopSync_001, TestSize.Level1)
{
    LOGE("testcase run OK");
    shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncDerived>();
    SyncType type = SyncType(0);
    bool forceFlag = true;
    int ret = CloudSyncManager::GetInstance().StartSync(type, forceFlag, callback);
    EXPECT_EQ(ret, E_OK);

    ret = CloudSyncManager::GetInstance().StopSync();
    EXPECT_EQ(ret, E_OK);
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
