/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "downgrade_download_callback_client.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

void DowngradeDownloadCallbackClient::OnDownloadProcess(const DowngradeProgress &progress)
{
    if (!callback_) {
        LOGE("callback_ is null, maybe not registered");
        return;
    }
    callback_->OnDownloadProcess(progress);
}

} // namespace OHOS::FileManagement::CloudSync
