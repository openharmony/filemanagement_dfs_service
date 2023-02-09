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
#include <sstream>
#include <string>
#include <sys/mount.h>
#include <system_error>
#include <unistd.h>
#include <memory>

#include "cloud_sync_service_proxy.h"
#include "cloud_sync_callback_proxy.h"
#include "cloud_sync_callback_manager.h"
#include "dfs_error.h"
#include "cloud_sync_manager.h"
#include "cloud_sync_callback_client.h"
#include "securec.h"
#include "utils_directory.h"
#include "utils_log.h"
#include "cloud_sync_service.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;

class CloudSyncDerived : public CloudSyncCallback  {
public:
    void OnSyncStateChanged(SyncType type, SyncPromptState state) {}
};

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
 * @tc.name: GetServiceProxy_001
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
    shared_ptr<CloudSyncCallback> callback = make_shared<CloudSyncDerived>();
    SyncType type = SyncType(0);
    bool forceFlag = true;
    int ret = CloudSyncManager::GetInstance().StartSync(type, forceFlag, callback);
    EXPECT_EQ(ret, E_OK);
}

} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS
