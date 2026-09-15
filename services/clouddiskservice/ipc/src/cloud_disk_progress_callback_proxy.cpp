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


#include "cloud_disk_progress_callback_proxy.h"

#include "message_parcel.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
BrokerDelegator<CloudDiskProgressCallbackProxy> CloudDiskProgressCallbackProxy::delegator_;

void CloudDiskProgressCallbackProxy::OnProgress(const HydrateProgress &progress)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);
    auto remote = Remote();
    if (remote == nullptr || !data.WriteInterfaceToken(GetDescriptor()) || !data.WriteParcelable(&progress)) {
        LOGE("Serialize hydration progress failed");
        return;
    }
    int32_t ret = remote->SendRequest(ON_PROGRESS, data, reply, option);
    if (ret != 0) {
        LOGE("Send hydration progress failed, ret:%{public}d", ret);
    }
}
} // namespace OHOS::FileManagement::CloudDiskService
