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

#ifndef OHOS_CLOUD_SYNC_DATA_SYNCER_RDB_COL_H
#define OHOS_CLOUD_SYNC_DATA_SYNCER_RDB_COL_H

#include <string>

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
/* basic */
const std::string DATA_SYNCER_ID = "dataSyncerId";
const std::string USER_ID = "userId";
const std::string BUNDLE_NAME = "bundleName";
const std::string SYNC_STATE = "syncState";
const std::string LAST_SYNC_TIME = "lastSyncTime";
const std::string DATA_SYNCER_UNIQUE_ID = "dataSyncerUniqueId";

const std::string DATA_SYNCER_TABLE = "DataSyncers";

const std::string DATA_SYNCER_UNIQUE_INDEX = "DataSyncersUniqueIndex";

const std::string CREATE_DATA_SYNCER_TABLE = "CREATE TABLE IF NOT EXISTS " +
    DATA_SYNCER_TABLE + " (" +
    DATA_SYNCER_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
    USER_ID + " INT, " +
    BUNDLE_NAME + " TEXT, " +
    SYNC_STATE + " INT DEFAULT 0, " +
    LAST_SYNC_TIME + " BIGINT DEFAULT 0, " +
    DATA_SYNCER_UNIQUE_ID + " TEXT)";

const std::string CREATE_DATA_SYNCER_UNIQUE_INDEX =
        "CREATE UNIQUE INDEX IF NOT EXISTS " + DATA_SYNCER_UNIQUE_INDEX +
        " ON " + DATA_SYNCER_TABLE + " (" + DATA_SYNCER_UNIQUE_ID + ")";

const std::set<std::string> DATA_SYNCER_COL = {
    DATA_SYNCER_ID, USER_ID, BUNDLE_NAME, SYNC_STATE, LAST_SYNC_TIME, DATA_SYNCER_UNIQUE_ID
};

const std::string EL1_CLOUDFILE_DIR = "/data/service/el1/public/cloudfile";
const std::string DATA_SYNCER_DB = "DataSyncers.db";

const int32_t CLOUD_DISK_RDB_VERSION = 2;
enum {
    VERSION_ADD_DATA_SYNCER_UNIQUE_INDEX = 2,
};
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_DATA_SYNCER_RDB_COL_H
