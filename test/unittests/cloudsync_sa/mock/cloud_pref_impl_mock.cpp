/*
 * Copyright (C) 2023 Huawei Device Co., Ltd. All rights reserved.
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
}

CloudPrefImpl::CloudPrefImpl(const std::string& fileName)
{
}

void CloudPrefImpl::SetString(const std::string& key, const std::string& value)
{
}

void CloudPrefImpl::GetString(const std::string& key, std::string &value)
{
}

void CloudPrefImpl::SetLong(const std::string& key, const int64_t value)
{
}

void CloudPrefImpl::GetLong(const std::string& key, int64_t &value)
{
}

void CloudPrefImpl::SetInt(const std::string& key, const int value)
{
}

void CloudPrefImpl::GetInt(const std::string& key, int32_t &value)
{
}

void CloudPrefImpl::SetBool(const std::string& key, const bool& value)
{
}

void CloudPrefImpl::GetBool(const std::string& key, bool& value)
{
}

void CloudPrefImpl::Clear()
{
}

void CloudPrefImpl::Delete(const std::string& key)
{
}
} // namespace OHOS::FileManagement::CloudSync