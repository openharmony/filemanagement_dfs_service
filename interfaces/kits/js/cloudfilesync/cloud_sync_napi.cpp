/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "cloud_sync_napi.h"

#include <sys/types.h>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "async_work.h"
#include "uv.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;

CloudSyncCallbackImpl::CloudSyncCallbackImpl(napi_env env, napi_value fun) : env_(env)
{
    if (fun != nullptr) {
        napi_create_reference(env_, fun, 1, &cbOnRef_);
    }
}

void CloudSyncCallbackImpl::OnComplete(UvChangeMsg *msg)
{
    auto cloudSyncCallback = msg->cloudSyncCallback_.lock();
    if (cloudSyncCallback == nullptr || cloudSyncCallback->cbOnRef_ == nullptr) {
        LOGE("cloudSyncCallback->cbOnRef_ is nullptr");
        return;
    }
    auto env = cloudSyncCallback->env_;
    auto ref = cloudSyncCallback->cbOnRef_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    napi_value jsCallback = nullptr;
    napi_status status = napi_get_reference_value(env, ref, &jsCallback);
    if (status != napi_ok) {
        LOGE("Create reference failed, status: %{public}d", status);
        napi_close_handle_scope(env, scope);
        return;
    }
    NVal obj = NVal::CreateObject(env);
    obj.AddProp("state", NVal::CreateInt32(env, (int32_t)msg->state_).val_);
    obj.AddProp("error", NVal::CreateInt32(env, (int32_t)msg->error_).val_);
    napi_value retVal = nullptr;
    napi_value global = nullptr;
    napi_get_global(env, &global);
    status = napi_call_function(env, global, jsCallback, ARGS_ONE, &(obj.val_), &retVal);
    if (status != napi_ok) {
        LOGE("napi call function failed, status: %{public}d", status);
    }
    napi_close_handle_scope(env, scope);
}

void CloudSyncCallbackImpl::OnSyncStateChanged(CloudSyncState state, ErrorType error)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new (nothrow) uv_work_t;
    if (work == nullptr) {
        LOGE("Failed to create uv work");
        return;
    }

    UvChangeMsg *msg = new (std::nothrow) UvChangeMsg(shared_from_this(), state, error);
    if (msg == nullptr) {
        delete work;
        return;
    }

    work->data = reinterpret_cast<void *>(msg);
    int ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            auto msg = reinterpret_cast<UvChangeMsg *>(work->data);
            OnComplete(msg);
            delete msg;
            delete work;
        });
    if (ret != 0) {
        LOGE("Failed to execute libuv work queue, ret: %{public}d", ret);
        delete msg;
        delete work;
    }
}

void CloudSyncCallbackImpl::DeleteReference()
{
    if (cbOnRef_ != nullptr) {
        napi_delete_reference(env_, cbOnRef_);
        cbOnRef_ = nullptr;
    }
}

void CloudSyncCallbackImpl::OnSyncStateChanged(SyncType type, SyncPromptState state)
{
    return;
}

napi_value CloudSyncNapi::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        NError(E_PARAMS).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    return funcArg.GetThisVar();
}

napi_value CloudSyncNapi::OnCallback(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        NError(E_PARAMS).ThrowErr(env, "Number of arguments unmatched");
        LOGE("OnCallback Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, type, ignore] = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!(succ && (type.get() == std::string("progress")))) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    if (!NVal(env, funcArg[(int)NARG_POS::SECOND]).TypeIs(napi_function)) {
        LOGE("Argument type mismatch");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    if (callback_ != nullptr) {
        LOGI("callback already exist");
        return NVal::CreateUndefined(env).val_;
    }

    callback_ = make_shared<CloudSyncCallbackImpl>(env, NVal(env, funcArg[(int)NARG_POS::SECOND]).val_);
    int32_t ret = CloudSyncManager::GetInstance().RegisterCallback(callback_);
    if (ret != E_OK) {
        LOGE("OnCallback Register error, result: %{public}d", ret);
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value CloudSyncNapi::OffCallback(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        NError(E_PARAMS).ThrowErr(env, "Number of arguments unmatched");
        LOGE("OffCallback Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, type, ignore] = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!(succ && (type.get() == std::string("progress")))) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO && !NVal(env, funcArg[(int)NARG_POS::SECOND]).TypeIs(napi_function)) {
        LOGE("Argument type mismatch");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    int32_t ret = CloudSyncManager::GetInstance().UnRegisterCallback();
    if (ret != E_OK) {
        LOGE("OffCallback UnRegister error, result: %{public}d", ret);
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        return nullptr;
    }
    if (callback_ != nullptr) {
        /* napi delete reference */
        callback_->DeleteReference();
        callback_ = nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value CloudSyncNapi::Start(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(E_PARAMS).ThrowErr(env);
    }

    auto cbExec = []() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().StartSync();
        if (ret != E_OK) {
            LOGE("Start Sync error, result: %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };

    std::string procedureName = "Start";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbComplete).val_;
}

napi_value CloudSyncNapi::Stop(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = []() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().StopSync();
        if (ret != E_OK) {
            LOGE("Stop Sync error, result: %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };

    std::string procedureName = "Stop";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbComplete).val_;
}

void CloudSyncNapi::SetClassName(std::string classname)
{
    className_ = classname;
}

std::string CloudSyncNapi::GetClassName()
{
    return className_;
}

bool CloudSyncNapi::ToExport(std::vector<napi_property_descriptor> props)
{
    std::string className = GetClassName();
    auto [succ, classValue] =
        NClass::DefineClass(exports_.env_, className, Constructor, std::move(props));
    if (!succ) {
        NError(E_GETRESULT).ThrowErr(exports_.env_);
        LOGE("Failed to define CloudSyncNapi class");
        return false;
    }

    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        NError(E_GETRESULT).ThrowErr(exports_.env_);
        LOGE("Failed to save CloudSyncNapi class");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

bool CloudSyncNapi::Export()
{
    std::vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("on", OnCallback),
        NVal::DeclareNapiFunction("off", OffCallback),
        NVal::DeclareNapiFunction("start", Start),
        NVal::DeclareNapiFunction("stop", Stop),
    };
    std::string className = GetClassName();
    auto [succ, classValue] =
        NClass::DefineClass(exports_.env_, className, CloudSyncNapi::Constructor, std::move(props));
    if (!succ) {
        NError(E_GETRESULT).ThrowErr(exports_.env_);
        LOGE("Failed to define GallerySync class");
        return false;
    }

    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        NError(E_GETRESULT).ThrowErr(exports_.env_);
        LOGE("Failed to save GallerySync class");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

} // namespace OHOS::FileManagement::CloudSync
