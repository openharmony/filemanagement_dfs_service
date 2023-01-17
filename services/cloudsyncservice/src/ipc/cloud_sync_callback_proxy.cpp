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

namespace OHOS::FileManagement::CloudSync {
using namespace std;

void CloudSyncCallbackProxy::OnStartSyncFinished(const std::string &appPackageName, int32_t errCode) {}
void CloudSyncCallbackProxy::OnStopSyncFinished(const std::string &appPackageName, int32_t errCode) {}

} // namespace OHOS::FileManagement::CloudSync