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

#include "ipc/cloud_sync_callback_proxy.h"

#include <sstream>

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

void CloudSyncCallbackProxy::OnStartSyncFinished(const std::string &appPackageName, int32_t errCode)
{
    LOGI("Start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(GetDescriptor());

    if (!data.WriteString(appPackageName)) {
        LOGE("Failed to send the appPackageName");
        return;
    }

    if (!data.WriteInt32(errCode)) {
        LOGE("Failed to send the appPackageName");
        return;
    }

    int32_t ret = Remote()->SendRequest(ICloudSyncCallback::SERVICE_CMD_ON_START_SYNC_FINISHED, data, reply, option);
    if (ret != E_OK) {
        stringstream ss;
        ss << "Failed to send out the requeset, errno:" << ret;
        LOGE("%{public}s, appPackageName:%{public}s", ss.str().c_str(), appPackageName.c_str());
        return;
    }
    LOGI("End");
    return;
}

void CloudSyncCallbackProxy::OnStopSyncFinished(const std::string &appPackageName, int32_t errCode) {}

} // namespace OHOS::FileManagement::CloudSync