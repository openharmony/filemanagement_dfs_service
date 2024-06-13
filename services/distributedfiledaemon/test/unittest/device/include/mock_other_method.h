/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_DFS_SERVICE_DEVICE_OTHER_METHOD_MOCK_H
#define OHOS_DFS_SERVICE_DEVICE_OTHER_METHOD_MOCK_H

#include <gmock/gmock.h>
#include <memory>
#include <string>

#include "message_parcel.h"
#include "iremote_broker.h"

namespace OHOS::Storage::DistributedFile {
class DfsDeviceOtherMethod {
public:
    virtual ~DfsDeviceOtherMethod() = default;
public:
    virtual int QueryActiveOsAccountIds(std::vector<int32_t>& ids) = 0;
public:
    static inline std::shared_ptr<DfsDeviceOtherMethod> otherMethod = nullptr;
};

class DfsDeviceOtherMethodMock : public DfsDeviceOtherMethod {
public:
    MOCK_METHOD1(QueryActiveOsAccountIds, int(std::vector<int32_t>& ids));
};
}
#endif