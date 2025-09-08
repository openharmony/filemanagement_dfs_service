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

#include "disk_utils.h"

#include <sys/fanotify.h>
#include <unistd.h>

#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;
const string PROC_SELF_FD = "/proc/self/fd/";

bool DiskUtils::ExtractFileName(int type, string &fileName, struct file_handle *fileHandle)
{
    if (type == FAN_EVENT_INFO_TYPE_FID || type == FAN_EVENT_INFO_TYPE_DFID) {
        fileName.clear();
    } else if (type == FAN_EVENT_INFO_TYPE_DFID_NAME) {
        fileName = reinterpret_cast<const char *>(fileHandle->f_handle + fileHandle->handle_bytes);
    } else {
        return false;
    }
    return true;
}

string DiskUtils::GetFilePath(int32_t eventFd, const string &fileName)
{
    char eventFilePath[PATH_MAX + 1];
    string procFdPath = PROC_SELF_FD + to_string(eventFd);
    ssize_t pathLen = readlink(procFdPath.c_str(), eventFilePath, sizeof(eventFilePath) - 1);
    if (pathLen == -1 || pathLen > PATH_MAX) {
        LOGI("readlink failed , errno: %{public}d", errno);
        return "";
    }
    eventFilePath[pathLen] = '\0';

    string filePath(eventFilePath);
    if (!fileName.empty() && fileName.compare(".") != 0) {
        filePath += "/" + fileName;
    }
    return filePath;
}

void DiskUtils::CloseFd(int &fd)
{
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

void DiskUtils::CloseDir(DIR *&dir)
{
    if (dir != nullptr) {
        closedir(dir);
        dir = nullptr;
    }
}
} // namespace OHOS::FileManagement::CloudDiskService