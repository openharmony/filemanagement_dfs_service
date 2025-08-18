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

#ifndef OHOS_FILEMGMT_FILE_VERSION_CALLBACK_ANI_H
#define OHOS_FILEMGMT_FILE_VERSION_CALLBACK_ANI_H

#include <ani.h>
#include <ani_signature_builder.h>
#include <atomic>

#include "cloud_download_callback.h"

namespace OHOS::FileManagement::CloudSync {

class VersionDownloadCallbackAniImpl : public CloudDownloadCallback,
                                  public std::enable_shared_from_this<VersionDownloadCallbackAniImpl> {
public:
    VersionDownloadCallbackAniImpl(ani_env *env, ani_ref fun);
    ~VersionDownloadCallbackAniImpl();
    void OnDownloadProcess(const DownloadProgressObj &progress) override;

private:
    ani_vm *vm_ = nullptr;
    ani_ref cbOnRef_ = nullptr;
    ani_status GetDownloadProgress(ani_env *env, const DownloadProgressObj &progress, const ani_class &cls,
        ani_object &pg);
};
} // OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_FILE_VERSION_CALLBACK_ANI_H
