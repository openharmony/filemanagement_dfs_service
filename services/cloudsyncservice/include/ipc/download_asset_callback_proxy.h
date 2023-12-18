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

#ifndef OHOS_FILEMGMT_DOWNLOAD_ASSET_CALLBACK_PROXY_H
#define OHOS_FILEMGMT_DOWNLOAD_ASSET_CALLBACK_PROXY_H

#include "iremote_proxy.h"

#include "i_download_asset_callback.h"

namespace OHOS::FileManagement::CloudSync {
class DownloadAssetCallbackProxy : public IRemoteProxy<IDownloadAssetCallback> {
public:
    explicit DownloadAssetCallbackProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IDownloadAssetCallback>(impl) {}
    ~DownloadAssetCallbackProxy() override {}

    void OnFinished(const TaskId taskId, const std::string &uri, const int32_t result) override;

private:
    static inline BrokerDelegator<DownloadAssetCallbackProxy> delegator_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_DOWNLOAD_ASSET_CALLBACK_PROXY_H