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

#include "operation_log_column.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

const std::string OperationLogColumn::TABLE_NAME = "operationLog";
const std::string OperationLogColumn::ID = "id";
const std::string OperationLogColumn::OP_TIME = "op_time";
const std::string OperationLogColumn::OP_TYPE = "op_type";
const std::string OperationLogColumn::FILE_PATH = "file_path";
const std::string OperationLogColumn::FILE_INODE = "file_inode";
const std::string OperationLogColumn::FILE_UID = "file_uid";
const std::string OperationLogColumn::PROCESS_NAME = "process_name";
const std::string OperationLogColumn::PROCESS_PID = "process_pid";
const std::string OperationLogColumn::PROCESS_UID = "process_uid";

const std::string OperationLogColumn::CREATE_TABLE_SQL = "CREATE TABLE IF NOT EXISTS " +
    TABLE_NAME + " (" +
    ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
    OP_TIME + " BIGINT NOT NULL, " +
    OP_TYPE + " INT NOT NULL, " +
    FILE_PATH + " TEXT, " +
    FILE_INODE + " BIGINT, " +
    FILE_UID + " BIGINT, " +
    PROCESS_NAME + " TEXT, " +
    PROCESS_PID + " BIGINT, " +
    PROCESS_UID + " BIGINT)";
const std::string OperationLogColumn::CREATE_INDEX_SQL = "CREATE INDEX IF NOT EXISTS idx_op_time ON " +
    TABLE_NAME + " (" + OP_TIME + ")";
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
