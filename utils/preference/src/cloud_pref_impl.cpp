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
CloudPrefImpl::CloudPrefImpl(const int32_t userId, const std::string& bundleName, const std::string& tableName)
{
    /* the file name varies from different userId and bundle name */
    std::string userIdDir = CLOUDFILE_DIR + std::to_string(userId);
    if (access(userIdDir.c_str(), F_OK) != 0) {
        if (mkdir(userIdDir.c_str(), S_IRWXU | S_IRWXG | S_IXOTH) != 0) {
            LOGE("CloudPrefImpl: mkdir failed");
        }
    }

    std::string bundleDir = userIdDir + "/" + bundleName;
    if (access(bundleDir.c_str(), F_OK) != 0) {
        if (mkdir(bundleDir.c_str(), S_IRWXU | S_IRWXG | S_IXOTH) != 0) {
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
    pref_->Flush();
}

void CloudPrefImpl::GetString(const std::string& key, std::string &value)
{
    value = pref_->GetString(key, "");
}

void CloudPrefImpl::SetLong(const std::string& key, const int64_t value)
{
    pref_->PutLong(key, value);
    pref_->Flush();
}

void CloudPrefImpl::GetLong(const std::string& key, int64_t &value)
{
    value = pref_->GetLong(key, 0);
}

void CloudPrefImpl::SetInt(const std::string& key, const int value)
{
    pref_->PutInt(key, value);
    pref_->Flush();
}

void CloudPrefImpl::GetInt(const std::string& key, int32_t &value)
{
    value = pref_->GetInt(key, 0);
}

void CloudPrefImpl::SetBool(const std::string& key, const bool& value)
{
    pref_->PutBool(key, value);
    pref_->Flush();
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
    pref_->FlushSync();
}
} // namespace OHOS::FileManagement::CloudSync