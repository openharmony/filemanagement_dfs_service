/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_DECOMPRESS_H
#define OHOS_FILEMGMT_DECOMPRESS_H

#include "visibility.h"
#include <cstdint>
#include <vector>

namespace OHOS {
namespace FileManagement {
namespace Decompress {

API_EXPORT bool CreateInnerFile(const std::string &sourcePath,
                                const std::string &targetPath,
                                const uint32_t &offset,
                                const uint32_t &size,
                                bool isSystemApp);

API_EXPORT bool CheckBundleSupported(const std::string &bundleName, const bool isKeepAlive);

API_EXPORT bool GetSystemFeature();

} // namespace Decompress
} // namespace FileManagement
} // namespace OHOS

#endif // OHOS_FILEMGMT_DECOMPRESS_H