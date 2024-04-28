/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "clouddisk_notify.h"

namespace OHOS::FileManagement::CloudDisk {

CloudDiskNotify &CloudDiskNotify::GetInstance()
{
    static CloudDiskNotify instance_;
    return instance_;
}

CloudDiskNotify::CloudDiskNotify() {}

CloudDiskNotify::~CloudDiskNotify() {}

void CloudDiskNotify::TryNotify(const NotifyParamDisk &paramDisk, const ParamDiskOthers &paramOthers) {}

void CloudDiskNotify::TryNotifyService(const NotifyParamService &paramService, const ParamServiceOther &paramOthers) {}

int32_t CloudDiskNotify::GetDeleteNotifyData(const vector<NativeRdb::ValueObject> &deleteIds,
                                             vector<NotifyData> &notifyDataList,
                                             const ParamServiceOther &paramOthers)
{
    return 0;
}

void CloudDiskNotify::AddNotify(NotifyData notifyData) {}

void CloudDiskNotify::NotifyChangeOuter() {}
} // namespace OHOS::FileManagement::CloudDisk
