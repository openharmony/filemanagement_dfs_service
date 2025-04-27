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
#include "preferences_helper.h"

namespace OHOS {
namespace NativePreferences {
std::mutex PreferencesHelper::mtx_;
std::shared_ptr<MockPreferenceHelperInterface> PreferencesHelper::instance_ = nullptr;
std::shared_ptr<Preferences> PreferencesHelper::GetPreferences(const std::string &filePath, int &errCode)
{
    if (instance_ == nullptr) {
        return nullptr;
    }
    return instance_->GetPreferences(filePath, errCode);
}

void PreferencesHelper::DeletePreferences(const std::string &path)
{
    if (instance_ != nullptr) {
        instance_->DeletePreferences(path);
    }
}

std::shared_ptr<MockPreferenceHelperInterface> PreferencesHelper::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mtx_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<MockPreferenceHelperInterface>();
        }
    }
    return instance_;
}

void PreferencesHelper::DeleteInstance()
{
    std::lock_guard<std::mutex> lock(mtx_);
    instance_.reset();
}
} // End of namespace NativePreferences
} // End of namespace OHOS