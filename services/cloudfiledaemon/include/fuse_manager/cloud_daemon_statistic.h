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

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;

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
    mutex mutex_;
private:
    CloudDaemonStatistic() = default;
    ~CloudDaemonStatistic() = default;
    void AddFileData();
    void ClearStat();
    void OutputToFile();
    vector<uint64_t> openSizeStat_ = vector<uint64_t>(OPEN_SIZE_MAX, 0);
    vector<vector<uint64_t>> openTimeStat_ =
        vector<vector<uint64_t>>(FILE_TYPE_MAX, vector<uint64_t>(OPEN_TIME_MAX, 0));
    vector<uint64_t> readSizeStat_ = vector<uint64_t>(READ_SIZE_MAX, 0);
    vector<vector<uint64_t>> readTimeStat_ =
        vector<vector<uint64_t>>(READ_SIZE_MAX, vector<uint64_t>(READ_TIME_MAX, 0));
};
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS

#endif // CLOUD_DAEMON_STATISTIC_H
