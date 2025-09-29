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
    MOCK_METHOD4(OnStatus, void(const std::string &networkId, int32_t status, const std::string &path, int32_t type));
    MOCK_METHOD0(AsObject, sptr<IRemoteObject>());
};

class ConnectCountTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

public:
    static inline uint32_t testCallingTokenId = 1234;
    static inline std::string testNetworkId = "testNetworkId";
    static inline sptr<IFileDfsListener> testListener = nullptr;

    const int32_t ON_STATUS_OFFLINE = 13900046;
};

void ConnectCountTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "ConnectCountTest SetUpTestCase";
    testListener = new MockFileDfsListener();  // Use Mock class
}

void ConnectCountTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "ConnectCountTest TearDownTestCase";
    testListener = nullptr;
}

void ConnectCountTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void ConnectCountTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    ConnectCount::GetInstance().RemoveAllConnect();
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
    auto &instance1 = ConnectCount::GetInstance();
    auto &instance2 = ConnectCount::GetInstance();
    EXPECT_EQ(&instance1, &instance2);  // Verify singleton instance
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
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(testNetworkId));  // Verify connection is added
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
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    auto networkIds = ConnectCount::GetInstance().GetNetworkIds(testCallingTokenId);
    EXPECT_EQ(networkIds.size(), 1);  // Verify only one networkId is added (count increased)
    GTEST_LOG_(INFO) << "AddConnect_002 end";
}

/**
 * @tc.name: ConnectCountTest_AddConnect_003
 * @tc.desc: Verify AddConnect adds multiple connections with different networkIds.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, AddConnect_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddConnect_003 start";
    std::string networkId1 = "network1";
    std::string networkId2 = "network2";
    
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, networkId1, testListener);
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, networkId2, testListener);
    
    auto networkIds = ConnectCount::GetInstance().GetNetworkIds(testCallingTokenId);
    EXPECT_EQ(networkIds.size(), 2);  // Verify both networkIds are added
    
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(networkId1));
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(networkId2));
    GTEST_LOG_(INFO) << "AddConnect_003 end";
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
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    auto networkIds = ConnectCount::GetInstance().RemoveConnect(testCallingTokenId);
    EXPECT_EQ(networkIds.size(), 1);  // Verify networkId is returned
    EXPECT_FALSE(ConnectCount::GetInstance().CheckCount(testNetworkId));  // Verify connection is removed
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
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    ConnectCount::GetInstance().RemoveConnect(testNetworkId);
    EXPECT_FALSE(ConnectCount::GetInstance().CheckCount(testNetworkId));  // Verify connection is removed
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
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    ConnectCount::GetInstance().RemoveConnect(testCallingTokenId, testNetworkId);
    EXPECT_FALSE(ConnectCount::GetInstance().CheckCount(testNetworkId));  // Verify connection is removed
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
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    ConnectCount::GetInstance().RemoveAllConnect();
    EXPECT_FALSE(ConnectCount::GetInstance().CheckCount(testNetworkId));  // Verify all connections are removed
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
                OnStatus(testNetworkId, ON_STATUS_OFFLINE, "", 0))
                .Times(1);  // 仅在当前测试用例中生效

    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    ConnectCount::GetInstance().NotifyRemoteReverseObj(testNetworkId, ON_STATUS_OFFLINE);

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
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    uint32_t foundCallingTokenId = 0;
    int32_t ret =
        ConnectCount::GetInstance().FindCallingTokenIdForListerner(testListener->AsObject(), foundCallingTokenId);
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
    int32_t ret = ConnectCount::GetInstance().FindCallingTokenIdForListerner(invalidListener, foundCallingTokenId);
    EXPECT_EQ(ret, FileManagement::ERR_BAD_VALUE);  // Verify error is returned
    GTEST_LOG_(INFO) << "FindCallingTokenIdForListerner_002 end";
}

/**
 * @tc.name: ConnectCountTest_DecreaseConnectCount_001
 * @tc.desc: Verify DecreaseConnectCount decrements count for existing connection.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, DecreaseConnectCount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecreaseConnectCount_001 start";
    
    // First add a connection multiple times to increase count
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener); // count = 2
    
    // Decrease the count
    ConnectCount::GetInstance().DecreaseConnectCount(testCallingTokenId, testNetworkId);
    
    // Verify connection still exists (count should be decreased but not removed)
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(testNetworkId));
    
    GTEST_LOG_(INFO) << "DecreaseConnectCount_001 end";
}

/**
 * @tc.name: ConnectCountTest_DecreaseConnectCount_002
 * @tc.desc: Verify DecreaseConnectCount removes connection when count reaches 1.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, DecreaseConnectCount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecreaseConnectCount_002 start";
    
    // Add a connection (count = 1)
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    
    // Verify connection exists
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(testNetworkId));
    
    // Decrease the count (should remove the connection)
    ConnectCount::GetInstance().DecreaseConnectCount(testCallingTokenId, testNetworkId);
    
    // Verify connection is removed
    EXPECT_FALSE(ConnectCount::GetInstance().CheckCount(testNetworkId));
    
    GTEST_LOG_(INFO) << "DecreaseConnectCount_002 end";
}

/**
 * @tc.name: ConnectCountTest_DecreaseConnectCount_003
 * @tc.desc: Verify DecreaseConnectCount handles non-existent connection gracefully.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, DecreaseConnectCount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecreaseConnectCount_003 start";
    
    std::string nonExistentNetworkId = "nonExistent";
    
    // Try to decrease a non-existent connection
    ConnectCount::GetInstance().DecreaseConnectCount(testCallingTokenId, nonExistentNetworkId);
    
    // Verify no crash occurred and non-existent connection doesn't exist
    EXPECT_FALSE(ConnectCount::GetInstance().CheckCount(nonExistentNetworkId));
    
    GTEST_LOG_(INFO) << "DecreaseConnectCount_003 end";
}

/**
 * @tc.name: ConnectCountTest_DecreaseConnectCount_004
 * @tc.desc: Verify DecreaseConnectCount works correctly with multiple decrements.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, DecreaseConnectCount_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecreaseConnectCount_004 start";
    
    // Add connection multiple times to increase count
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener); // count = 3
    
    // Verify connection exists
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(testNetworkId));
    
    // Decrease multiple times
    ConnectCount::GetInstance().DecreaseConnectCount(testCallingTokenId, testNetworkId); // count: 3->2
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(testNetworkId));
    
    ConnectCount::GetInstance().DecreaseConnectCount(testCallingTokenId, testNetworkId); // count: 2->1
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(testNetworkId));
    
    ConnectCount::GetInstance().DecreaseConnectCount(testCallingTokenId, testNetworkId); // count: 1->0 (remove)
    EXPECT_FALSE(ConnectCount::GetInstance().CheckCount(testNetworkId));
    
    GTEST_LOG_(INFO) << "DecreaseConnectCount_004 end";
}

/**
 * @tc.name: ConnectCountTest_DecreaseConnectCount_005
 * @tc.desc: Verify DecreaseConnectCount handles wrong callingTokenId gracefully.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, DecreaseConnectCount_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecreaseConnectCount_005 start";
    
    uint32_t wrongTokenId = 9999;
    
    // Add a connection with testCallingTokenId
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    
    // Try to decrease with wrong tokenId
    ConnectCount::GetInstance().DecreaseConnectCount(wrongTokenId, testNetworkId);
    
    // Verify original connection still exists
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(testNetworkId));
    
    GTEST_LOG_(INFO) << "DecreaseConnectCount_005 end";
}

/**
 * @tc.name: ConnectCountTest_AddAndDecreaseConnectCount_Integration_001
 * @tc.desc: Verify integration of AddConnect and DecreaseConnectCount methods.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, AddAndDecreaseConnectCount_Integration_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddAndDecreaseConnectCount_Integration_001 start";
    
    uint32_t tokenId1 = 1001;
    uint32_t tokenId2 = 1002;
    std::string networkId1 = "net1";
    std::string networkId2 = "net2";
    
    // Add multiple connections
    ConnectCount::GetInstance().AddConnect(tokenId1, networkId1, testListener);
    ConnectCount::GetInstance().AddConnect(tokenId1, networkId2, testListener);
    ConnectCount::GetInstance().AddConnect(tokenId2, networkId1, testListener);
    
    // Add more connections to increase counts
    ConnectCount::GetInstance().AddConnect(tokenId1, networkId1, testListener);
    ConnectCount::GetInstance().AddConnect(tokenId1, networkId1, testListener); // count = 3
    ConnectCount::GetInstance().AddConnect(tokenId2, networkId1, testListener); // count = 2
    
    // Verify all connections exist
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(networkId1));
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(networkId2));
    
    // Decrease counts
    ConnectCount::GetInstance().DecreaseConnectCount(tokenId1, networkId1); // count: 3->2
    ConnectCount::GetInstance().DecreaseConnectCount(tokenId2, networkId1); // count: 2->1
    
    // Verify connections still exist
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(networkId1));
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(networkId2));
    
    // Remove remaining connections
    ConnectCount::GetInstance().DecreaseConnectCount(tokenId1, networkId1); // count: 2->1
    ConnectCount::GetInstance().DecreaseConnectCount(tokenId1, networkId1); // count: 1->0 (remove)
    ConnectCount::GetInstance().DecreaseConnectCount(tokenId2, networkId1); // count: 1->0 (remove)
    ConnectCount::GetInstance().DecreaseConnectCount(tokenId1, networkId2); // count: 1->0 (remove)
    
    // Verify all connections are removed
    EXPECT_FALSE(ConnectCount::GetInstance().CheckCount(networkId1));
    EXPECT_FALSE(ConnectCount::GetInstance().CheckCount(networkId2));
    
    GTEST_LOG_(INFO) << "AddAndDecreaseConnectCount_Integration_001 end";
}

/**
 * @tc.name: ConnectCountTest_FileConnectMethods_001
 * @tc.desc: Verify AddFileConnect and RmFileConnect work correctly.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, FileConnectMethods_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileConnectMethods_001 start";
    
    std::string instanceId = "testInstanceId";
    
    // Add file connect
    ConnectCount::GetInstance().AddFileConnect(instanceId, testListener);
    
    // Remove file connect
    bool result = ConnectCount::GetInstance().RmFileConnect(instanceId);
    EXPECT_TRUE(result);
    
    GTEST_LOG_(INFO) << "FileConnectMethods_001 end";
}

/**
 * @tc.name: ConnectCountTest_GetNetworkIds_001
 * @tc.desc: Verify GetNetworkIds returns correct network IDs.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, GetNetworkIds_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNetworkIds_001 start";
    
    std::string networkId1 = "network1";
    std::string networkId2 = "network2";
    
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, networkId1, testListener);
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, networkId2, testListener);
    
    auto networkIds = ConnectCount::GetInstance().GetNetworkIds(testCallingTokenId);
    EXPECT_EQ(networkIds.size(), 2);
    
    // Verify both network IDs are in the result
    EXPECT_TRUE(std::find(networkIds.begin(), networkIds.end(), networkId1) != networkIds.end());
    EXPECT_TRUE(std::find(networkIds.begin(), networkIds.end(), networkId2) != networkIds.end());
    
    GTEST_LOG_(INFO) << "GetNetworkIds_001 end";
}

/**
 * @tc.name: ConnectCountTest_CheckCount_001
 * @tc.desc: Verify CheckCount returns correct status.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(ConnectCountTest, CheckCount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckCount_001 start";
    
    // Initially should return false
    EXPECT_FALSE(ConnectCount::GetInstance().CheckCount(testNetworkId));
    
    // After adding connection, should return true
    ConnectCount::GetInstance().AddConnect(testCallingTokenId, testNetworkId, testListener);
    EXPECT_TRUE(ConnectCount::GetInstance().CheckCount(testNetworkId));
    
    // After removing connection, should return false
    ConnectCount::GetInstance().RemoveConnect(testCallingTokenId, testNetworkId);
    EXPECT_FALSE(ConnectCount::GetInstance().CheckCount(testNetworkId));
    
    GTEST_LOG_(INFO) << "CheckCount_001 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS