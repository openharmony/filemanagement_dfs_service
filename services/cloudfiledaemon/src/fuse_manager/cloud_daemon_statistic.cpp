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

#include "cloud_daemon_statistic.h"

#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

#include "dfs_error.h"
#include "meta_file.h"
#include "hisysevent.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;

#define FILE_SIZE_BYTE_TO_KB 1024
#define FILE_SIZE_KB_TO_MB 1024
#define STAT_TIME_MS_TO_S 1000
#define CLOUD_FILE_DIR_MOD 0770
#define CLOUD_FILE_MOD 0660
#define CLOUD_SYNC_SYS_EVENT(eventName, type, ...)    \
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, eventName,    \
                    type, ##__VA_ARGS__)    \

static const string STAT_DATA_DIR_NAME = "/data/service/el1/public/cloudfile/cloud_data_statistic";
static const string STAT_DATA_FILE_NAME = "cloud_sync_read_file_stat";
static const vector<uint64_t> OPEN_SIZE_RANGE_VECTOR = { 256, 512, 1 * FILE_SIZE_KB_TO_MB, 2 * FILE_SIZE_KB_TO_MB,
    4 * FILE_SIZE_KB_TO_MB, 6 * FILE_SIZE_KB_TO_MB, 8 * FILE_SIZE_KB_TO_MB, 15 * FILE_SIZE_KB_TO_MB };
static const vector<uint64_t> OPEN_TIME_RANGE_VECTOR = { 250, 500, 1000, 1500, 2000, 5000 };
static const vector<uint64_t> READ_SIZE_RANGE_VECTOR = { 128, 256, 512, 1 * FILE_SIZE_KB_TO_MB, 2 * FILE_SIZE_KB_TO_MB,
    4 * FILE_SIZE_KB_TO_MB };
static const vector<uint64_t> READ_TIME_RANGE_VECTOR = { 300, 600, 900, 1200, 1500, 2000, 3000, 5000, 8000 };

static uint32_t GetRangeIndex(uint64_t value, const vector<uint64_t> rangeVector)
{
    uint32_t index = 0;
    for (; index < rangeVector.size(); index++) {
        if (value <= rangeVector[index]) {
            break;
        }
    }
    return index;
}

static void CheckOverflow(uint64_t &data, uint64_t addValue)
{
    if (data >= UINT64_MAX - addValue) {
        LOGE("update fail, overflow, data = %{public}llu, addValue = %{public}llu",
            static_cast<unsigned long long>(data),
            static_cast<unsigned long long>(addValue));
        data = 0;
    } else {
        data += addValue;
    }
}

static void CheckOverflow(uint32_t &data, uint32_t addValue)
{
    if (data > UINT32_MAX - addValue) {
        LOGE("update fail, overflow, data = %{public}d, addValue = %{public}d",
            data, addValue);
        data = UINT32_MAX;
    } else {
        data += addValue;
    }
}

void CloudDaemonStatistic::UpdateOpenSizeStat(uint64_t size)
{
    uint32_t index = GetRangeIndex(size / FILE_SIZE_BYTE_TO_KB, OPEN_SIZE_RANGE_VECTOR);
    if (index >= OPEN_SIZE_MAX) {
        LOGE("update open size stat fail, index overflow, index = %{public}u.", index);
        return;
    }
    CheckOverflow(openSizeStat_[index], 1);
}

void CloudDaemonStatistic::UpdateOpenTimeStat(uint32_t type, uint64_t time)
{
    uint32_t index = GetRangeIndex(time, OPEN_TIME_RANGE_VECTOR);
    if (index >= OPEN_TIME_MAX) {
        LOGE("update open time stat fail, index overflow, index = %{public}u.", index);
        return;
    }
    CheckOverflow(openTimeStat_[type][index], 1);
}

void CloudDaemonStatistic::UpdateReadSizeStat(uint64_t size)
{
    uint32_t index = GetRangeIndex(size / FILE_SIZE_BYTE_TO_KB, READ_SIZE_RANGE_VECTOR);
    if (index >= READ_SIZE_MAX) {
        LOGE("update read size stat fail, index overflow, index = %{public}u.", index);
        return;
    }
    CheckOverflow(readSizeStat_[index], 1);
}

