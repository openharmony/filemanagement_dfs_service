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

#ifndef CLOUD_DISK_SERVICE_SYNC_FOLDER_LIFECYCLE_MOCK_H
#define CLOUD_DISK_SERVICE_SYNC_FOLDER_LIFECYCLE_MOCK_H

#include <cstdint>

namespace OHOS::FileManagement::CloudDiskService::Test {

void ResetCloudDiskServiceSyncFolderLifecycleMock();
void SetRegisterSyncFolderResult(int32_t result);
void SetUnregisterSyncFolderResult(int32_t result);
uint32_t GetRegisterSyncFolderCalls();
uint32_t GetUnregisterSyncFolderCalls();

} // namespace OHOS::FileManagement::CloudDiskService::Test

#endif // CLOUD_DISK_SERVICE_SYNC_FOLDER_LIFECYCLE_MOCK_H
