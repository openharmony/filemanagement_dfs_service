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


#include "cloud_disk_progress_callback_stub.h"

#include <memory>

#include "cloud_disk_service_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
int32_t CloudDiskProgressCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        LOGE("Invalid progress interface token");
        return E_INVALID_ARG;
    }
    if (code != ON_PROGRESS) {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    std::unique_ptr<HydrateProgress> progress(data.ReadParcelable<HydrateProgress>());
    if (progress == nullptr) {
        LOGE("Invalid hydration progress parcel");
        return E_INVALID_ARG;
    }
    OnProgress(*progress);
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudDiskService
