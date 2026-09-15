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


#ifndef OHOS_FILEMGMT_CLOUD_DISK_PROGRESS_CALLBACK_PROXY_H
#define OHOS_FILEMGMT_CLOUD_DISK_PROGRESS_CALLBACK_PROXY_H

#include "i_cloud_disk_progress_callback.h"
#include "iremote_proxy.h"

namespace OHOS::FileManagement::CloudDiskService {
class CloudDiskProgressCallbackProxy final : public IRemoteProxy<ICloudDiskProgressCallback> {
public:
    explicit CloudDiskProgressCallbackProxy(const sptr<IRemoteObject> &remote)
        : IRemoteProxy<ICloudDiskProgressCallback>(remote) {}
    void OnProgress(const HydrateProgress &progress) override;

private:
    static BrokerDelegator<CloudDiskProgressCallbackProxy> delegator_;
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // OHOS_FILEMGMT_CLOUD_DISK_PROGRESS_CALLBACK_PROXY_H
