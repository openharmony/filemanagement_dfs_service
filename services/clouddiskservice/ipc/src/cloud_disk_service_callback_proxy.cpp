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

#include "cloud_disk_service_callback_proxy.h"

#include "cloud_disk_service_error.h"
#include "cloud_file_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;

BrokerDelegator<CloudDiskServiceCallbackTableProxy> CloudDiskServiceCallbackTableProxy::delegator_;

void CloudDiskServiceCallbackProxy::OnChangeData(const std::string &sandboxPath,
                                                 const std::vector<ChangeData> &changeData)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return;
    }

    if (changeData.size() > static_cast<size_t>(VECTOR_MAX_SIZE)) {
        return;
    }

    data.WriteString(sandboxPath);
    data.WriteInt32(changeData.size());
    for (auto item = changeData.begin(); item != changeData.end(); ++item) {
        if (!data.WriteParcelable(&(*item))) {
            return;
        }
    }
    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return;
    }
    int32_t ret = remote->SendRequest(ICloudDiskServiceCallback::SERVICE_CMD_ON_CHANGE_DATA, data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, ret:%{public}d", ret);
        return;
    }
    return;
}

void CloudDiskServiceCallbackTableProxy::OnCallback(const CloudDiskCallbackReqHead &reqHead,
                                                    CloudDiskCallbackContext &reqContext)
{
    (void)SendCallback(reqHead, reqContext);
}

int32_t CloudDiskServiceCallbackTableProxy::SendCallback(const CloudDiskCallbackReqHead &reqHead,
                                                         CloudDiskCallbackContext &reqContext)
{
    MessageParcel data;
    MessageParcel reply;
    bool isOneWay = reqHead.callbackType == CloudDiskCallbackType::FETCH_DATA ||
                    reqHead.callbackType == CloudDiskCallbackType::CANCEL_FETCH_DATA;
    MessageOption option(isOneWay ? MessageOption::TF_ASYNC : MessageOption::TF_SYNC);
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write callback table interface token");
        return E_INVALID_ARG;
    }
    if (!WriteCallbackParcel(data, reqHead, reqContext)) {
        LOGE("Failed to write callback request");
        return E_INVALID_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("Callback table remote is nullptr");
        return E_IPC_FAILED;
    }
    int32_t ret = remote->SendRequest(ICloudDiskServiceCallbackTable::SERVICE_CMD_ON_CALLBACK, data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send callback request, ret: %{public}d", ret);
        return E_IPC_FAILED;
    }
    if (!isOneWay && !ReadCallbackReply(reply, reqHead.callbackType, reqContext)) {
        LOGE("Failed to read callback response");
        return E_IPC_FAILED;
    }
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudDiskService
