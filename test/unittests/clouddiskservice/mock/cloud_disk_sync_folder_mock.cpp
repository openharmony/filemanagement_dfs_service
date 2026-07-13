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

void CloudDiskSyncFolder::AddSyncFolder(const uint32_t &syncFolderIndex, const SyncFolderValue &syncFolderValue)
{
    syncFolderMap[syncFolderIndex] = syncFolderValue;
}

void CloudDiskSyncFolder::DeleteSyncFolder(const uint32_t &syncFolderIndex)
{
    syncFolderMap.erase(syncFolderIndex);
}

int32_t CloudDiskSyncFolder::GetSyncFolderSize()
{
    return static_cast<int32_t>(syncFolderMap.size());
}

bool CloudDiskSyncFolder::CheckSyncFolder(const uint32_t &syncFolderIndex)
{
    return true;
}

bool CloudDiskSyncFolder::GetSyncFolderByIndex(const uint32_t syncFolderIndex, std::string &path)
{
    auto iter = syncFolderMap.find(syncFolderIndex);
    if (iter != syncFolderMap.end()) {
        path = iter->second.path;
        return true;
    }
    return false;
}

bool CloudDiskSyncFolder::GetSyncFolderValueByIndex(const uint32_t syncFolderIndex, SyncFolderValue &syncFolderValue)
{
    auto iter = syncFolderMap.find(syncFolderIndex);
    if (iter == syncFolderMap.end()) {
        return false;
    }
    syncFolderValue = iter->second;
    return true;
}

bool CloudDiskSyncFolder::GetIndexBySyncFolder(uint32_t &syncFolderIndex, const std::string &path)
{
    return true;
}

void CloudDiskSyncFolder::RemoveXattr(std::string &path, const std::string &attrName)
{
}

void CloudDiskSyncFolder::RemovePlaceholderFilesBatch(const std::string &path)
{
}

int32_t CloudDiskSyncFolder::PathToPhysicalPath(const std::string &path,
                                                const std::string &userId, std::string &realpath)
{
    const std::string sandboxPath = "/storage/Users/currentUser";
    const std::string physicalPrefix = "/data/service/el2/" + userId + "/hmdfs/account/files/Docs";
    if (path == "" || path == "/test/mockFailed" || path.find("mockPhysicalFailed") != std::string::npos) {
        return E_SYNC_FOLDER_PATH_NOT_EXIST;
    }
    if (path.find(sandboxPath) == 0) {
        realpath = physicalPrefix + path.substr(sandboxPath.length());
        return E_OK;
    }
    realpath = path;
    return E_OK;
}

int32_t CloudDiskSyncFolder::PathToMntPathBySandboxPath(const std::string &path,
                                                        const std::string &userId, std::string &realpath)
{
    const std::string sandboxPath = "/storage/Users/currentUser";
    const std::string mntPrefix = "/mnt/hmdfs/" + userId + "/account/device_view/local/files/Docs";
    if (path == "/test/mockFailed" || path.find("mockMntFailed") != std::string::npos) {
        return E_SYNC_FOLDER_PATH_NOT_EXIST;
    }
    if (path.find(sandboxPath) == 0) {
        realpath = mntPrefix + path.substr(sandboxPath.length());
        return E_OK;
    }
    realpath = path;
    return E_OK;
}

bool CloudDiskSyncFolder::PathToMntPathByPhysicalPath(const std::string &path,
                                                      const std::string &userId, std::string &realpath)
{
    const std::string physicalPath = "/data/service/el2/" + userId + "/hmdfs/account/files/Docs";
    const std::string replacementPath = "/mnt/hmdfs/" + userId + "/account/device_view/local/files/Docs";
    if (path == "/test/mockFailed") {
        return false;
    }
    if (path.substr(0, physicalPath.length()) != physicalPath) {
        return false;
    }
    realpath = replacementPath + path.substr(physicalPath.length());
    return true;
}

bool CloudDiskSyncFolder::PathToSandboxPathByPhysicalPath(const std::string &path,
                                                          const std::string &userId, std::string &realpath)
{
    if (path == "invalid_path") {
        return false;
    }
    return true;
}

std::unordered_map<uint32_t, SyncFolderValue> CloudDiskSyncFolder::GetSyncFolderMap()
{
    return syncFolderMap;
}

void CloudDiskSyncFolder::ClearMap()
{
    syncFolderMap.clear();
}
}
