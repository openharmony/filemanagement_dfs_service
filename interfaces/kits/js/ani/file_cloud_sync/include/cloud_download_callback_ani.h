/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_FILEMGMT_CLOUD_DOWNLOAD_CALLBACK_ANI_H
#define OHOS_FILEMGMT_CLOUD_DOWNLOAD_CALLBACK_ANI_H

#include <ani.h>
#include <ani_signature_builder.h>

#include "cloud_download_callback_middle.h"

namespace OHOS::FileManagement::CloudSync {

class CloudDownloadCallbackAniImpl : public CloudDownloadCallbackMiddle,
                                     public std::enable_shared_from_this<CloudDownloadCallbackAniImpl> {
public:
    CloudDownloadCallbackAniImpl(ani_env *env, ani_ref fun, bool isBatch = false);
    ~CloudDownloadCallbackAniImpl() override = default;
    void OnDownloadProcess(const DownloadProgressObj &progress) override;
    void DeleteReference() override;

private:
    void GetDownloadProgress(const DownloadProgressObj &progress, const ani_class &cls, ani_object &pg);
    ani_env *env_;
    ani_ref cbOnRef_ = nullptr;
    bool isBatch_;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_DOWNLOAD_CALLBACK_ANI_H
