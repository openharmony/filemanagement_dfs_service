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

#include "register_callback_manager_ani.h"

#include <mutex>

#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
// JS thread invokes native function
void RegisterCallbackManagerAni::InitVm(ani_env *env)
{
    if (vm_ == nullptr) {
        env->GetVM(&vm_);
    }
}

ani_status RegisterCallbackManagerAni::RegisterCallback(ani_env *env, ani_object callback)
{
    if (callback == nullptr) {
        return ANI_INVALID_ARGS;
    }
    ani_ref ref;
    ani_status ret = env->GlobalReference_Create(reinterpret_cast<ani_ref>(callback), &ref);
    if (ret != ANI_OK) {
        LOGE("Failed to create reference: %{public}d", ret);
        return ret;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackMtx_);
    if (IsExisted(env, ref)) {
        return ANI_OK;
    }

    callbackList_.push_back(std::make_pair(true, ref));
    validRefNum_++;
    return ANI_OK;
}

ani_status RegisterCallbackManagerAni::UnregisterCallback(ani_env *env, ani_object callback)
{
    if (validRefNum_.load() == 0) {
        return ANI_OK;
    }
    std::lock_guard<std::recursive_mutex> lock(callbackMtx_);
    if (callback == nullptr) {
        for (auto &iter : callbackList_) {
            iter.first = false;
        }
        validRefNum_ = 0;
        return ANI_OK;
    }
    ani_ref ref;
    ani_status ret = env->GlobalReference_Create(reinterpret_cast<ani_ref>(callback), &ref);
    if (ret != ANI_OK) {
        LOGE("Failed to create reference: %{public}d", ret);
        return ret;
    }
    ani_boolean isSame = false;
    for (auto &iter : callbackList_) {
        ret = env->Reference_StrictEquals(iter.second, ref, &isSame);
        if (ret != ANI_OK) {
            LOGE("Failed to strict compare: %{public}d", ret);
            continue;
        }
        if (isSame) {
            if (iter.first) {
                iter.first = false;
                validRefNum_--;
            }
            break;
        }
    }

    return ANI_OK;
}

// No need to lock
bool RegisterCallbackManagerAni::IsExisted(ani_env *env, ani_ref ref)
{
    ani_boolean isSame = false;
    for (auto &iter : callbackList_) {
        ani_status ret = env->Reference_StrictEquals(iter.second, ref, &isSame);
        if (ret != ANI_OK) {
            LOGE("Failed to strict compare: %{public}d", ret);
            continue;
        }
        if (isSame) {
            return iter.first;
        }
    }

    return false;
}

// Running in JS thread
void RegisterCallbackManagerAni::OnJsCallback(ani_env *env, ani_object value, uint32_t argc)
{
    std::lock_guard<std::recursive_mutex> lock(callbackMtx_);
    for (auto iter = callbackList_.begin(); iter != callbackList_.end();) {
        if (!iter->first) {
            env->GlobalReference_Delete(iter->second);
            iter = callbackList_.erase(iter);
            continue;
        }
        ani_ref ref;
        ani_fn_object etsCb = reinterpret_cast<ani_fn_object>(iter->second);
        std::vector<ani_ref> vec = {value};
        ani_status ret = env->FunctionalObject_Call(etsCb, argc, vec.data(), &ref);
        if (ret != ANI_OK) {
            LOGE("ani call function failed. ret = %{public}d", ret);
        }
        iter++;
    }
}

ani_env *RegisterCallbackManagerAni::GetEnv(bool &isAttached)
{
    if (vm_ == nullptr) {
        return nullptr;
    }

    ani_env *env{nullptr};
    ani_options aniArgs{0, nullptr};
    ani_status ret = vm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env);
    if (ret != ANI_OK) {
        ret = vm_->GetEnv(ANI_VERSION_1, &env);
        if (ret != ANI_OK) {
            LOGE("vm GetEnv, err: %{private}d", ret);
            return nullptr;
        }
    } else {
        isAttached = true;
    }
    return env;
}

void RegisterCallbackManagerAni::DetachEnv(const bool &isAttached)
{
    if (!isAttached) {
        return;
    }

    if (vm_ == nullptr) {
        return;
    }

    ani_status ret;
    if ((ret = vm_->DetachCurrentThread()) != ANI_OK) {
        LOGE("detach current env failed, err: %{private}d", ret);
    }
}

RegisterCallbackManagerAni::~RegisterCallbackManagerAni()
{
    CleanAllCallback(true);
}

void RegisterCallbackManagerAni::CleanAllCallback(bool force)
{
    std::lock_guard<std::recursive_mutex> lock(callbackMtx_);
    if (!force) {
        if (validRefNum_.load() != 0) {
            return;
        }
    }
    bool isAttached = false;
    ani_env *env = GetEnv(isAttached);
    if (env != nullptr) {
        for (auto &iter : callbackList_) {
            env->GlobalReference_Delete(iter.second);
        }
    } else {
        LOGE("Failed to delete reference before clean callback");
    }

    DetachEnv(isAttached);

    callbackList_.clear();
    validRefNum_ = 0;
    LOGI("Clean all callback, valid callback count is 0");
}
} // namespace OHOS::FileManagement::CloudSync