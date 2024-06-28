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
#ifndef OHOS_CLOUD_FILE_SERVICE_GALLERY_DOWNLOAD_FILE_STAT_H
#define OHOS_CLOUD_FILE_SERVICE_GALLERY_DOWNLOAD_FILE_STAT_H
#include <string>
#include <vector>

#include "nocopyable.h"
#include "hisysevent.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
#define GALLERY_DOWNLOAD_ERROR_STAT_LEN 10
#define GALLERY_DOWNLOAD_IMAGE_SIZE_LEN 6
#define GALLERY_DOWNLOAD_IMAGE_SPEED_LEN 16
#define GALLERY_DOWNLOAD_VIDEO_SIZE_LEN 7
#define GALLERY_DOWNLOAD_VIDEO_SPEED_LEN 16
struct DownloadFileStatInfo {
public:
    DownloadFileStatInfo() : downloadFileError(GALLERY_DOWNLOAD_ERROR_STAT_LEN),
                             imageSize(GALLERY_DOWNLOAD_IMAGE_SIZE_LEN),
                             imageDownloadSpeed(GALLERY_DOWNLOAD_IMAGE_SPEED_LEN),
                             videoSize(GALLERY_DOWNLOAD_VIDEO_SIZE_LEN),
                             videoDownloadSpeed(GALLERY_DOWNLOAD_VIDEO_SPEED_LEN)
    {
    }
    ~DownloadFileStatInfo() = default;

    std::string bundleName;
    std::vector<uint64_t> downloadFileError;
    std::vector<uint64_t> imageSize;
    std::vector<uint64_t> imageDownloadSpeed;
    std::vector<uint64_t> videoSize;
    std::vector<uint64_t> videoDownloadSpeed;
};

class GalleryDownloadFileStat : public NoCopyable {
public:
    static GalleryDownloadFileStat &GetInstance();
    GalleryDownloadFileStat(const GalleryDownloadFileStat&) = delete;
    GalleryDownloadFileStat& operator=(const GalleryDownloadFileStat&) = delete;

    void UpdateDownloadStat(uint32_t mediaType, uint64_t size, uint64_t duration);
    void OutputToFile();
    void Report();
private:
    GalleryDownloadFileStat();
    ~GalleryDownloadFileStat() = default;
    void ClearDownloadFileStat();
    void UpdateDownloadSpeedStat(uint32_t mediaType, uint64_t size, uint64_t duration);
    void UpdateDownloadSizeStat(uint32_t mediaType, uint64_t size, uint64_t duration);
    DownloadFileStatInfo ReadVecFromLocal();

    DownloadFileStatInfo stat_;
};
} // CloudFile
} // FileManagement
} // OHOS
#endif // OHOS_CLOUD_FILE_SERVICE_GALLERY_DOWNLOAD_FILE_STAT_H