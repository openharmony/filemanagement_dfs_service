/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef CLOUD_DAEMON_STATISTIC_H
#define CLOUD_DAEMON_STATISTIC_H

#include <mutex>
#include <string>

#define FILE_TYPE_MAX 3
#define OPEN_SIZE_MAX 9
#define OPEN_TIME_MAX 7
#define READ_SIZE_MAX 6
#define READ_TIME_MAX 10
#define VIDEO_READ_INFO 2

enum VideoReadInfoIndex {
    CACHE_SUM = 0,
    READ_SUM,
};

enum ReadSize {
    READ_SIZE_128K = 0,
    READ_SIZE_256K,
    READ_SIZE_512K,
    READ_SIZE_1M,
    READ_SIZE_2M,
    READ_SIZE_4M
};

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;

struct CloudDaemonStatisticInfo {
public:
    CloudDaemonStatisticInfo() : openSizeStat(OPEN_SIZE_MAX, 0),
        openTimeStat(FILE_TYPE_MAX, vector<uint64_t>(OPEN_TIME_MAX, 0)),
        readSizeStat(READ_SIZE_MAX, 0),
        readTimeStat(READ_SIZE_MAX, vector<uint64_t>(READ_TIME_MAX, 0)),
        videoReadInfo(VIDEO_READ_INFO, 0),
        bundleName("")
    {
    }
    ~CloudDaemonStatisticInfo() = default;

    vector<uint64_t> openSizeStat;
    vector<vector<uint64_t>> openTimeStat;
    vector<uint64_t> readSizeStat;
    vector<vector<uint64_t>> readTimeStat;
    vector<uint32_t> videoReadInfo;
    std::string bundleName;
};

class CloudDaemonStatistic final {
public:
    static CloudDaemonStatistic &GetInstance();
    CloudDaemonStatistic(const CloudDaemonStatistic&) = delete;
    CloudDaemonStatistic& operator=(const CloudDaemonStatistic&) = delete;

    /* size should be BYTE based, time should be MS based */
    void UpdateOpenSizeStat(uint64_t size);
    void UpdateOpenTimeStat(uint32_t type, uint64_t time);
    void UpdateReadSizeStat(uint64_t size);
    void UpdateReadTimeStat(uint64_t size, uint64_t time);
    void UpdateStatData();
    void UpdateReadInfo(uint32_t index);
    void UpdateBundleName(const std::string &bundleName);
    mutex mutex_;
private:
    CloudDaemonStatistic() = default;
    ~CloudDaemonStatistic() = default;
    void AddFileData(CloudDaemonStatisticInfo &info);
    void ClearStat();
    void OutputToFile();
    int32_t CheckFileStat();
    void SumTwoReadStat(const CloudDaemonStatisticInfo &info);
    int32_t ReportReadStat(const CloudDaemonStatisticInfo &info);
    void HandleBundleName(const CloudDaemonStatisticInfo &info);

    vector<uint64_t> openSizeStat_ = vector<uint64_t>(OPEN_SIZE_MAX, 0);
    vector<vector<uint64_t>> openTimeStat_ =
        vector<vector<uint64_t>>(FILE_TYPE_MAX, vector<uint64_t>(OPEN_TIME_MAX, 0));
    vector<uint64_t> readSizeStat_ = vector<uint64_t>(READ_SIZE_MAX, 0);
    vector<vector<uint64_t>> readTimeStat_ =
        vector<vector<uint64_t>>(READ_SIZE_MAX, vector<uint64_t>(READ_TIME_MAX, 0));
    vector<uint32_t> videoReadInfo_ = vector<uint32_t>(VIDEO_READ_INFO, 0);
    std::string bundleName_{""};
};
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS

#endif // CLOUD_DAEMON_STATISTIC_H
