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

namespace OHOS::FileManagement::CloudSync {
class CloudSyncAni final {
public:
    static void CloudSyncConstructor(ani_env *env, ani_object object);
    static void CloudSyncConstructor0(ani_env *env, ani_object object);
    static void CloudSyncConstructor1(ani_env *env, ani_object object, ani_string bundleName);
    static void CloudSyncOn(ani_env *env, ani_object object, ani_string evt, ani_object fun);
    static void CloudSyncOff0(ani_env *env, ani_object object, ani_string evt, ani_object fun);
    static void CloudSyncOff1(ani_env *env, ani_object object, ani_string evt);
    static void CloudSyncStart(ani_env *env, ani_object object);
    static void CloudSyncStop(ani_env *env, ani_object object);
    static ani_int GetFileSyncState(ani_env *env, ani_class clazz, ani_string path);
    static ani_double CloudyncGetLastSyncTime(ani_env *env, ani_object object);
    static int32_t RegisterToObs(const RegisterParams &registerParams);
    static bool CheckRef(ani_env *env, ani_ref ref, ChangeListenerAni &listObj, const string &uri);
    static int32_t GetRegisterParams(
        ani_env *env, ani_string uri, ani_boolean recursion, ani_object fun, RegisterParams &registerParams);
    static void RegisterChange(ani_env *env, ani_class clazz, ani_string uri, ani_boolean recursion, ani_object fun);
    static void UnRegisterChange(ani_env *env, ani_class clazz, ani_string uri);
    static void UnregisterFromObs(ani_env *env, std::string uri);
    static std::mutex sOnOffMutex_;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_SYNC_ANI_H
