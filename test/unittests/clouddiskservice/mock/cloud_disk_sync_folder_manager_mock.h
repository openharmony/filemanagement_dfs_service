/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef TEST_UNITTEST_CLOUD_DISK_SYNC_FOLDER_MANAGER_MOCK_H
#define TEST_UNITTEST_CLOUD_DISK_SYNC_FOLDER_MANAGER_MOCK_H

#include "cloud_disk_sync_folder_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDiskSyncFolderManagerMock : public CloudDiskSyncFolderManager {
public:
    static CloudDiskSyncFolderManagerMock &GetInstance()
    {
        static CloudDiskSyncFolderManagerMock instance;
        return instance;
    }
    MOCK_METHOD1(Register, int32_t(const SyncFolder));
    MOCK_METHOD1(Unregister, int32_t(const string));
    MOCK_METHOD1(Active, int32_t(const string));
    MOCK_METHOD1(Deactive, int32_t(const string));
    MOCK_METHOD1(GetSyncFolders, int32_t(vector<SyncFolder> &));
    MOCK_METHOD2(UpdateDisplayName, int32_t(const string, const string));
    MOCK_METHOD1(UnregisterForSa, int32_t(const string));
    MOCK_METHOD1(GetAllSyncFoldersForSa, int32_t(std::vector<SyncFolderExt> &));
};
} // OHOS::FileManagement::Test
#endif