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

#ifndef OHOS_FILEMGMT_FILE_SYNC_ANI_H
#define OHOS_FILEMGMT_FILE_SYNC_ANI_H

#include "file_sync_core.h"

namespace OHOS::FileManagement::CloudSync {
class FileSyncAni final {
public:
    static void FileSyncConstructor0(ani_env *env, ani_object object);
    static void FileSyncConstructor1(ani_env *env, ani_object object, ani_string bundleName);
    static void FileSyncOn(ani_env *env, ani_object object, ani_string evt, ani_object fun);
    static void FileSyncOff0(ani_env *env, ani_object object, ani_string evt, ani_object fun);
    static void FileSyncOff1(ani_env *env, ani_object object, ani_string evt);
    static void FileSyncStart(ani_env *env, ani_object object);
    static void FileSyncStop(ani_env *env, ani_object object);
    static ani_double FileSyncGetLastSyncTime(ani_env *env, ani_object object);
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_FILE_SYNC_ANI_H
