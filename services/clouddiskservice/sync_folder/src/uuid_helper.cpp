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

#include "uuid_helper.h"

namespace OHOS::FileManagement::CloudDiskService {

std::string UuidHelper::GenerateUuid()
{
    std::string guid("");
    uuid_t uuid;
    uuid_generate(uuid);
    char str[50] = {};
    uuid_unparse(uuid, str);
    guid.assign(str);
    return guid;
}

std::string UuidHelper::GenerateUuidWithoutDelim()
{
    std::string str = GenerateUuid();
    std::string uuid;
    for (auto &ch : str) {
        if (ch != '-') {
            uuid += ch;
        }
    }
    return uuid;
}

std::string UuidHelper::GenerateUuidOnly()
{
    std::string guid("");
    uuid_t uuid;
    uuid_generate(uuid);
    int length = 16;
    guid.assign(reinterpret_cast<char *>(uuid), length);
    return guid;
}

} // namespace OHOS::FileManagement::CloudDiskService