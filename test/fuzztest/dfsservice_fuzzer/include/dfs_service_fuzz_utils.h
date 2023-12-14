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
#ifndef DFS_SERVICE_DFS_SERVICE_FUZZ_UTILS_H
#define DFS_SERVICE_DFS_SERVICE_FUZZ_UTILS_H

#include <cstddef>
#include <cstdint>

namespace OHOS {
class DfsServiceFuzzUtils {
public:
    static void FuzzTestRemoteRequest(const uint8_t *rawData, size_t size, uint32_t code);
};
} // namespace OHOS
#endif // DFS_SERVICE_DFS_SERVICE_FUZZ_UTILS_H
