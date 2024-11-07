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
#include "gallery_download_file_stat.h"

#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

#include "dfs_error.h"
#include "hisysevent.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
#define TYPE_DOWNLOAD_FILE_IMAGE 1
#define TYPE_DOWNLOAD_FILE_VIDEO 2
#define DOWNLOAD_FILE_BYTE_SIZE 1e6
#define DIR_MODE 0770
#define FILE_MODE 0660
#define CLOUD_SYNC_SYS_EVENT(eventName, type, ...)    \
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, eventName,    \
                    type, ##__VA_ARGS__)    \

const std::string DOWNLOAD_FILE_STAT_LOCAL_PATH = "/data/service/el1/public/cloudfile/cloud_data_statistic/";
const std::string DOWNLOAD_FILE_STAT_NAME = "cloud_sync_download_file_stat";
const std::vector<uint64_t> DOWNLOAD_IMAGE_SIZE_RANGE_VECTOR = { 2, 4, 6, 8, 15 };
const std::vector<uint64_t> DOWNLOAD_VIDEO_SIZE_RANGE_VECTOR = { 20, 40, 80, 200, 400, 800 };

enum DownloadFileStatIndex {
    DOWNLOAD_FILE_ERROR = 1,
    DOWNLOAD_IMAGE_SIZE,
    DOWNLOAD_IMAGE_SPEED,
    DOWNLOAD_VIDEO_SIZE,
    DOWNLOAD_VIDEO_SPEED,
};

static int32_t CreateDataStatisticFolder()
{
    std::string path = DOWNLOAD_FILE_STAT_LOCAL_PATH;
    if (access(path.c_str(), F_OK) == 0) {
        LOGI("cloud data statistic folder already exists");
        return E_OK;
    }
    int32_t ret = mkdir(path.c_str(), DIR_MODE);
    if (ret != E_OK) {
        LOGE("Create cloud data statistic folder fail errno = %{public}d", errno);
        return E_PATH;
    }
    return E_OK;
}

static int32_t CreateDownloadFileStatData()
{
    const std::string path = DOWNLOAD_FILE_STAT_LOCAL_PATH + DOWNLOAD_FILE_STAT_NAME;
    if (access(path.c_str(), F_OK) == 0) {
        LOGI("download file statistics data file already exists");
        return E_OK;
    }
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, FILE_MODE);
    if (fd == -1) {
        LOGE("create file downloadstatistic report fail, ret = %{public}d", errno);
        return errno;
    }
    close(fd);
    return E_OK;
}

GalleryDownloadFileStat::GalleryDownloadFileStat()
{
    auto ret = CreateDataStatisticFolder();
    if (ret !=E_OK) {
        LOGE("create data statistic failed with error %{public}d", ret);
        return;
    }
    ret = CreateDownloadFileStatData();
    if (ret != E_OK) {
        LOGE("create download file stat failed with error %{public}d", ret);
        return;
    }
}

static uint32_t GetRangeIndex(uint64_t value, const std::vector<uint64_t> rangeVector)
{
    uint32_t index = 0;
    for (; index < rangeVector.size(); index++) {
        if (value <= rangeVector[index] * DOWNLOAD_FILE_BYTE_SIZE) {
            break;
        }
    }
    return index;
}

void GalleryDownloadFileStat::UpdateDownloadSpeedStat(uint32_t mediaType, uint64_t size, uint64_t duration)
{
    double sizeMb = static_cast<double>(size) / DOWNLOAD_FILE_BYTE_SIZE;
    double time = static_cast<double>(duration) / DOWNLOAD_FILE_BYTE_SIZE;

    double precision = 1e-10;
    uint32_t indexSpeed = 15;
    if (abs(time) > precision) {
        double speed = sizeMb / time;
        indexSpeed = static_cast<uint32_t>(floor(speed));
    }
    if (mediaType == TYPE_DOWNLOAD_FILE_IMAGE && indexSpeed < stat_.imageDownloadSpeed.size()) {
        stat_.imageDownloadSpeed[indexSpeed]++;
    }
    if (mediaType == TYPE_DOWNLOAD_FILE_VIDEO && indexSpeed < stat_.videoDownloadSpeed.size()) {
        stat_.videoDownloadSpeed[indexSpeed]++;
    }
}

void GalleryDownloadFileStat::UpdateDownloadSizeStat(uint32_t mediaType, uint64_t size, uint64_t duration)
{
    std::vector<uint64_t> rangeVector;
    if (mediaType == TYPE_DOWNLOAD_FILE_IMAGE) {
        rangeVector = DOWNLOAD_IMAGE_SIZE_RANGE_VECTOR;
        uint32_t index = GetRangeIndex(size, rangeVector);
        if (index >= stat_.imageSize.size()) {
            return;
        }
        stat_.imageSize[index]++;
    } else {
        rangeVector = DOWNLOAD_VIDEO_SIZE_RANGE_VECTOR;
        uint32_t index = GetRangeIndex(size, rangeVector);
        if (index >= stat_.videoSize.size()) {
            return;
        }
        stat_.videoSize[index]++;
    }
}

void GalleryDownloadFileStat::UpdateDownloadStat(uint32_t mediaType, uint64_t size, uint64_t duration)
{
    UpdateDownloadSizeStat(mediaType, size, duration);
    UpdateDownloadSpeedStat(mediaType, size, duration);
}

static std::string VectorToString(std::vector<uint64_t> &vec)
{
    std::ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        oss << vec[i];
        if (i <= vec.size() -1) {
            oss << " ";
        }
    }
    return oss.str();
}

