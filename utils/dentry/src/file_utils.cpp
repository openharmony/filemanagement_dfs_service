/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "file_utils.h"

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>

#include "dfs_error.h"
#include "utils_log.h"
#include "xcollie_helper.h"

namespace OHOS {
namespace FileManagement {
int64_t FileUtils::ReadFile(int fd, off_t offset, size_t size, void *data)
{
    if ((fd < 0) || (offset < 0) || (size < 0) || (data == nullptr)) {
        LOGE("invalid params, fd %{public}d, offset %{public}d, size %{public}zu, or buf is null", fd,
             static_cast<int>(offset), size);
        return -1;
    }

    off_t err = lseek(fd, offset, SEEK_SET);
    if (err < 0) {
        return -errno;
    }

    size_t readLen = 0;
    while (readLen < size) {
        ssize_t ret = read(fd, data, size - readLen);
        if (ret < 0) {
            LOGE("read failed, errno %{public}d, fd=%{public}d", errno, fd);
            return ret;
        } else if (ret == 0) {
            break;
        }
        readLen += ret;
    }

    return readLen;
}

int64_t FileUtils::WriteFile(int fd, const void *data, off_t offset, size_t size)
{
    if ((fd < 0) || (offset < 0) || (size < 0) || (data == nullptr)) {
        LOGE("invalid params, fd %{public}d, offset %{public}d, size %{public}zu, or buf is null", fd,
             static_cast<int>(offset), size);
        return -1;
    }

    off_t err = lseek(fd, offset, SEEK_SET);
    if (err < 0) {
        return -errno;
    }

    size_t writeLen = 0;
    while (writeLen < size) {
        ssize_t ret = write(fd, data, size - writeLen);
        if (ret <= 0) {
            LOGE("write failed, errno %{public}d, fd=%{public}d", errno, fd);
            return ret;
        }
        writeLen += ret;
    }

    return writeLen;
}

FileRangeLock::FileRangeLock(int fd, off_t offset, size_t size) : fd_(fd), offset_(offset), size_(size)
{
    const int DFX_DELAY_S = 5;
    xcollieId_ = XCollieHelper::SetTimer("CloudSyncService_RangeLockTask", DFX_DELAY_S, nullptr, nullptr, true);
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = offset;
    fl.l_len = static_cast<decltype(fl.l_len)>(size);
    if (fcntl(fd, F_SETLKW, &fl) < 0) {
        LOGE("fcntl set F_WRLCK failed: %{public}d", errno);
        lockFailed_ = true;
    }
}

FileRangeLock::~FileRangeLock()
{
    if (lockFailed_) {
        XCollieHelper::CancelTimer(xcollieId_);
        return;
    }

    struct flock fl;
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = offset_;
    fl.l_len = static_cast<decltype(fl.l_len)>(size_);
    if (fcntl(fd_, F_SETLKW, &fl) < 0) {
        LOGE("fcntl F_UNLCK failed: %{public}d", errno);
    }
    XCollieHelper::CancelTimer(xcollieId_);
}

} // namespace FileManagement
} // namespace OHOS
