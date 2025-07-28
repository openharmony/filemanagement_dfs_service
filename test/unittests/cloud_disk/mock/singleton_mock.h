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

#ifndef CLOUD_DISK_SINGLETON_MOCK_TEST_H
#define CLOUD_DISK_SINGLETON_MOCK_TEST_H

#include <memory>

namespace OHOS {
template <typename T>
class DelayedSingleton {
public:
    static std::shared_ptr<T> GetInstance()
    {
        if (instance_ == nullptr) {
            return nullptr;
        }
        return instance_;
    }
    static inline std::shared_ptr<T> instance_{nullptr};
};
} // namespace OHOS

#endif // CLOUD_DISK_SINGLETON_MOCK_TEST_H