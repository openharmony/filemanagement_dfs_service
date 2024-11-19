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

#include "cloud_status.h"

#include "cloud_file_kit.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
int32_t CloudStatus::GetCurrentCloudInfo(const std::string &bundleName, const int32_t userId)
{
    auto instance = CloudFile::CloudFileKit::GetInstance();
    if (instance == nullptr) {
        LOGE("get cloud file helper instance failed");
        return E_NULLPTR;
    }

    auto ret = instance->GetCloudUserInfo(userId, userInfo_);
    if (ret != E_OK) {
        return ret;
    }

    bool switchStatus = false;
    ret = instance->GetAppSwitchStatus(bundleName, userId, switchStatus);
    if (ret != E_OK) {
        return ret;
    }
    /* insert key-value */
    appSwitches_.insert(std::make_pair(bundleName, switchStatus));
    userId_ = userId;
    return E_OK;
}

uint64_t CloudStatus::GetCurrentRemainSpace(const int32_t userId)
{
    auto instance = CloudFile::CloudFileKit::GetInstance();
    return instance->GetRemainSpace(userId);
}

bool CloudStatus::IsCloudStatusOkay(const std::string &bundleName, const int32_t userId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    /* User switching */
    if (userId_ != userId) {
        appSwitches_.erase(bundleName);
    }

    /* Obtain cloud information only during first sync */
    auto iter = appSwitches_.find(bundleName);
    if (iter == appSwitches_.end()) {
        LOGI("appSwitches unknown, bundleName:%{private}s, userId:%{public}d", bundleName.c_str(), userId);
        auto ret = GetCurrentCloudInfo(bundleName, userId);
        if (ret) {
            return false;
        }
    }

    LOGI("bundleName:%{private}s, cloudSatus:%{public}d, switcheStatus:%{public}d", bundleName.c_str(),
         userInfo_.enableCloud, appSwitches_[bundleName]);
    return appSwitches_[bundleName];
}

int32_t CloudStatus::ChangeAppSwitch(const std::string &bundleName, const int32_t userId, bool appSwitchStatus)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (appSwitchStatus == true) {
        auto iter = appSwitches_.find(bundleName);
        if (iter != appSwitches_.end()) {
            LOGI("change app swtich, originStatus:%{public}d, currentStatus:%{public}d", appSwitches_[bundleName],
                 appSwitchStatus);
            appSwitches_[bundleName] = appSwitchStatus;
        }
    } else {
        /* Actively obtaining cloud information when next sync */
        appSwitches_.erase(bundleName);
    }

    return E_OK;
}

bool CloudStatus::IsAccountIdChanged(const std::string &accountId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if ((userInfo_.accountId != "") && (userInfo_.accountId != accountId)) {
        /* accountId Changed, clear init flag */
        appSwitches_.clear();
        return true;
    }
    return false;
}
} // namespace OHOS::FileManagement::CloudSync