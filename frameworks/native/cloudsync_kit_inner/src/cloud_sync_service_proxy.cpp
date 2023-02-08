/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "cloud_sync_service_proxy.h"

#include <sstream>

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

int32_t CloudSyncServiceProxy::RegisterCallbackInner(const string &appPackageName,
                                                     const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Start RegisterCallbackInner");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!remoteObject) {
        LOGI("Empty callback stub");
        return E_INVAL_ARG;
    }
    data.WriteInterfaceToken(GetDescriptor());

    if (!data.WriteRemoteObject(remoteObject)) {
        LOGE("Failed to send the callback stub");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(appPackageName)) {
        LOGE("Failed to send the appPackageName");
        return E_INVAL_ARG;
    }

    int32_t ret = Remote()->SendRequest(ICloudSyncService::SERVICE_CMD_REGISTER_CALLBACK, data, reply, option);
    if (ret != E_OK) {
        stringstream ss;
        ss << "Failed to send out the requeset, errno:" << ret;
        LOGE("%{public}s, appPackageName:%{public}s", ss.str().c_str(), appPackageName.c_str());
        return E_BROKEN_IPC;
    }
    LOGI("RegisterCallbackInner Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::StartSyncInner(const std::string &appPackageName, SyncType type, bool forceFlag)
{
    return E_OK;
}
int32_t CloudSyncServiceProxy::StopSyncInner(const std::string &appPackageName)
{
    return E_OK;
}

sptr<ICloudSyncService> CloudSyncServiceProxy::GetInstance()
{
    return serviceProxy_;
}

void CloudSyncServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
}

void CloudSyncServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId) {}

} // namespace OHOS::FileManagement::CloudSync