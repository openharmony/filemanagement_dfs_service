/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "cloud_daemon_service_proxy.h"

#include <sstream>

#include "cloud_file_daemon_interface_code.h"
#include "dfs_error.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudFile {
using namespace std;
using namespace OHOS::FileManagement;

int32_t CloudDaemonServiceProxy::StartFuse(int32_t userId, int32_t devFd, const string &path)
{
    LOGI("Start fuse");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteInt32(userId)) {
        LOGE("Failed to send user id");
        return E_INVAL_ARG;
    }

    if (!data.WriteFileDescriptor(devFd)) {
        LOGE("Failed to send the device file fd");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(path)) {
        LOGE("Failed to send the device file path");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(CloudFileDaemonInterfaceCode::CLOUD_DAEMON_CMD_START_FUSE),
                                      data, reply, option);
    if (ret != E_OK) {
        stringstream ss;
        ss << "Failed to send out the requeset, errno:" << ret;
        LOGE("%{public}s", ss.str().c_str());
        return E_BROKEN_IPC;
    }
    LOGI("StartFuseInner Success");
    return reply.ReadInt32();
}

sptr<ICloudDaemon> CloudDaemonServiceProxy::GetInstance()
{
    LOGI("getinstance");
    unique_lock<mutex> lock(proxyMutex_);
    if (serviceProxy_ != nullptr) {
        return serviceProxy_;
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Samgr is nullptr");
        return nullptr;
    }

    auto object = samgr->CheckSystemAbility(FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID);
    if (object == nullptr) {
        LOGE("CloudDaemon::Connect object == nullptr");
        return nullptr;
    }

    serviceProxy_ = iface_cast<ICloudDaemon>(object);
    if (serviceProxy_ == nullptr) {
        LOGE("CloudDaemon::Connect service == nullptr");
        return nullptr;
    }
    return serviceProxy_;
}

void CloudDaemonServiceProxy::InvaildInstance()
{
    LOGI("invalid instance");
    unique_lock<mutex> lock(proxyMutex_);
    serviceProxy_ = nullptr;
}
} // namespace OHOS::FileManagement::CloudFile
