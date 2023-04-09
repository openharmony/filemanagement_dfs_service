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

#include "cloud_sync_callback_proxy.h"
#include "dfs_error.h"
#include "service_callback_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

class CloudSyncCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<CloudSyncCallbackProxy> proxy_ = nullptr;
    sptr<CloudSyncCallbackMock> mock_ = nullptr;
};

void CloudSyncCallbackProxyTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncCallbackProxyTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncCallbackProxyTest::SetUp(void)
{
    mock_ = sptr(new CloudSyncCallbackMock());
    proxy_ = make_shared<CloudSyncCallbackProxy>(mock_);
    std::cout << "SetUp" << std::endl;
}

void CloudSyncCallbackProxyTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: OnSyncStateChangedTest
 * @tc.desc: Verify the OnSyncStateChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackProxyTest, OnSyncStateChangedTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnSyncStateChanged Start";
    try {
        EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(1).WillOnce(Return(E_OK));
        SyncType syncType = SyncType::UPLOAD;
        SyncPromptState syncPromptState = SyncPromptState::SYNC_STATE_SYNCING;
        proxy_->OnSyncStateChanged(syncType, syncPromptState);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " OnSyncStateChanged ERROR";
    }
    GTEST_LOG_(INFO) << "OnSyncStateChanged End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
