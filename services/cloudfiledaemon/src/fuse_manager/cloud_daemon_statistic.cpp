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

static const string STAT_DATA_DIR_NAME = "/data/service/el1/public/cloudfile/cloud_data_statistic";
static const string STAT_DATA_FILE_NAME = "cloud_sync_read_file_stat";
static const vector<uint64_t> OPEN_SIZE_RANGE_VECTOR = { 256, 512, 1 * FILE_SIZE_KB_TO_MB, 2 * FILE_SIZE_KB_TO_MB,
    4 * FILE_SIZE_KB_TO_MB, 6 * FILE_SIZE_KB_TO_MB, 8 * FILE_SIZE_KB_TO_MB, 15 * FILE_SIZE_KB_TO_MB };
static const vector<uint64_t> OPEN_TIME_RANGE_VECTOR = { 250, 500, 1000, 1500, 2000, 5000 };
static const vector<uint64_t> READ_SIZE_RANGE_VECTOR = { 128, 256, 512, 1 * FILE_SIZE_KB_TO_MB, 2 * FILE_SIZE_KB_TO_MB,
    4 * FILE_SIZE_KB_TO_MB };
static const vector<uint64_t> READ_TIME_RANGE_VECTOR = { 300, 600, 900, 1200, 1500, 2000, 3000, 5000, 8000 };

static int32_t GetRangeIndex(uint64_t value, const vector<uint64_t> rangeVector)
{
    int32_t index = 0;
    for (; index < rangeVector.size(); index++) {
        if (value < rangeVector[index]) {
            break;
        }
    }
    return index;
}

void CloudDaemonStatistic::UpdateOpenSizeStat(uint64_t size)
{
    uint32_t index = GetRangeIndex(size / FILE_SIZE_BYTE_TO_KB, OPEN_SIZE_RANGE_VECTOR);
    openSizeStat_[index]++;
}

void CloudDaemonStatistic::UpdateOpenTimeStat(uint32_t type, uint64_t time)
{
    uint32_t index = GetRangeIndex(time, OPEN_TIME_RANGE_VECTOR);
    openTimeStat_[type][index]++;
}

void CloudDaemonStatistic::UpdateReadSizeStat(uint64_t size)
{
    uint32_t index = GetRangeIndex(size / FILE_SIZE_BYTE_TO_KB, READ_SIZE_RANGE_VECTOR);
    readSizeStat_[index]++;
}

void CloudDaemonStatistic::UpdateReadTimeStat(uint64_t size, uint64_t time)
{
    uint32_t indexSize = GetRangeIndex(size / FILE_SIZE_BYTE_TO_KB, READ_SIZE_RANGE_VECTOR);
    uint32_t indexTime = GetRangeIndex(time, READ_TIME_RANGE_VECTOR);
    readTimeStat_[indexSize][indexTime]++;
}

void CloudDaemonStatistic::AddFileData()
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
    
    uint64_t tmpData;
    for (uint32_t i = 0; i < OPEN_SIZE_MAX; i++) {
        statDataFile >> tmpData;
        openSizeStat_[i] += tmpData;
    }
    for (uint32_t i = 0; i < FILE_TYPE_MAX; i++) {
        for (uint32_t j = 0; j < OPEN_TIME_MAX; j++) {
            statDataFile >> tmpData;
            openTimeStat_[i][j] += tmpData;
        }
    }
    for (uint32_t i = 0; i < READ_SIZE_MAX; i++) {
        statDataFile >> tmpData;
        readSizeStat_[i] += tmpData;
    }
    for (uint32_t i = 0; i < READ_SIZE_MAX; i++) {
        for (uint32_t j = 0; j < READ_TIME_MAX; j++) {
            statDataFile >> tmpData;
            readTimeStat_[i][j] += tmpData;
        }
    }
    statDataFile.close();
}

void CloudDaemonStatistic::OutputToFile()
{
    string tmpStr = "";

    if (access(STAT_DATA_DIR_NAME.c_str(), F_OK) != 0) {
        auto ret = mkdir(STAT_DATA_DIR_NAME.c_str(), CLOUD_FILE_DIR_MOD);
        if (ret != 0) {
            LOGE("mkdir cloud_data_statistic fail, ret = %{public}d.", ret);
            return;
        }
    }
    string statFilePath = STAT_DATA_DIR_NAME + "/" + STAT_DATA_FILE_NAME;
    if (access(statFilePath.c_str(), F_OK) != 0) {
        auto fd = creat(statFilePath.c_str(), CLOUD_FILE_MOD);
        if (fd < 0) {
            LOGE("create file cloud_sync_read_file_stat fail.");
            return;
        }
        close(fd);
    }
    
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
}

void CloudDaemonStatistic::UpdateStatData()
{
    AddFileData();
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