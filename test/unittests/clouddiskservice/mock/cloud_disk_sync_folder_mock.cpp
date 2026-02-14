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

#include "cloud_disk_sync_folder.h"
#include "cloud_disk_service_error.h"

namespace OHOS::FileManagement::CloudDiskService {
#define E_OK 0
using namespace std;
const int32_t userIdIndex = 15;

CloudDiskSyncFolder &CloudDiskSyncFolder::GetInstance()
{
    static CloudDiskSyncFolder instance;
    return instance;
}

void CloudDiskSyncFolder::AddSyncFolder(const uint32_t &syncFolderIndex, const SyncFolderValue &syncFolderValu)
{
}

void CloudDiskSyncFolder::DeleteSyncFolder(const uint32_t &syncFolderIndex)
{
}

int32_t CloudDiskSyncFolder::GetSyncFolderSize()
{
    if (syncFolderMap.empty()) {
        return 0;
    } else {
        return 1;
    }
}

bool CloudDiskSyncFolder::CheckSyncFolder(const uint32_t &syncFolderIndex)
{
    return true;
}

bool CloudDiskSyncFolder::GetSyncFolderByIndex(const uint32_t syncFolderIndex, std::string &path)
{
    return true;
}

bool CloudDiskSyncFolder::GetIndexBySyncFolder(uint32_t &syncFolderIndex, const std::string &path)
{
    return true;
}

void CloudDiskSyncFolder::RemoveXattr(std::string &path, const std::string &attrName)
{
}

int32_t CloudDiskSyncFolder::PathToPhysicalPath(const std::string &path,
                                                const std::string &userId, std::string &realpath)
{
    if (path == "") {
        return E_SYNC_FOLDER_PATH_NOT_EXIST;
    } else {
        return E_OK;
    }
}

int32_t CloudDiskSyncFolder::PathToMntPathBySandboxPath(const std::string &path,
                                                        const std::string &userId, std::string &realpath)
{
    if (path == "/test/mockFailed") {
        return E_SYNC_FOLDER_PATH_NOT_EXIST;
    }
    return E_OK;
}

bool CloudDiskSyncFolder::PathToMntPathByPhysicalPath(const std::string &path,
                                                      const std::string &userId, std::string &realpath)
{
    if (path == "/test/mockFailed") {
        return false;
    }
    return true;
}

bool CloudDiskSyncFolder::PathToSandboxPathByPhysicalPath(const std::string &path,
                                                          const std::string &userId, std::string &realpath)
{
    return true;
}

std::unordered_map<uint32_t, SyncFolderValue> CloudDiskSyncFolder::GetSyncFolderMap()
{
    return unordered_map<uint32_t, SyncFolderValue>();
}
}