void CloudDaemonStatistic::UpdateReadTimeStat(uint64_t size, uint64_t time)
{
    uint32_t indexSize = GetRangeIndex(size / FILE_SIZE_BYTE_TO_KB, READ_SIZE_RANGE_VECTOR);
    uint32_t indexTime = GetRangeIndex(time, READ_TIME_RANGE_VECTOR);
    if (indexSize >= READ_SIZE_MAX || indexTime >= READ_TIME_MAX) {
        LOGE("update read time stat fail, index overflow, indexSize = %{public}u, indexTime = %{public}u.",
            indexSize, indexTime);
        return;
    }
    CheckOverflow(readTimeStat_[indexSize][indexTime], 1);
}

void CloudDaemonStatistic::UpdateReadInfo(uint32_t index)
{
    if (index >= VIDEO_READ_INFO) {
        return;
    }
    CheckOverflow(videoReadInfo_[index], 1);
}

void CloudDaemonStatistic::UpdateBundleName(const std::string &bundleName)
{
    bundleName_ = bundleName;
}

void CloudDaemonStatistic::AddFileData(CloudDaemonStatisticInfo &info)
{
    /* file not exist means first time, no former data, normal case */
    auto ret = access(STAT_DATA_DIR_NAME.c_str(), F_OK);
    if (ret != 0) {
        LOGI("dir cloud_data_statistic not exist, ret = %{public}d.", ret);
        return;
    }
    std::ifstream statDataFile(STAT_DATA_DIR_NAME + "/" + STAT_DATA_FILE_NAME);
    if (!statDataFile) {
        LOGI("file cloud_sync_read_file_stat not exist.");
        return;
    }
    
    for (uint32_t i = 0; i < OPEN_SIZE_MAX; i++) {
        statDataFile >> info.openSizeStat[i];
    }
    for (uint32_t i = 0; i < FILE_TYPE_MAX; i++) {
        for (uint32_t j = 0; j < OPEN_TIME_MAX; j++) {
            statDataFile >> info.openTimeStat[i][j];
        }
    }
    for (uint32_t i = 0; i < READ_SIZE_MAX; i++) {
        statDataFile >> info.readSizeStat[i];
    }
    for (uint32_t i = 0; i < READ_SIZE_MAX; i++) {
        for (uint32_t j = 0; j < READ_TIME_MAX; j++) {
            statDataFile >> info.readTimeStat[i][j];
        }
    }
    for (uint32_t i = 0; i < VIDEO_READ_INFO; i++) {
        statDataFile >> info.videoReadInfo[i];
    }
    statDataFile >> info.bundleName;
    statDataFile.close();
}

int32_t CloudDaemonStatistic::CheckFileStat()
{
    if (access(STAT_DATA_DIR_NAME.c_str(), F_OK) != 0) {
        auto ret = mkdir(STAT_DATA_DIR_NAME.c_str(), CLOUD_FILE_DIR_MOD);
        if (ret != 0) {
            LOGE("mkdir cloud_data_statistic fail, ret = %{public}d.", ret);
            return ret;
        }
    }
    string statFilePath = STAT_DATA_DIR_NAME + "/" + STAT_DATA_FILE_NAME;
    if (access(statFilePath.c_str(), F_OK) != 0) {
        auto fd = creat(statFilePath.c_str(), CLOUD_FILE_MOD);
        if (fd < 0) {
            LOGE("create file cloud_sync_read_file_stat fail.");
            return E_NO_SUCH_FILE;
        }
        close(fd);
    }
    return E_OK;
}

void CloudDaemonStatistic::OutputToFile()
{
    string tmpStr = "";

    int32_t ret = CheckFileStat();
    if (ret != E_OK) {
        return;
    }
    string statFilePath = STAT_DATA_DIR_NAME + "/" + STAT_DATA_FILE_NAME;
    std::ofstream statDataFile(statFilePath);
    if (!statDataFile) {
        LOGE("open out stream file cloud_sync_read_file_stat fail.");
        return;
    }
    for (uint32_t i = 0; i < OPEN_SIZE_MAX; i++) {
        tmpStr += (to_string(openSizeStat_[i]) + " ");
    }
    statDataFile << tmpStr << endl << endl;
    tmpStr = "";
    for (uint32_t i = 0; i < FILE_TYPE_MAX; i++) {
        for (uint32_t j = 0; j < OPEN_TIME_MAX; j++) {
            tmpStr += (to_string(openTimeStat_[i][j]) + " ");
        }
        tmpStr += "\n";
    }
    statDataFile << tmpStr << endl;
    tmpStr = "";
    for (uint32_t i = 0; i < READ_SIZE_MAX; i++) {
        tmpStr += (to_string(readSizeStat_[i]) + " ");
    }
    statDataFile << tmpStr << endl << endl;
    tmpStr = "";
    for (uint32_t i = 0; i < READ_SIZE_MAX; i++) {
        for (uint32_t j = 0; j < READ_TIME_MAX; j++) {
            tmpStr += (to_string(readTimeStat_[i][j]) + " ");
        }
        tmpStr += "\n";
    }
    statDataFile << tmpStr << endl;
    tmpStr = "";
    for (uint32_t i = 0; i < VIDEO_READ_INFO; i++) {
        tmpStr += (to_string(videoReadInfo_[i]) + " ");
    }
    statDataFile << tmpStr << endl;
    statDataFile << bundleName_ << endl;
    statDataFile.close();
}

