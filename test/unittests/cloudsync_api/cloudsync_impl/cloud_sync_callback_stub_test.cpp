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
#include <gmock/gmock.h>

#include "cloud_sync_callback_stub.h"
#include "service_callback_mock.h"
#include "i_cloud_sync_callback.h"
#include "dfs_error.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MockCallback final : public CloudSyncCallbackStub {
public:
    MOCK_METHOD2(OnSyncStateChanged, void(SyncType type, SyncPromptState state));
};

class CloudSyncCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncCallbackStubTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncCallbackStubTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncCallbackStubTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudSyncCallbackStubTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: HandleOnSyncStateChangedTest
 * @tc.desc: Verify the HandleOnSyncStateChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackStubTest, HandleOnSyncStateChangedTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnSyncStateChanged Start";
    try {
        MockCallback callback;
        EXPECT_CALL(callback, OnSyncStateChanged(_, _)).WillOnce(Return());

        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncCallback::GetDescriptor()));
        
        const string bundleName = "com.ohos.photos";
        EXPECT_TRUE(data.WriteInt32(static_cast<int>((SyncType)0)));
        EXPECT_TRUE(data.WriteInt32(static_cast<int>((SyncPromptState)0)));

        EXPECT_EQ(E_OK, callback.OnRemoteRequest(
            ICloudSyncCallback::SERVICE_CMD_ON_SYNC_STATE_CHANGED, data, reply, option));
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnSyncStateChanged ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnSyncStateChanged End";
}

} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS
