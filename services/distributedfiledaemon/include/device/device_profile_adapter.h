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
 
#ifndef FILEMANAGEMENT_DFS_SERVICE_DEVICE_PROFILE_ADAPTER_H
#define FILEMANAGEMENT_DFS_SERVICE_DEVICE_PROFILE_ADAPTER_H
 
#include <string>
#include <sstream>
#include <map>
 
namespace OHOS {
namespace Storage {
namespace DistributedFile {
enum class VersionPackageNames {
    DFS_VERSION = 1,
};
struct DfsVersion {
    uint32_t majorVersionNum = 0;
    uint32_t minorVersionNum = 0;
    uint32_t featureVersionNum = 0;

    std::string dump() const {
        std::stringstream ss;
        ss << majorVersionNum << "." << minorVersionNum << "." << featureVersionNum;
        return ss.str();
    }
};
 
class DeviceProfileAdapter {
public:
    ~DeviceProfileAdapter() = default;
    static DeviceProfileAdapter &GetInstance()
    {
        static DeviceProfileAdapter instance;
        return instance;
    }
    bool IsRemoteDfsVersionLower(const std::string& remoteNetworkId,
        VersionPackageNames packageName = VersionPackageNames::DFS_VERSION);
    bool IsRemoteDfsVersionLower(const std::string& remoteNetworkId, 
        const DfsVersion& thresholdDfsVersion, VersionPackageNames packageName = VersionPackageNames::DFS_VERSION);
    int32_t GetDfsVersionFromNetworkId(const std::string &networketId,
        DfsVersion& dfsVersion, VersionPackageNames packageName = VersionPackageNames::DFS_VERSION);
    bool CompareDfsVersion(const DfsVersion &dfsVersion, const DfsVersion &thresholdDfsVersion);
    int32_t GetDeviceStatus(const std::string &networkId, bool &status);
    int32_t PutDeviceStatus(bool status);

private:
    std::string GetPackageName(VersionPackageNames packageName)
    {
        static std::map<VersionPackageNames, std::string> packageNamesMap = {
            {VersionPackageNames::DFS_VERSION, "dfsVersion"},
        };
        auto it = packageNamesMap.find(packageName);
        if (it == packageNamesMap.end()) {
            return "";
        }
        return it->second;
    }
    int32_t GetDfsVersionDataFromAppInfo(const std::string& packageNamesData,
        const std::string& versionsData, VersionPackageNames packageName, std::string& dfsVersionData);
    int32_t GetAppInfoFromDP(const std::string& udid, const std::string& serviceName, std::string& appInfoJsonData);
    int32_t GetDfsVersion(const std::string &udid, VersionPackageNames packageName,
        DfsVersion& dfsVersion, bool IsVerifyCode);
    bool ParseDfsVersion(const std::string& dfsVersionData, DfsVersion& dfsVersion);
    int32_t ParseAppInfo(const std::string& appInfoJsonData, std::string& packageNamesData, std::string& versionsData);
    int32_t GetLocalDfsVersion(VersionPackageNames packageName, DfsVersion& dfsVersion);
    std::string GetUdidByNetworkId(const std::string &networkId);
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_ALL_CONNECT_MANAGER_H