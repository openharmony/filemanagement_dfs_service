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
#include "rdb_assistant.h"
#include "rdb_sql_utils.h"
#include "clouddisk_rdbstore.h"
#include "clouddisk_rdb_transaction.h"

using namespace OHOS::FileManagement::CloudDisk;

std::shared_ptr<RdbStore> RdbHelper::GetRdbStore(
    const RdbStoreConfig &config, int version, RdbOpenCallback &openCallback, int &errCode)
{
    return Assistant::ins->GetRdbStore(config, version, openCallback, errCode);
}

int RdbHelper::DeleteRdbStore(const std::string &dbFileName, bool shouldClose)
{
    return Assistant::ins->DeleteRdbStore(dbFileName);
}

std::string RdbSqlUtils::GetDefaultDatabasePath(const std::string &baseDir, const std::string &name, int &errorCode)
{
    return Assistant::ins->GetDefaultDatabasePath(baseDir, name, errorCode);
}