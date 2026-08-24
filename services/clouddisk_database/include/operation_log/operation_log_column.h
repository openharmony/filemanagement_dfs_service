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

#ifndef OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_COLUMN_H
#define OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_COLUMN_H

#include <string>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

class OperationLogColumn {
public:
    static const std::string TABLE_NAME;
    static const std::string ID;
    static const std::string OP_TIME;
    static const std::string OP_TYPE;
    static const std::string FILE_PATH;
    static const std::string FILE_INODE;
    static const std::string FILE_UID;
    static const std::string PROCESS_NAME;
    static const std::string PROCESS_PID;
    static const std::string PROCESS_UID;

    static const std::string CREATE_TABLE_SQL;
    static const std::string CREATE_INDEX_SQL;
};

} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_COLUMN_H
