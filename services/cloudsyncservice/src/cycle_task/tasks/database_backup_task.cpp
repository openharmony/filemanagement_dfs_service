/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "database_backup_task.h"
#include "clouddisk_rdbstore.h"
#include "parameters.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace OHOS::FileManagement::CloudDisk;
static const std::string FILEMANAGER_KEY = "persist.kernel.bundle_name.filemanager";
static const uint32_t STAT_MODE_DIR = 0771;
DatabaseBackupTask::DatabaseBackupTask(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
    : CycleTask("database_backup_task", {system::GetParameter(FILEMANAGER_KEY, "")}, ONE_DAY, dataSyncManager)
{
}

int32_t DatabaseBackupTask::RunTaskForBundle(int32_t userId, std::string bundleName)
{
    LOGI("%{public}s backup database entry", bundleName.c_str());
    auto directoryName = "/data/service/el2/" + to_string(userId_) +
        "/hmdfs/cloudfile_manager/" + bundleName + "/backup";
    if (access(directoryName.c_str(), F_OK) != 0) {
        if (errno == ENOENT) {
            if (mkdir(directoryName.c_str(), STAT_MODE_DIR) != 0) {
                LOGE("mkdir backup failed :%{public}s err:%{public}d", GetAnonyString(directoryName).c_str(), errno);
                return errno;
            }
        } else {
            LOGE("access backup dir failed :%{public}s err:%{public}d", GetAnonyString(directoryName).c_str(), errno);
            return errno;
        }
    }
    auto fileName = directoryName + "/clouddisk_backup.db";
    std::thread backup([fileName, userId, bundleName] {
        auto cloudDiskRdbStore = make_shared<CloudDisk::CloudDiskRdbStore>(bundleName, userId);
        auto rdb = cloudDiskRdbStore->GetRaw();
        if (!rdb) {
            LOGE("clouddisk rdb init fail");
            return;
        }
        auto ret = rdb->Backup(fileName);
        if (ret != 0) {
            LOGE("clouddisk backup failed, ret %{public}d", ret);
        }
    });
    backup.detach();
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS