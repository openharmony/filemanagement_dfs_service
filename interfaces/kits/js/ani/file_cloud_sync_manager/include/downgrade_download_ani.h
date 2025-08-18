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

#ifndef OHOS_FILEMGMT_DOWNGRADE_DOWNLOAD_ANI_H
#define OHOS_FILEMGMT_DOWNGRADE_DOWNLOAD_ANI_H

#include <ani.h>
#include <ani_signature_builder.h>

#include "cloud_sync_manager_core.h"
#include "downgrade_callback_ani.h"
#include "downgrade_download_core.h"

namespace OHOS::FileManagement::CloudSync {
class DowngradeDownloadAni final {
public:
    static void DowngradeDownloadConstructor(ani_env *env, ani_object object, ani_string bundleName);
    static ani_ref DowngradeDownloadGetCloudFileInfo(ani_env *env, ani_object object);
    static void DowngradeDownloadStartDownload(ani_env *env, ani_object object, ani_object fun);
    static void DowngradeDownloadStopDownload(ani_env *env, ani_object object);
};

} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_DOWNGRADE_DOWNLOAD_ANI_H