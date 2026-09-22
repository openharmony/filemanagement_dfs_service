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

#include "WarmupFileSystemMock.h"

namespace OHOS::Storage::DistributedFile::Utils {

void ForceCreateDirectoryWarmupMock(const std::string &path, mode_t mode)
{
    (void)path;
    (void)mode;
    FileManagement::CloudDiskService::Test::WarmupFileSystemMock::RecordForceCreateDirectory();
}

} // namespace OHOS::Storage::DistributedFile::Utils

namespace OHOS::FileManagement::CloudDiskService {

int32_t GetFilePlaceholderStateWarmupMock(const std::string &path, uint8_t &placeholderState)
{
    (void)path;
    placeholderState = 0;
    Test::WarmupFileSystemMock::RecordPlaceholderState();
    return 0;
}

} // namespace OHOS::FileManagement::CloudDiskService
