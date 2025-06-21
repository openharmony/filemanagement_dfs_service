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
#include <fcntl.h>

#include "assistant.h"
#include "file_utils.h"

using namespace OHOS::FileManagement::CloudDisk;

namespace OHOS::FileManagement {
int64_t FileUtils::ReadFile(int fd, off_t offset, size_t size, void *data)
{
    return Assistant::ins->ReadFile(fd, offset, size, data);
}

int64_t FileUtils::WriteFile(int fd, const void *data, off_t offset, size_t size)
{
    return Assistant::ins->WriteFile(fd, data, offset, size);
}

int FileRangeLock::FilePosLock(int fd, off_t offset, size_t size, int type)
{
    return Assistant::ins->FilePosLock(fd, offset, size, type);
}
} // namespace OHOS::FileManagement