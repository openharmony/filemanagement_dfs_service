/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "device/device_profile_adapter.h"

#include "device_manager.h"
#include "dfs_error.h"
#include "ipc/i_daemon.h"
#include "nlohmann/json.hpp"
#include "string_ex.h"
#include "utils_log.h"

#ifdef SUPPORT_DEVICE_PROFILE
#include "distributed_device_profile_client.h"
#endif
 
namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
using namespace OHOS::FileManagement;

#ifdef SUPPORT_DEVICE_PROFILE
using namespace OHOS::DistributedDeviceProfile;

namespace {
constexpr const char *DFS_SERVICE_ID = "distributedfiledaemon";
constexpr const char *DMS_SERVICE_ID = "dmsfwk_svr_id";
constexpr const char *DFS_CHAR_ID = "static_capability";
constexpr const char *PACKAGE_NAMES = "packageNames";
constexpr const char *VERSIONS = "versions";
constexpr const char *SWITCH_ID = "SwitchStatus_Distributed_Super_Manager";
constexpr const char *CHARACTER_ID = "SwitchStatus";
constexpr const char *STATUS_ENABLE = "1";
constexpr const char *STATUS_DISABLE = "0";
const static int32_t DFS_VERSION_LENGTH = 3;
const static int32_t DFS_MAJOR_VERSION_INDEX = 0;
const static int32_t DFS_MINOR_VERSION_INDEX = 1;
const static int32_t DFS_FEATURE_VERSION_INDEX = 2;
const static DfsVersion NO_VERSION = {0, 0, 0};
const static int32_t MAX_JSON_SIZE = 9999;
}

static inline std::string Bool2Str(bool value)
{
    return value ? STATUS_ENABLE : STATUS_DISABLE;
}
 
static inline bool Str2Bool(const std::string &value)
{
    return value == STATUS_ENABLE;
}
#endif

bool DeviceProfileAdapter::IsRemoteDfsVersionLowerThanLocal(const std::string &remoteNetworkId,
    VersionPackageName packageName)
{
    LOGI("remoteDevice.networkId: %{public}.6s", remoteNetworkId.c_str());
#ifdef SUPPORT_DEVICE_PROFILE
    DfsVersion localDfsVersion;
    int32_t ret = GetLocalDfsVersion(packageName, localDfsVersion);
    if (ret != FileManagement::ERR_OK) {
        LOGE("GetLocalDfsVersion failed, ret=%{public}d", ret);
        return true;
    }
    return IsRemoteDfsVersionLowerThanGiven(remoteNetworkId, localDfsVersion, packageName);
#else
    return true;
#endif
}

// Comparison of version numbers later than the current version number is not supported.
bool DeviceProfileAdapter::IsRemoteDfsVersionLowerThanGiven(const std::string &remoteNetworkId,
    const DfsVersion &givenDfsVersion, VersionPackageName packageName)
{
    LOGI("remoteDevice.networkId: %{public}.6s", remoteNetworkId.c_str());
#ifdef SUPPORT_DEVICE_PROFILE
    if (remoteNetworkId.empty()) {
        LOGE("remoteNetworkId is empty");
        return true;
    }
    DfsVersion remoteDfsVersion;
    int32_t ret = GetDfsVersionFromNetworkId(remoteNetworkId, remoteDfsVersion, packageName);
    if (ret != FileManagement::ERR_OK) {
        LOGE("GetDfsVersionFromNetworkId failed, ret=%{public}d", ret);
        return true;
    }
    return CompareDfsVersion(remoteDfsVersion, givenDfsVersion);
#else
    return true;
#endif
}

int32_t DeviceProfileAdapter::GetDeviceStatus(const std::string &networkId, bool &status)
{
#ifdef SUPPORT_DEVICE_PROFILE
    auto udid = GetUdidByNetworkId(networkId);
    if (udid.empty()) {
        LOGE("remote GetUdidByNetworkId failed");
        return ERR_GET_UDID;
    }

    CharacteristicProfile profile;
    int32_t ret = DistributedDeviceProfileClient::GetInstance().GetCharacteristicProfile(udid,
        SWITCH_ID, CHARACTER_ID, profile);
    if (ret != DistributedDeviceProfile::DP_SUCCESS) {
        LOGE("GetDeviceStatus failed, ret=%{public}d, udid=%{public}.5s, status=%{public}d", ret, udid.c_str(),
            status);
        return ret;
    }
    std::string enabledStatus = profile.GetCharacteristicValue();
    status = Str2Bool(enabledStatus);
    LOGI("GetDeviceStatus success, udid=%{public}.5s, status=%{public}d", udid.c_str(), status);
#endif
    return FileManagement::ERR_OK;
}

int32_t DeviceProfileAdapter::PutDeviceStatus(bool status)
{
#ifdef SUPPORT_DEVICE_PROFILE
    DistributedHardware::DmDeviceInfo localDeviceInfo{};
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    int ret = deviceManager.GetLocalDeviceInfo(IDaemon::SERVICE_NAME, localDeviceInfo);
    if (ret != FileManagement::ERR_OK) {
        LOGE("GetLocalDeviceInfo failed, errCode = %{public}d", ret);
        return ret;
    }
    LOGD("GetLocalDeviceInfo success, localDeviceInfo.networkId = %{public}.5s", localDeviceInfo.networkId);
    auto udid = GetUdidByNetworkId(localDeviceInfo.networkId);
    if (udid.empty()) {
        LOGE("remote GetUdidByNetworkId failed");
        return ERR_GET_UDID;
    }

    std::string enabledStatus = Bool2Str(status);
    CharacteristicProfile profile;
    profile.SetDeviceId(udid);
    profile.SetServiceName(SWITCH_ID);
    profile.SetCharacteristicKey(CHARACTER_ID);
    profile.SetCharacteristicValue(enabledStatus);
 
    ret = DistributedDeviceProfileClient::GetInstance().PutCharacteristicProfile(profile);
    if (ret != DistributedDeviceProfile::DP_SUCCESS) {
        LOGE("PutDeviceStatus failed, ret=%{public}d, udid=%{public}.5s, status=%{public}d", ret, udid.c_str(), status);
        return ret;
    }
    LOGI("PutDeviceStatus success, udid=%{public}.5s, status=%{public}d", udid.c_str(), status);
#endif
    return FileManagement::ERR_OK;
}

#ifdef SUPPORT_DEVICE_PROFILE
int32_t DeviceProfileAdapter::GetLocalDfsVersion(VersionPackageName packageName, DfsVersion &dfsVersion)
{
    DistributedHardware::DmDeviceInfo localDeviceInfo{};
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    int ret = deviceManager.GetLocalDeviceInfo(IDaemon::SERVICE_NAME, localDeviceInfo);
    if (ret != FileManagement::ERR_OK) {
        LOGE("GetLocalDeviceInfo failed, errCode = %{public}d", ret);
        return ret;
    }
    LOGD("GetLocalDeviceInfo success, localDeviceInfo.networkId=%{public}.5s", localDeviceInfo.networkId);
    auto udid = GetUdidByNetworkId(localDeviceInfo.networkId);
    if (udid.empty()) {
        LOGE("remote GetUdidByNetworkId failed, networkId=%{public}.5s", localDeviceInfo.networkId);
        return ERR_GET_UDID;
    }
    return GetDfsVersion(udid, packageName, dfsVersion, false);
}
#endif

int32_t DeviceProfileAdapter::GetDfsVersionFromNetworkId(
    const std::string &networkId, DfsVersion &dfsVersion, VersionPackageName packageName)
{
#ifdef SUPPORT_DEVICE_PROFILE
    if (networkId.empty()) {
        LOGE("networkId: %{public}.5s is empty", networkId.c_str());
        return ERR_NULLPTR;
    }
    auto udid = GetUdidByNetworkId(networkId);
    if (udid.empty()) {
        LOGE("remote GetUdidByNetworkId failed, networkId=%{public}.5s ", networkId.c_str());
        return ERR_GET_UDID;
    }
    return GetDfsVersion(udid, packageName, dfsVersion, true);
#else
    return FileManagement::ERR_OK;
#endif
}

