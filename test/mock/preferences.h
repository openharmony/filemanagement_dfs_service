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

#ifndef CLOUD_PREFERENCES_H
#define CLOUD_PREFERENCES_H
#include <gmock/gmock.h>
#include <string>

namespace OHOS {
namespace NativePreferences {
class PreferencesInterface {
public:
    virtual ~PreferencesInterface() = default;
    virtual int GetInt(const std::string &key, const int &defValue = {}) = 0;
    virtual std::string GetString(const std::string &key, const std::string &defValue = {}) = 0;
    virtual bool GetBool(const std::string &key, const bool &defValue = {}) = 0;
    virtual int64_t GetLong(const std::string &key, const int64_t &defValue = {}) = 0;
    virtual int PutInt(const std::string &key, int value) = 0;
    virtual int PutString(const std::string &key, const std::string &value) = 0;
    virtual int PutBool(const std::string &key, bool value) = 0;
    virtual int PutLong(const std::string &key, int64_t value) = 0;
    virtual int Delete(const std::string &key) = 0;
    virtual int Clear() = 0;
    virtual int FlushSync() = 0;
};

class Preferences : public PreferencesInterface {
public:
    MOCK_METHOD2(GetInt, int(const std::string &key, const int &defValue));
    MOCK_METHOD2(GetString, std::string(const std::string &key, const std::string &defValue));
    MOCK_METHOD2(GetBool, bool(const std::string &key, const bool &defValue));
    MOCK_METHOD2(GetLong, int64_t(const std::string &key, const int64_t &defValue));
    MOCK_METHOD2(PutInt, int(const std::string &key, int value));
    MOCK_METHOD2(PutString, int(const std::string &key, const std::string &value));
    MOCK_METHOD2(PutBool, int(const std::string &key, bool value));
    MOCK_METHOD2(PutLong, int(const std::string &key, int64_t value));
    MOCK_METHOD1(Delete, int(const std::string &key));
    MOCK_METHOD0(Clear, int());
    MOCK_METHOD0(FlushSync, int());
};
} // namespace NativePreferences
} // namespace OHOS
#endif // CLOUD_PREFERENCES_H