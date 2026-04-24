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

#include "migration_rdb_mock.h"
#include "rdb_sql_utils.h"

namespace OHOS::FileManagement::CloudDisk::Test {

shared_ptr<RdbStore> GetMockedRdbStore(const RdbStoreConfig &config, int version,
    RdbOpenCallback &openCallback, int &errCode)
{
    return MigrationAssistantMock::ins->GetRdbStore(config, version, openCallback, errCode);
}

string GetMockedDatabasePath(const string &baseDir, const string &name, int &errorCode)
{
    return MigrationAssistantMock::ins->GetDefaultDatabasePath(baseDir, name, errorCode);
}

} // namespace OHOS::FileManagement::CloudDisk::Test

namespace OHOS::NativeRdb {

std::shared_ptr<RdbStore> RdbHelper::GetRdbStore(const RdbStoreConfig &config, int version,
    RdbOpenCallback &openCallback, int &errCode)
{
    return OHOS::FileManagement::CloudDisk::Test::MigrationAssistantMock::ins->GetRdbStore(
        config, version, openCallback, errCode);
}

std::string RdbSqlUtils::GetDefaultDatabasePath(const std::string &baseDir, const std::string &name,
    int &errorCode)
{
    return OHOS::FileManagement::CloudDisk::Test::MigrationAssistantMock::ins->GetDefaultDatabasePath(
        baseDir, name, errorCode);
}

} // namespace OHOS::NativeRdb