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
#ifndef OHOS_FILEMGMT_CLOUD_PREF_IMPL_H
#define OHOS_FILEMGMT_CLOUD_PREF_IMPL_H
#include "preferences.h"
#include "preferences_helper.h"

namespace OHOS::FileManagement::CloudSync {
const static std::string START_CURSOR = "start_cursor";
const static std::string NEXT_CURSOR = "next_cursor";
const static std::string TEMP_START_CURSOR = "temp_start_cursor";
const static std::string CLOUDFILE_DIR = "/data/service/el1/public/cloudfile/";
class CloudPrefImpl {
public:
    CloudPrefImpl(const std::string& fileName);
    CloudPrefImpl(const int32_t userId, const std::string& bundleName, const std::string& tableName);
    ~CloudPrefImpl() = default;;

    void SetString(const std::string& key, const std::string& value);
    void GetString(const std::string& key, std::string& value);
    void SetLong(const std::string& key, const int64_t value);
    void GetLong(const std::string& key, int64_t& value);
    void SetInt(const std::string& key, const int value);
    void GetInt(const std::string& key, int32_t& value);
    void SetBool(const std::string& key, const bool& value);
    void GetBool(const std::string& key, bool& value);
    void Clear();
    void Delete(const std::string& key);

private:
    std::string fileName_;
    std::shared_ptr<NativePreferences::Preferences> pref_;
};
}
#endif // OHOS_FILEMGMT_CLOUD_PREF_IMPL_H