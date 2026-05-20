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
#include "cloud_disk_download_file_stat.h"
 
#include <fcntl.h>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
 
#include "dfs_error.h"
#include "hisysevent.h"
#include "utils_log.h"
#include "parameter.h"
 
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
 
static const std::string FILEMANAGER_KEY = "persist.kernel.bundle_name.filemanager";
const std::string DOWNLOAD_FILE_STAT_LOCAL_PATH = "/data/service/el1/public/cloudfile/cloud_data_statistic";
const std::string DOWNLOAD_FILE_STAT_NAME = "/cloud_disk_sync_download_file_stat";
const std::vector<uint64_t> DOWNLOAD_FILE_SIZE_RANGE_VECTOR = { 2, 4, 6, 8, 15 };
 
enum DownloadFileStatIndex {
    DOWNLOAD_BUNDLE_NAME = 0,
    DOWNLOAD_FILE_ERROR,
    DOWNLOAD_FILE_SIZE,
    DOWNLOAD_FILE_SPEED,
};
 
std::string CloudDiskDownloadFileStat::GetLocalPath()
{
    return DOWNLOAD_FILE_STAT_LOCAL_PATH + "/" + bundleName_;
}
 
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
 
int32_t CloudDiskDownloadFileStat::CreateDownloadFileStatData()
{
    std::string localPath = GetLocalPath();
    if (access(localPath.c_str(), F_OK) != 0) {
        mkdir(localPath.c_str(), DIR_MODE);
    }
    const std::string path = localPath + DOWNLOAD_FILE_STAT_NAME;
    if (access(path.c_str(), F_OK) == 0) {
        LOGI("download file statistics data file already exists");
        return E_OK;
    }
    int fd = creat(path.c_str(), FILE_MODE);
    if (fd < 0) {
        LOGE("create file downloadstatistic report fail, ret = %{public}d", errno);
        return errno;
    }
    if (close(fd)) {
        LOGE("close file downloadstatistic report fail, ret = %{public}d", errno);
        return errno;
    }
    return E_OK;
}
 
CloudDiskDownloadFileStat::CloudDiskDownloadFileStat()
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
 
void CloudDiskDownloadFileStat::UpdateDownloadSpeedStat(uint64_t size, uint64_t duration)
{
    double sizeMb = static_cast<double>(size) / DOWNLOAD_FILE_BYTE_SIZE;
    double time = static_cast<double>(duration) / DOWNLOAD_FILE_BYTE_SIZE;
 
    double precision = 1e-10;
    uint32_t indexSpeed = 15;
    if (abs(time) > precision) {
        double speed = sizeMb / time;
        indexSpeed = static_cast<uint32_t>(floor(speed));
    }
    if (indexSpeed < stat_.fileDownloadSpeed.size()) {
        stat_.fileDownloadSpeed[indexSpeed]++;
    }
}
 
void CloudDiskDownloadFileStat::UpdateDownloadSizeStat(uint64_t size, uint64_t duration)
{
    uint32_t index = GetRangeIndex(size, DOWNLOAD_FILE_SIZE_RANGE_VECTOR);
    if (index >= stat_.fileSize.size()) {
        return;
    }
    stat_.fileSize[index]++;
}
 
void CloudDiskDownloadFileStat::UpdateDownloadStat(uint64_t size, uint64_t duration)
{
    UpdateDownloadSizeStat(size, duration);
    UpdateDownloadSpeedStat(size, duration);
}
 
void CloudDiskDownloadFileStat::UpdateDownloadBundleName(const std::string &bundleName)
{
    std::unique_lock<std::mutex> lock(mutex_);
    stat_.bundleName = bundleName;
    bundleName_ = bundleName;
}
 
static std::string VectorToString(const std::vector<uint64_t> &vec)
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
 
static inline CloudDiskDownloadFileStatInfo SumTwoDownloadFileStat(CloudDiskDownloadFileStatInfo dataOne,
    CloudDiskDownloadFileStatInfo dataTwo)
{
    SumTwoVector(dataOne.downloadFileError, dataTwo.downloadFileError);
    SumTwoVector(dataOne.fileSize, dataTwo.fileSize);
    SumTwoVector(dataOne.fileDownloadSpeed, dataTwo.fileDownloadSpeed);
    if (dataOne.bundleName.empty()) {
        dataOne.bundleName = dataTwo.bundleName;
    }
    return dataOne;
}
 
