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


#ifndef OHOS_FILEMGMT_CLOUD_DISK_PROGRESS_CALLBACK_CLIENT_H
#define OHOS_FILEMGMT_CLOUD_DISK_PROGRESS_CALLBACK_CLIENT_H

#include <mutex>
#include <unordered_map>

#include "cloud_disk_progress_callback_stub.h"

namespace OHOS::FileManagement::CloudDiskService {
class CloudDiskProgressCallbackClient final : public CloudDiskProgressCallbackStub {
public:
    int32_t Add(uint64_t accessorId, const sptr<ICloudDiskProgressCallback> &callback, bool &added);
    bool Remove(uint64_t accessorId, const sptr<ICloudDiskProgressCallback> &callback);
    void OnProgress(const HydrateProgress &progress) override;

private:
    std::mutex mutex_;
    std::unordered_map<uint64_t, sptr<ICloudDiskProgressCallback>> accessorCallbacks_;
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // OHOS_FILEMGMT_CLOUD_DISK_PROGRESS_CALLBACK_CLIENT_H
