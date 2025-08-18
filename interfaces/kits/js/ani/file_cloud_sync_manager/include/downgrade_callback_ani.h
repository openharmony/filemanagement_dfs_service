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

#ifndef OHOS_FILEMGMT_DOWNGRADE_CALLBACK_ANI_H
#define OHOS_FILEMGMT_DOWNGRADE_CALLBACK_ANI_H

#include <ani.h>
#include <ani_signature_builder.h>

#include "cloud_sync_common.h"
#include "downgrade_dl_callback.h"

namespace OHOS::FileManagement::CloudSync {

struct SingleBdProgress {
    int32_t state{0};
    int32_t successfulCount{0};
    int32_t failedCount{0};
    int64_t downloadedSize{0};
    int64_t totalSize{0};
    int32_t totalCount{0};
    int32_t stopReason{0};
};

class DowngradeCallbackAniImpl : public DowngradeDlCallback,
                                 public std::enable_shared_from_this<DowngradeCallbackAniImpl> {
public:
    DowngradeCallbackAniImpl(ani_env *env, ani_ref fun);
    ~DowngradeCallbackAniImpl();
    void OnDownloadProcess(const DowngradeProgress &progress) override;
    ani_object ConvertToObject(ani_env *env, const DowngradeProgress &progress);

public:
    ani_ref cbOnRef_ = nullptr;
    ani_vm *vm_ = nullptr;

private:
    void UpdateDowngradeProgress(const DowngradeProgress &progress);
    std::shared_ptr<SingleBdProgress> progress_;
};

struct DowngradeEntity {
    explicit DowngradeEntity(std::string bundle) : bundleName(bundle) {}
    std::string bundleName;
    std::shared_ptr<DowngradeCallbackAniImpl> callbackImpl{nullptr};
};

} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_DOWNGRADE_CALLBACK_ANI_H