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
#ifndef CLOUD_PREFERENCES_HELPER_H
#define CLOUD_PREFERENCES_HELPER_H
#include "preferences.h"
#include <gmock/gmock.h>
#include <memory>
#include <mutex>
#include <string>

namespace OHOS {
namespace NativePreferences {
class PreferenceHelperInterface {
public:
    virtual ~PreferenceHelperInterface() = default;
    virtual std::shared_ptr<Preferences> GetPreferences(const std::string &filePath, int &errCode) = 0;
    virtual void DeletePreferences(const std::string &path) = 0;
};

class MockPreferenceHelperInterface : public PreferenceHelperInterface {
public:
    MockPreferenceHelperInterface() = default;
    ~MockPreferenceHelperInterface() override = default;
    MOCK_METHOD2(GetPreferences, std::shared_ptr<Preferences>(const std::string &filePath, int &errCode));
    MOCK_METHOD1(DeletePreferences, void(const std::string &path));
};

class PreferencesHelper {
public:
    static std::shared_ptr<Preferences> GetPreferences(const std::string &filePath, int &errCode);
    static void DeletePreferences(const std::string &path);
    static std::shared_ptr<MockPreferenceHelperInterface> GetInstance();
    static void DeleteInstance();

private:
    static std::shared_ptr<MockPreferenceHelperInterface> instance_;
    static std::mutex mtx_;
};
} // namespace NativePreferences
} // namespace OHOS
#endif // CLOUD_PREFERENCES_HELPER_H