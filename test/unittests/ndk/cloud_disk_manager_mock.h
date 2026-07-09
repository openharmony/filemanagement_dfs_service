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

#ifndef OHOS_FILEMGMT_CLOUD_DISK_MANAGER_MOCK_H
#define OHOS_FILEMGMT_CLOUD_DISK_MANAGER_MOCK_H

#include <cstdint>
#include <string>

namespace OHOS::FileManagement::CloudDiskService::Test {
struct XattrMockState {
    bool managerCalled = false;
    bool managerCreateCalled = false;
    int32_t managerRet = 0;
    bool managerIsPlaceholder = false;
    std::string managerSyncFolder;
    std::string managerPath;
    std::string managerRelativePath;
    uint64_t managerLogicalSize = 0;
    uint64_t managerAtimeMs = 0;
    uint64_t managerMtimeMs = 0;
};

XattrMockState &GetXattrMockState();
void ResetXattrMock();
} // namespace OHOS::FileManagement::CloudDiskService::Test

#endif // OHOS_FILEMGMT_CLOUD_DISK_MANAGER_MOCK_H
