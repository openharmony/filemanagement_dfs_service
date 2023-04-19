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

#include "album_data_handler.h"

#include "medialibrary_db_const.h"
#include "medialibrary_type_const.h"

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace NativeRdb;
using namespace DriveKit;

AlbumDataHandler::AlbumDataHandler(std::shared_ptr<RdbStore> rdb)
    : RdbDataHandler(const_cast<string &>(tableName_), rdb)
{
}

int32_t AlbumDataHandler::GetFetchCondition()
{
    return E_OK;
}

int32_t AlbumDataHandler::OnFetchRecords(const shared_ptr<const map<DKRecordId, DKRecord>> &map)
{
    return E_OK;
}

int32_t AlbumDataHandler::GetCreatedRecords(vector<DKRecord> &records)
{
    return E_OK;
}

int32_t AlbumDataHandler::GetDeletedRecords(vector<DKRecord> &records)
{
    return E_OK;
}

int32_t AlbumDataHandler::GetModifiedRecords(vector<DKRecord> &records)
{
    return E_OK;
}

int32_t AlbumDataHandler::OnCreateRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    return E_OK;
}

int32_t AlbumDataHandler::OnDeleteRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    return E_OK;
}

int32_t AlbumDataHandler::OnModifyRecords(const map<DKRecordId, DKRecordOperResult> &map)
{
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
