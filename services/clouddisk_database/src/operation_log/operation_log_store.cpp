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

#include "operation_log_store.h"

#include <sys/stat.h>
#include <unistd.h>

#include "dfs_error.h"
#include "rdb_errno.h"
#include "rdb_sql_utils.h"
#include "utils_directory.h"
#include "utils_log.h"

#include "operation_log_column.h"
#include "operation_log_const.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

using namespace std;
using namespace OHOS::NativeRdb;

constexpr uid_t OID_DFS = 1009;

OperationLogStore& OperationLogStore::GetInstance()
{
    static OperationLogStore instance;
    return instance;
}

int32_t OperationLogStore::Init(int32_t userId)
{
    lock_guard<mutex> lock(mutex_);
    if (isInited_) {
        return E_OK;
    }
    userId_ = userId;
    int32_t ret = RdbInit(userId);
    if (ret != E_OK) {
        LOGE("OperationLogStore Init failed, ret=%{public}d", ret);
        return ret;
    }
    isInited_ = true;
    return E_OK;
}

int32_t OperationLogStore::RdbInit(int32_t userId)
{
    string databaseDir = OperationLogConst::DATABASE_ROOT_DIR + to_string(userId) +
        OperationLogConst::DATABASE_RELATIVE_DIR;
    int32_t ret = CheckAndCreateDir(databaseDir);
    if (ret != E_OK) {
        LOGE("check and create operation log db dir failed, ret = %{public}d", ret);
        return ret;
    }

    string databasePath = RdbSqlUtils::GetDefaultDatabasePath(databaseDir,
        OperationLogConst::DATABASE_NAME, ret);
    if (ret != NativeRdb::E_OK) {
        LOGE("create default database path failed, errCode = %{public}d", ret);
        return E_PATH;
    }

    config_.SetName(OperationLogConst::DATABASE_NAME);
    config_.SetPath(databasePath);
    config_.SetReadConSize(OperationLogConst::CONNECT_SIZE);
    config_.SetWalLimitSize(OperationLogConst::RDB_WAL_LIMIT_SIZE);

    if (!TryOpenRdbStore(databaseDir, databasePath, ret)) {
        LOGE("open operation log rdb store failed, errCode = %{public}d", ret);
        return ret;
    }
    LOGI("OperationLogStore RdbInit success, path = %{public}s", GetAnonyString(databasePath).c_str());
    return E_OK;
}

int32_t OperationLogStore::CheckAndCreateDir(const string& dirPath)
{
    struct stat dirStat;
    if (lstat(dirPath.c_str(), &dirStat) == 0) {
        if (S_ISLNK(dirStat.st_mode)) {
            LOGE("path is a symlink, reject fro security, dir = %{public}s", dirPath.c_str());
            return E_PATH;
        }
        return E_OK;
    }
    if (errno != ENOENT) {
        LOGE("lstat failed, dir = %{public}s, errno = %{public}d", dirPath.c_str(), errno);
        return E_PATH;
    }
    OHOS::Storage::DistributedFile::Utils::ForceCreateDirectory(dirPath, OperationLogConst::DIR_MODE);
    if (access(dirPath.c_str(), F_OK) != 0) {
        LOGE("create log db dir failed, dir = %{public}s, errno = %{public}d", dirPath.c_str(), errno);
        return E_PATH;
    }
    if (chown(dirPath.c_str(), OID_DFS, OID_DFS) != 0) {
        LOGE("chmod failed, dir = %{public}s, errno = %{public}d", dirPath.c_str(), errno);
        return E_PATH;
    }
    return E_OK;
}

bool OperationLogStore::TryOpenRdbStore(const string& customDir, const string& databasePath, int32_t& errCode)
{
    OperationLogRdbCallBack callBack;
    rdbStore_ = RdbHelper::GetRdbStore(config_, OperationLogConst::DATABASE_VERSION, callBack, errCode);
    if (rdbStore_ != nullptr) {
        return true;
    }
    LOGE("get rdb store failed, errCode = %{public}d", errCode);
    if (errCode == NativeRdb::E_SQLITE_CANTOPEN || errCode == NativeRdb::E_SQLITE_PERM) {
        struct stat dirStat;
        if (lstat(customDir.c_str(), &dirStat) == 0 &&
            dirStat.st_uid == static_cast<uid_t>(OID_DFS) &&
            dirStat.st_gid == static_cast<gid_t>(OID_DFS)) {
            if (chmod(customDir.c_str(), OperationLogConst::DIR_MODE) == 0) {
                LOGE("recreate operation log db dir failed");
                rdbStore_ = RdbHelper::GetRdbStore(config_, OperationLogConst::DATABASE_VERSION, callBack, errCode);
            }
        }
    }
    return rdbStore_ != nullptr;
}

shared_ptr<RdbStore> OperationLogStore::GetRaw()
{
    lock_guard<mutex> lock(mutex_);
    return rdbStore_;
}

void OperationLogStore::Stop()
{
    lock_guard<mutex> lock(mutex_);
    if (rdbStore_ == nullptr) {
        return;
    }
    rdbStore_ = nullptr;
    isInited_ = false;
}

int32_t OperationLogRdbCallBack::OnCreate(RdbStore& store)
{
    int32_t ret = store.ExecuteSql(OperationLogColumn::CREATE_TABLE_SQL);
    if (ret != NativeRdb::E_OK) {
        LOGE("create operation log table failed, ret = %{public}d", ret);
        return NativeRdb::E_ERROR;
    }

    ret = store.ExecuteSql(OperationLogColumn::CREATE_INDEX_SQL);
    if (ret != NativeRdb::E_OK) {
        LOGE("create operation log index failed, ret = %{public}d", ret);
        return NativeRdb::E_ERROR;
    }
    LOGI("create operation log table success");
    return NativeRdb::E_OK;
}

int32_t OperationLogRdbCallBack::OnUpgrade(RdbStore& store, int32_t oldVersion, int32_t newVersion)
{
    LOGI("operation log db upgrade, oldVersion = %{public}d, newVersion = %{public}d", oldVersion, newVersion);
    return NativeRdb::E_OK;
}

} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
