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
#include "cloud_sync_constants.h"
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
    MOCK_METHOD2(OnSyncStateChanged, void(CloudSyncState state, ErrorType error));
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
 * @tc.name: HandleOnSyncStateChangedTest001
 * @tc.desc: Verify the HandleOnSyncStateChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackStubTest, HandleOnSyncStateChangedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest001 Start";
    try {
        MockCallback callback;
        EXPECT_CALL(callback, OnSyncStateChanged(_, _)).WillOnce(Return());

        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncCallback::GetDescriptor()));

        const string bundleName = "com.ohos.photos";
        EXPECT_TRUE(data.WriteInt32(static_cast<int>((CloudSyncState)0)));
        EXPECT_TRUE(data.WriteInt32(static_cast<int>((ErrorType)0)));

        EXPECT_EQ(E_OK, callback.OnRemoteRequest(
            ICloudSyncCallback::SERVICE_CMD_ON_SYNC_STATE_CHANGED, data, reply, option));
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnSyncStateChangedTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest001 End";
}


/**
 * @tc.name: HandleOnSyncStateChangedTest002
 * @tc.desc: Verify the HandleOnSyncStateChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackStubTest, HandleOnSyncStateChangedTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest002 Start";
    try {
        MockCallback callback;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        const string bundleName = "com.ohos.photos";
        EXPECT_TRUE(data.WriteInt32(static_cast<int>((CloudSyncState)0)));
        EXPECT_TRUE(data.WriteInt32(static_cast<int>((ErrorType)0)));

        EXPECT_NE(E_OK, callback.OnRemoteRequest(
            ICloudSyncCallback::SERVICE_CMD_ON_SYNC_STATE_CHANGED, data, reply, option));
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnSyncStateChangedTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest002 End";
}

/**
 * @tc.name: HandleOnSyncStateChangedTest003
 * @tc.desc: Verify the HandleOnSyncStateChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackStubTest, HandleOnSyncStateChangedTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest003 Start";
    try {
        MockCallback callback;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_TRUE(data.WriteInterfaceToken(ICloudSyncCallback::GetDescriptor()));
        callback.opToInterfaceMap_.clear();

        const string bundleName = "com.ohos.photos";
        EXPECT_TRUE(data.WriteInt32(static_cast<int>((CloudSyncState)0)));
        EXPECT_TRUE(data.WriteInt32(static_cast<int>((ErrorType)0)));

        EXPECT_NE(E_OK, callback.OnRemoteRequest(
            ICloudSyncCallback::SERVICE_CMD_ON_SYNC_STATE_CHANGED, data, reply, option));
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnSyncStateChangedTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest003 End";
}
} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS
