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

#ifndef RADAR_REPORT_H
#define RADAR_REPORT_H

#include <string>
#include <thread>

#include "dfs_radar.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace FileManagement;

class RadarReportAdapter {
public:
    static RadarReportAdapter &GetInstance()
    {
        static RadarReportAdapter instance;
        return instance;
    }

public:
    void InitRadar();
    void UnInitRadar();
    void SetUserStatistics(const RadarStatisticInfoType type);
    void ReportDfxStatistics();

private:
    void StorageRadarThd();
 
    std::atomic<bool> stopRadarReport_{false};
    std::chrono::time_point<std::chrono::system_clock> lastRadarReportTime_;
    RadarStatisticInfo opStatistics_;
    std::thread callRadarStatisticReportThread_;
    std::mutex onRadarReportLock_;

    RadarReportAdapter() = default;
    ~RadarReportAdapter() = default;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // RADAR_REPORT_H
