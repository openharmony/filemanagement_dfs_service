/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_CLOUDDISK_PARSE_XATTR_INT_H
#define OHOS_FILEMGMT_CLOUDDISK_PARSE_XATTR_INT_H

#include <charconv>
#include <cstdint>
#include <string>
#include <system_error>

namespace OHOS::FileManagement::CloudDisk {

/* Fail closed: empty / junk / overflow must not throw (leftover isdigit+stoi). */
inline bool ParseXattrInt32(const std::string &str, int32_t &out)
{
    if (str.empty()) {
        return false;
    }
    int32_t value = 0;
    const char *first = str.data();
    const char *last = str.data() + str.size();
    auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

inline bool ParseXattrUint32(const std::string &str, uint32_t &out)
{
    if (str.empty()) {
        return false;
    }
    uint32_t value = 0;
    const char *first = str.data();
    const char *last = str.data() + str.size();
    auto result = std::from_chars(first, last, value);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

} // namespace OHOS::FileManagement::CloudDisk

#endif // OHOS_FILEMGMT_CLOUDDISK_PARSE_XATTR_INT_H
