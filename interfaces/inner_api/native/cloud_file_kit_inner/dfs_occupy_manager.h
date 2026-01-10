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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_DFS_OCCUPY_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_SYNC_DFS_OCCUPY_MANAGER_H
#include <map>
#include <mutex>

namespace OHOS::FileManagement::CloudSync {
class DfsOccupyManager final {
public:
    static DfsOccupyManager &GetInstance();

    /* Add occupy numbers to memory */
    void AddOccupyToMem(const int32_t userId, int64_t num);

    /* Sub occupy numbers to memory */
    void SubOccupyToMem(const int32_t userId, int64_t num);

    /* Save the occupy numbers to file */
    void SaveOccupyToFile(const int32_t userId);

    /* Save the specified occupy numbers to file and memory */
    void SaveOccupyToFileMemByNum(const int32_t userId, int64_t num);

    /* Get the occupy numbers from file */
    void GetOccupyFromFile(const int32_t userId, int64_t &occupyNum);

    DfsOccupyManager(const DfsOccupyManager&) = delete;
    DfsOccupyManager& operator=(const DfsOccupyManager&) = delete;

private:
    DfsOccupyManager() = default;
    ~DfsOccupyManager() = default;
    std::mutex occupyMutex_;
    std::map<int32_t, int64_t> occupyNums_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_DFS_OCCUPY_MANAGER_H