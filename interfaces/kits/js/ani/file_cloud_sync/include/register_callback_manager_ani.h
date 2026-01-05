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

#ifndef OHOS_FILEMGMT_REGISTER_CALLBACK_MANAGER_ANI_H
#define OHOS_FILEMGMT_REGISTER_CALLBACK_MANAGER_ANI_H

#include <list>
#include <mutex>

#include "ani_utils.h"

namespace OHOS::FileManagement::CloudSync {
class RegisterCallbackManagerAni {
public:
    void InitVm(ani_env *env);
    bool IsExisted(ani_env *env, ani_ref ref);
    ani_status RegisterCallback(ani_env *env, ani_object callback);
    ani_status UnregisterCallback(ani_env *env, ani_object callback);

    void OnJsCallback(ani_env *env, ani_object value, uint32_t argc);
    ani_env *GetEnv(bool &isAttached);
    void DetachEnv(const bool &isAttached);
    virtual ~RegisterCallbackManagerAni();

protected:
    void CleanAllCallback(bool force);

protected:
    ani_vm *vm_{nullptr};
    std::recursive_mutex callbackMtx_;
    std::atomic<uint32_t> validRefNum_{0};
    std::list<std::pair<bool, ani_ref>> callbackList_;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_REGISTER_CALLBACK_MANAGER_ANI_H