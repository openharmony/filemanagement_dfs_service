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

#ifndef OHOS_FILEMGMT_DOWNLOAD_ASSET_CALLBACK_MANAGER_H
#define OHOS_FILEMGMT_DOWNLOAD_ASSET_CALLBACK_MANAGER_H

#include "nocopyable.h"

#include "i_download_asset_callback.h"
#include "svc_death_recipient.h"

namespace OHOS::FileManagement::CloudSync {
class DownloadAssetCallbackManager final : public NoCopyable {
public:
    using TaskId = uint64_t;
    static DownloadAssetCallbackManager &GetInstance();
    void AddCallback(const sptr<IDownloadAssetCallback> &callback);
    void OnDownloadFinshed(const TaskId taskId, const std::string &uri, const int32_t result);

private:
    sptr<IDownloadAssetCallback> callbackProxy_;
    sptr<SvcDeathRecipient> deathRecipient_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_DOWNLOAD_ASSET_CALLBACK_MANAGER_H