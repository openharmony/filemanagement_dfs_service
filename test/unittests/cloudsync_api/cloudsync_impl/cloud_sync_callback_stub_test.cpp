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
#include "message_parcel_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::Storage::DistributedFile;

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
    std::shared_ptr<MessageParcelMock> messageParcelMock_;
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
    messageParcelMock_ = std::make_shared<MessageParcelMock>();
    DfsMessageParcel::messageParcel = messageParcelMock_;
    std::cout << "SetUp" << std::endl;
}

void CloudSyncCallbackStubTest::TearDown(void)
{
    DfsMessageParcel::messageParcel = nullptr;
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: HandleOnSyncStateChangedTest001
 * @tc.desc: Verify HandleOnSyncStateChanged function - ReadInt32(stateValue) failed
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackStubTest, HandleOnSyncStateChangedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest001 Start";
    try {
        MockCallback callback;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        EXPECT_CALL(*messageParcelMock_, ReadInt32(_)).WillOnce(Return(false));
        int ret = callback.HandleOnSyncStateChanged(data, reply);

        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnSyncStateChangedTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest001 End";
}

/**
 * @tc.name: HandleOnSyncStateChangedTest002
 * @tc.desc: Verify HandleOnSyncStateChanged function - ReadInt32(errorValue) failed
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

        EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
            .WillOnce(DoAll(SetArgReferee<0>(0), Return(true)))
            .WillOnce(Return(false));
        int ret = callback.HandleOnSyncStateChanged(data, reply);

        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnSyncStateChangedTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest002 End";
}

/**
 * @tc.name: HandleOnSyncStateChangedTest003
 * @tc.desc: Verify HandleOnSyncStateChanged function - all reads succeed
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

        EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
            .WillOnce(DoAll(SetArgReferee<0>(0), Return(true)))
            .WillOnce(DoAll(SetArgReferee<0>(0), Return(true)));
        EXPECT_CALL(callback, OnSyncStateChanged(_, _)).WillOnce(Return());

        int ret = callback.HandleOnSyncStateChanged(data, reply);
        EXPECT_EQ(ret, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnSyncStateChangedTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest003 End";
}

/**
 * @tc.name: HandleOnSyncStateChangedTest004
 * @tc.desc: Verify HandleOnSyncStateChanged function - stateValue < UPLOADING
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackStubTest, HandleOnSyncStateChangedTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest004 Start";
    try {
        MockCallback callback;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;

        EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
            .WillOnce(DoAll(SetArgReferee<0>(-1), Return(true)));

        int ret = callback.HandleOnSyncStateChanged(data, reply);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnSyncStateChangedTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest004 End";
}

/**
 * @tc.name: HandleOnSyncStateChangedTest005
 * @tc.desc: Verify HandleOnSyncStateChanged function - stateValue > STOPPED
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackStubTest, HandleOnSyncStateChangedTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest005 Start";
    try {
        MockCallback callback;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
            .WillOnce(DoAll(SetArgReferee<0>(6), Return(true)));

        int ret = callback.HandleOnSyncStateChanged(data, reply);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnSyncStateChangedTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest005 End";
}

/**
 * @tc.name: HandleOnSyncStateChangedTest006
 * @tc.desc: Verify HandleOnSyncStateChanged function - errorValue < NO_ERROR
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackStubTest, HandleOnSyncStateChangedTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest006 Start";
    try {
        MockCallback callback;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
            .WillOnce(DoAll(SetArgReferee<0>(0), Return(true)))
            .WillOnce(DoAll(SetArgReferee<0>(-1), Return(true)));
        
        int ret = callback.HandleOnSyncStateChanged(data, reply);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnSyncStateChangedTest006 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest006 End";
}

/**
 * @tc.name: HandleOnSyncStateChangedTest007
 * @tc.desc: Verify HandleOnSyncStateChanged function - errorValue > INNER_ERROR
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncCallbackStubTest, HandleOnSyncStateChangedTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest007 Start";
    try {
        MockCallback callback;
        MessageParcel data;
        MessageParcel reply;
        MessageOption option;
        EXPECT_CALL(*messageParcelMock_, ReadInt32(_))
            .WillOnce(DoAll(SetArgReferee<0>(0), Return(true)))
            .WillOnce(DoAll(SetArgReferee<0>(12), Return(true)));
        
        int ret = callback.HandleOnSyncStateChanged(data, reply);
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " HandleOnSyncStateChangedTest007 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleOnSyncStateChangedTest007 End";
}
} // namespace Test
} // namespace FileManagement::CloudSync {
} // namespace OHOS
