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
#include "database_manager.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;
DatabaseManager &DatabaseManager::GetInstance()
{
    static DatabaseManager instance_;
    return instance_;
}

shared_ptr<CloudDiskRdbStore> DatabaseManager::GetRdbStore(const string &bundleName, int32_t userId)
{
    std::unique_lock<std::shared_mutex> wLock(mapLock_, std::defer_lock);
    string key = to_string(userId) + bundleName;

    wLock.lock();
    if (rdbMap_.find(key) == rdbMap_.end()) {
        rdbMap_[key] = make_shared<CloudDiskRdbStore>(bundleName, userId);
    }
    wLock.unlock();

    return rdbMap_[key];
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
