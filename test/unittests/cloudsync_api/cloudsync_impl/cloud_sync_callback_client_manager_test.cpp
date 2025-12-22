/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dfs_error.h"
#include "cloud_sync_callback_client_manager.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;

class CloudSyncCallbackClientManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncCallbackClientManagerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncCallbackClientManagerTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncCallbackClientManagerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudSyncCallbackClientManagerTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: AddCallbackTest001
 * @tc.desc: Verify the AddCallback function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncCallbackClientManagerTest, AddCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCallbackTest001 start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudSyncCallback>();
        int32_t ret = CloudSyncCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        EXPECT_EQ(ret, E_OK);
        ret = CloudSyncCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddCallbackTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "AddCallbackTest001 end";
}

/**
 * @tc.name: RemoveCallbackTest001
 * @tc.desc: Verify the RemoveCallback function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncCallbackClientManagerTest, RemoveCallbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveCallbackTest001 start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = nullptr;
        int32_t ret = CloudSyncCallbackClientManager::GetInstance().RemoveCallback(callbackInfo);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveCallbackTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RemoveCallbackTest001 end";
}

/**
 * @tc.name: RemoveCallbackTest002
 * @tc.desc: Verify the RemoveCallback function.
 * @tc.type: FUNC
 * @tc.require: #NA
 */
HWTEST_F(CloudSyncCallbackClientManagerTest, RemoveCallbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveCallbackTest002 start";
    try {
        CallbackInfo callbackInfo;
        callbackInfo.callbackId = "0x0000005a40d3b680";
        callbackInfo.bundleName = "com.ohos.photos";
        callbackInfo.callback = make_shared<CloudSyncCallback>();
        int32_t ret = CloudSyncCallbackClientManager::GetInstance().AddCallback(callbackInfo);
        EXPECT_EQ(ret, E_OK);
        callbackInfo.callback = nullptr;
        ret = CloudSyncCallbackClientManager::GetInstance().RemoveCallback(callbackInfo);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RemoveCallbackTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RemoveCallbackTest002 end";
}
} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS
