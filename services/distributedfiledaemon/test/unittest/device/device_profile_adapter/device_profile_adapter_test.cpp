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

#include "device/device_profile_adapter.h"
#include "device_manager_impl.h"
#include "dfs_error.h"
#include "distributed_device_profile_client.h"

namespace {
constexpr const char* DMS_SERVICE_ID = "dmsfwk_svr_id";
std::string PROFILE = "{\n\t\"packageNames\":\t\"dfsVersion\",\n\t\"versions\":\t\"7.0.0\"\n}";
std::string g_Profile = "";
int32_t g_GetCharacteristicProfile;
int32_t g_GetCharacteristicProfile2;
int32_t g_PutCharacteristicProfile;
int32_t g_GetLocalDeviceInfo;
int32_t g_GetUdidByNetworkId;
}

namespace OHOS::DistributedDeviceProfile {
int32_t DistributedDeviceProfileClient::GetCharacteristicProfile(const std::string& deviceId,
    const std::string& serviceName, const std::string& characteristicId, CharacteristicProfile &characteristicProfile)
{
    std::string profile = g_Profile.empty() ? PROFILE : g_Profile;
    characteristicProfile.SetCharacteristicValue(profile);
    return serviceName == DMS_SERVICE_ID ? g_GetCharacteristicProfile2 : g_GetCharacteristicProfile;
}

int32_t DistributedDeviceProfileClient::PutCharacteristicProfile(const CharacteristicProfile &characteristicProfile)
{
    return g_PutCharacteristicProfile;
}
}

namespace OHOS::DistributedHardware {
int32_t DeviceManagerImpl::GetLocalDeviceInfo(const std::string &pkgName, DmDeviceInfo &info)
{
    std::string networkId = "localTest";
    auto res = strcpy_s(info.networkId, DM_MAX_DEVICE_ID_LEN, networkId.c_str());
    if (res != NO_ERROR) {
        return -1;
    }
    return g_GetLocalDeviceInfo;
}

int32_t DeviceManagerImpl::GetUdidByNetworkId(const std::string &pkgName,
    const std::string &netWorkId, std::string &udid)
{
    udid = "localTest";
    return g_GetUdidByNetworkId;
}
} // namespace OHOS::DistributedHardware

namespace OHOS::Storage::DistributedFile {
namespace Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class DeviceProfileAdapterTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DeviceProfileAdapterTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DeviceProfileAdapterTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DeviceProfileAdapterTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DeviceProfileAdapterTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DeviceProfileAdapterTest_IsRemoteDfsVersionLowerThanLocal_001
 * @tc.desc: verify PublishSA.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_IsRemoteDfsVersionLowerThanLocal_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_IsRemoteDfsVersionLowerThanLocal_001 begin";

    g_GetLocalDeviceInfo = ERR_BAD_VALUE;
    auto ret = DeviceProfileAdapter::GetInstance().IsRemoteDfsVersionLowerThanLocal("");
    EXPECT_EQ(ret, true);

    g_GetLocalDeviceInfo = FileManagement::ERR_OK;
    g_GetUdidByNetworkId = FileManagement::ERR_OK;
    g_GetCharacteristicProfile = FileManagement::ERR_OK;
    ret = DeviceProfileAdapter::GetInstance().IsRemoteDfsVersionLowerThanLocal("");
    EXPECT_EQ(ret, true);

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_IsRemoteDfsVersionLowerThanLocal_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_IsRemoteDfsVersionLowerThanGiven_001
 * @tc.desc: verify PublishSA.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_IsRemoteDfsVersionLowerThanGiven_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_IsRemoteDfsVersionLowerThanGiven_001 begin";

    auto ret = DeviceProfileAdapter::GetInstance().IsRemoteDfsVersionLowerThanGiven("", {0, 0, 0});
    EXPECT_EQ(ret, true);

    g_GetUdidByNetworkId = -1;
    std::string remoteNetworkId = "remoteTest";
    ret = DeviceProfileAdapter::GetInstance().IsRemoteDfsVersionLowerThanGiven(remoteNetworkId, {0, 0, 0});
    EXPECT_EQ(ret, true);

    g_GetUdidByNetworkId = FileManagement::ERR_OK;
    g_GetCharacteristicProfile = FileManagement::ERR_OK;
    DfsVersion thresholdDfsVersion = {99, 99, 99};
    ret = DeviceProfileAdapter::GetInstance().IsRemoteDfsVersionLowerThanGiven(remoteNetworkId, thresholdDfsVersion);
    EXPECT_EQ(ret, true);

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_IsRemoteDfsVersionLowerThanGiven_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_GetDeviceStatus_001
 * @tc.desc: verify GetDeviceStatus.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_GetDeviceStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetDeviceStatus_001 begin";

    g_GetUdidByNetworkId = -1;
    bool status = false;
    auto ret = DeviceProfileAdapter::GetInstance().GetDeviceStatus("test", status);
    EXPECT_EQ(ret, ERR_GET_UDID);

    g_GetUdidByNetworkId = FileManagement::ERR_OK;
    g_GetCharacteristicProfile = -1;
    ret = DeviceProfileAdapter::GetInstance().GetDeviceStatus("test", status);
    EXPECT_EQ(ret, -1);

    g_GetCharacteristicProfile = FileManagement::ERR_OK;
    g_Profile = "1";
    ret = DeviceProfileAdapter::GetInstance().GetDeviceStatus("test", status);
    EXPECT_EQ(ret, FileManagement::ERR_OK);
    g_Profile = "";

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetDeviceStatus_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_PutDeviceStatus_001
 * @tc.desc: verify PutDeviceStatus.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_PutDeviceStatus_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_PutDeviceStatus_001 begin";

    g_GetLocalDeviceInfo = -1;
    auto ret = DeviceProfileAdapter::GetInstance().PutDeviceStatus(false);
    EXPECT_EQ(ret, -1);

    g_GetLocalDeviceInfo = FileManagement::ERR_OK;
    g_GetUdidByNetworkId = -1;
    ret = DeviceProfileAdapter::GetInstance().PutDeviceStatus(false);
    EXPECT_EQ(ret, ERR_GET_UDID);

    g_GetUdidByNetworkId = FileManagement::ERR_OK;
    g_PutCharacteristicProfile = -1;
    ret = DeviceProfileAdapter::GetInstance().PutDeviceStatus(false);
    EXPECT_EQ(ret, -1);

