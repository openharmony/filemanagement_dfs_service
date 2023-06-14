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

#include "dfs_error.h"
#include "iservice_registry.h"
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
 * @tc.name: RemoveCallbackTest
 * @tc.desc: Verify the RemoveCallback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackManagerTest, RemoveCallbackTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveCallbackTest Start";
    try {
        const string bundleName = "com.ohos.photos";
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        const int userId = 0;
        g_managePtr_->AddCallback(bundleName, userId, callback);
        CloudSyncCallbackManager::CallbackInfo cbInfo;
        int res = g_managePtr_->callbackListMap_.Find(bundleName, cbInfo);
        EXPECT_EQ(res, true);
        EXPECT_NE(cbInfo.callbackProxy, nullptr);
        g_managePtr_->RemoveCallback(bundleName);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RemoveCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "RemoveCallbackTest End";
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
        const string bundleName = "com.ohos.photos";
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        const int userId = 0;
        g_managePtr_->AddCallback(bundleName, userId, callback);
        CloudSyncCallbackManager::CallbackInfo cbInfo;
        int res = g_managePtr_->callbackListMap_.Find(bundleName, cbInfo);
        EXPECT_EQ(res, true);
        EXPECT_NE(cbInfo.callbackProxy, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " AddCallbackTest FAILED";
    }
    GTEST_LOG_(INFO) << "AddCallbackTest End";
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
        const string bundleName = "com.ohos.photos";
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        const int userId = 0;
        g_managePtr_->AddCallback(bundleName, userId, callback);
        CloudSyncCallbackManager::CallbackInfo cbInfo;
        int res = g_managePtr_->callbackListMap_.Find(bundleName, cbInfo);
        EXPECT_TRUE(res);
        g_managePtr_->SetDeathRecipient(bundleName, cbInfo);
        cbInfo.callbackProxy = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SetDeathRecipient ERROR";
    }
    GTEST_LOG_(INFO) << "SetDeathRecipient End";
}

/*
 * @tc.name: NotifyTest
 * @tc.desc: Verify the Notify function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackManagerTest, NotifyTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Notify Start";
    try {
        const string bundleName = "com.ohos.photos";
        CloudSyncState state = CloudSyncState::COMPLETED;
        ErrorType error = ErrorType::NO_ERROR;
        sptr<CloudSyncCallbackMock> callback = sptr(new CloudSyncCallbackMock());
        const int userId = 0;
        g_managePtr_->AddCallback(bundleName, userId, callback);
        CloudSyncCallbackManager::CallbackInfo cbInfo;
        int res = g_managePtr_->callbackListMap_.Find(bundleName, cbInfo);
        EXPECT_TRUE(res);
        auto callbackProxy = cbInfo.callbackProxy;
        EXPECT_NE(callbackProxy, nullptr);
        callbackProxy->OnSyncStateChanged(state, error);
        g_managePtr_->Notify(bundleName, cbInfo, state, error);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " Notify ERROR";
    }
    GTEST_LOG_(INFO) << "Notify End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
