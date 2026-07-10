/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef CLOUD_DISK_SERVICE_IOCTL_H
#define CLOUD_DISK_SERVICE_IOCTL_H

#include <cstdint>
#include <sys/ioctl.h>

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
constexpr unsigned int HMDFS_CLOUD_DISK_SERVICE_IOC = 0xf2;

struct HmdfsPlaceholderAttr {
    uint64_t logicalSize;
    uint64_t atimeMs;
    uint64_t mtimeMs;
};

#define HMDFS_IOC_CREATE_PLACEHOLDER _IOW(HMDFS_CLOUD_DISK_SERVICE_IOC, 0x0d, struct HmdfsPlaceholderAttr)

} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS

#endif // CLOUD_DISK_SERVICE_IOCTL_H
