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

#include <dirent.h>
#include <sys/stat.h>
#include <sys/xattr.h>

#include "disk_monitor.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {
using namespace std;

CloudDiskSyncFolder &CloudDiskSyncFolder::GetInstance()
{
    static CloudDiskSyncFolder instance;
    return instance;
}

void CloudDiskSyncFolder::AddSyncFolder(const uint32_t &syncFolderIndex, const SyncFolderValue &syncFolderValue)
{
    unique_lock<mutex> lock(mutex_);
    auto item = syncFolderMap.find(syncFolderIndex);
    if (item == syncFolderMap.end()) {
        syncFolderMap[syncFolderIndex] = syncFolderValue;
    }
}

void CloudDiskSyncFolder::DeleteSyncFolder(const uint32_t &syncFolderIndex)
{
    unique_lock<mutex> lock(mutex_);
    auto item = syncFolderMap.find(syncFolderIndex);
    if (item != syncFolderMap.end()) {
        syncFolderMap.erase(syncFolderIndex);
    }
}

int32_t CloudDiskSyncFolder::GetSyncFolderSize()
{
    unique_lock<mutex> lock(mutex_);
    return syncFolderMap.size();
}

bool CloudDiskSyncFolder::CheckSyncFolder(const uint32_t &syncFolderIndex)
{
    unique_lock<mutex> lock(mutex_);
    auto it = syncFolderMap.find(syncFolderIndex);
    if (it == syncFolderMap.end()) {
        return false;
    }
    return true;
}

bool CloudDiskSyncFolder::GetSyncFolderByIndex(const uint32_t syncFolderIndex, string &path)
{
    unique_lock<mutex> lock(mutex_);
    auto item = syncFolderMap.find(syncFolderIndex);
    if (item == syncFolderMap.end()) {
        return false;
    }
    path = item->second.path;
    return true;
}

bool CloudDiskSyncFolder::GetSyncFolderValueByIndex(const uint32_t syncFolderIndex, SyncFolderValue &syncFolderValue)
{
    unique_lock<mutex> lock(mutex_);
    auto item = syncFolderMap.find(syncFolderIndex);
    if (item == syncFolderMap.end()) {
        return false;
    }
    syncFolderValue = item->second;
    return true;
}

bool CloudDiskSyncFolder::GetIndexBySyncFolder(uint32_t &syncFolderIndex, const string &path)
{
    unique_lock<mutex> lock(mutex_);
    for (const auto &item : syncFolderMap) {
        if (item.second.path == path) {
            syncFolderIndex = item.first;
            return true;
        }
    }
    return false;
}

std::unordered_map<uint32_t, SyncFolderValue> CloudDiskSyncFolder::GetSyncFolderMap()
{
    unique_lock<mutex> lock(mutex_);
    return syncFolderMap;
}

void CloudDiskSyncFolder::RemoveXattr(string &path, const string &attrName)
{
    DIR *dir = opendir(path.c_str());
    if (!dir) {
        LOGE("Open failed, err:%{public}d", errno);
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char realPath[PATH_MAX] = {'\0'};
        std::string pathToRemove = path + "/" + entry->d_name;
        if (realpath(pathToRemove.c_str(), realPath) == nullptr) {
            LOGE("get realPath failed, errno: %{public}d", errno);
            continue;
        }

        struct stat st;
        if (lstat(realPath, &st) == -1) {
            LOGE("lstat failed");
            continue;
        }
        if (removexattr(realPath, attrName.c_str()) == -1 && errno != ENODATA) {
            LOGE("removexattr failed for path:%{public}s, errno:%{public}d",
                GetAnonyStringStrictly(realPath).c_str(), errno);
        }
        if (S_ISDIR(st.st_mode)) {
            RemoveXattr(pathToRemove, attrName);
        }
    }
    closedir(dir);
}

/* Verify the path specified by inputPath, replace its prefix oldPrefix with the new prefix newPrefix. */
bool CloudDiskSyncFolder::ReplacePathPrefix(const string &oldPrefix, const string &newPrefix,
                                            const string &inputPath, string &outputPath)
{
    if ((inputPath.size() < oldPrefix.size()) || (inputPath.substr(0, oldPrefix.size()) != oldPrefix)) {
        LOGE("Invalid path prefix: %{public}s, input: %{public}s",
            GetAnonyStringStrictly(oldPrefix).c_str(), GetAnonyStringStrictly(inputPath).c_str());
        return false;
    }

    string newPath = newPrefix + inputPath.substr(oldPrefix.size());

    char realPathBuffer[PATH_MAX + 1] { '\0' };
    if (realpath(newPath.c_str(), realPathBuffer) == nullptr) {
        LOGE("Realpath error: %{public}d, path=%{public}s", errno, GetAnonyStringStrictly(newPath).c_str());
        return false;
    }

    string newRealPath = string(realPathBuffer);
    if (newRealPath.substr(0, newPrefix.size()) != newPrefix) {
        LOGE("Path traversal prefix: %{public}s, newRealPath: %{public}s",
            GetAnonyStringStrictly(newPrefix).c_str(), GetAnonyStringStrictly(newRealPath).c_str());
        return false;
    }

    outputPath = newRealPath;
    return true;
}

bool CloudDiskSyncFolder::PathToPhysicalPath(const string &path, const string &userId, string &realPath)
{
    string sandboxPath = "/storage/Users/currentUser";
    string replacementPath = "/data/service/el2/" + userId + "/hmdfs/account/files/Docs";

    return ReplacePathPrefix(sandboxPath, replacementPath, path, realPath);
}

bool CloudDiskSyncFolder::PathToMntPathBySandboxPath(const string &path,
                                                     const string &userId,
                                                     string &realPath)
{
    string sandboxPath = "/storage/Users/currentUser";
    string replacementPath = "/mnt/hmdfs/" + userId + "/account/device_view/local/files/Docs";

    return ReplacePathPrefix(sandboxPath, replacementPath, path, realPath);
}

bool CloudDiskSyncFolder::PathToMntPathByPhysicalPath(const string &path,
                                                      const string &userId,
                                                      string &realPath)
{
    string physicalPath = "/data/service/el2/" + userId + "/hmdfs/account/files/Docs";
    string replacementPath = "/mnt/hmdfs/" + userId + "/account/device_view/local/files/Docs";

    if (path.substr(0, physicalPath.length()) != physicalPath) {
        LOGE("replace path failed");
        return false;
    }
    realPath = replacementPath + path.substr(physicalPath.length());
    return true;
}

bool CloudDiskSyncFolder::PathToSandboxPathByPhysicalPath(const string &path,
                                                          const string &userId,
                                                          string &realPath)
{
    string physicalPath = "/data/service/el2/" + userId + "/hmdfs/account/files/Docs";
    string replacementPath = "/storage/Users/currentUser";

    if (path.substr(0, physicalPath.length()) != physicalPath) {
        LOGE("replace path failed");
        return false;
    }
    realPath = replacementPath + path.substr(physicalPath.length());
    return true;
}

void CloudDiskSyncFolder::ClearMap()
{
    unique_lock<mutex> lock(mutex_);
    syncFolderMap.clear();
}
} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS