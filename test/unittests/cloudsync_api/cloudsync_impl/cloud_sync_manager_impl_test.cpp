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

std::shared_ptr<CloudSyncManagerImpl> g_managePtr_;

class CloudSyncManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class CloudSyncCallbackDerived : public CloudSyncCallback {
public:
    void OnSyncStateChanged(SyncType type, SyncPromptState state)
    {
        std::cout << "OnSyncStateChanged" << std::endl;
    }
};

void CloudSyncManagerTest::SetUpTestCase(void)
{
    if (g_managePtr_ == nullptr) {
        g_managePtr_ = make_shared<CloudSyncManagerImpl>();
        ASSERT_TRUE(g_managePtr_ != nullptr) << "CallbackManager failed";
    }
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncManagerTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncManagerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudSyncManagerTest::TearDown(void)
{
    g_managePtr_ = nullptr;
    std::cout << "TearDown" << std::endl;
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
        int res = g_managePtr_->StartSync(forceFlag, callback);
        EXPECT_EQ(res, E_OK);
        forceFlag = true;
        res = g_managePtr_->StartSync(forceFlag, callback);
        EXPECT_EQ(res, E_OK);
        forceFlag = true;
        res = g_managePtr_->StartSync(forceFlag, nullptr);
        EXPECT_NE(res, E_OK);
        forceFlag = false;
        res = g_managePtr_->StartSync(forceFlag, nullptr);
        EXPECT_NE(res, E_OK);
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
        int res = g_managePtr_->StopSync();
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " StopSyncTest FAILED";
    }
    GTEST_LOG_(INFO) << "StopSyncTest End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
