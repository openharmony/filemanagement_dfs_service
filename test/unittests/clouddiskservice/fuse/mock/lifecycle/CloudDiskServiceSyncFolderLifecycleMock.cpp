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

#include "CloudDiskServiceSyncFolderLifecycleMock.h"

#include "cloud_disk_service_syncfolder.h"

namespace OHOS::FileManagement::CloudDiskService {
namespace {
int32_t g_registerResult = 0;
int32_t g_unregisterResult = 0;
uint32_t g_registerCalls = 0;
uint32_t g_unregisterCalls = 0;
} // namespace

namespace Test {
void ResetCloudDiskServiceSyncFolderLifecycleMock()
{
    g_registerResult = 0;
    g_unregisterResult = 0;
    g_registerCalls = 0;
    g_unregisterCalls = 0;
}

void SetRegisterSyncFolderResult(int32_t result)
{
    g_registerResult = result;
}

void SetUnregisterSyncFolderResult(int32_t result)
{
    g_unregisterResult = result;
}

uint32_t GetRegisterSyncFolderCalls()
{
    return g_registerCalls;
}

uint32_t GetUnregisterSyncFolderCalls()
{
    return g_unregisterCalls;
}
} // namespace Test

int32_t CloudDiskServiceSyncFolder::RegisterSyncFolder(const int32_t userId,
                                                       const uint32_t syncFolderIndex,
                                                       const std::string &path)
{
    (void)userId;
    (void)syncFolderIndex;
    (void)path;
    ++g_registerCalls;
    return g_registerResult;
}

int32_t CloudDiskServiceSyncFolder::UnRegisterSyncFolder(const int32_t userId, const uint32_t syncFolderIndex)
{
    (void)userId;
    (void)syncFolderIndex;
    ++g_unregisterCalls;
    return g_unregisterResult;
}

void CloudDiskServiceSyncFolder::RegisterSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex)
{
    (void)userId;
    (void)syncFolderIndex;
}

void CloudDiskServiceSyncFolder::UnRegisterSyncFolderChanges(const int32_t userId, const uint32_t syncFolderIndex)
{
    (void)userId;
    (void)syncFolderIndex;
}

int32_t CloudDiskServiceSyncFolder::GetSyncFolderChanges(const int32_t userId,
                                                         const uint32_t syncFolderIndex,
                                                         const uint64_t start,
                                                         const uint64_t count,
                                                         ChangesResult &changesResult)
{
    (void)userId;
    (void)syncFolderIndex;
    (void)start;
    (void)count;
    (void)changesResult;
    return 0;
}

int32_t CloudDiskServiceSyncFolder::SetSyncFolderChanges(const EventInfo &eventInfo)
{
    (void)eventInfo;
    return 0;
}

void CloudDiskServiceSyncFolder::CloudDiskServiceClearAll() {}

} // namespace OHOS::FileManagement::CloudDiskService
