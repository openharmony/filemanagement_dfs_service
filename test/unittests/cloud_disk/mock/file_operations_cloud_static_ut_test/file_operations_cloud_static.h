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

#ifndef TEST_UNITTESTS_CLOUD_DISK_FILE_OPERATIONS_CLOUD_STATIC_H
#define TEST_UNITTESTS_CLOUD_DISK_FILE_OPERATIONS_CLOUD_STATIC_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "file_operations_helper.h"
#include "database_manager.h"
#include "clouddisk_rdbstore.h"

namespace OHOS::FileManagement::CloudDisk {
/*
 * DatabaseManager and CloudDiskRdbStore are declared `final`, and the methods of
 * FileOperationsHelper are static, so these mock classes do not inherit from the
 * real classes. The real member methods are rewritten in file_operations_cloud_static.cpp
 * to delegate to the corresponding proxy_ when it is set.
 */
class FileOperationsHelperMock {
public:
    MOCK_METHOD2(FindCloudDiskInode,
                 std::shared_ptr<CloudDiskInode>(struct CloudDiskFuseData *, int64_t));
    static inline std::shared_ptr<FileOperationsHelperMock> proxy_ = nullptr;
};

class DatabaseManagerMock {
public:
    MOCK_METHOD2(IsBundleCloudSyncEnabled, bool(int32_t, const std::string &));
    MOCK_METHOD2(GetRdbStore, std::shared_ptr<CloudDiskRdbStore>(const std::string &, int32_t));
    static inline std::shared_ptr<DatabaseManagerMock> proxy_ = nullptr;
};

class CloudDiskRdbStoreMock {
public:
    MOCK_METHOD2(ReadDir, int32_t(const std::string &, std::vector<CloudDiskFileInfo> &));
    static inline std::shared_ptr<CloudDiskRdbStoreMock> proxy_ = nullptr;
};
} // namespace OHOS::FileManagement::CloudDisk
#endif // TEST_UNITTESTS_CLOUD_DISK_FILE_OPERATIONS_CLOUD_STATIC_H
