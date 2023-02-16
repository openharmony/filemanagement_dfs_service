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

#include "cloud_sync_manager_impl.h"
#include "cloud_sync_callback_client.h"
#include "cloud_sync_service_proxy.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "ipc_skeleton.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
constexpr uint64_t TOKEN_ID_LOWMASK = 0xffffffff;
using namespace std;
CloudSyncManagerImpl &CloudSyncManagerImpl::GetInstance()
{
    static CloudSyncManagerImpl instance;
    return instance;
}

int32_t CloudSyncManagerImpl::StartSync(SyncType type, bool forceFlag,
                                        const std::shared_ptr<CloudSyncCallback> callback)
{
    if (!callback) {
        LOGE("callback is null");
        return E_INVAL_ARG;
    }
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    if (!isFirstCall_) {
        LOGI("Register callback");
        uint32_t tokenId = IPCSkeleton::GetSelfTokenID() & TOKEN_ID_LOWMASK;
        int32_t ret = DfsuAccessTokenHelper::GetPackageNameByToken(tokenId, appPackageName_);
        if (ret) {
            LOGE("get package name failed");
            return ret;
        }

        ret = CloudSyncServiceProxy->RegisterCallbackInner(appPackageName_,
                                                           sptr(new (std::nothrow) CloudSyncCallbackClient(callback)));
        if (ret) {
            LOGE("Register callback failed");
            return ret;
        }
        SetDeathRecipient(CloudSyncServiceProxy->AsObject());
        isFirstCall_ = true;
    }

    return CloudSyncServiceProxy->StartSyncInner(appPackageName_, type, forceFlag);
}

int32_t CloudSyncManagerImpl::StopSync()
{
    auto CloudSyncServiceProxy = CloudSyncServiceProxy::GetInstance();
    if (!CloudSyncServiceProxy) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }

    if (appPackageName_.empty()) {
        LOGE("appPackageName is empty");
        return E_INVAL_ARG;
    }

    return CloudSyncServiceProxy->StopSyncInner(appPackageName_);
}

void CloudSyncManagerImpl::SetDeathRecipient(const sptr<IRemoteObject> &remoteObject)
{
    auto deathCallback = [&](const wptr<IRemoteObject> &obj) {
        LOGI("service died. Died remote obj = %{private}p", obj.GetRefPtr());
        isFirstCall_ = false;
    };
    deathRecipient_ = sptr(new SvcDeathRecipient(deathCallback));
    remoteObject->AddDeathRecipient(deathRecipient_);
}
} // namespace OHOS::FileManagement::CloudSync