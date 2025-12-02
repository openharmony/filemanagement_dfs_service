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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dfs_radar.h"
#include "os_account_manager.h"
#include "device_manager_impl_mock.h"
#include "mock_other_method.h"

const std::string NETWORKID_ONE = "testNetWork1";
class ISoftBusServer {
public:
    virtual int32_t GetNodeKeyInfo(const char *pkgName, const  char *networkId,
    NodeDeviceInfoKey key, uint8_t *info, int32_t infoLen) = 0;
    static inline std::shared_ptr<ISoftBusServer> dfsSoftBusServer = nullptr;
    virtual ~ISoftBusServer() = default;
};

class SoftBusServerMock : public ISoftBusServer {
public:
    MOCK_METHOD(int32_t, GetNodeKeyInfo, (const char *, const char *, NodeDeviceInfoKey, uint8_t *, int32_t));
};

namespace OHOS::Storage::DistributedFile {
int32_t GetNodeKeyInfo(const char *pkgName, const char *networkId, NodeDeviceInfoKey key,
                       uint8_t *info, int32_t infoLen)
{
    GTEST_LOG_(INFO) << "GetNodeKeyInfo start";
    return ISoftBusServer::dfsSoftBusServer->GetNodeKeyInfo(pkgName, networkId, key, info, infoLen);
}
} // namespace OHOS::Storage::DistributedFile

namespace OHOS::FileManagement::Test {
using namespace std;
using namespace OHOS;
using namespace OHOS::FileManagement;
using namespace OHOS::Storage::DistributedFile;
using namespace testing::ext;
using namespace testing;

constexpr const char *DEFAULT_PKGNAME = "dfsService";
const int32_t DEFAULT_SIZE = -1;
const int32_t INVALID_USER_ID = -1;

class DfsRadarTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<Storage::DistributedFile::DeviceManagerImplMock> deviceManagerImplMock_ = nullptr;
    static inline std::shared_ptr<SoftBusServerMock> softBusServerMock_ = nullptr;
    static inline std::shared_ptr<DfsDeviceOtherMethodMock> otherMethodMock_ = nullptr;
};

void DfsRadarTest::SetUpTestCase()
{
    deviceManagerImplMock_ = std::make_shared<Storage::DistributedFile::DeviceManagerImplMock>();
    Storage::DistributedFile::DfsDeviceManagerImpl::dfsDeviceManagerImpl = deviceManagerImplMock_;
    softBusServerMock_ = std::make_shared<SoftBusServerMock>();
    ISoftBusServer::dfsSoftBusServer = softBusServerMock_;
    otherMethodMock_ = make_shared<DfsDeviceOtherMethodMock>();
    DfsDeviceOtherMethodMock::otherMethod = otherMethodMock_;
}

void DfsRadarTest::TearDownTestCase()
{
    Storage::DistributedFile::DfsDeviceManagerImpl::dfsDeviceManagerImpl = nullptr;
    deviceManagerImplMock_ = nullptr;
    ISoftBusServer::dfsSoftBusServer = nullptr;
    softBusServerMock_ = nullptr;
    DfsDeviceOtherMethodMock::otherMethod = nullptr;
    otherMethodMock_ = nullptr;
}

void DfsRadarTest::SetUp()
{
}

void DfsRadarTest::TearDown()
{
}