void CloudDiskDownloadFileStat::HandleBundleName(const CloudDiskDownloadFileStatInfo &info)
{
    if (!info.bundleName.empty() && info.bundleName != stat_.bundleName &&
        !stat_.bundleName.empty()) {
        auto ret = ReportDownloadFileStat(info);
        if (ret != E_OK) {
            LOGE("report CLOUD_SYNC_DOWNLOAD_FILE_STAT error %{public}d", ret);
        }
    } else {
        stat_ = SumTwoDownloadFileStat(stat_, info);
    }
}
 
void CloudDiskDownloadFileStat::OutputToFile()
{
    std::lock_guard<std::mutex> lck(mutex_);
    std::string localPath = GetLocalPath();
    if (access(localPath.c_str(), F_OK) != 0) {
        mkdir(localPath.c_str(), DIR_MODE);
    }
    CloudDiskDownloadFileStatInfo tmpInfo = ReadVecFromLocal();
    HandleBundleName(tmpInfo);
 
    std::vector<std::string> lines;
    /*  Keep code order below */
    lines.emplace_back(stat_.bundleName);
    lines.emplace_back(VectorToString(stat_.downloadFileError));
    lines.emplace_back(VectorToString(stat_.fileSize));
    lines.emplace_back(VectorToString(stat_.fileDownloadSpeed));
    
    std::ofstream localData(localPath + DOWNLOAD_FILE_STAT_NAME, std::ios::trunc);
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
 
CloudDiskDownloadFileStatInfo CloudDiskDownloadFileStat::ReadVecFromLocal()
{
    std::vector<uint64_t> vec;
    CloudDiskDownloadFileStatInfo tmpStat;
    std::string localPath = GetLocalPath();
    std::ifstream localData(localPath + DOWNLOAD_FILE_STAT_NAME);
    if (localData) {
        uint32_t rowCount = 0;
        std::string line;
        while (std::getline(localData, line)) {
            if (rowCount != 0) {
                vec = StringToVector(line);
            }
            if (rowCount == DOWNLOAD_BUNDLE_NAME) {
                tmpStat.bundleName = line;
            }
            if (rowCount == DOWNLOAD_FILE_ERROR) {
                tmpStat.downloadFileError = vec;
            }
            if (rowCount == DOWNLOAD_FILE_SIZE) {
                tmpStat.fileSize = vec;
            }
            if (rowCount == DOWNLOAD_FILE_SPEED) {
                tmpStat.fileDownloadSpeed = vec;
            }
            rowCount += 1;
        }
        localData.close();
    } else {
        LOGE("Open cloud data statistic local data fail %{public}d", errno);
    }
    return tmpStat;
}
 
void CloudDiskDownloadFileStat::ClearDownloadFileStat()
{
    std::fill(stat_.downloadFileError.begin(), stat_.downloadFileError.end(), 0);
    std::fill(stat_.fileSize.begin(), stat_.fileSize.end(), 0);
    std::fill(stat_.fileDownloadSpeed.begin(), stat_.fileDownloadSpeed.end(), 0);
}
 
int32_t CloudDiskDownloadFileStat::ReportDownloadFileStat(const CloudDiskDownloadFileStatInfo &info)
{
    int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_SYNC_DOWNLOAD_FILE_STAT",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "bundle_name", info.bundleName,
        "download_file", info.downloadFileError,
        "file_size", info.fileSize,
        "file_download_speed", info.fileDownloadSpeed);
    return ret;
}
 
void CloudDiskDownloadFileStat::Report()
{
    std::lock_guard<std::mutex> lck(mutex_);
    std::string localPath = GetLocalPath();
    const std::string path = localPath + DOWNLOAD_FILE_STAT_NAME;
    if (access(path.c_str(), F_OK) == -1) {
        LOGE("download file statistics data file not exists");
        return;
    }
 
    /* read stat from dist */
    stat_ = ReadVecFromLocal();
 
    int32_t ret = ReportDownloadFileStat(stat_);
    if (ret != E_OK) {
        LOGE("report CLOUD_SYNC_DOWNLOAD_FILE_STAT error %{public}d", ret);
    }
    ret = unlink(path.c_str());
    if (ret != 0) {
        LOGE("fail to delete local statistic data, errno %{public}d", errno);
    }
}
 
CloudDiskDownloadFileStat &CloudDiskDownloadFileStat::GetInstance()
{
    static CloudDiskDownloadFileStat downloadStat_;
    return downloadStat_;
}
 
} // CloudFile
} // FileManagement
} // OHOS