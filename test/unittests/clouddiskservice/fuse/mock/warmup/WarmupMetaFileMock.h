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

#ifndef CLOUD_DISK_WARMUP_META_FILE_MOCK_H
#define CLOUD_DISK_WARMUP_META_FILE_MOCK_H

#include <cstdint>
#include <vector>

namespace OHOS::FileManagement::CloudDiskService::Test {

void ResetWarmupMetaFileMock();
void SetWarmupMetaFileExists(uint64_t inode, bool exists);
void SetWarmupGenericDentryResult(int32_t result);
void SetWarmupLookupByNameResults(const std::vector<int32_t> &results);
uint32_t GetWarmupGenericDentryCalls();
uint32_t GetWarmupLookupByNameCalls();
uint32_t GetWarmupCreateMetaFileCalls();

} // namespace OHOS::FileManagement::CloudDiskService::Test

#endif // CLOUD_DISK_WARMUP_META_FILE_MOCK_H
