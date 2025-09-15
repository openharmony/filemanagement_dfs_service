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
#ifndef TEST_UNITTEST_CLOUD_DISK_SERVICE_MANAGER_MOCK_H
#define TEST_UNITTEST_CLOUD_DISK_SERVICE_MANAGER_MOCK_H

#include "cloud_disk_service_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDiskServiceManagerMock final : public CloudDiskServiceManager {
public:
    static CloudDiskServiceManagerMock &GetInstance()
    {
        static CloudDiskServiceManagerMock instance;
        return instance;
    }

    MOCK_METHOD2(RegisterSyncFolderChanges, int32_t(const std::string &,
        const std::shared_ptr<CloudDiskServiceCallback>));
    MOCK_METHOD1(UnregisterSyncFolderChanges, int32_t(const std::string &));
    MOCK_METHOD4(GetSyncFolderChanges, int32_t(const std::string &, uint64_t, uint64_t, ChangesResult &));
    MOCK_METHOD3(SetFileSyncStates, int32_t(const std::string &,
        const std::vector<FileSyncState> &, std::vector<FailedList> &));
    MOCK_METHOD3(GetFileSyncStates, int32_t(const std::string &, const std::vector<std::string> &,
        std::vector<ResultList> &));
    MOCK_METHOD3(RegisterSyncFolder, int32_t(int32_t, const std::string &, const std::string &));
    MOCK_METHOD3(UnregisterSyncFolder, int32_t(int32_t, const std::string &, const std::string &));
    MOCK_METHOD1(UnregisterForSa, int32_t(const string&));
};
}

#endif