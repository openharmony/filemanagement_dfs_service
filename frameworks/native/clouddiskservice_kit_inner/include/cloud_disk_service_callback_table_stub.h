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

#ifndef OHOS_FILEMGMT_CLOUD_DISK_SERVICE_CALLBACK_TABLE_STUB_H
#define OHOS_FILEMGMT_CLOUD_DISK_SERVICE_CALLBACK_TABLE_STUB_H

#include <functional>
#include <map>

#include "i_cloud_disk_service_callback_table.h"
#include "iremote_stub.h"

namespace OHOS::FileManagement::CloudDiskService {
using CallbackTableInterface = std::function<int32_t(MessageParcel &data, MessageParcel &reply)>;

class CloudDiskServiceCallbackTableStub : public IRemoteStub<ICloudDiskServiceCallbackTable> {
public:
    CloudDiskServiceCallbackTableStub();
    ~CloudDiskServiceCallbackTableStub() override = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t HandleOnCallback(MessageParcel &data, MessageParcel &reply);

    std::map<uint32_t, CallbackTableInterface> opToInterfaceMap_;
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // OHOS_FILEMGMT_CLOUD_DISK_SERVICE_CALLBACK_TABLE_STUB_H
