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
 
#define FUSE_USE_VERSION 34

#include "clouddisk_rdb_utils.h"

#include <fuse_lowlevel.h>

#include "utils_log.h"
#include "dfs_error.h"
#include "clouddisk_db_const.h"
#include "file_column.h"
#include "result_set.h"

namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using namespace NativeRdb;

static const int32_t LOCAL_ID_OFFSET = 100;

int32_t CloudDiskRdbUtils::GetInt(const string &key, int32_t &val,
                                  const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

int32_t CloudDiskRdbUtils::GetLong(const string &key, int64_t &val,
                                   const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

int32_t CloudDiskRdbUtils::GetString(const string &key, string &val,
                                     const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

static void FillFileInfo(const RowEntity &rowEntity, CloudDiskFileInfo &info)
{
    rowEntity.Get(FileColumn::FILE_NAME).GetString(info.name);
    rowEntity.Get(FileColumn::CLOUD_ID).GetString(info.cloudId);
    rowEntity.Get(FileColumn::PARENT_CLOUD_ID).GetString(info.parentCloudId);
    int32_t int_variable;
    rowEntity.Get(FileColumn::POSITION).GetInt(int_variable);
    info.location = static_cast<uint32_t>(int_variable);
    int64_t long_variable;
    rowEntity.Get(FileColumn::FILE_SIZE).GetLong(long_variable);
    info.size = static_cast<unsigned long long>(long_variable);
    rowEntity.Get(FileColumn::FILE_TIME_ADDED).GetLong(long_variable);
    info.atime = static_cast<unsigned long long>(long_variable);
    rowEntity.Get(FileColumn::META_TIME_EDITED).GetLong(long_variable);
    info.ctime = static_cast<unsigned long long>(long_variable);
    rowEntity.Get(FileColumn::FILE_TIME_EDITED).GetLong(long_variable);
    info.mtime = static_cast<unsigned long long>(long_variable);
    rowEntity.Get(FileColumn::IS_DIRECTORY).GetInt(int_variable);
    info.IsDirectory = (int_variable == DIRECTORY);
    rowEntity.Get(FileColumn::ROW_ID).GetLong(long_variable);
    info.localId = static_cast<long long>(long_variable) + LOCAL_ID_OFFSET;
}

int32_t CloudDiskRdbUtils::ResultSetToFileInfo(const shared_ptr<ResultSet> resultSet,
                                               CloudDiskFileInfo &info)
{
    return E_OK;
}

size_t CloudDiskRdbUtils::FuseDentryAlignSize(const char *name)
{
    return fuse_add_direntry({}, nullptr, 0, name, nullptr, 0);
}

int32_t CloudDiskRdbUtils::ResultSetToFileInfos(const shared_ptr<ResultSet> resultSet,
                                                vector<CloudDiskFileInfo> &infos)
{
    return E_OK;
}
}