/**
 * @tc.name: DfsRadarTest_ReportStatistics_001
 * @tc.desc: verify ReportStatistics.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DfsRadarTest, DfsRadarTest_ReportStatistics_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsRadarTest_ReportStatistics_001 Start";
    RadarStatisticInfo radarInfo = {0};
    EXPECT_NO_FATAL_FAILURE(DfsRadar::GetInstance().ReportStatistics(radarInfo));
    GTEST_LOG_(INFO) << "DfsRadarTest_ReportStatistics_001 End";
}

/**
 * @tc.name: DfsRadarTest_RecordFunctionResult_001
 * @tc.desc: verify RecordFunctionResult.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DfsRadarTest, DfsRadarTest_RecordFunctionResult_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsRadarTest_RecordFunctionResult_001 Start";
    RadarParameter param = {
        .orgPkg = DEFAULT_PKGNAME,
        .hostPkg = "hostPkg",
        .funcName = "test",
        .faultLevel = ReportLevel::DEFAULT,
        .bizScene = BizScene::GENERATE_DIS_URI,
        .bizStage = DfxBizStage::DEFAULT,
        .toCallPkg = "toCallPkg",
        .fileSize = DEFAULT_SIZE,
        .fileCount = DEFAULT_SIZE,
        .operateTime = DEFAULT_SIZE,
        .resultCode = DEFAULT_ERR,
        .errorInfo = "errorInfo"
    };
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillRepeatedly(Return(INVALID_USER_ID));
    bool res = DfsRadar::GetInstance().RecordFunctionResult(param);
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DfsRadarTest_RecordFunctionResult_001 End";
}

/**
 * @tc.name: DfsRadarTest_RecordFunctionResult_002
 * @tc.desc: verify RecordFunctionResult.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DfsRadarTest, DfsRadarTest_RecordFunctionResult_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsRadarTest_RecordFunctionResult_002 Start";
    RadarParameter param = {
        .orgPkg = DEFAULT_PKGNAME,
        .hostPkg = "hostPkg",
        .funcName = "test",
        .faultLevel = ReportLevel::DEFAULT,
        .bizScene = BizScene::GENERATE_DIS_URI,
        .bizStage = DfxBizStage::DEFAULT,
        .toCallPkg = "toCallPkg",
        .fileSize = DEFAULT_SIZE,
        .fileCount = DEFAULT_SIZE,
        .operateTime = DEFAULT_SIZE,
        .resultCode = 0,
        .errorInfo = "errorInfo"
    };
    bool res = DfsRadar::GetInstance().RecordFunctionResult(param);
    EXPECT_TRUE(res);
    GTEST_LOG_(INFO) << "DfsRadarTest_RecordFunctionResult_002 End";
}

/**
 * @tc.name: DfsRadarTest_GetCurrentUserId_001
 * @tc.desc: verify GetCurrentUserId.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DfsRadarTest, DfsRadarTest_GetCurrentUserId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsRadarTest_GetCurrentUserId_001 Start";
    std::vector<int32_t> userIds;
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(INVALID_USER_ID)));
    int32_t res = DfsRadar::GetInstance().GetCurrentUserId();
    EXPECT_EQ(res, INVALID_USER_ID);

    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(NO_ERROR)));
    res = DfsRadar::GetInstance().GetCurrentUserId();
    EXPECT_EQ(res, INVALID_USER_ID);

    userIds.push_back(100);
    EXPECT_CALL(*otherMethodMock_, QueryActiveOsAccountIds(_))
        .WillOnce(DoAll(SetArgReferee<0>(userIds), Return(NO_ERROR)));
    res = DfsRadar::GetInstance().GetCurrentUserId();
    EXPECT_EQ(res, userIds[0]);
    GTEST_LOG_(INFO) << "DfsRadarTest_GetCurrentUserId_001 End";
}

/**
 * @tc.name: DfsRadarTest_GetLocalNetIdAndUdid_001
 * @tc.desc: verify GetLocalNetIdAndUdid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DfsRadarTest, DfsRadarTest_GetLocalNetIdAndUdid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsRadarTest_GetLocalNetIdAndUdid_001 Start";
    RadarParameter param = {
        .orgPkg = DEFAULT_PKGNAME,
        .hostPkg = "hostPkg",
        .funcName = "test",
        .faultLevel = ReportLevel::DEFAULT,
        .bizScene = BizScene::GENERATE_DIS_URI,
        .bizStage = DfxBizStage::DEFAULT,
        .toCallPkg = "toCallPkg",
        .fileSize = DEFAULT_SIZE,
        .fileCount = DEFAULT_SIZE,
        .operateTime = DEFAULT_SIZE,
        .peerUdid = "peerUdid",
        .peerNetId = "peerNetId",
        .resultCode = 0,
        .errorInfo = "errorInfo"
    };
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(Return(-1));
    EXPECT_NO_FATAL_FAILURE(DfsRadar::GetInstance().GetLocalNetIdAndUdid(param));

    EXPECT_CALL(*deviceManagerImplMock_,
                GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(NETWORKID_ONE), Return(0)));
    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(0));
    EXPECT_NO_FATAL_FAILURE(DfsRadar::GetInstance().GetLocalNetIdAndUdid(param));

    EXPECT_CALL(*deviceManagerImplMock_,
                GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(NETWORKID_ONE), Return(0)));
    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(0));
    EXPECT_NO_FATAL_FAILURE(DfsRadar::GetInstance().GetLocalNetIdAndUdid(param));
    GTEST_LOG_(INFO) << "DfsRadarTest_GetLocalNetIdAndUdid_001 End";
}

/**
 * @tc.name: DfsRadarTest_GetPeerUdid_001
 * @tc.desc: verify GetPeerUdid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DfsRadarTest, DfsRadarTest_GetPeerUdid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsRadarTest_GetPeerUdid_001 Start";
    RadarParameter param = {
        .orgPkg = DEFAULT_PKGNAME,
        .hostPkg = "hostPkg",
        .funcName = "test",
        .faultLevel = ReportLevel::DEFAULT,
        .bizScene = BizScene::GENERATE_DIS_URI,
        .bizStage = DfxBizStage::DEFAULT,
        .toCallPkg = "toCallPkg",
        .fileSize = DEFAULT_SIZE,
        .fileCount = DEFAULT_SIZE,
        .operateTime = DEFAULT_SIZE,
        .resultCode = 0,
        .errorInfo = "errorInfo"
    };
    EXPECT_NO_FATAL_FAILURE(DfsRadar::GetInstance().GetPeerUdid(param, ""));

    std::string networkId = "networkId";
    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(0));
    EXPECT_NO_FATAL_FAILURE(DfsRadar::GetInstance().GetPeerUdid(param, networkId));

    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(-1));
    EXPECT_NO_FATAL_FAILURE(DfsRadar::GetInstance().GetPeerUdid(param, networkId));
    GTEST_LOG_(INFO) << "DfsRadarTest_GetPeerUdid_001 End";
}

/**
 * @tc.name: DfsRadarTest_GetAnonymStr_001
 * @tc.desc: verify GetAnonymStr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DfsRadarTest, DfsRadarTest_GetAnonymStr_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsRadarTest_GetAnonymStr_001 Start";
    string value = "";
    string res = DfsRadar::GetInstance().GetAnonymStr(value);
    EXPECT_EQ(res, "");

    value = "123";
    res = DfsRadar::GetInstance().GetAnonymStr(value);
    EXPECT_EQ(res, "");

    value = "12345678901";
    res = DfsRadar::GetInstance().GetAnonymStr(value);
    EXPECT_EQ(res, "12345**78901");
    GTEST_LOG_(INFO) << "DfsRadarTest_GetAnonymStr_001 End";
}
}
