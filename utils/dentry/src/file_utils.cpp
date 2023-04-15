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

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {

int32_t FileUtils::Stat(int fd, struct stat &s)
{
    if (fd < 0) {
        LOGE("invalid fd %{public}d", fd);
        return EBADF;
    }

    if (fstat(fd, &s) != 0) {
        LOGE("errno %{public}d, fd=%{public}d", errno, fd);
        return errno;
    }

    return E_OK;
}

int32_t FileUtils::TruncateFile(int fd, size_t size)
{
    if (fd < 0) {
        LOGE("invalid fd %{public}d", fd);
        return EBADF;
    }

    int ret = ftruncate(fd, size);
    if (ret < 0) {
        LOGE("errno %{public}d, fd=%{public}d", errno, fd);
        return errno;
    }

    return E_OK;
}

int64_t FileUtils::ReadFile(int fd, off_t offset, size_t size, void *data)
{
    if ((fd < 0) || (offset < 0) || (size < 0) || (data == nullptr)) {
        LOGE("invalid params, fd %{public}d, offset %{public}d, size %{public}zu, or buf is null", fd,
             static_cast<int>(offset), size);
        return -1;
    }

    size_t readLen = 0;
    lseek(fd, offset, 0);
    while (readLen < size) {
        ssize_t ret = read(fd, data, size - readLen);
        if (ret < 0) {
            LOGE("errno %{public}d, fd=%{public}d", errno, fd);
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

    size_t writeLen = 0;
    lseek(fd, offset, 0);
    while (writeLen < size) {
        ssize_t ret = write(fd, data, size - writeLen);
        if ((ret < 0) || (ret == 0)) {
            LOGE("errno %{public}d, fd=%{public}d", errno, fd);
            return ret;
        }
        writeLen += ret;
    }

    return writeLen;
}
} // namespace FileManagement
} // namespace OHOS
