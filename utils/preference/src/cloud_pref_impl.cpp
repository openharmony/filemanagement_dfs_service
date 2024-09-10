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

#include "cloud_pref_impl.h"
#include <sys/stat.h>
#include <unistd.h>
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
namespace {
    static const uint32_t STAT_MODE_DIR = 0771;
}

CloudPrefImpl::CloudPrefImpl(const int32_t userId, const std::string& bundleName, const std::string& tableName)
{
    /* the file name varies from different userId and bundle name */
    std::string userIdDir = "/data/service/el2/" + std::to_string(userId) + "/hmdfs/cloudfile_manager";
    if (access(userIdDir.c_str(), F_OK) != 0) {
        if (mkdir(userIdDir.c_str(), STAT_MODE_DIR) != 0) {
            LOGE("CloudPrefImpl: mkdir failed");
        }
    }

    std::string bundleDir = userIdDir + "/" + bundleName;
    if (access(bundleDir.c_str(), F_OK) != 0) {
        if (mkdir(bundleDir.c_str(), STAT_MODE_DIR) != 0) {
            LOGE("CloudPrefImpl: mkdir failed");
        }
    }
    fileName_ = bundleDir + "/" + tableName;
    int32_t errCode = 0;
    pref_ = NativePreferences::PreferencesHelper::GetPreferences(fileName_, errCode);
    if (!pref_) {
        LOGE("CloudPrefImpl: Preference get null, errcode: %{public}d", errCode);
    }
}

CloudPrefImpl::CloudPrefImpl(const std::string& fileName)
{
    int32_t errCode = 0;
    fileName_ = fileName;
    pref_ = NativePreferences::PreferencesHelper::GetPreferences(fileName_, errCode);
    if (!pref_) {
        LOGE("CloudPrefImpl: Preference get null, errcode: %{public}d", errCode);
    }
}

void CloudPrefImpl::SetString(const std::string& key, const std::string& value)
{
    pref_->PutString(key, value);
    int ret = pref_->FlushSync();
    if (ret) {
        LOGE("CloudPrefImpl: SetString has error, ret = %{public}d", ret);
    }
}

void CloudPrefImpl::GetString(const std::string& key, std::string &value)
{
    value = pref_->GetString(key, "");
}

void CloudPrefImpl::SetLong(const std::string& key, const int64_t value)
{
    pref_->PutLong(key, value);
    int ret = pref_->FlushSync();
    if (ret) {
        LOGE("CloudPrefImpl: SetLong has error, ret = %{public}d", ret);
    }
}

void CloudPrefImpl::GetLong(const std::string& key, int64_t &value)
{
    value = pref_->GetLong(key, 0);
}

void CloudPrefImpl::SetInt(const std::string& key, const int value)
{
    pref_->PutInt(key, value);
    int ret = pref_->FlushSync();
    if (ret) {
        LOGE("CloudPrefImpl: SetInt has error, ret = %{public}d", ret);
    }
}

void CloudPrefImpl::GetInt(const std::string& key, int32_t &value)
{
    value = pref_->GetInt(key, 0);
}

void CloudPrefImpl::SetBool(const std::string& key, const bool& value)
{
    pref_->PutBool(key, value);
    int ret = pref_->FlushSync();
    if (ret) {
        LOGE("CloudPrefImpl: SetBool has error, ret = %{public}d", ret);
    }
}

void CloudPrefImpl::GetBool(const std::string& key, bool& value)
{
    value = pref_->GetBool(key, false);
}

void CloudPrefImpl::Clear()
{
    pref_->Clear();
    NativePreferences::PreferencesHelper::DeletePreferences(fileName_);
}

void CloudPrefImpl::Delete(const std::string& key)
{
    pref_->Delete(key);
    int ret = pref_->FlushSync();
    if (ret) {
        LOGE("CloudPrefImpl: Delete has error, ret = %{public}d", ret);
    }
}
} // namespace OHOS::FileManagement::CloudSync