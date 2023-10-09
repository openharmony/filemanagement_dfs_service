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

#include "sync_rule/cloud_status.h"

#include "dfs_error.h"
#include "drive_kit.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace DriveKit;
int32_t CloudStatus::GetCurrentCloudInfo(const std::string &bundleName, const int32_t userId)
{
    auto driveKit = DriveKit::DriveKitNative::GetInstance(userId);
    if (driveKit == nullptr) {
        LOGE("sdk helper get drive kit instance fail");
        return E_CLOUD_SDK;
    }

    auto err = driveKit->GetCloudUserInfo(userInfo_);
    if (err.HasError()) {
        LOGE("GetCloudUserInfo failed, server err:%{public}d and dk err:%{public}d", err.serverErrorCode,
             err.dkErrorCode);
        return E_CLOUD_SDK;
    }

    if (!(userInfo_.cloudStatus == DKCloudStatus::DK_CLOUD_STATUS_LOGIN)) {
        LOGE("cloudstatus:%{public}d, spaceStatus:%{public}d", userInfo_.cloudStatus, userInfo_.spaceStatus);
        return E_CLOUD_SDK;
    }

    std::map<DriveKit::DKAppBundleName, DriveKit::DKAppInfo> appInfos;
    err = driveKit->GetCloudAppInfo({bundleName}, appInfos);
    if (err.HasError()) {
        LOGE("GetCloudAppInfo failed, server err:%{public}d and dk err:%{public}d", err.serverErrorCode,
             err.dkErrorCode);
        return E_CLOUD_SDK;
    }
    auto appInfo = appInfos[bundleName];
    if (appInfo.enableCloud != 1) {
        LOGE("unexpected status:%{public}d, bundleName:%{private}s", appInfo.enableCloud, bundleName.c_str());
        return E_CLOUD_SDK;
    }
    auto switchStatus = appInfo.switchStatus;
    /* Record the log when the switch is not open */
    if (switchStatus != DKAppSwitchStatus::DK_APP_SWITCH_STATUS_OPEN) {
        LOGI("bundleName:%{private}s, switchStatus:%{public}d", bundleName.c_str(), switchStatus);
    }
    /* insert key-value */
    appSwitches_.insert(std::make_pair(bundleName, switchStatus == DKAppSwitchStatus::DK_APP_SWITCH_STATUS_OPEN));
    userId_ = userId;
    return E_OK;
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

    LOGI("bundleName:%{private}s, cloudSatus:%{public}d, spaceStatus:%{public}d, switcheStatus:%{public}d",
         bundleName.c_str(), userInfo_.cloudStatus, userInfo_.spaceStatus, appSwitches_[bundleName]);
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