    g_PutCharacteristicProfile = FileManagement::ERR_OK;
    ret = DeviceProfileAdapter::GetInstance().PutDeviceStatus(true);
    EXPECT_EQ(ret, FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_PutDeviceStatus_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_GetLocalDfsVersion_001
 * @tc.desc: verify PublishSA.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_GetLocalDfsVersion_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetLocalDfsVersion_001 begin";

    g_GetLocalDeviceInfo = -1;
    DfsVersion dfsVersion;
    auto ret = DeviceProfileAdapter::GetInstance().GetLocalDfsVersion(VersionPackageName::DFS_VERSION, dfsVersion);
    EXPECT_EQ(ret, -1);

    g_GetLocalDeviceInfo = FileManagement::ERR_OK;
    g_GetUdidByNetworkId = -1;
    ret = DeviceProfileAdapter::GetInstance().GetLocalDfsVersion(VersionPackageName::DFS_VERSION, dfsVersion);
    EXPECT_EQ(ret, ERR_GET_UDID);

    g_GetUdidByNetworkId = FileManagement::ERR_OK;
    g_GetCharacteristicProfile = FileManagement::ERR_OK;
    ret = DeviceProfileAdapter::GetInstance().GetLocalDfsVersion(VersionPackageName::DFS_VERSION, dfsVersion);
    EXPECT_EQ(ret, FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetLocalDfsVersion_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_GetDfsVersionFromNetworkId_001
 * @tc.desc: verify GetDfsVersionFromNetworkId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_GetDfsVersionFromNetworkId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetDfsVersionFromNetworkId_001 begin";

    DfsVersion dfsVersion;
    auto ret = DeviceProfileAdapter::GetInstance().GetDfsVersionFromNetworkId("", dfsVersion);
    EXPECT_EQ(ret, ERR_NULLPTR);

    g_GetUdidByNetworkId = -1;
    ret = DeviceProfileAdapter::GetInstance().GetDfsVersionFromNetworkId("test", dfsVersion);
    EXPECT_EQ(ret, ERR_GET_UDID);

    g_GetUdidByNetworkId = FileManagement::ERR_OK;
    g_GetCharacteristicProfile = FileManagement::ERR_OK;
    ret = DeviceProfileAdapter::GetInstance().GetLocalDfsVersion(VersionPackageName::DFS_VERSION, dfsVersion);
    EXPECT_EQ(ret, FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetDfsVersionFromNetworkId_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_GetDfsVersion_001
 * @tc.desc: verify PublishSA.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_GetDfsVersion_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetDfsVersion_001 begin";
    auto dpa = DeviceProfileAdapter::GetInstance();

    DfsVersion dfsVersion;
    g_GetCharacteristicProfile = -1;
    auto ret = dpa.GetDfsVersion("", VersionPackageName::DFS_VERSION, dfsVersion, false);
    EXPECT_EQ(ret, -1);

    g_GetCharacteristicProfile = DistributedDeviceProfile::DP_NOT_FOUND_FAIL;
    g_GetCharacteristicProfile2 = -1;
    ret = dpa.GetDfsVersion("", VersionPackageName::DFS_VERSION, dfsVersion, false);
    EXPECT_EQ(ret, -1);

    g_GetCharacteristicProfile2 = DistributedDeviceProfile::DP_SUCCESS;
    ret = dpa.GetDfsVersion("", VersionPackageName::DFS_VERSION, dfsVersion, true);
    EXPECT_EQ(ret, FileManagement::ERR_OK);

    g_GetLocalDeviceInfo = DistributedDeviceProfile::DP_NOT_FOUND_FAIL;
    ret = dpa.GetDfsVersion("", VersionPackageName::DFS_VERSION, dfsVersion, true);
    EXPECT_EQ(ret, FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetDfsVersion_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_GetDfsVersion_002
 * @tc.desc: verify GetDfsVersion.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_GetDfsVersion_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetDfsVersion_002 begin";
    auto dpa = DeviceProfileAdapter::GetInstance();

    DfsVersion dfsVersion;
    g_GetCharacteristicProfile = DistributedDeviceProfile::DP_SUCCESS;
    g_Profile = "test";
    auto ret = dpa.GetDfsVersion("", VersionPackageName::DFS_VERSION, dfsVersion, false);
    EXPECT_EQ(ret, ERR_DFS_VERSION_EMPTY);

    g_Profile = "{\n\t\"packageNames\":\t\"version\",\n\t\"versions\":\t\"7.0.0\"\n}";
    ret = dpa.GetDfsVersion("", VersionPackageName::DFS_VERSION, dfsVersion, false);
    EXPECT_EQ(ret, FileManagement::ERR_OK);


    g_Profile = "{\n\t\"packageNames\":\t\"dfsVersion,copyVersion\",\n\t\"versions\":\t\"7.0.0\"\n}";
    ret = dpa.GetDfsVersion("", VersionPackageName::DFS_VERSION, dfsVersion, false);
    EXPECT_EQ(ret, ERR_DFS_VERSION_PARSE_EXCEPTION);

    g_Profile = "{\n\t\"packageNames\":\t\"dfsVersion,copyVersion\",\n\t\"versions\":\t\"7.0\"\n}";
    ret = dpa.GetDfsVersion("", VersionPackageName::DFS_VERSION, dfsVersion, false);
    EXPECT_EQ(ret, ERR_DFS_VERSION_PARSE_EXCEPTION);

    g_Profile = "";
    ret = dpa.GetDfsVersion("", VersionPackageName::DFS_VERSION, dfsVersion, false);
    EXPECT_EQ(ret, FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetDfsVersion_002 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_ParseDfsVersion_001
 * @tc.desc: verify PublishSA.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_ParseDfsVersion_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_ParseDfsVersion_001 begin";

    DfsVersion dfsVersion;
    EXPECT_FALSE(DeviceProfileAdapter::GetInstance().ParseDfsVersion("1.0", dfsVersion));
    EXPECT_FALSE(DeviceProfileAdapter::GetInstance().ParseDfsVersion("a.0.0", dfsVersion));
    EXPECT_FALSE(DeviceProfileAdapter::GetInstance().ParseDfsVersion("-1.0.0", dfsVersion));
    EXPECT_FALSE(DeviceProfileAdapter::GetInstance().ParseDfsVersion("1.a.0", dfsVersion));
    EXPECT_FALSE(DeviceProfileAdapter::GetInstance().ParseDfsVersion("1.-1.0", dfsVersion));
    EXPECT_FALSE(DeviceProfileAdapter::GetInstance().ParseDfsVersion("1.1.a", dfsVersion));
    EXPECT_FALSE(DeviceProfileAdapter::GetInstance().ParseDfsVersion("1.1.-1", dfsVersion));
    EXPECT_TRUE(DeviceProfileAdapter::GetInstance().ParseDfsVersion("1.1.1", dfsVersion));

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_ParseDfsVersion_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_ParseAppInfo_001
 * @tc.desc: verify ParseAppInfo.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_ParseAppInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_ParseAppInfo_001 begin";

    std::string packageNamesData;
    std::string versionsData;
    auto ret = DeviceProfileAdapter::GetInstance().ParseAppInfo("", packageNamesData, versionsData);
    EXPECT_EQ(ret, ERR_DFS_VERSION_EMPTY);

    ret = DeviceProfileAdapter::GetInstance().ParseAppInfo("test", packageNamesData, versionsData);
    EXPECT_EQ(ret, ERR_DFS_VERSION_EMPTY);

    std::string appInfoJsonData = "{\n\t\"Names\":\t\"dfsVersion\",\n\t\"versions\":\t\"7.0.0\"\n}";
    ret = DeviceProfileAdapter::GetInstance().ParseAppInfo(appInfoJsonData, packageNamesData, versionsData);
    EXPECT_EQ(ret, ERR_DFS_VERSION_EMPTY);

    appInfoJsonData = "{\n\t\"packageNames\":\t\"dfsVersion\",\n\t\"version\":\t\"7.0.0\"\n}";
    ret = DeviceProfileAdapter::GetInstance().ParseAppInfo(appInfoJsonData, packageNamesData, versionsData);
    EXPECT_EQ(ret, ERR_DFS_VERSION_EMPTY);

    appInfoJsonData = "{\n\t\"packageNames\":\t\"dfsVersion\",\n\t\"versions\":\t\"7.0.0\"\n}";
    ret = DeviceProfileAdapter::GetInstance().ParseAppInfo(appInfoJsonData, packageNamesData, versionsData);
    EXPECT_EQ(ret, FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_ParseAppInfo_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_GetDfsVersionDataFromAppInfo_001
 * @tc.desc: verify GetDfsVersionDataFromAppInfo.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_GetDfsVersionDataFromAppInfo_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetDfsVersionDataFromAppInfo_001 begin";

    std::string dfsVersionData;
    auto dpa = DeviceProfileAdapter::GetInstance();
    auto ret = dpa.GetDfsVersionDataFromAppInfo("", "", VersionPackageName::DFS_VERSION, dfsVersionData);
    EXPECT_EQ(ret, ERR_DFS_VERSION_EMPTY);
    ret = dpa.GetDfsVersionDataFromAppInfo("test", "", VersionPackageName::DFS_VERSION, dfsVersionData);
    EXPECT_EQ(ret, ERR_DFS_VERSION_EMPTY);
    ret = dpa.GetDfsVersionDataFromAppInfo("", "test", VersionPackageName::DFS_VERSION, dfsVersionData);
    EXPECT_EQ(ret, ERR_DFS_VERSION_EMPTY);

    std::string packageNames = "dfsVersion,copyVersion";
    std::string versions = "6.0.0";
    ret = dpa.GetDfsVersionDataFromAppInfo(packageNames, versions, VersionPackageName::DFS_VERSION, dfsVersionData);
    EXPECT_EQ(ret, ERR_DFS_VERSION_PARSE_EXCEPTION);

    versions = "6.0.0,7.0.0";
    ret = dpa.GetDfsVersionDataFromAppInfo(packageNames, versions, VersionPackageName::DFS_VERSION, dfsVersionData);
    EXPECT_EQ(ret, FileManagement::ERR_OK);

    packageNames = "assetVersion,copyVersion";
    ret = dpa.GetDfsVersionDataFromAppInfo(packageNames, versions, VersionPackageName::DFS_VERSION, dfsVersionData);
    EXPECT_EQ(ret, ERR_NO_FIND_PACKAGE_NAME);

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetDfsVersionDataFromAppInfo_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_GetAppInfoFromDP_001
 * @tc.desc: verify GetAppInfoFromDP.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_GetAppInfoFromDP_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_ParseAppInfo_001 begin";

    g_GetCharacteristicProfile = -1;
    std::string appInfoJsonData;
    auto ret = DeviceProfileAdapter::GetInstance().GetAppInfoFromDP("", "", appInfoJsonData);
    EXPECT_EQ(ret, -1);

    g_GetCharacteristicProfile = FileManagement::ERR_OK;
    ret = DeviceProfileAdapter::GetInstance().GetAppInfoFromDP("", "", appInfoJsonData);
    EXPECT_EQ(ret, FileManagement::ERR_OK);

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetAppInfoFromDP_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_CompareDfsVersion_001
 * @tc.desc: verify CompareDfsVersion.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_CompareDfsVersion_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_CompareDfsVersion_001 begin";

    auto dpa = DeviceProfileAdapter::GetInstance();
    EXPECT_TRUE(dpa.CompareDfsVersion({1, 0, 0}, {2, 0, 0}));
    EXPECT_TRUE(dpa.CompareDfsVersion({1, 0, 0}, {1, 2, 0}));
    EXPECT_TRUE(dpa.CompareDfsVersion({1, 2, 0}, {1, 2, 2}));
    EXPECT_FALSE(dpa.CompareDfsVersion({2, 0, 0}, {1, 0, 0}));
    EXPECT_FALSE(dpa.CompareDfsVersion({1, 2, 0}, {1, 1, 0}));
    EXPECT_FALSE(dpa.CompareDfsVersion({1, 1, 2}, {1, 1, 1}));
    EXPECT_FALSE(dpa.CompareDfsVersion({1, 1, 1}, {1, 1, 1}));

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_CompareDfsVersion_001 end";
}

/**
 * @tc.name: DeviceProfileAdapterTest_GetUdidByNetworkId_001
 * @tc.desc: verify GetUdidByNetworkId.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(DeviceProfileAdapterTest, DeviceProfileAdapterTest_GetUdidByNetworkId_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetUdidByNetworkId_001 begin";

    g_GetUdidByNetworkId = -1;
    auto ret = DeviceProfileAdapter::GetInstance().GetUdidByNetworkId("");
    EXPECT_EQ(ret, "");

    g_GetUdidByNetworkId = FileManagement::ERR_OK;
    ret = DeviceProfileAdapter::GetInstance().GetUdidByNetworkId("");
    EXPECT_EQ(ret, "localTest");

    GTEST_LOG_(INFO) << "DeviceProfileAdapterTest_GetUdidByNetworkId_001 end";
}
} // namespace Test
} // namespace OHOS::Storage::DistributedFile