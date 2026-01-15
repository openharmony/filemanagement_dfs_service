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

#include "radar_report.h"
#include "dfs_error.h"
#include "utils_log.h"

#include <unistd.h>

#if defined(DFS_ENABLE_DISTRIBUTED_ABILITY) && !defined(DFS_DISABLE_RADAR_ABILITY)
#include "device_manager.h"
#include "softbus_bus_center.h"
#endif

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
constexpr unsigned int RADAR_REPORT_STATISTIC_INTERVAL_MINUTES = 360;
constexpr unsigned int UPDATE_RADAR_STATISTIC_INTERVAL_SECONDS = 300;
const int PREFIX_LEN = 5;
} // namespace

#if defined(DFS_ENABLE_DISTRIBUTED_ABILITY) && !defined(DFS_DISABLE_RADAR_ABILITY)
constexpr const char *DFS_SERVICE_NAME = "ohos.storage.distributedfile.daemon";
#endif
void RadarReportAdapter::InitRadar()
{
    std::unique_lock<std::mutex> lock(onRadarReportLock_);
    opStatistics_ = {0, 0, 0, 0, 0, 0};
    callRadarStatisticReportThread_ = std::thread([this]() { StorageRadarThd(); });
    if (!callRadarStatisticReportThread_.joinable()) {
        return;
    }
    callRadarStatisticReportThread_.detach();
    lastRadarReportTime_ = std::chrono::system_clock::now();
}

void RadarReportAdapter::UnInitRadar()
{
    std::unique_lock<std::mutex> lock(onRadarReportLock_);
    stopRadarReport_ = true;
}

void RadarReportAdapter::StorageRadarThd()
{
    while (!stopRadarReport_.load()) {
        {
            std::unique_lock<std::mutex> lock(onRadarReportLock_);
            std::chrono::time_point<std::chrono::system_clock> nowTime = std::chrono::system_clock::now();
            int64_t intervalMinutes =
                std::chrono::duration_cast<std::chrono::minutes>(nowTime - lastRadarReportTime_).count();
            if (intervalMinutes > RADAR_REPORT_STATISTIC_INTERVAL_MINUTES) {
                ReportDfxStatistics();
            }
        }
        sleep(UPDATE_RADAR_STATISTIC_INTERVAL_SECONDS);
    }
}

void RadarReportAdapter::SetUserStatistics(const RadarStatisticInfoType type)
{
    std::unique_lock<std::mutex> lock(onStatisticsLock_);
    switch (type) {
        case RadarStatisticInfoType::CONNECT_DFS_SUCC_CNT:
            opStatistics_.connectSuccCount++;
            break;
        case RadarStatisticInfoType::CONNECT_DFS_FAIL_CNT:
            opStatistics_.connectFailCount++;
            break;
        case RadarStatisticInfoType::GENERATE_DIS_URI_SUCC_CNT:
            opStatistics_.generateUriSuccCount++;
            break;
        case RadarStatisticInfoType::GENERATE_DIS_URI_FAIL_CNT:
            opStatistics_.generateUriFailCount++;
            break;
        case RadarStatisticInfoType::FILE_ACCESS_SUCC_CNT:
            opStatistics_.fileAccessSuccCount++;
            break;
        case RadarStatisticInfoType::FILE_ACCESS_FAIL_CNT:
            opStatistics_.fileAccessFailCount++;
            break;
        default:
            break;
    }
}

void RadarReportAdapter::ReportDfxStatistics()
{
    if (opStatistics_.empty()) {
        LOGI("DFX no need to report");
        return;
    }
    DfsRadar::GetInstance().ReportStatistics(opStatistics_);
    lastRadarReportTime_ = std::chrono::system_clock::now();
    opStatistics_.clear();
}

void RadarReportAdapter::ReportLinkConnectionAdapter(const RadarParaInfo &info)
{
    std::string localDeviceNetId;
    std::string localDeviceUdid;
    std::string peerDeviceNetId;
    std::string peerDeviceUdid;
    GetLocalNetIdAndUdid(localDeviceNetId, localDeviceUdid);
    GetPeerUdid(info.peerNetId, peerDeviceNetId, peerDeviceUdid);
    RadarIDInfo radarIdInfo = {
        .localDeviceNetId = localDeviceNetId,
        .localDeviceUdid = localDeviceUdid,
        .peerDeviceNetId = peerDeviceNetId,
        .peerDeviceUdid = peerDeviceUdid
    };
    DfsRadar::GetInstance().ReportLinkConnection(info, radarIdInfo);
}

