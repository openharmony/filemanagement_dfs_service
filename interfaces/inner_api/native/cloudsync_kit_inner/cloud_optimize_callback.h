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

#ifndef OHOS_FILEMGMT_CLOUD_OPTIMIZE_CALLBACK_H
#define OHOS_FILEMGMT_CLOUD_OPTIMIZE_CALLBACK_H

#include <functional>
#include "cloud_sync_constants.h"

namespace OHOS::FileManagement::CloudSync {

class CloudOptimizeCallback {
public:
    virtual ~CloudOptimizeCallback() = default;
    virtual void OnOptimizeProcess(const OptimizeState state, const int32_t progress) = 0;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_OPTIMIZE_CALLBACK_H