#ifdef SUPPORT_DEVICE_PROFILE
int32_t DeviceProfileAdapter::GetDfsVersion(const std::string &udid,
    VersionPackageName packageName, DfsVersion &dfsVersion, bool IsVerifyCode)
{
    std::string appInfoJsonData;
    int32_t ret = GetAppInfoFromDP(udid, DFS_SERVICE_ID, appInfoJsonData);
    if (ret == DistributedDeviceProfile::DP_NOT_FOUND_FAIL) {
        auto dmsResult = GetAppInfoFromDP(udid, DMS_SERVICE_ID, appInfoJsonData);
        if (IsVerifyCode && dmsResult == DistributedDeviceProfile::DP_NOT_FOUND_FAIL) {
            LOGW("The DP version of the remote device is later, return local dfsVersion.");
            GetLocalDfsVersion(packageName, dfsVersion);
            return FileManagement::ERR_OK;
        } else if (dmsResult != DistributedDeviceProfile::DP_SUCCESS) {
            LOGE("get app info failed, result: %{public}d!", dmsResult);
            return dmsResult;
        }
        LOGW("The dfs version number has not been added to the remote device.");
        dfsVersion = NO_VERSION;
        return FileManagement::ERR_OK;
    } else if (ret != DistributedDeviceProfile::DP_SUCCESS) {
        LOGE("get app info failed, result: %{public}d!", ret);
        return ret;
    }

    std::string packageNamesData;
    std::string versionsData;
    ret = ParseAppInfo(appInfoJsonData, packageNamesData, versionsData);
    if (ret != FileManagement::ERR_OK) {
        LOGE("parse app info failed ret=%{public}d, appInfoJsonData=%{public}s", ret, appInfoJsonData.c_str());
        return ret;
    }

    std::string dfsVersionData;
    ret = GetDfsVersionDataFromAppInfo(packageNamesData, versionsData, packageName, dfsVersionData);
    if (ret != FileManagement::ERR_OK) {
        if (ret == ERR_NO_FIND_PACKAGE_NAME) {
            LOGW("not find packageName: %{public}d, %{public}s", static_cast<int>(packageName), dfsVersionData.c_str());
            dfsVersion = NO_VERSION;
            return FileManagement::ERR_OK;
        }
        LOGE("get dfs version data failed, result: %{public}d!", ret);
        return ret;
    }

    if (!ParseDfsVersion(dfsVersionData, dfsVersion)) {
        LOGE("parse dfs version failed, dfsVersionData=%{public}s", dfsVersionData.c_str());
        return ERR_DFS_VERSION_PARSE_EXCEPTION;
    }
    LOGI("GetDfsVersion success, %{public}s", dfsVersion.dump().c_str());
    return FileManagement::ERR_OK;
}

bool DeviceProfileAdapter::ParseDfsVersion(const std::string &dfsVersionData, DfsVersion &dfsVersion)
{
    std::vector<std::string> versionNumList;
    SplitStr(dfsVersionData, ".", versionNumList);
    if (versionNumList.size() != DFS_VERSION_LENGTH) {
        return false;
    }
    int32_t majorVersionNum = -1;
    if (!OHOS::StrToInt(versionNumList[DFS_MAJOR_VERSION_INDEX], majorVersionNum) || majorVersionNum < 0) {
        return false;
    }
    dfsVersion.majorVersionNum = static_cast<uint32_t>(majorVersionNum);

    int32_t minorVersionNum = -1;
    if (!OHOS::StrToInt(versionNumList[DFS_MINOR_VERSION_INDEX], minorVersionNum) || minorVersionNum < 0) {
        return false;
    }
    dfsVersion.minorVersionNum = static_cast<uint32_t>(minorVersionNum);

    int32_t featureVersionNum = -1;
    if (!OHOS::StrToInt(versionNumList[DFS_FEATURE_VERSION_INDEX], featureVersionNum) || featureVersionNum < 0) {
        return false;
    }
    dfsVersion.featureVersionNum = static_cast<uint32_t>(featureVersionNum);
    return true;
}

int32_t DeviceProfileAdapter::ParseAppInfo(const std::string &appInfoJsonData, std::string &packageNamesData,
    std::string &versionsData)
{
    if (appInfoJsonData.empty() || appInfoJsonData.size() > MAX_JSON_SIZE) {
        return ERR_DFS_VERSION_EMPTY;
    }
    nlohmann::json appInfoJson = nlohmann::json::parse(appInfoJsonData.c_str(), nullptr, false);
    if (appInfoJson.is_discarded()) {
        return ERR_DFS_VERSION_EMPTY;
    }
    if (appInfoJson.find(PACKAGE_NAMES) == appInfoJson.end() || !appInfoJson.at(PACKAGE_NAMES).is_string()) {
        return ERR_DFS_VERSION_EMPTY;
    }
    appInfoJson.at(PACKAGE_NAMES).get_to(packageNamesData);

    if (appInfoJson.find(VERSIONS) == appInfoJson.end() || !appInfoJson.at(VERSIONS).is_string()) {
        return ERR_DFS_VERSION_EMPTY;
    }
    appInfoJson.at(VERSIONS).get_to(versionsData);

    return FileManagement::ERR_OK;
}

int32_t DeviceProfileAdapter::GetDfsVersionDataFromAppInfo(const std::string &packageNamesData,
    const std::string &versionsData, VersionPackageName packageName, std::string &dfsVersionData)
{
    std::string packageNameString = GetPackageName(packageName);
    if (packageNamesData.empty() || versionsData.empty() || packageNameString.empty()) {
        return ERR_DFS_VERSION_EMPTY;
    }
    
    std::vector<std::string> packageNameList;
    std::vector<std::string> versionsList;
    SplitStr(packageNamesData, ",", packageNameList);
    SplitStr(versionsData, ",", versionsList);
    if (packageNameList.size() != versionsList.size()) {
        return ERR_DFS_VERSION_PARSE_EXCEPTION;
    }
    for (std::size_t i = 0; i < packageNameList.size(); i++) {
        if (packageNameList[i] == packageNameString) {
            dfsVersionData = versionsList[i];
            return FileManagement::ERR_OK;
        }
    }
    return ERR_NO_FIND_PACKAGE_NAME;
}

int32_t DeviceProfileAdapter::GetAppInfoFromDP(const std::string &udid,
    const std::string &serviceName, std::string &appInfoJsonData)
{
    DistributedDeviceProfile::CharacteristicProfile profile;
    int32_t ret = DistributedDeviceProfileClient::GetInstance().GetCharacteristicProfile(
        udid, serviceName, DFS_CHAR_ID, profile);
    if (ret != DistributedDeviceProfile::DP_SUCCESS) {
        return ret;
    }
    appInfoJsonData = profile.GetCharacteristicValue();
    return FileManagement::ERR_OK;
}

bool DeviceProfileAdapter::CompareDfsVersion(const DfsVersion &dfsVersion, const DfsVersion &thresholdDfsVersion)
{
    if (dfsVersion.majorVersionNum != thresholdDfsVersion.majorVersionNum) {
        return dfsVersion.majorVersionNum < thresholdDfsVersion.majorVersionNum;
    }
    if (dfsVersion.minorVersionNum != thresholdDfsVersion.minorVersionNum) {
        return dfsVersion.minorVersionNum < thresholdDfsVersion.minorVersionNum;
    }
    return dfsVersion.featureVersionNum < thresholdDfsVersion.featureVersionNum;
}

std::string DeviceProfileAdapter::GetUdidByNetworkId(const std::string &networkId)
{
    std::string udid;
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    auto ret = deviceManager.GetUdidByNetworkId(IDaemon::SERVICE_NAME, networkId, udid);
    if (ret != FileManagement::ERR_OK) {
        LOGE("GetUdidByNetworkId failed, ret = %{public}d", ret);
        return "";
    }
    return udid;
}
#endif
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