void CloudDaemonStatistic::ClearStat()
{
    for (uint32_t i = 0; i < OPEN_SIZE_MAX; i++) {
        openSizeStat_[i] = 0;
    }
    for (uint32_t i = 0; i < FILE_TYPE_MAX; i++) {
        for (uint32_t j = 0; j < OPEN_TIME_MAX; j++) {
            openTimeStat_[i][j] = 0;
        }
    }
    for (uint32_t i = 0; i < READ_SIZE_MAX; i++) {
        readSizeStat_[i] = 0;
    }
    for (uint32_t i = 0; i < READ_SIZE_MAX; i++) {
        for (uint32_t j = 0; j < READ_TIME_MAX; j++) {
            readTimeStat_[i][j] = 0;
        }
    }
    for (uint32_t i = 0; i < VIDEO_READ_INFO; i++) {
        videoReadInfo_[i] = 0;
    }
}

void CloudDaemonStatistic::SumTwoReadStat(const CloudDaemonStatisticInfo &info)
{
    for (uint32_t i = 0; i < OPEN_SIZE_MAX; i++) {
        CheckOverflow(openSizeStat_[i], info.openSizeStat[i]);
    }
    for (uint32_t i = 0; i < FILE_TYPE_MAX; i++) {
        for (uint32_t j = 0; j < OPEN_TIME_MAX; j++) {
            CheckOverflow(openTimeStat_[i][j], info.openTimeStat[i][j]);
        }
    }
    for (uint32_t i = 0; i < READ_SIZE_MAX; i++) {
        CheckOverflow(readSizeStat_[i], info.readSizeStat[i]);
    }
    for (uint32_t i = 0; i < READ_SIZE_MAX; i++) {
        for (uint32_t j = 0; j < READ_TIME_MAX; j++) {
            CheckOverflow(readTimeStat_[i][j], info.readTimeStat[i][j]);
        }
    }
    for (uint32_t i = 0; i < VIDEO_READ_INFO; i++) {
        CheckOverflow(videoReadInfo_[i], info.videoReadInfo[i]);
    }
}

int32_t CloudDaemonStatistic::ReportReadStat(const CloudDaemonStatisticInfo &info)
{
    int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_SYNC_READ_FILE_STAT",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "open_size", info.openSizeStat,
        "open_thm_time", info.openTimeStat[FILE_TYPE_THUMBNAIL],
        "open_lcd_time", info.openTimeStat[FILE_TYPE_LCD],
        "open_content_time", info.openTimeStat[FILE_TYPE_CONTENT],
        "read_size", info.readSizeStat,
        "read_time_128KB", info.readTimeStat[READ_SIZE_128K],
        "read_time_256KB", info.readTimeStat[READ_SIZE_256K],
        "read_time_512KB", info.readTimeStat[READ_SIZE_512K],
        "read_time_1M", info.readTimeStat[READ_SIZE_1M],
        "read_time_2M", info.readTimeStat[READ_SIZE_2M],
        "read_time_4M", info.readTimeStat[READ_SIZE_4M],
        "video_read_info", info.videoReadInfo,
        "bundle_name", info.bundleName
    );
    return ret;
}

void CloudDaemonStatistic::HandleBundleName(const CloudDaemonStatisticInfo &info)
{
    if (!info.bundleName.empty() && info.bundleName != bundleName_) {
        auto ret = ReportReadStat(info);
        if (ret != E_OK) {
            LOGE("report CLOUD_SYNC_READ_FILE_STAT error %{public}d", ret);
        }
    } else {
        SumTwoReadStat(info);
    }
}

void CloudDaemonStatistic::UpdateStatData()
{
    lock_guard<mutex> lock(mutex_);
    CloudDaemonStatisticInfo info;
    AddFileData(info);
    HandleBundleName(info);
    OutputToFile();
    ClearStat();
}

CloudDaemonStatistic &CloudDaemonStatistic::GetInstance()
{
    static CloudDaemonStatistic instance_;
    return instance_;
}

} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS