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

#include "dfs_error.h"
#include "ipc/cloud_sync_callback_manager.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

CloudSyncCallbackManager &CloudSyncCallbackManager::GetInstance()
{
    static CloudSyncCallbackManager instance;
    return instance;
}

void CloudSyncCallbackManager::AddCallback(const std::string &bundleName,
                                           const int32_t userId,
                                           const sptr<ICloudSyncCallback> &callback)
{
}

void CloudSyncCallbackManager::SetDeathRecipient(const std::string &bundleName, CallbackInfo &cbInfo) {}

void CloudSyncCallbackManager::NotifySyncStateChanged(const std::string &bundleName,
    const CloudSyncState state, const ErrorType error)
{
}
} // namespace OHOS::FileManagement::CloudSync