void RadarReportAdapter::ReportGenerateDisUriAdapter(const RadarParaInfo &info)
{
    std::string localDeviceNetId;
    std::string localDeviceUdid;
    std::string peerDeviceNetId;
    std::string peerDeviceUdid;
    GetLocalNetIdAndUdid(localDeviceNetId, localDeviceUdid);
    GetPeerUdid(info.peerNetId, peerDeviceNetId, peerDeviceUdid);
    RadarIDInfo radarIdInfo = {
        .localDeviceNetId = localDeviceNetId,
        .localDeviceUdid = localDeviceUdid,
        .peerDeviceNetId = peerDeviceNetId,
        .peerDeviceUdid = peerDeviceUdid
    };
    DfsRadar::GetInstance().ReportGenerateDisUri(info, radarIdInfo);
}

void RadarReportAdapter::ReportFileAccessAdapter(const RadarParaInfo &info)
{
    std::string localDeviceNetId;
    std::string localDeviceUdid;
    std::string peerDeviceNetId;
    std::string peerDeviceUdid;
    GetLocalNetIdAndUdid(localDeviceNetId, localDeviceUdid);
    GetPeerUdid(info.peerNetId, peerDeviceNetId, peerDeviceUdid);
    RadarIDInfo radarIdInfo = {
        .localDeviceNetId = localDeviceNetId,
        .localDeviceUdid = localDeviceUdid,
        .peerDeviceNetId = peerDeviceNetId,
        .peerDeviceUdid = peerDeviceUdid
    };
    DfsRadar::GetInstance().ReportFileAccess(info, radarIdInfo);
}

void RadarReportAdapter::GetLocalNetIdAndUdid(std::string &localDeviceNetId, std::string &localDeviceUdid)
{
#if defined(DFS_ENABLE_DISTRIBUTED_ABILITY) && !defined(DFS_DISABLE_RADAR_ABILITY)
    std::string localNetworkId = "";
    std::string localUdid = "";
    if (DistributedHardware::DeviceManager::GetInstance()
        .GetLocalDeviceNetWorkId(DFS_SERVICE_NAME, localNetworkId) != 0) {
        LOGE("GetLocalDeviceNetWorkId failed");
        return;
    }

    uint8_t udid[UDID_BUF_LEN] = { 0 };
    int32_t ret = GetNodeKeyInfo(DFS_SERVICE_NAME, localNetworkId.c_str(),
        NodeDeviceInfoKey::NODE_KEY_UDID, udid, sizeof(udid));
    if (ret != E_OK) {
        LOGE("GetNodeKeyInfo failed");
    }
    localUdid = reinterpret_cast<char *>(udid);
    localDeviceNetId = GetAnonymStr(localNetworkId);
    localDeviceUdid = GetAnonymStr(localUdid);
#endif
}

void RadarReportAdapter::GetPeerUdid(const std::string &networkId,
                                     std::string &peerDeviceNetId,
                                     std::string &peerDeviceUdid)
{
#if defined(DFS_ENABLE_DISTRIBUTED_ABILITY) && !defined(DFS_DISABLE_RADAR_ABILITY)
    if (networkId.empty()) {
        peerDeviceUdid = "";
        peerDeviceNetId = "";
        return;
    }
    uint8_t udid[UDID_BUF_LEN] = { 0 };
    int32_t ret = GetNodeKeyInfo(DFS_SERVICE_NAME, networkId.c_str(),
        NodeDeviceInfoKey::NODE_KEY_UDID, udid, sizeof(udid));
    if (ret != E_OK) {
        LOGE("GetNodeKeyInfo failed");
        return;
    }
    std::string peerUdid = reinterpret_cast<char *>(udid);
    peerDeviceNetId = GetAnonymStr(networkId);
    peerDeviceUdid = GetAnonymStr(peerUdid);
#endif
}

std::string RadarReportAdapter::GetAnonymStr(const std::string &value)
{
    const size_t minLength = 10;
    if (value.empty() || value.length() < minLength) {
        return "";
    }
    return value.substr(0, PREFIX_LEN) + "**" + value.substr(value.length() - PREFIX_LEN);
}
} // namespace DistributedFile
}
}