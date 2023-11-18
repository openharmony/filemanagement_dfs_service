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
    int32_t index;
    int32_t err = resultSet->GetColumnIndex(key, index);
    if (err != E_OK) {
        LOGE("result set get column index err %{public}d", err);
        return E_RDB;
    }
    err = resultSet->GetInt(index, val);
    if (err != 0) {
        LOGE("result set get int err %{public}d", err);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbUtils::GetLong(const string &key, int64_t &val,
                                   const shared_ptr<ResultSet> resultSet)
{
    int32_t index;
    int32_t err = resultSet->GetColumnIndex(key, index);
    if (err != E_OK) {
        LOGE("result set get column index err %{public}d", err);
        return E_RDB;
    }
    err = resultSet->GetLong(index, val);
    if (err != 0) {
        LOGE("result set get int err %{public}d", err);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbUtils::GetString(const string &key, string &val,
                                     const shared_ptr<ResultSet> resultSet)
{
    int32_t index;
    int32_t err = resultSet->GetColumnIndex(key, index);
    if (err != E_OK) {
        LOGE("result set get column index err %{public}d", err);
        return E_RDB;
    }
    err = resultSet->GetString(index, val);
    if (err != 0) {
        LOGE("result set get string err %{public}d", err);
        return E_RDB;
    }
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoFileName(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    string fileName;
    int32_t ret = GetString(FileColumn::FILE_NAME, fileName, resultSet);
    if (ret != E_OK) {
        LOGE("fill fileName failed, ret = %{public}d", ret);
        return ret;
    }
    info.fileName = fileName;
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoCloudId(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    string cloudId;
    int32_t ret = GetString(FileColumn::CLOUD_ID, cloudId, resultSet);
    if (ret != E_OK) {
        LOGE("fill cloudId failed, ret = %{public}d", ret);
        return ret;
    }
    info.cloudId = cloudId;
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoParentId(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    string parentCloudId;
    int32_t ret = GetString(FileColumn::PARENT_CLOUD_ID, parentCloudId, resultSet);
    if (ret != E_OK) {
        LOGE("fill parentCloudId failed, ret = %{public}d", ret);
        return ret;
    }
    info.parentCloudId = parentCloudId;
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoLocation(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    int32_t location;
    int32_t ret = GetInt(FileColumn::POSITION, location, resultSet);
    if (ret != E_OK) {
        LOGE("fill location failed, ret = %{public}d", ret);
        return ret;
    }
    info.location = static_cast<uint32_t>(location);
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoFileSize(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    int64_t size;
    int32_t ret = GetLong(FileColumn::FILE_SIZE, size, resultSet);
    if (ret != E_OK) {
        LOGE("fill size failed, ret = %{public}d", ret);
        return ret;
    }
    info.size = static_cast<unsigned long long>(size);
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoFileATime(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    int64_t aTime;
    int32_t ret = GetLong(FileColumn::FILE_TIME_VISIT, aTime, resultSet);
    if (ret != E_OK) {
        LOGE("fill atime failed, ret = %{public}d", ret);
        return ret;
    }
    info.atime = static_cast<unsigned long long>(aTime);
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoFileCTime(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    int64_t cTime;
    int32_t ret = GetLong(FileColumn::FILE_TIME_ADDED, cTime, resultSet);
    if (ret != E_OK) {
        LOGE("fill ctime failed, ret = %{public}d", ret);
        return ret;
    }
    info.ctime = static_cast<unsigned long long>(cTime);
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoFileMTime(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    int64_t mTime;
    int32_t ret = GetLong(FileColumn::FILE_TIME_EDITED, mTime, resultSet);
    if (ret != E_OK) {
        LOGE("fill mtime failed, ret = %{public}d", ret);
        return ret;
    }
    info.mtime = static_cast<unsigned long long>(mTime);
    return E_OK;
}

int32_t CloudDiskRdbUtils::FillInfoFileType(CloudDiskFileInfo &info, const shared_ptr<ResultSet> resultSet)
{
    int32_t fileType;
    int32_t ret = GetInt(FileColumn::IS_DIRECTORY, fileType, resultSet);
    if (ret != E_OK) {
        LOGE("fill fileType failed, ret = %{public}d", ret);
        return ret;
    }
    info.IsDirectory = (fileType == DIRECTORY);
    return E_OK;
}

int32_t CloudDiskRdbUtils::ResultSetToFileInfo(const shared_ptr<ResultSet> resultSet,
                                               vector<CloudDiskFileInfo> &infos)
{
    int32_t rowCount = 0;
    int32_t ret = E_OK;
    if (resultSet) {
        ret = resultSet->GetRowCount(rowCount);
    }
    if (resultSet == nullptr || ret != E_OK || rowCount < 0) {
        LOGE("result set is nullptr or get result set rowCount is failed, ret %{public}d, rowCount %{public}d",
            ret, rowCount);
        return E_RDB;
    }
    while (resultSet->GoToNextRow() == E_OK) {
        CloudDiskFileInfo info;
        RETURN_ON_ERR(FillInfoFileName(info, resultSet));
        RETURN_ON_ERR(FillInfoCloudId(info, resultSet));
        RETURN_ON_ERR(FillInfoParentId(info, resultSet));
        RETURN_ON_ERR(FillInfoLocation(info, resultSet));
        RETURN_ON_ERR(FillInfoFileSize(info, resultSet));
        RETURN_ON_ERR(FillInfoFileATime(info, resultSet));
        RETURN_ON_ERR(FillInfoFileCTime(info, resultSet));
        RETURN_ON_ERR(FillInfoFileMTime(info, resultSet));
        RETURN_ON_ERR(FillInfoFileType(info, resultSet));
        infos.emplace_back(info);
    }
    return E_OK;
}
}