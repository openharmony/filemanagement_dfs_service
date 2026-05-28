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

#include "device_manager_impl_mock.h"
#include "mock_other_method.h"
#include "radar_report.h"

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

namespace OHOS::FileManagement {
class DfsRadarMock {
public:
    MOCK_METHOD(void, ReportLinkConnection, (const RadarParaInfo&, const RadarIDInfo&));
    MOCK_METHOD(void, ReportGenerateDisUri, (const RadarParaInfo&, const RadarIDInfo&));
    MOCK_METHOD(void, ReportFileAccess, (const RadarParaInfo&, const RadarIDInfo&));
    MOCK_METHOD(void, ReportStatistics, (const RadarStatisticInfo&));
    static inline std::shared_ptr<DfsRadarMock> dfsRadarMock = nullptr;
};

void DfsRadar::ReportLinkConnection(const RadarParaInfo &info, const RadarIDInfo &radarIdInfo)
{
    GTEST_LOG_(INFO) << "DfsRadar::ReportLinkConnection start";
    if (DfsRadarMock::dfsRadarMock != nullptr) {
        DfsRadarMock::dfsRadarMock->ReportLinkConnection(info, radarIdInfo);
    }
}

void DfsRadar::ReportGenerateDisUri(const RadarParaInfo &info, const RadarIDInfo &radarIdInfo)
{
    GTEST_LOG_(INFO) << "DfsRadar::ReportGenerateDisUri start";
    if (DfsRadarMock::dfsRadarMock != nullptr) {
        DfsRadarMock::dfsRadarMock->ReportGenerateDisUri(info, radarIdInfo);
    }
}

void DfsRadar::ReportFileAccess(const RadarParaInfo &info, const RadarIDInfo &radarIdInfo)
{
    GTEST_LOG_(INFO) << "DfsRadar::ReportFileAccess start";
    if (DfsRadarMock::dfsRadarMock != nullptr) {
        DfsRadarMock::dfsRadarMock->ReportFileAccess(info, radarIdInfo);
    }
}

void DfsRadar::ReportStatistics(const RadarStatisticInfo radarInfo)
{
    GTEST_LOG_(INFO) << "DfsRadar::ReportStatistics start";
    if (DfsRadarMock::dfsRadarMock != nullptr) {
        DfsRadarMock::dfsRadarMock->ReportStatistics(radarInfo);
    }
}
} // namespace OHOS::FileManagement

namespace OHOS::Storage::DistributedFile {
using namespace std;
using namespace OHOS;
using namespace OHOS::FileManagement;
using namespace OHOS::Storage::DistributedFile;
using namespace testing::ext;
using namespace testing;

class RadarReportAdapterTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<Storage::DistributedFile::DeviceManagerImplMock> deviceManagerImplMock_ = nullptr;
    static inline std::shared_ptr<SoftBusServerMock> softBusServerMock_ = nullptr;
    static inline std::shared_ptr<DfsRadarMock> dfsRadarMock_ = nullptr;
};

void RadarReportAdapterTest::SetUpTestCase()
{
    deviceManagerImplMock_ = std::make_shared<Storage::DistributedFile::DeviceManagerImplMock>();
    Storage::DistributedFile::DfsDeviceManagerImpl::dfsDeviceManagerImpl = deviceManagerImplMock_;
    softBusServerMock_ = std::make_shared<SoftBusServerMock>();
    ISoftBusServer::dfsSoftBusServer = softBusServerMock_;
}

void RadarReportAdapterTest::TearDownTestCase()
{
    deviceManagerImplMock_ = nullptr;
    Storage::DistributedFile::DfsDeviceManagerImpl::dfsDeviceManagerImpl = nullptr;
    softBusServerMock_ = nullptr;
    ISoftBusServer::dfsSoftBusServer = nullptr;
}

void RadarReportAdapterTest::SetUp() {}

void RadarReportAdapterTest::TearDown() {}

