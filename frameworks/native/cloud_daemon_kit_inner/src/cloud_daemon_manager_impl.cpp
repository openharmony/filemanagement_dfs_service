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

#include "cloud_daemon_manager_impl.h"
#include "cloud_daemon_service_proxy.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudFile {
using namespace std;
using namespace OHOS::FileManagement;
CloudDaemonManagerImpl &CloudDaemonManagerImpl::GetInstance()
{
    static CloudDaemonManagerImpl instance;
    return instance;
}

int32_t CloudDaemonManagerImpl::StartFuse(int32_t userId, int32_t devFd, const string &path)
{
    auto CloudDaemonServiceProxy = CloudDaemonServiceProxy::GetInstance();
    if (!CloudDaemonServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    if (!isFirstCall_.test_and_set()) {
        SetDeathRecipient(CloudDaemonServiceProxy->AsObject());
    }

    return CloudDaemonServiceProxy->StartFuse(userId, devFd, path);
}

void CloudDaemonManagerImpl::SetDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
    auto deathCallback = [this](const wptr<IRemoteObject> &obj) {
        LOGE("service died. Died remote obj");
        CloudDaemonServiceProxy::InvaildInstance();
        isFirstCall_.clear();
    };
    deathRecipient_ = sptr(new SvcDeathRecipient(deathCallback));
    remoteObject->AddDeathRecipient(deathRecipient_);
}
} // namespace OHOS::FileManagement::CloudFile
