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
#include "dfs_occupy_manager.h"

#include <string>

#include "cloud_pref_impl.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

static const std::string GLOBAL_CONFIG_FILE_PATH = "globalConfig.xml";
static const std::string SUM_OCCUPY_FLAG = "SumOccupyFlag";

DfsOccupyManager &DfsOccupyManager::GetInstance()
{
    static DfsOccupyManager instance_;
    return instance_;
}

static void CheckAdd(int64_t &data, int64_t addValue)
{
    if (data > INT64_MAX - addValue) {
        data = 0;
    } else {
        data += addValue;
    }
}

static void CheckSub(int64_t &data, int64_t subValue)
{
    if (data > subValue) {
        data -= subValue;
    } else {
        data = 0;
    }
}

void DfsOccupyManager::AddOccupyToMem(const int32_t userId, int64_t num)
{
    std::lock_guard<std::mutex> lock(occupyMutex_);
    auto it = occupyNums_.find(userId);
    if (it != occupyNums_.end()) {
        CheckAdd(it->second, num);
    } else {
        int64_t fileOccupy = 0;
        GetOccupyFromFile(userId, fileOccupy);
        CheckAdd(fileOccupy, num);
        occupyNums_[userId] = fileOccupy;
    }
}

void DfsOccupyManager::SubOccupyToMem(const int32_t userId, int64_t num)
{
    std::lock_guard<std::mutex> lock(occupyMutex_);
    auto it = occupyNums_.find(userId);
    if (it != occupyNums_.end()) {
        CheckSub(it->second, num);
    } else {
        int64_t fileOccupy = 0;
        GetOccupyFromFile(userId, fileOccupy);
        CheckSub(fileOccupy, num);
        occupyNums_[userId] = fileOccupy;
    }
}

void DfsOccupyManager::SaveOccupyToFile(const int32_t userId)
{
    std::lock_guard<std::mutex> lock(occupyMutex_);
    auto it = occupyNums_.find(userId);
    if (it != occupyNums_.end()) {
        CloudPrefImpl cloudPrefImpl(userId, "", GLOBAL_CONFIG_FILE_PATH);
        cloudPrefImpl.SetLong(SUM_OCCUPY_FLAG, it->second);
        LOGI("save occupy nums: %{public}s", std::to_string(it->second).c_str());
    }
}

void DfsOccupyManager::SaveOccupyToFileMemByNum(const int32_t userId, int64_t num)
{
    CloudPrefImpl cloudPrefImpl(userId, "", GLOBAL_CONFIG_FILE_PATH);
    cloudPrefImpl.SetLong(SUM_OCCUPY_FLAG, num);
    std::lock_guard<std::mutex> lock(occupyMutex_);
    auto it = occupyNums_.find(userId);
    if (it != occupyNums_.end()) {
        it->second = num;
    } else {
        occupyNums_[userId] = num;
    }
    LOGI("save occupy by nums: %{public}s", std::to_string(num).c_str());
}

void DfsOccupyManager::GetOccupyFromFile(const int32_t userId, int64_t &occupyNum)
{
    CloudPrefImpl cloudPrefImpl(userId, "", GLOBAL_CONFIG_FILE_PATH);
    cloudPrefImpl.GetLong(SUM_OCCUPY_FLAG, occupyNum);
    LOGI("get occupy nums: %{public}s", std::to_string(occupyNum).c_str());
}
} // namespace OHOS::FileManagement::CloudSync