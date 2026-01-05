/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#ifndef OHOS_CLOUD_STATUS_MOCK_H
#define OHOS_CLOUD_STATUS_MOCK_H

#include <gmock/gmock.h>
#include <memory>
#include <string>
#include "cloud_status.h"

namespace OHOS::FileManagement::CloudSync {
class CloudStatusMethod {
public:
    virtual ~CloudStatusMethod() = default;
    virtual bool IsCloudStatusOkay(const std::string &bundleName, const int32_t userId);
    static inline std::shared_ptr<CloudStatusMethod> proxy_ = nullptr;
};

class CloudStatusMethodMock : public CloudStatusMethod {
public:
    MOCK_METHOD2(IsCloudStatusOkay, bool(const std::string &bundleName, const int32_t userId));
};
}
#endif