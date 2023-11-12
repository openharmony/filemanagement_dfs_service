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

#include "clouddisk_rdb_utils.h"

#include "utils_log.h"
#include "dfs_error.h"
#include "clouddisk_db_const.h"
#include "file_column.h"
#include "result_set.h"

namespace OHOS::FileManagement::CloudDisk {
using namespace std;
using namespace NativeRdb;

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

int32_t CloudDiskRdbUtils::FillInfoFileName(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoCloudId(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoParentId(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoLocation(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoFileSize(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoFileATime(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoFileCTime(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoFileMTime(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoFileType(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    return E_OK;
}

int32_t CloudDiskRdbUtils::ResultSetToFileInfo(const shared_ptr<ResultSet> resultSet,
                                               vector<CloudDiskFileInfo> &infos)
{
    return E_OK;
}
}