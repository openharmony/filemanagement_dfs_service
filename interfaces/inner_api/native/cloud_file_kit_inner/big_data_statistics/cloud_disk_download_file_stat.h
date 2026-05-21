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
#ifndef OHOS_CLOUD_FILE_SERVICE_CLOUD_DISK_DOWNLOAD_FILE_STAT_H
#define OHOS_CLOUD_FILE_SERVICE_CLOUD_DISK_DOWNLOAD_FILE_STAT_H
#include <shared_mutex>
#include <string>
#include <vector>
 
#include "nocopyable.h"
#include "hisysevent.h"
 
namespace OHOS {
namespace FileManagement {
namespace CloudFile {
#define CLOUD_DISK_DOWNLOAD_ERROR_STAT_LEN 10
#define CLOUD_DISK_DOWNLOAD_FILE_SIZE_LEN 6
#define CLOUD_DISK_DOWNLOAD_FILE_SPEED_LEN 16
struct CloudDiskDownloadFileStatInfo {
public:
    CloudDiskDownloadFileStatInfo() : downloadFileError(CLOUD_DISK_DOWNLOAD_ERROR_STAT_LEN),
        fileSize(CLOUD_DISK_DOWNLOAD_FILE_SIZE_LEN), fileDownloadSpeed(CLOUD_DISK_DOWNLOAD_FILE_SPEED_LEN)
    {
    }
    ~CloudDiskDownloadFileStatInfo() = default;
 
    std::string bundleName;
    std::vector<uint64_t> downloadFileError;
    std::vector<uint64_t> fileSize;
    std::vector<uint64_t> fileDownloadSpeed;
};
 
class CloudDiskDownloadFileStat : public NoCopyable {
public:
    static CloudDiskDownloadFileStat &GetInstance();
    CloudDiskDownloadFileStat(const CloudDiskDownloadFileStat&) = delete;
    CloudDiskDownloadFileStat& operator=(const CloudDiskDownloadFileStat&) = delete;
 
    void UpdateDownloadStat(uint64_t size, uint64_t duration);
    void UpdateDownloadBundleName(const std::string &bundleName);
    void OutputToFile();
    void Report();
private:
    CloudDiskDownloadFileStat();
    ~CloudDiskDownloadFileStat() = default;
    void ClearDownloadFileStat();
    void UpdateDownloadSpeedStat(uint64_t size, uint64_t duration);
    void UpdateDownloadSizeStat(uint64_t size, uint64_t duration);
    CloudDiskDownloadFileStatInfo ReadVecFromLocal();
    int32_t ReportDownloadFileStat(const CloudDiskDownloadFileStatInfo &info);
    void HandleBundleName(const CloudDiskDownloadFileStatInfo &info);
    std::string GetLocalPath();
    int32_t CreateDownloadFileStatData();
 
    CloudDiskDownloadFileStatInfo stat_;
    std::mutex mutex_;
    std::string bundleName_;
};
} // CloudFile
} // FileManagement
} // OHOS
#endif // OHOS_CLOUD_FILE_SERVICE_GALLERY_DOWNLOAD_FILE_STAT_H