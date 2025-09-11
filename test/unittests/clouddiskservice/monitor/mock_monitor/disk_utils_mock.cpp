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

bool DiskUtils::ExtractFileName(int type, string &fileName, struct file_handle *fileHandle)
{
    fileName = "";
    return true;
}

string DiskUtils::GetFilePath(int32_t eventFd, const string &fileName)
{
    return "";
}

void DiskUtils::CloseFd(int &fd)
{
}

void DiskUtils::CloseDir(DIR *&dir)
{
}
} // namespace OHOS::FileManagement::CloudDiskService