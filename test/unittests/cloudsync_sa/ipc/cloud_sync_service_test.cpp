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
#include "service_callback_mock.h"
#include "utils_log.h"
#include "dfs_error.h"

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
 * @tc.name: OnStartTest
 * @tc.desc: Verify the OnStart function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, OnStartTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStart start";
    try {
        g_servicePtr_->OnStart();
    } catch(...) {
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
    } catch(...) {
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
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RegisterCallbackInner FAILED";
    }
    GTEST_LOG_(INFO) << "RegisterCallbackInner end";
}

/**
 * @tc.name:StartSyncInnerTest
 * @tc.desc:Verify the StartSyncInner function.
 * @tc.type:FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceTest, StartSyncInnerTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartSyncInner start";
    try {
        bool forceFlag = false;
        int ret = g_servicePtr_->StartSyncInner(forceFlag);
        EXPECT_EQ(ret, E_OK);
        
        forceFlag = true;
        ret = g_servicePtr_->StartSyncInner(forceFlag);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartSyncInner FAILED";
    }
    GTEST_LOG_(INFO) << "StartSyncInner end";
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
        int ret = g_servicePtr_->StopSyncInner();
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StopSyncInner FAILED";
    }
    GTEST_LOG_(INFO) << "StopSyncInner End";
}

} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS
