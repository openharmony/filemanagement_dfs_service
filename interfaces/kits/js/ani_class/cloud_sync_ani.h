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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_ANI_H
#define OHOS_FILEMGMT_CLOUD_SYNC_ANI_H

#include "cloud_sync_core.h"

class CloudSyncAni final {
public:
    static void GallerySyncConstructor(ani_env *env, ani_object object);
    static void GallerySyncOn(ani_env *env, ani_object object, ani_string evt, ani_object fun);
    static void GallerySyncOff0(ani_env *env, ani_object object, ani_string evt, ani_object fun);
    static void GallerySyncOff1(ani_env *env, ani_object object, ani_string evt);
    static void GallerySyncStart(ani_env *env, ani_object object);
    static void GallerySyncStop(ani_env *env, ani_object object);
};
#endif // OHOS_FILEMGMT_CLOUD_SYNC_ANI_H