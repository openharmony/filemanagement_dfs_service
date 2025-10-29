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

#include "convertor.h"

#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {

std::string Convertor::ConvertToHex(uint64_t value)
{
    int length = 16;
    uint64_t base = 10;
    uint64_t offset = 4;
    char buffer[length + 1];
    char *p = buffer + length;
    *p = '\0';

    for (int i = 0; i < length; i++) {
        --p;
        uint64_t digit = value & 0xF;
        *p = static_cast<char>(digit < base ? '0' + digit : 'a' + digit - base);
        value >>= offset;
    }

    return std::string(buffer);
}

uint64_t Convertor::ConvertFromHex(const std::string &hex)
{
    uint64_t value = 0;
    uint64_t base = 10;
    uint64_t offset = 4;
    for (char c : hex) {
        value <<= offset;
        if (c >= '0' && c <= '9') {
            value |= c - '0';
        } else if (c >= 'a' && c <= 'f') {
            value |= c - 'a' + base;
        } else if (c >= 'A' && c <= 'F') {
            value |= c - 'A' + base;
        } else {
            LOGE("Invalid hex string: %{public}s", hex.c_str());
            return 0;
        }
    }
    return value;
}

} // namespace OHOS::FileManagement::CloudDiskService