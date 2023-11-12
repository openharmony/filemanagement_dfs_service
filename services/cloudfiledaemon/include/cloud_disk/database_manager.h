/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef CLOUD_FILE_DAEMON_CLOUD_DISK_DATABASE_MANAGER_H
#define CLOUD_FILE_DAEMON_CLOUD_DISK_DATABASE_MANAGER_H
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>

#include "clouddisk_rdbstore.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
class DatabaseManager final {
public:
    static DatabaseManager &GetInstance();
    std::shared_ptr<CloudDiskRdbStore> GetRdbStore(const std::string &bundleName,
                                                   int32_t userId);
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
private:
    DatabaseManager() = default;
    ~DatabaseManager() = default;
    std::shared_mutex mapLock_;
    std::unordered_map<std::string, std::shared_ptr<CloudDiskRdbStore>>rdbMap_;
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_FILE_DAEMON_CLOUD_DISK_DATABASE_MANAGER_H
