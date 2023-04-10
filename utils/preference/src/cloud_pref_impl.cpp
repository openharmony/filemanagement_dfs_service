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
CloudPrefImpl::CloudPrefImpl(std::string fileName)
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

void CloudPrefImpl::SetInt(const std::string& key, const int value)
{
    pref_->PutInt(key, value);
    pref_->Flush();
}

void CloudPrefImpl::GetInt(const std::string& key, int32_t &value)
{
    value = pref_->GetInt(key, 0);
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