static std::vector<uint64_t> StringToVector(std::string line)
{
    std::vector<uint64_t> vec;
    std::istringstream iss(line);
    uint64_t num;
    while (iss >> num) {
        vec.push_back(num);
    }
    return vec;
}

static inline void SumTwoVector(std::vector<uint64_t> &dataOne, std::vector<uint64_t> dataTwo)
{
    if (dataOne.size() != dataTwo.size()) {
        return;
    }
    for (size_t i = 0; i < dataOne.size(); i++) {
        dataOne[i] += dataTwo[i];
    }
    return;
}

static inline DownloadFileStatInfo SumTwoDownloadFileStat(DownloadFileStatInfo dataOne, DownloadFileStatInfo dataTwo)
{
    SumTwoVector(dataOne.downloadFileError, dataTwo.downloadFileError);
    SumTwoVector(dataOne.imageSize, dataTwo.imageSize);
    SumTwoVector(dataOne.imageDownloadSpeed, dataTwo.imageDownloadSpeed);
    SumTwoVector(dataOne.videoSize, dataTwo.videoSize);
    SumTwoVector(dataOne.videoDownloadSpeed, dataTwo.videoDownloadSpeed);
    return dataOne;
}

void GalleryDownloadFileStat::OutputToFile()
{
    DownloadFileStatInfo tmpInfo = ReadVecFromLocal();
    stat_ = SumTwoDownloadFileStat(stat_, tmpInfo);

    std::vector<std::string> lines;
    /*  Keep code order below */
    lines.emplace_back(stat_.bundleName);
    lines.emplace_back(VectorToString(stat_.downloadFileError));
    lines.emplace_back(VectorToString(stat_.imageSize));
    lines.emplace_back(VectorToString(stat_.imageDownloadSpeed));
    lines.emplace_back(VectorToString(stat_.videoSize));
    lines.emplace_back(VectorToString(stat_.videoDownloadSpeed));

    std::ofstream localData(DOWNLOAD_FILE_STAT_LOCAL_PATH + DOWNLOAD_FILE_STAT_NAME, std::ios::trunc);
    if (!localData.is_open()) {
        LOGE("Open cloud data statistic local data fail %{public}d", errno);
        return;
    }
    for (std::string line : lines) {
        localData << line << std::endl;
    }
    localData.close();
    ClearDownloadFileStat();
    return;
}

DownloadFileStatInfo GalleryDownloadFileStat::ReadVecFromLocal()
{
    std::vector<uint64_t> vec;
    DownloadFileStatInfo tmpStat;
    std::ifstream localData(DOWNLOAD_FILE_STAT_LOCAL_PATH + DOWNLOAD_FILE_STAT_NAME);
    if (localData) {
        uint32_t rowCount = 0;
        std::string line;
        while (std::getline(localData, line)) {
            if (rowCount != 0) {
                vec = StringToVector(line);
            }
            if (rowCount == DOWNLOAD_FILE_ERROR) {
                tmpStat.downloadFileError = vec;
            }
            if (rowCount == DOWNLOAD_IMAGE_SIZE) {
                tmpStat.imageSize = vec;
            }
            if (rowCount == DOWNLOAD_IMAGE_SPEED) {
                tmpStat.imageDownloadSpeed = vec;
            }
            if (rowCount == DOWNLOAD_VIDEO_SIZE) {
                tmpStat.videoSize = vec;
            }
            if (rowCount == DOWNLOAD_VIDEO_SPEED) {
                tmpStat.videoDownloadSpeed = vec;
            }
            rowCount += 1;
        }
        localData.close();
    } else {
        LOGE("Open cloud data statistic local data fail %{public}d", errno);
    }
    return tmpStat;
}

void GalleryDownloadFileStat::ClearDownloadFileStat()
{
    std::fill(stat_.downloadFileError.begin(), stat_.downloadFileError.end(), 0);
    std::fill(stat_.imageSize.begin(), stat_.imageSize.end(), 0);
    std::fill(stat_.imageDownloadSpeed.begin(), stat_.imageDownloadSpeed.end(), 0);
    std::fill(stat_.videoSize.begin(), stat_.videoSize.end(), 0);
    std::fill(stat_.videoDownloadSpeed.begin(), stat_.videoDownloadSpeed.end(), 0);
}

void GalleryDownloadFileStat::Report()
{
    const std::string path = DOWNLOAD_FILE_STAT_LOCAL_PATH + DOWNLOAD_FILE_STAT_NAME;
    if (access(path.c_str(), F_OK) == -1) {
        LOGE("download file statistics data file not exists");
        return;
    }

    /* read stat from dist */
    stat_ = ReadVecFromLocal();

    int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_SYNC_DOWNLOAD_FILE_STAT",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "bundle_name", stat_.bundleName,
        "download_file", stat_.downloadFileError,
        "image_size", stat_.imageSize,
        "image_download_speed", stat_.imageDownloadSpeed,
        "video_size", stat_.videoSize,
        "video_download_speed", stat_.videoDownloadSpeed);
    if (ret != E_OK) {
        LOGE("report CLOUD_SYNC_DOWNLOAD_FILE_STAT error %{public}d", ret);
    }
    ret = unlink(path.c_str());
    if (ret != 0) {
        LOGE("fail to delete local statistic data, errno %{public}d", errno);
    }
}

GalleryDownloadFileStat &GalleryDownloadFileStat::GetInstance()
{
    static GalleryDownloadFileStat downloadStat_;
    return downloadStat_;
}
} // CloudFile
} // FileManagement
} // OHOS