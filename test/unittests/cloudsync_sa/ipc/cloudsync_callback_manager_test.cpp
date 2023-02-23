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

#include "cloud_sync_callback_manager.h"
#include "cloud_sync_callback_proxy.h"

#include "iservice_registry.h"
#include "dfs_error.h"
#include "service_callback_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

std::shared_ptr<CloudSyncCallbackManager> g_managePtr_;

class CloudSyncCallbackManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncCallbackManagerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncCallbackManagerTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncCallbackManagerTest::SetUp(void)
{
    if (g_managePtr_ == nullptr) {
        g_managePtr_ = make_shared<CloudSyncCallbackManager>();
        ASSERT_TRUE(g_managePtr_ != nullptr) << "CallbackManager failed";
    }
    std::cout << "SetUp" << std::endl;
}

void CloudSyncCallbackManagerTest::TearDown(void)
{
    g_managePtr_ = nullptr;
    std::cout << "TearDown" << std::endl;
}

/*
 * @tc.name: AddCallbackTest
 * @tc.desc: Verify the AddCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackManagerTest, AddCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddCallbackTest Start";
    try {
        const string appPackageName = "com.ohos.photos";
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        const int userId = 0;
        g_managePtr_->AddCallback(appPackageName, userId, callback);
        CloudSyncCallbackManager::CallbackInfo cbInfo;
        int res = g_managePtr_->callbackListMap_.Find(appPackageName, cbInfo);
        EXPECT_EQ(res, true);
        EXPECT_NE(cbInfo.callbackProxy_, nullptr);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AddCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "AddCallbackTest End";
}
 
 /*
  * @tc.name: GetCallbackProxyTest
  * @tc.desc: Verify the GetCallbackProxy function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncCallbackManagerTest, GetCallbackProxyTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallbackProxy Start";
    try {
        const string appPackageName = "com.ohos.photos";
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        const int userId = 0;
        auto callbackProxy_ = g_managePtr_->GetCallbackProxy(appPackageName, userId);
        EXPECT_EQ(callbackProxy_, nullptr);
        g_managePtr_->AddCallback(appPackageName, userId, callback);
        CloudSyncCallbackManager::CallbackInfo cbInfo;
        int res = g_managePtr_->callbackListMap_.Find(appPackageName, cbInfo);
        EXPECT_EQ(res, true);
        EXPECT_NE(cbInfo.callbackProxy_, nullptr);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetCallbackProxy ERROR";
    }
    GTEST_LOG_(INFO) << "GetCallbackProxy End";
}

/*
 * @tc.name: SetDeathRecipientTest
 * @tc.desc: Verify the SetDeathRecipient function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackManagerTest, SetDeathRecipientTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetDeathRecipient Start";
    try {
        const string appPackageName = "com.ohos.photos";
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        const int userId = 0;
        g_managePtr_->AddCallback(appPackageName, userId, callback);
        CloudSyncCallbackManager::CallbackInfo cbInfo;
        int res = g_managePtr_->callbackListMap_.Find(appPackageName, cbInfo);
        EXPECT_TRUE(res);
        g_managePtr_->SetDeathRecipient(appPackageName, cbInfo);
        cbInfo.callbackProxy_ = nullptr;
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetDeathRecipient ERROR";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipient End";
}

} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS
