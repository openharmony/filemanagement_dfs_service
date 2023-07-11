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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "preferences_helper.h"

namespace OHOS {
namespace NativePreferences {

class PreferencesHelperMock : public Preferences {
public:
    MOCK_METHOD2(Get, PreferencesValue(const std::string &key, const PreferencesValue &defValue));
    MOCK_METHOD2(Put, int(const std::string &key, const PreferencesValue &value));
    MOCK_METHOD2(GetInt, int(const std::string &key, const int &defValue));
    MOCK_METHOD2(GetString, std::string(const std::string &key, const std::string &defValue));
    MOCK_METHOD2(GetBool, bool(const std::string &key, const bool &defValue));
    MOCK_METHOD2(GetFloat, float(const std::string &key, const float &defValue));
    MOCK_METHOD2(GetDouble, double(const std::string &key, const double &defValue));
    MOCK_METHOD2(GetLong, int64_t(const std::string &key, const int64_t &defValue));
    MOCK_METHOD0(GetAll, std::map<std::string, PreferencesValue>());
    MOCK_METHOD1(HasKey, bool(const std::string &key));
    MOCK_METHOD2(PutInt, int(const std::string &key, int value));
    MOCK_METHOD2(PutString, int(const std::string &key, const std::string &value));
    MOCK_METHOD2(PutBool, int(const std::string &key, bool value));
    MOCK_METHOD2(PutLong, int(const std::string &key, int64_t value));
    MOCK_METHOD2(PutFloat, int(const std::string &key, float value));
    MOCK_METHOD2(PutDouble, int(const std::string &key, double value));
    MOCK_METHOD1(Delete, int(const std::string &key));
    MOCK_METHOD0(Clear, int());
    MOCK_METHOD0(Flush, void());
    MOCK_METHOD0(FlushSync, int());
    MOCK_METHOD2(RegisterObserver, int(std::shared_ptr<PreferencesObserver> preferencesObserver, RegisterMode mode));
    MOCK_METHOD2(UnRegisterObserver, int(std::shared_ptr<PreferencesObserver> preferencesObserver, RegisterMode mode));
};

std::string PreferencesHelper::GetRealPath(const std::string &path, int &errorCode)
{
    return "";
}

std::shared_ptr<Preferences> PreferencesHelper::GetPreferences(const Options &options, int &errCode)
{
    if (options.filePath == "") {
        return nullptr;
    }
    std::shared_ptr<PreferencesHelperMock> pref = std::make_shared<PreferencesHelperMock>();
    return pref;
}

int PreferencesHelper::DeletePreferences(const std::string &path)
{
    return 0;
}

int PreferencesHelper::RemovePreferencesFromCache(const std::string &path)
{
    return 0;
}
} // End of namespace NativePreferences
} // End of namespace OHOS