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

#ifndef FUSE_IOCTL_H
#define FUSE_IOCTL_H

#include <sys/ioctl.h>

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
static const unsigned int HMDFS_IOC = 0xf2;
static const unsigned int HMDFS_DAEMON_IOC_MASK = 0x80;

#define HMDFS_IOC_GET_WRITEOPEN_CNT _IOR(HMDFS_IOC, 0x02, uint32_t)
#define HMDFS_IOC_HAS_CACHE _IOW(HMDFS_IOC, 0x06, struct HmdfsHasCache)
#define HMDFS_IOC_CANCEL_READ _IO(HMDFS_IOC, 0x08)
#define HMDFS_IOC_RESET_READ _IO(HMDFS_IOC, 0x09)
#define HMDFS_IOC_SET_CLOUD_GENERATION _IOR(HMDFS_IOC, 0x0b, uint32_t)
#define HMDFS_IOC_COPY_FILE _IOW(HMDFS_IOC, 0x0c, struct CloudDiskCopy)

/* New daemon-specific ioctls must be defined with the HMDFS_DAEMON_IOC_MASK for proper handling */
#define HMDFS_IOC_CLEAN_CACHE_DAEMON _IO(HMDFS_IOC, HMDFS_DAEMON_IOC_MASK + 0x01)

} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS
#endif // FUSE_IOCTL_H
