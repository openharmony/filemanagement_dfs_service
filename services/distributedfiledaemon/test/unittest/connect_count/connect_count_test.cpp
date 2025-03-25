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

#include "connect_count/connect_count.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "ipc/i_file_dfs_listener.h"
#include "dfs_error.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace OHOS::FileManagement;

class MockFileDfsListener : public IFileDfsListener {
public:
    MOCK_METHOD2(OnStatus, void(const std::string &networkId, int32_t status));
    MOCK_METHOD0(AsObject, sptr<IRemoteObject>());
};

class ConnectCountTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    static inline std::shared_ptr<ConnectCount> connectCount_ = nullptr;
    static inline uint32_t testCallingTokenId = 1234;
    static inline std::string testNetworkId = "testNetworkId";
    static inline sptr<IFileDfsListener> testListener = nullptr;

    const int32_t ON_STATUS_OFFLINE = 13900046;
};

void ConnectCountTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "ConnectCountTest SetUpTestCase";
    connectCount_ = ConnectCount::GetInstance();
    testListener = new MockFileDfsListener();  // Use Mock class
}

void ConnectCountTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "ConnectCountTest TearDownTestCase";

    connectCount_ = nullptr;
    testListener = nullptr;
}

void ConnectCountTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void ConnectCountTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    connectCount_->RemoveAllConnect();
}

/**
 * @tc.name: ConnectCountTest_GetInstance_001
 * @tc.desc: Verify GetInstance returns the same instance.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, GetInstance_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstance_001 start";
    auto instance1 = ConnectCount::GetInstance();
    auto instance2 = ConnectCount::GetInstance();
    EXPECT_EQ(instance1, instance2);  // Verify singleton instance
    GTEST_LOG_(INFO) << "GetInstance_001 end";
}

/**
 * @tc.name: ConnectCountTest_AddConnect_001
 * @tc.desc: Verify AddConnect adds a new connection.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, AddConnect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddConnect_001 start";
    connectCount_->AddConnect(testCallingTokenId, testNetworkId, testListener);
    EXPECT_TRUE(connectCount_->CheckCount(testNetworkId));  // Verify connection is added
    GTEST_LOG_(INFO) << "AddConnect_001 end";
}

/**
 * @tc.name: ConnectCountTest_AddConnect_002
 * @tc.desc: Verify AddConnect increments the count for existing connection.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, AddConnect_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddConnect_002 start";
    connectCount_->AddConnect(testCallingTokenId, testNetworkId, testListener);
    connectCount_->AddConnect(testCallingTokenId, testNetworkId, testListener);
    auto networkIds = connectCount_->GetNetworkIds(testCallingTokenId);
    EXPECT_EQ(networkIds.size(), 1);  // Verify only one networkId is added
    GTEST_LOG_(INFO) << "AddConnect_002 end";
}

/**
 * @tc.name: ConnectCountTest_RemoveConnect_001
 * @tc.desc: Verify RemoveConnect removes a connection by callingTokenId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, RemoveConnect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveConnect_001 start";
    connectCount_->AddConnect(testCallingTokenId, testNetworkId, testListener);
    auto networkIds = connectCount_->RemoveConnect(testCallingTokenId);
    EXPECT_EQ(networkIds.size(), 1);  // Verify networkId is returned
    EXPECT_FALSE(connectCount_->CheckCount(testNetworkId));  // Verify connection is removed
    GTEST_LOG_(INFO) << "RemoveConnect_001 end";
}

/**
 * @tc.name: ConnectCountTest_RemoveConnect_002
 * @tc.desc: Verify RemoveConnect removes a connection by networkId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, RemoveConnect_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveConnect_002 start";
    connectCount_->AddConnect(testCallingTokenId, testNetworkId, testListener);
    connectCount_->RemoveConnect(testNetworkId);
    EXPECT_FALSE(connectCount_->CheckCount(testNetworkId));  // Verify connection is removed
    GTEST_LOG_(INFO) << "RemoveConnect_002 end";
}

/**
 * @tc.name: ConnectCountTest_RemoveConnect_003
 * @tc.desc: Verify RemoveConnect removes a connection by callingTokenId and networkId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, RemoveConnect_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveConnect_003 start";
    connectCount_->AddConnect(testCallingTokenId, testNetworkId, testListener);
    connectCount_->RemoveConnect(testCallingTokenId, testNetworkId);
    EXPECT_FALSE(connectCount_->CheckCount(testNetworkId));  // Verify connection is removed
    GTEST_LOG_(INFO) << "RemoveConnect_003 end";
}

/**
 * @tc.name: ConnectCountTest_RemoveAllConnect_001
 * @tc.desc: Verify RemoveAllConnect removes all connections.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, RemoveAllConnect_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RemoveAllConnect_001 start";
    connectCount_->AddConnect(testCallingTokenId, testNetworkId, testListener);
    connectCount_->RemoveAllConnect();
    EXPECT_FALSE(connectCount_->CheckCount(testNetworkId));  // Verify all connections are removed
    GTEST_LOG_(INFO) << "RemoveAllConnect_001 end";
}

/**
 * @tc.name: ConnectCountTest_NotifyRemoteReverseObj_001
 * @tc.desc: Verify NotifyRemoteReverseObj notifies the listener.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, NotifyRemoteReverseObj_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyRemoteReverseObj_001 start";

    // 设置 EXPECT_CALL
    EXPECT_CALL(*static_cast<MockFileDfsListener*>(testListener.GetRefPtr()),
                OnStatus(testNetworkId, ON_STATUS_OFFLINE))
                .Times(1);  // 仅在当前测试用例中生效

    connectCount_->AddConnect(testCallingTokenId, testNetworkId, testListener);
    connectCount_->NotifyRemoteReverseObj(testNetworkId, ON_STATUS_OFFLINE);

    // 清除 EXPECT_CALL 断言
    testing::Mock::VerifyAndClearExpectations(testListener.GetRefPtr());

    GTEST_LOG_(INFO) << "NotifyRemoteReverseObj_001 end";
}

/**
 * @tc.name: ConnectCountTest_FindCallingTokenIdForListerner_001
 * @tc.desc: Verify FindCallingTokenIdForListerner finds the correct callingTokenId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, FindCallingTokenIdForListerner_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindCallingTokenIdForListerner_001 start";
    connectCount_->AddConnect(testCallingTokenId, testNetworkId, testListener);
    uint32_t foundCallingTokenId = 0;
    int32_t ret = connectCount_->FindCallingTokenIdForListerner(testListener->AsObject(), foundCallingTokenId);
    EXPECT_EQ(ret, FileManagement::E_OK);  // Verify success
    EXPECT_EQ(foundCallingTokenId, testCallingTokenId);  // Verify correct tokenId is found
    GTEST_LOG_(INFO) << "FindCallingTokenIdForListerner_001 end";
}

/**
 * @tc.name: ConnectCountTest_FindCallingTokenIdForListerner_002
 * @tc.desc: Verify FindCallingTokenIdForListerner returns error for invalid listener.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, FindCallingTokenIdForListerner_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindCallingTokenIdForListerner_002 start";
    sptr<IRemoteObject> invalidListener = nullptr;
    uint32_t foundCallingTokenId = 0;
    int32_t ret = connectCount_->FindCallingTokenIdForListerner(invalidListener, foundCallingTokenId);
    EXPECT_EQ(ret, FileManagement::ERR_BAD_VALUE);  // Verify error is returned
    GTEST_LOG_(INFO) << "FindCallingTokenIdForListerner_002 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS