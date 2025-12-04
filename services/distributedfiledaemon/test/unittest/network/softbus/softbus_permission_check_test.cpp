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

#include "gtest/gtest.h"
#include <memory>
#include <unistd.h>

#include "device_manager_impl_mock.h"
#include "dfs_error.h"
#include "mock_other_method.h"
#include "network/softbus/softbus_permission_check.h"
#include "socket_mock.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

#ifdef SUPPORT_SAME_ACCOUNT
const std::string NETWORKID_ONE = "testNetWork1";
const std::string NETWORKID_TWO = "testNetWork2";
#endif

class SoftbusPermissionCheckTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() {};
    void TearDown() {};
public:
    static inline shared_ptr<DfsDeviceOtherMethodMock> otherMethodMock_ = nullptr;
    static inline shared_ptr<SocketMock> socketMock_ = nullptr;
    static inline shared_ptr<DeviceManagerImplMock> deviceManagerImplMock_ = nullptr;
};

void SoftbusPermissionCheckTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    otherMethodMock_ = make_shared<DfsDeviceOtherMethodMock>();
    DfsDeviceOtherMethodMock::otherMethod = otherMethodMock_;
    deviceManagerImplMock_ = make_shared<DeviceManagerImplMock>();
    DeviceManagerImplMock::dfsDeviceManagerImpl = deviceManagerImplMock_;
    socketMock_ = make_shared<SocketMock>();
    SocketMock::dfsSocket = socketMock_;
}

void SoftbusPermissionCheckTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    SocketMock::dfsSocket = nullptr;
    socketMock_ = nullptr;
    DeviceManagerImplMock::dfsDeviceManagerImpl = nullptr;
    deviceManagerImplMock_ = nullptr;
    DfsDeviceOtherMethodMock::otherMethod = nullptr;
    otherMethodMock_ = nullptr;
}

/**
 * @tc.name: SoftbusPermissionCheckTest_GetLocalNetworkId_001
 * @tc.desc: Verify the GetLocalNetworkId function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_GetLocalNetworkId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_GetLocalNetworkId_001 start";
    std::string networkId;
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    bool res = SoftBusPermissionCheck::GetLocalNetworkId(networkId);
    EXPECT_TRUE(res == true);

    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(INVALID_USER_ID));
    res = SoftBusPermissionCheck::GetLocalNetworkId(networkId);
    EXPECT_TRUE(res == false);
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_GetLocalNetworkId_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_GetCurrentUserId_001
 * @tc.desc: Verify the GetCurrentUserId function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_GetCurrentUserId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_GetCurrentUserId_001 start";
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_)).WillRepeatedly(Return(INVALID_USER_ID));
    auto userId = SoftBusPermissionCheck::GetCurrentUserId();
    EXPECT_EQ(userId, INVALID_USER_ID);

    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_)).WillRepeatedly(Return(FileManagement::E_OK));
    userId = SoftBusPermissionCheck::GetCurrentUserId();
    EXPECT_EQ(userId, INVALID_USER_ID);

    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    userId = SoftBusPermissionCheck::GetCurrentUserId();
    EXPECT_EQ(userId, userIds[0]);
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_GetCurrentUserId_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_FillLocalInfo_001
 * @tc.desc: Verify the FillLocalInfo function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_FillLocalInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_FillLocalInfo_001 start";
    SocketAccessInfo *localInfoPtr = nullptr;
    bool res = SoftBusPermissionCheck::FillLocalInfo(localInfoPtr);
    EXPECT_TRUE(res == false);

    SocketAccessInfo localInfo;
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_)).WillRepeatedly(Return(INVALID_USER_ID));
    res = SoftBusPermissionCheck::FillLocalInfo(&localInfo);
    EXPECT_TRUE(res == false);

    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    res = SoftBusPermissionCheck::FillLocalInfo(&localInfo);
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_FillLocalInfo_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_TransCallerInfo_001
 * @tc.desc: Verify the TransCallerInfo function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_TransCallerInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_TransCallerInfo_001 start";
    SocketAccessInfo *localInfoPtr = nullptr;
    AccountInfo callerAccountInfo;
    std::string networkId;
    bool res = SoftBusPermissionCheck::TransCallerInfo(localInfoPtr, callerAccountInfo, networkId);
    EXPECT_TRUE(res == true);

    SocketAccessInfo localInfo;
    localInfo.extraAccessInfo = nullptr;
    res = SoftBusPermissionCheck::TransCallerInfo(&localInfo, callerAccountInfo, networkId);
    EXPECT_TRUE(res == true);

    localInfo.extraAccessInfo = const_cast<char*>("test");
    res = SoftBusPermissionCheck::TransCallerInfo(&localInfo, callerAccountInfo, networkId);
    EXPECT_TRUE(res == false);

    localInfo.extraAccessInfo = const_cast<char*>("{\"accountId\":1}");
    res = SoftBusPermissionCheck::TransCallerInfo(&localInfo, callerAccountInfo, networkId);
    EXPECT_TRUE(res == false);

    localInfo.extraAccessInfo = const_cast<char*>("{\"accountId\":\"test\"}");
    res = SoftBusPermissionCheck::TransCallerInfo(&localInfo, callerAccountInfo, networkId);
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_TransCallerInfo_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_GetLocalAccountInfo_001
 * @tc.desc: Verify the GetLocalAccountInfo function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_GetLocalAccountInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_GetLocalAccountInfo_001 start";
    AccountInfo localAccountInfo;
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_)).WillRepeatedly(Return(INVALID_USER_ID));
    bool res = SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo);
    EXPECT_TRUE(res == false);

    std::vector<int32_t> userIds{100, 101};
#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    AccountSA::OhosAccountInfo osAccountInfo;
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(INVALID_USER_ID)));
    res = SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo);
    EXPECT_EQ(res, false);

    osAccountInfo.uid_ = ""; // accountinfo uid = ""
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    res = SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo);
    EXPECT_EQ(res, false);
#endif

    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))// GetLocalDeviceInfo = false
        .WillRepeatedly(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
#ifdef SUPPORT_SAME_ACCOUNT
    osAccountInfo.uid_ = "test";
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
#endif
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(INVALID_USER_ID));
    res = SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo);
    EXPECT_EQ(res, false);

    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
#endif
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    res = SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo);
    EXPECT_EQ(res, true);
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_GetLocalAccountInfo_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_GetLocalAccountInfo_002
 * @tc.desc: Verify the GetLocalAccountInfo function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_GetLocalAccountInfo_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_GetLocalAccountInfo_002 start";
    AccountInfo localAccountInfo;
    int32_t testUserId = 100; // 100: test userId
    bool res = false;
#ifdef SUPPORT_SAME_ACCOUNT
    AccountSA::OhosAccountInfo osAccountInfo;
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(INVALID_USER_ID)));
    res = SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo, testUserId);
    EXPECT_EQ(res, false);

    osAccountInfo.uid_ = ""; // accountinfo uid = ""
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    res = SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo, testUserId);
    EXPECT_EQ(res, false);

    osAccountInfo.uid_ = "test";
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(INVALID_USER_ID));
    res = SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo, testUserId);
    EXPECT_EQ(res, false);

    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(FileManagement::E_OK));
    res = SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo, testUserId);
    EXPECT_EQ(res, true);
#else
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(INVALID_USER_ID));
    res = SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo, testUserId);
    EXPECT_EQ(res, false);

    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(FileManagement::E_OK));
    res = SoftBusPermissionCheck::GetLocalAccountInfo(localAccountInfo, testUserId);
    EXPECT_EQ(res, true);
#endif
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_GetLocalAccountInfo_002 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_SetAccessInfoToSocket_001
 * @tc.desc: Verify the SetAccessInfoToSocket function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_SetAccessInfoToSocket_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_SetAccessInfoToSocket_001 start";
    int32_t socketId = 0;
    bool res = false;
#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_)).WillRepeatedly(Return(INVALID_USER_ID));
    SoftBusPermissionCheck::SetAccessInfoToSocket(socketId);
    EXPECT_TRUE(res == false);

    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));

    AccountSA::OhosAccountInfo osAccountInfo;
    osAccountInfo.uid_ = "test";
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*socketMock_, SetAccessInfo(_, _)).WillOnce(Return(INVALID_USER_ID));
    res = SoftBusPermissionCheck::SetAccessInfoToSocket(socketId);
    EXPECT_TRUE(res == false);

    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*socketMock_, SetAccessInfo(_, _)).WillOnce(Return(FileManagement::E_OK));
    res = SoftBusPermissionCheck::SetAccessInfoToSocket(socketId);
#else
    std::vector<int32_t> userIds{100, 101};
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*socketMock_, SetAccessInfo(_, _)).WillOnce(Return(FileManagement::E_OK));
    res = SoftBusPermissionCheck::SetAccessInfoToSocket(socketId);
#endif
    EXPECT_TRUE(res == true);
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_SetAccessInfoToSocket_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_CheckSrcIsSameAccount_001
 * @tc.desc: Verify the CheckSrcIsSameAccount function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_CheckSrcIsSameAccount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSrcIsSameAccount_001 start";
    std::string networkId;
    AccountInfo callerAccountInfo;
#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_CALL(*deviceManagerImplMock_, CheckSrcIsSameAccount(_, _)).WillOnce(Return(true));
    bool res = SoftBusPermissionCheck::CheckSrcIsSameAccount(networkId, callerAccountInfo);
    EXPECT_TRUE(res == true);

    EXPECT_CALL(*deviceManagerImplMock_, CheckSrcIsSameAccount(_, _)).WillOnce(Return(false));
    res = SoftBusPermissionCheck::CheckSrcIsSameAccount(networkId, callerAccountInfo);
    EXPECT_TRUE(res == false);
#else
    bool res = SoftBusPermissionCheck::CheckSrcIsSameAccount(networkId, callerAccountInfo);
    EXPECT_TRUE(res == true);
#endif
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSrcIsSameAccount_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_CheckSinkIsSameAccount_001
 * @tc.desc: Verify the CheckSinkIsSameAccount function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_CheckSinkIsSameAccount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSinkIsSameAccount_001 start";
    const AccountInfo callerAccountInfo;
    const AccountInfo calleeAccountInfo;
#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_CALL(*deviceManagerImplMock_, CheckSinkIsSameAccount(_, _)).WillOnce(Return(true));
    bool res = SoftBusPermissionCheck::CheckSinkIsSameAccount(callerAccountInfo, calleeAccountInfo);
    EXPECT_TRUE(res == true);

    EXPECT_CALL(*deviceManagerImplMock_, CheckSinkIsSameAccount(_, _)).WillOnce(Return(false));
    res = SoftBusPermissionCheck::CheckSinkIsSameAccount(callerAccountInfo, calleeAccountInfo);
    EXPECT_TRUE(res == false);
#else
    bool res = SoftBusPermissionCheck::CheckSinkIsSameAccount(callerAccountInfo, calleeAccountInfo);
    EXPECT_TRUE(res == true);
#endif
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSinkIsSameAccount_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_CheckSrcAccessControl_001
 * @tc.desc: Verify the CheckSrcAccessControl function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_CheckSrcAccessControl_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSrcAccessControl_001 start";
    std::string networkId;
    AccountInfo localAccountInfo;
    EXPECT_CALL(*deviceManagerImplMock_, CheckSrcAccessControl(_, _)).WillOnce(Return(true));
    bool res = SoftBusPermissionCheck::CheckSrcAccessControl(networkId, localAccountInfo);
    EXPECT_TRUE(res == true);

    EXPECT_CALL(*deviceManagerImplMock_, CheckSrcAccessControl(_, _)).WillOnce(Return(false));
    res = SoftBusPermissionCheck::CheckSrcAccessControl(networkId, localAccountInfo);
    EXPECT_TRUE(res == false);
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSrcAccessControl_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_CheckSinkAccessControl_001
 * @tc.desc: Verify the CheckSinkAccessControl function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_CheckSinkAccessControl_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSinkAccessControl_001 start";
    const AccountInfo callerAccountInfo;
    const AccountInfo calleeAccountInfo;
    EXPECT_CALL(*deviceManagerImplMock_, CheckSinkAccessControl(_, _)).WillOnce(Return(true));
    bool res = SoftBusPermissionCheck::CheckSinkAccessControl(callerAccountInfo, calleeAccountInfo);
    EXPECT_TRUE(res == true);

    EXPECT_CALL(*deviceManagerImplMock_, CheckSinkAccessControl(_, _)).WillOnce(Return(false));
    res = SoftBusPermissionCheck::CheckSinkAccessControl(callerAccountInfo, calleeAccountInfo);
    EXPECT_TRUE(res == false);
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSinkAccessControl_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_CheckSrcPermission_001
 * @tc.desc: Verify the CheckSrcPermission function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_CheckSrcPermission_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSrcPermission_001 start";
    std::string networkId; // GetLocalAccount failed
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_)).WillRepeatedly(Return(INVALID_USER_ID));
    bool res = SoftBusPermissionCheck::CheckSrcPermission(networkId);
    EXPECT_TRUE(res == false);

    std::vector<int32_t> userIds{100, 101};
#ifdef SUPPORT_SAME_ACCOUNT
    AccountSA::OhosAccountInfo osAccountInfo;
    osAccountInfo.uid_ = "test";
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, CheckSrcIsSameAccount(_, _)).WillOnce(Return(true));
    res = SoftBusPermissionCheck::CheckSrcPermission(networkId);
    EXPECT_TRUE(res == true);

    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, CheckSrcIsSameAccount(_, _)).WillOnce(Return(false));
    res = SoftBusPermissionCheck::CheckSrcPermission(networkId);
    EXPECT_TRUE(res == false);
#else
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    res = SoftBusPermissionCheck::CheckSrcPermission(networkId);
    EXPECT_TRUE(res == true);
#endif
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSrcPermission_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_CheckSinkPermission_001
 * @tc.desc: Verify the CheckSinkPermission function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_CheckSinkPermission_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSinkPermission_001 start";
    AccountInfo callerAccountInfo;
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_)).WillRepeatedly(Return(INVALID_USER_ID));
    bool res = SoftBusPermissionCheck::CheckSinkPermission(callerAccountInfo);
    EXPECT_TRUE(res == false);

    std::vector<int32_t> userIds{100, 101};
#ifdef SUPPORT_SAME_ACCOUNT
    AccountSA::OhosAccountInfo osAccountInfo;
    osAccountInfo.uid_ = "test";
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, CheckSinkIsSameAccount(_, _)).WillOnce(Return(true));
    res = SoftBusPermissionCheck::CheckSinkPermission(callerAccountInfo);
    EXPECT_TRUE(res == true);

    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*otherMethodMock_, GetOhosAccountInfo(_))
        .WillOnce(DoAll(SetArgReferee<0>(osAccountInfo), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    EXPECT_CALL(*deviceManagerImplMock_, CheckSinkIsSameAccount(_, _)).WillOnce(Return(false));
    res = SoftBusPermissionCheck::CheckSinkPermission(callerAccountInfo);
    EXPECT_TRUE(res == false);
#else
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(FileManagement::E_OK)));
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceInfo(_, _)).WillOnce(Return(0));
    res = SoftBusPermissionCheck::CheckSinkPermission(callerAccountInfo);
    EXPECT_TRUE(res == true);
#endif
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_CheckSinkPermission_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_IsSameAccount_001
 * @tc.desc: Verify the CheckSinkPermission function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_IsSameAccount_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_IsSameAccount_001 start";
#ifdef SUPPORT_SAME_ACCOUNT
    DistributedHardware::DmDeviceInfo deviceInfo = {
        .deviceId = "test",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
        .networkId = "testNetWork1",
        .authForm = DmAuthForm::ACROSS_ACCOUNT,
    };
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _)).WillOnce(Return(0));
    bool res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_TWO);
    EXPECT_EQ(res, false);
    
    std::vector<DmDeviceInfo> deviceList;
    deviceList.resize(10001);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_TWO);
    EXPECT_EQ(res, false);

    deviceList.clear();
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_TWO);
    EXPECT_EQ(res, false);

    deviceList.clear();
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_ONE);
    EXPECT_EQ(res, false);

    deviceList.clear();
    deviceInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_ONE);
    EXPECT_EQ(res, false);
#endif
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_IsSameAccount_001 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_IsSameAccount_002
 * @tc.desc: Verify the CheckSinkPermission function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_IsSameAccount_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_IsSameAccount_002 start";
#ifdef SUPPORT_SAME_ACCOUNT
    DistributedHardware::DmDeviceInfo deviceInfo = {
        .deviceId = "test",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
        .networkId = "testNetWork1",
        .authForm = DmAuthForm::ACROSS_ACCOUNT,
    };
    deviceInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
    deviceInfo.extraData.clear();
    std::vector<DmDeviceInfo> deviceList;
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    auto res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_ONE);
    EXPECT_EQ(res, false);

    deviceList.clear();
    deviceInfo.extraData = "{\"OS_TYPE\":\"aa\"}";
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_ONE);
    EXPECT_EQ(res, false);

    deviceList.clear();
    deviceInfo.extraData = "{\"DEVICE_TYPE\":11}";
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_ONE);
    EXPECT_EQ(res, false);

    deviceList.clear();
    deviceInfo.extraData = "is_discarded";
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_ONE);
    EXPECT_EQ(res, false);
#endif
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_IsSameAccount_002 end";
}

/**
 * @tc.name: SoftbusPermissionCheckTest_IsSameAccount_003
 * @tc.desc: Verify the CheckSinkPermission function.
 * @tc.type: FUNC
 * @tc.require: IC987N
 */
HWTEST_F(SoftbusPermissionCheckTest, SoftbusPermissionCheckTest_IsSameAccount_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_IsSameAccount_003 start";
#ifdef SUPPORT_SAME_ACCOUNT
    DistributedHardware::DmDeviceInfo deviceInfo = {
        .deviceId = "test",
        .deviceName = "testdevname",
        .deviceTypeId = 1,
        .networkId = "testNetWork1",
        .authForm = DmAuthForm::ACROSS_ACCOUNT,
    };
    std::vector<DmDeviceInfo> deviceList;
    deviceInfo.authForm = DmAuthForm::IDENTICAL_ACCOUNT;
    deviceInfo.extraData = "{\"OS_TYPE\":11}";
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    auto res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_ONE);
    EXPECT_EQ(res, false);

    deviceList.clear();
    deviceInfo.extraData = "{\"OS_TYPE\":11111111111111}";
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_ONE);
    EXPECT_EQ(res, false);

    deviceList.clear();
    deviceInfo.extraData = "{\"OS_TYPE\":10}";
    deviceList.push_back(deviceInfo);
    EXPECT_CALL(*deviceManagerImplMock_, GetTrustedDeviceList(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(deviceList), Return(0)));
    res = SoftBusPermissionCheck::IsSameAccount(NETWORKID_ONE);
    EXPECT_EQ(res, true);
#endif
    GTEST_LOG_(INFO) << "SoftbusPermissionCheckTest_IsSameAccount_003 end";
}
} // namespace Test
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
