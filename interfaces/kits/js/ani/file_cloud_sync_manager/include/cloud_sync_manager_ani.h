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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_ANI_H
#define OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_ANI_H

#include <ani.h>
#include <ani_signature_builder.h>

#include "cloud_sync_manager_core.h"

namespace OHOS::FileManagement::CloudSync {
class CloudSyncManagerAni final {
public:
    static void ChangeAppCloudSwitch(
        ani_env *env, ani_class clazz, ani_string accoutId, ani_string bundleName, ani_boolean status);
    static void NotifyDataChange(ani_env *env, ani_class clazz, ani_string accoutId, ani_string bundleName);
    static void EnableCloud(ani_env *env, ani_class clazz, ani_string accoutId, ani_object record);
    static void DisableCloud(ani_env *env, ani_class clazz, ani_string accoutId);
    static void Clean(ani_env *env, ani_class clazz, ani_string accoutId, ani_object record);
    static void NotifyEventChange(ani_env *env, ani_class clazz, ani_double userId, ani_object extraData);
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_ANI_H