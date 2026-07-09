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

#include "cloud_disk_manager_mock.h"

#include <memory>
#include <vector>

#include "cloud_disk_service_manager.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
namespace {
CloudDiskManagerMockState g_cloudDiskManagerMockState;
}

CloudDiskManagerMockState &GetCloudDiskManagerMockState()
{
    return g_cloudDiskManagerMockState;
}

void ResetCloudDiskManagerMock()
{
    g_cloudDiskManagerMockState = CloudDiskManagerMockState();
}
} // namespace OHOS::FileManagement::CloudDiskService::Test

namespace OHOS::FileManagement::CloudDiskService {
class CloudDiskServiceManagerMock final : public CloudDiskServiceManager {
public:
    int32_t RegisterSyncFolderChanges(const std::string &syncFolder,
                                      const std::shared_ptr<CloudDiskServiceCallback> callback) override
    {
        return CloudDiskService::Test::GetCloudDiskManagerMockState().managerRet;
    }

    int32_t UnregisterSyncFolderChanges(const std::string &syncFolder) override
    {
        return CloudDiskService::Test::GetCloudDiskManagerMockState().managerRet;
    }

    int32_t GetSyncFolderChanges(const std::string &syncFolder, uint64_t count, uint64_t startUsn,
                                 ChangesResult &changesResult) override
    {
        return CloudDiskService::Test::GetCloudDiskManagerMockState().managerRet;
    }

    int32_t SetFileSyncStates(const std::string &syncFolder, const std::vector<FileSyncState> &fileSyncStates,
                              std::vector<FailedList> &failedList) override
    {
        return CloudDiskService::Test::GetCloudDiskManagerMockState().managerRet;
    }

    int32_t GetFileSyncStates(const std::string &syncFolder, const std::vector<std::string> &pathArray,
                              std::vector<ResultList> &resultList) override
    {
        return CloudDiskService::Test::GetCloudDiskManagerMockState().managerRet;
    }

    int32_t CreatePlaceholderFile(const std::string &syncFolder, const std::string &relativePath,
                                  const PlaceholderInfo &info) override
    {
        auto &mockState = CloudDiskService::Test::GetXattrMockState();
        mockState.managerCreateCalled = true;
        mockState.managerSyncFolder = syncFolder;
        mockState.managerRelativePath = relativePath;
        mockState.managerLogicalSize = info.logicalSize;
        mockState.managerAtimeMs = info.atimeMs;
        mockState.managerMtimeMs = info.mtimeMs;
        return mockState.managerRet;
    }

    int32_t IsPlaceholderFile(const std::string &syncFolder, const std::string &path, bool &isPlaceholder) override
    {
        auto &mockState = CloudDiskService::Test::GetCloudDiskManagerMockState();
        mockState.managerCalled = true;
        mockState.managerSyncFolder = syncFolder;
        mockState.managerPath = path;
        isPlaceholder = mockState.managerIsPlaceholder;
        return mockState.managerRet;
    }

    int32_t RegisterSyncFolder(int32_t userId, const std::string &bundleName, const std::string &path) override
    {
        return CloudDiskService::Test::GetCloudDiskManagerMockState().managerRet;
    }

    int32_t UnregisterSyncFolder(int32_t userId, const std::string &bundleName, const std::string &path) override
    {
        return CloudDiskService::Test::GetCloudDiskManagerMockState().managerRet;
    }

    int32_t UnregisterForSa(const std::string &path) override
    {
        return CloudDiskService::Test::GetCloudDiskManagerMockState().managerRet;
    }
};

CloudDiskServiceManager &CloudDiskServiceManager::GetInstance()
{
    static CloudDiskServiceManagerMock instance;
    return instance;
}
} // namespace OHOS::FileManagement::CloudDiskService