/**
 * @tc.name: RadarReportAdapterTest_StorageRadarThd_001
 * @tc.desc: verify StorageRadarThd.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_StorageRadarThd_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_StorageRadarThd_001 begin";
    RadarReportAdapter::GetInstance().stopRadarReport_ = true;
    RadarReportAdapter::GetInstance().InitRadar();
    sleep(1);
    RadarReportAdapter::GetInstance().UnInitRadar();
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_StorageRadarThd_001 end";
}

/**
 * @tc.name: RadarReportAdapterTest_SetUserStatistics_001
 * @tc.desc: verify SetUserStatistics.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_SetUserStatistics_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_SetUserStatistics_001 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    EXPECT_NO_FATAL_FAILURE(
        RadarReportAdapter::GetInstance().SetUserStatistics(RadarStatisticInfoType::CONNECT_DFS_SUCC_CNT));
    EXPECT_NO_FATAL_FAILURE(
        RadarReportAdapter::GetInstance().SetUserStatistics(RadarStatisticInfoType::CONNECT_DFS_FAIL_CNT));
    EXPECT_NO_FATAL_FAILURE(
        RadarReportAdapter::GetInstance().SetUserStatistics(RadarStatisticInfoType::GENERATE_DIS_URI_SUCC_CNT));
    EXPECT_NO_FATAL_FAILURE(
        RadarReportAdapter::GetInstance().SetUserStatistics(RadarStatisticInfoType::GENERATE_DIS_URI_FAIL_CNT));
    EXPECT_NO_FATAL_FAILURE(
        RadarReportAdapter::GetInstance().SetUserStatistics(RadarStatisticInfoType::FILE_ACCESS_SUCC_CNT));
    EXPECT_NO_FATAL_FAILURE(
        RadarReportAdapter::GetInstance().SetUserStatistics(RadarStatisticInfoType::FILE_ACCESS_FAIL_CNT));
    EXPECT_NO_FATAL_FAILURE(
        RadarReportAdapter::GetInstance().SetUserStatistics(static_cast<RadarStatisticInfoType>(6)));
#endif
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_SetUserStatistics_001 end";
}

/**
 * @tc.name: RadarReportAdapterTest_ReportDfxStatistics_001
 * @tc.desc: verify ReportDfxStatistics.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_ReportDfxStatistics_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportDfxStatistics_001 begin";
    RadarReportAdapter::GetInstance().opStatistics_ = {0, 0, 0, 0, 0, 0};
    RadarReportAdapter::GetInstance().ReportDfxStatistics();
    EXPECT_TRUE(RadarReportAdapter::GetInstance().opStatistics_.empty());
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportDfxStatistics_001 end";
}

/**
 * @tc.name: DfsRadarTest_GetLocalNetIdAndUdid_001
 * @tc.desc: verify GetLocalNetIdAndUdid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_GetLocalNetIdAndUdid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsRadarTest_GetLocalNetIdAndUdid_001 Start";
#ifdef SUPPORT_SAME_ACCOUNT
    std::string localDeviceNetId;
    std::string localDeviceUdid;
    EXPECT_CALL(*deviceManagerImplMock_, GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(Return(-1));
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().GetLocalNetIdAndUdid(localDeviceNetId, localDeviceUdid));

    EXPECT_CALL(*deviceManagerImplMock_,
                GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(NETWORKID_ONE), Return(0)));
    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(0));
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().GetLocalNetIdAndUdid(localDeviceNetId, localDeviceUdid));

    EXPECT_CALL(*deviceManagerImplMock_,
                GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(NETWORKID_ONE), Return(0)));
    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(-1));
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().GetLocalNetIdAndUdid(localDeviceNetId, localDeviceUdid));
#endif
    GTEST_LOG_(INFO) << "DfsRadarTest_GetLocalNetIdAndUdid_001 End";
}

/**
 * @tc.name: DfsRadarTest_GetPeerUdid_001
 * @tc.desc: verify GetPeerUdid.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_GetPeerUdid_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsRadarTest_GetPeerUdid_001 Start";
#ifdef SUPPORT_SAME_ACCOUNT
    std::string networkId = "networkId";
    std::string peerDeviceNetId;
    std::string peerDeviceUdid;
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().GetPeerUdid("", peerDeviceNetId, peerDeviceUdid));

    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(0));
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().GetPeerUdid(networkId, peerDeviceNetId, peerDeviceUdid));

    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(-1));
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().GetPeerUdid(networkId, peerDeviceNetId, peerDeviceUdid));
#endif
    GTEST_LOG_(INFO) << "DfsRadarTest_GetPeerUdid_001 End";
}

/**
 * @tc.name: DfsRadarTest_GetAnonymStr_001
 * @tc.desc: verify GetAnonymStr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_GetAnonymStr_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsRadarTest_GetAnonymStr_001 Start";
    string value = "";
    string res = RadarReportAdapter::GetInstance().GetAnonymStr(value);
    EXPECT_EQ(res, "");

    value = "123";
    res = RadarReportAdapter::GetInstance().GetAnonymStr(value);
    EXPECT_EQ(res, "");

    value = "12345678901";
    res = RadarReportAdapter::GetInstance().GetAnonymStr(value);
    EXPECT_EQ(res, "12345**78901");
    GTEST_LOG_(INFO) << "DfsRadarTest_GetAnonymStr_001 End";
}

/**
 * @tc.name: RadarReportAdapterTest_ReportDfxStatistics_002
 * @tc.desc: verify ReportDfxStatistics with non-empty statistics.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_ReportDfxStatistics_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportDfxStatistics_002 begin";
    dfsRadarMock_ = std::make_shared<DfsRadarMock>();
    DfsRadarMock::dfsRadarMock = dfsRadarMock_;
    RadarReportAdapter::GetInstance().opStatistics_.connectSuccCount = 1;
    EXPECT_CALL(*dfsRadarMock_, ReportStatistics(_)).Times(1);
    try {
        RadarReportAdapter::GetInstance().ReportDfxStatistics();
        EXPECT_TRUE(RadarReportAdapter::GetInstance().opStatistics_.empty());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    DfsRadarMock::dfsRadarMock = nullptr;
    dfsRadarMock_ = nullptr;
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportDfxStatistics_002 end";
}

/**
 * @tc.name: RadarReportAdapterTest_ReportLinkConnectionAdapter_001
 * @tc.desc: verify ReportLinkConnectionAdapter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_ReportLinkConnectionAdapter_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportLinkConnectionAdapter_001 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    dfsRadarMock_ = std::make_shared<DfsRadarMock>();
    DfsRadarMock::dfsRadarMock = dfsRadarMock_;
    RadarParaInfo info;
    info.peerNetId = "testNetworkId";
    EXPECT_CALL(*deviceManagerImplMock_,
                GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(NETWORKID_ONE), Return(0)));
    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*dfsRadarMock_, ReportLinkConnection(_, _)).Times(1);
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().ReportLinkConnectionAdapter(info));
    DfsRadarMock::dfsRadarMock = nullptr;
    dfsRadarMock_ = nullptr;
#endif
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportLinkConnectionAdapter_001 end";
}

/**
 * @tc.name: RadarReportAdapterTest_ReportGenerateDisUriAdapter_001
 * @tc.desc: verify ReportGenerateDisUriAdapter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_ReportGenerateDisUriAdapter_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportGenerateDisUriAdapter_001 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    dfsRadarMock_ = std::make_shared<DfsRadarMock>();
    DfsRadarMock::dfsRadarMock = dfsRadarMock_;
    RadarParaInfo info;
    info.peerNetId = "testNetworkId";
    EXPECT_CALL(*deviceManagerImplMock_,
                GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(NETWORKID_ONE), Return(0)));
    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*dfsRadarMock_, ReportGenerateDisUri(_, _)).Times(1);
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().ReportGenerateDisUriAdapter(info));
    DfsRadarMock::dfsRadarMock = nullptr;
    dfsRadarMock_ = nullptr;
#endif
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportGenerateDisUriAdapter_001 end";
}

/**
 * @tc.name: RadarReportAdapterTest_ReportFileAccessAdapter_001
 * @tc.desc: verify ReportFileAccessAdapter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_ReportFileAccessAdapter_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportFileAccessAdapter_001 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    dfsRadarMock_ = std::make_shared<DfsRadarMock>();
    DfsRadarMock::dfsRadarMock = dfsRadarMock_;
    RadarParaInfo info;
    info.peerNetId = "testNetworkId";
    EXPECT_CALL(*deviceManagerImplMock_,
                GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(NETWORKID_ONE), Return(0)));
    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*dfsRadarMock_, ReportFileAccess(_, _)).Times(1);
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().ReportFileAccessAdapter(info));
    DfsRadarMock::dfsRadarMock = nullptr;
    dfsRadarMock_ = nullptr;
#endif
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_ReportFileAccessAdapter_001 end";
}

/**
 * @tc.name: RadarReportAdapterTest_RptConnectAdapter_001
 * @tc.desc: verify RptConnectAdapter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_RptConnectAdapter_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_RptConnectAdapter_001 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    dfsRadarMock_ = std::make_shared<DfsRadarMock>();
    DfsRadarMock::dfsRadarMock = dfsRadarMock_;
    RadarParaInfo info;
    info.peerNetId = "testNetworkId";
    ReportLevel level = OHOS::FileManagement::ReportLevel::DEFAULT;
    int32_t result = 0;
    std::string error = "test error";
    EXPECT_CALL(*deviceManagerImplMock_,
                GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(NETWORKID_ONE), Return(0)));
    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*dfsRadarMock_, ReportLinkConnection(_, _)).Times(1);
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().RptConnectAdapter(info, level, result, error));
    EXPECT_EQ(info.faultLevel, level);
    EXPECT_EQ(info.resultCode, result);
    EXPECT_EQ(info.errorInfo, error);
    DfsRadarMock::dfsRadarMock = nullptr;
    dfsRadarMock_ = nullptr;
#endif
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_RptConnectAdapter_001 end";
}

/**
 * @tc.name: RadarReportAdapterTest_RptFileAccAdapter_001
 * @tc.desc: verify RptFileAccAdapter.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(RadarReportAdapterTest, RadarReportAdapterTest_RptFileAccAdapter_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_RptFileAccAdapter_001 begin";
#ifdef SUPPORT_SAME_ACCOUNT
    dfsRadarMock_ = std::make_shared<DfsRadarMock>();
    DfsRadarMock::dfsRadarMock = dfsRadarMock_;
    RadarParaInfo info;
    info.peerNetId = "testNetworkId";
    ReportLevel level = OHOS::FileManagement::ReportLevel::DEFAULT;
    int32_t result = 0;
    std::string error = "test error";
    EXPECT_CALL(*deviceManagerImplMock_,
                GetLocalDeviceNetWorkId(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(NETWORKID_ONE), Return(0)));
    EXPECT_CALL(*softBusServerMock_, GetNodeKeyInfo(_, _, _, _, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*dfsRadarMock_, ReportFileAccess(_, _)).Times(1);
    EXPECT_NO_FATAL_FAILURE(RadarReportAdapter::GetInstance().RptFileAccAdapter(info, level, result, error));
    EXPECT_EQ(info.faultLevel, level);
    EXPECT_EQ(info.resultCode, result);
    EXPECT_EQ(info.errorInfo, error);
    DfsRadarMock::dfsRadarMock = nullptr;
    dfsRadarMock_ = nullptr;
#endif
    GTEST_LOG_(INFO) << "RadarReportAdapterTest_RptFileAccAdapter_001 end";
}
} // namespace OHOS::Storage::DistributedFile