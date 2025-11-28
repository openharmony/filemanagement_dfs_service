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
#include "utils_log.h"

#include <unistd.h>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
constexpr unsigned int RADAR_REPORT_STATISTIC_INTERVAL_MINUTES = 360;
constexpr unsigned int UPDATE_RADAR_STATISTIC_INTERVAL_SECONDS = 300;
} // namespace

void RadarReportAdapter::InitRadar()
{
    opStatistics_ = {0, 0, 0, 0, 0, 0};
    callRadarStatisticReportThread_ = std::thread([this]() { StorageRadarThd(); });
    if (!callRadarStatisticReportThread_.joinable()) {
        return;
    }
    lastRadarReportTime_ = std::chrono::system_clock::now();
}

void RadarReportAdapter::UnInitRadar()
{
    std::unique_lock<std::mutex> lock(onRadarReportLock_);
    stopRadarReport_ = true;
    if (callRadarStatisticReportThread_.joinable()) {
        callRadarStatisticReportThread_.join();
    }
}

void RadarReportAdapter::StorageRadarThd()
{
    while (!stopRadarReport_.load()) {
        std::unique_lock<std::mutex> lock(onRadarReportLock_);
        sleep(UPDATE_RADAR_STATISTIC_INTERVAL_SECONDS);
        std::chrono::time_point<std::chrono::system_clock> nowTime = std::chrono::system_clock::now();
        int64_t intervalMinutes =
            std::chrono::duration_cast<std::chrono::minutes>(nowTime - lastRadarReportTime_).count();
        if (intervalMinutes > RADAR_REPORT_STATISTIC_INTERVAL_MINUTES) {
            ReportDfxStatistics();
        }
    }
}

void RadarReportAdapter::SetUserStatistics(const RadarStatisticInfoType type)
{
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
}
}
}