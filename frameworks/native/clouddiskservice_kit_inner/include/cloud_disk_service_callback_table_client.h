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

#ifndef OHOS_FILEMGMT_CLOUD_DISK_SERVICE_CALLBACK_TABLE_CLIENT_H
#define OHOS_FILEMGMT_CLOUD_DISK_SERVICE_CALLBACK_TABLE_CLIENT_H

#include <atomic>

#include "cloud_disk_service_callback_table.h"
#include "cloud_disk_service_callback_table_stub.h"

namespace OHOS::FileManagement::CloudDiskService {
class CloudDiskServiceCallbackTableClient final : public CloudDiskServiceCallbackTableStub {
public:
    explicit CloudDiskServiceCallbackTableClient(const std::shared_ptr<CloudDiskServiceCallbackTable> &callbackTable)
        : callbackTable_(callbackTable)
    {
    }

    void OnCallback(const CloudDiskCallbackReqHead &reqHead, CloudDiskCallbackContext &reqContext) override;
    void SetActive(bool active);

private:
    std::shared_ptr<CloudDiskServiceCallbackTable> callbackTable_{nullptr};
    std::atomic<bool> active_{true};
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // OHOS_FILEMGMT_CLOUD_DISK_SERVICE_CALLBACK_TABLE_CLIENT_H
