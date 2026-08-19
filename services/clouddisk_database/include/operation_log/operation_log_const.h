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

#ifndef OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_CONST_H
#define OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_CONST_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

class OperationLogConst {
public:
    static const std::string DATABASE_NAME;
    static const std::string DATABASE_ROOT_DIR;
    static const std::string DATABASE_RELATIVE_DIR;

    static constexpr int32_t DATABASE_VERSION = 1;
    static constexpr int32_t CONNECT_SIZE = 4;
    static constexpr int64_t RDB_WAL_LIMIT_SIZE = 0x40000000;

    //数据库清理上限
    static constexpr int32_t MAX_RECORD_COUNT = 100000;
    static constexpr int32_t MAX_RETRIES = 3;

    static constexpr int32_t OP_TYPE_DELETE = 0;

    static constexpr uint32_t DIR_MODE = 0700;
    static constexpr uint32_t FILE_MODE = 0600;

    static constexpr int32_t CLEAN_THRESHOLD_DAYS = 90;
    static constexpr int64_t MILLISECONDS_PER_DAY = 24 * 60 * 60 * 1000;
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_DISK_SERVICE_OPERATION_LOG_CONST_H
