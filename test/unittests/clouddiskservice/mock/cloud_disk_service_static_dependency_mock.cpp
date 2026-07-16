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

#include "cloud_disk_service_callback_manager.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_service_syncfolder.h"
#include "uuid_helper.h"

namespace OHOS::FileManagement::CloudDiskService {
CloudDiskServiceCallbackManager &CloudDiskServiceCallbackManager::GetInstance()
{
    static CloudDiskServiceCallbackManager instance;
    return instance;
}

bool CloudDiskServiceCallbackManager::RegisterSyncFolderMap(
    std::string &bundleName, uint32_t syncFolderIndex, const sptr<ICloudDiskServiceCallback> &callback)
{
    (void)bundleName;
    (void)syncFolderIndex;
    (void)callback;
    return true;
}

void CloudDiskServiceCallbackManager::UnregisterSyncFolderMap(const std::string &bundleName, uint32_t syncFolderIndex)
{
    (void)bundleName;
    (void)syncFolderIndex;
}

bool CloudDiskServiceCallbackManager::UnregisterSyncFolderForChangesMap(
    std::string &bundleName, uint32_t syncFolderIndex)
{
    (void)bundleName;
    (void)syncFolderIndex;
    return true;
}

void CloudDiskServiceCallbackManager::AddCallback(
    const std::string &bundleName, const sptr<ICloudDiskServiceCallback> &callback)
{
    (void)bundleName;
    (void)callback;
}

void CloudDiskServiceCallbackManager::OnChangeData(const uint32_t syncFolderIndex,
                                                   const std::vector<ChangeData> &changeData)
{
    (void)syncFolderIndex;
    (void)changeData;
}

void CloudDiskServiceCallbackManager::ClearMap() {}

int32_t CloudDiskServiceSyncFolder::RegisterSyncFolder(const int32_t userId,
                                                       const uint32_t syncFolderIndex,
                                                       const std::string &path)
{
    (void)userId;
    (void)syncFolderIndex;
    (void)path;
    return E_OK;
}

int32_t CloudDiskServiceSyncFolder::UnRegisterSyncFolder(const int32_t userId, const uint32_t syncFolderIndex)
{
    (void)userId;
    (void)syncFolderIndex;
    return E_OK;
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
                                                         struct ChangesResult &changesResult)
{
    (void)userId;
    (void)syncFolderIndex;
    (void)start;
    (void)count;
    (void)changesResult;
    return E_OK;
}

int32_t CloudDiskServiceSyncFolder::SetSyncFolderChanges(const struct EventInfo &eventInfos)
{
    (void)eventInfos;
    return E_OK;
}

void CloudDiskServiceSyncFolder::CloudDiskServiceClearAll() {}

std::string UuidHelper::GenerateUuid()
{
    return "mock-uuid";
}

std::string UuidHelper::GenerateUuidWithoutDelim()
{
    return "mockuuid";
}

std::string UuidHelper::GenerateUuidOnly()
{
    return "mock_record_id";
}
} // namespace OHOS::FileManagement::CloudDiskService
