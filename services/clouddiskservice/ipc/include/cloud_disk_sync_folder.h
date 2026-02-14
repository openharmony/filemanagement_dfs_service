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

#ifndef CLOUD_DISK_SYNC_FOLDER_H
#define CLOUD_DISK_SYNC_FOLDER_H

#include <memory>
#include <mutex>
#include <string>

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {

struct SyncFolderValue {
    std::string bundleName;
    std::string path;
};
class CloudDiskSyncFolder {
public:
    static CloudDiskSyncFolder &GetInstance();
    void AddSyncFolder(const uint32_t &syncFolderIndex, const SyncFolderValue &syncFolderValue);
    void DeleteSyncFolder(const uint32_t &syncFolderIndex);
    int32_t GetSyncFolderSize();
    bool CheckSyncFolder(const uint32_t &syncFolderIndex);
    bool GetSyncFolderByIndex(const uint32_t syncFolderIndex, std::string &path);
    bool GetSyncFolderValueByIndex(const uint32_t syncFolderIndex, SyncFolderValue &syncFolderValue);
    bool GetIndexBySyncFolder(uint32_t &syncFolderIndex, const std::string &path);
    void RemoveXattr(std::string &path, const std::string &attrName);
    int32_t PathToPhysicalPath(const std::string &path, const std::string &userId, std::string &physicalPath);
    int32_t PathToMntPathBySandboxPath(const std::string &path, const std::string &userId, std::string &realPath);
    bool PathToMntPathByPhysicalPath(const std::string &path, const std::string &userId, std::string &realPath);
    bool PathToSandboxPathByPhysicalPath(const std::string &path, const std::string &userId, std::string &realPath);
    std::unordered_map<uint32_t, SyncFolderValue> GetSyncFolderMap();
    void ClearMap();

private:
    int32_t ReplacePathPrefix(const std::string &oldPrefix, const std::string &newPrefix,
                           const std::string &inputPath, std::string &outputPath);
    std::unordered_map<uint32_t, SyncFolderValue> syncFolderMap;
    std::mutex mutex_;
};
} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_DISK_SYNC_FOLDER_H
