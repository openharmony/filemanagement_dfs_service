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

#ifndef TEST_OHOS_FILEMANAGEMENT_CLOUDDISKSERVICE_CLOUDDISKSERVICESTUB_H
#define TEST_OHOS_FILEMANAGEMENT_CLOUDDISKSERVICE_CLOUDDISKSERVICESTUB_H

#include <iremote_stub.h>
#include "icloud_disk_service.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDiskService {

class CloudDiskServiceStub : public IRemoteStub<ICloudDiskService> {
public:
    CloudDiskServiceStub(bool serialInvokeFlag = false): IRemoteStub(serialInvokeFlag){};
    int32_t OnRemoteRequest(
        uint32_t code,
        MessageParcel& data,
        MessageParcel& reply,
        MessageOption& option) override;
};
} // namespace CloudDiskService
} // namespace FileManagement
} // namespace OHOS
#endif // TEST_OHOS_FILEMANAGEMENT_CLOUDDISKSERVICE_CLOUDDISKSERVICESTUB_H