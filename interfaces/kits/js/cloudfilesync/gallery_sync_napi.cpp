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

#include "gallery_sync_napi.h"

#include <sys/types.h>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "uv.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

CloudSyncCallbackImpl::CloudSyncCallbackImpl(napi_env env, napi_value fun) : env_(env)
{
    if (fun != nullptr) {
        napi_create_reference(env_, fun, 1, &cbOnRef_);
    }
}

CloudSyncCallbackImpl::~CloudSyncCallbackImpl()
{
    napi_delete_reference(env_, cbOnRef_);
}

void CloudSyncCallbackImpl::OnCloudSyncStateChanged(CloudSyncState state, ErrorType type)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new (nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }

    UvChangeMsg *msg = new (std::nothrow) UvChangeMsg(env_, cbOnRef_, state, type);
    if (msg == nullptr) {
        delete work;
        return;
    }

    work->data = reinterpret_cast<void *>(msg);
    int ret = uv_queue_work(loop, work, [](uv_work_t *w) {}, [](uv_work_t *w, int s) {
            // js thread
            if (w == nullptr) {
                return;
            }
            UvChangeMsg *msg = reinterpret_cast<UvChangeMsg *>(w->data);
            do {
                if (msg == nullptr || msg->ref_ == nullptr) {
                    LOGE("UvChangeMsg is null");
                    break;
                }

                napi_value jsCallback = nullptr;
                napi_status status = napi_get_reference_value(msg->env_, msg->ref_, &jsCallback);
                if (status != napi_ok) {
                    LOGE("Create reference fail, status: %{public}d", status);
                    break;
                }
                if (jsCallback == nullptr) {
                    LOGE("jsCallback is null");
                    break;
                }
                NVal obj = NVal::CreateObject(msg->env_);
                obj.AddProp("state", NVal::CreateInt32(msg->env_, (int32_t)msg->state_).val_);
                obj.AddProp("error", NVal::CreateInt32(msg->env_, (int32_t)msg->type_).val_);
                napi_value retVal = nullptr;
                status = napi_call_function(msg->env_, nullptr, jsCallback, ARGS_ONE, &(obj.val_), &retVal);
                if (status != napi_ok) {
                    break;
                }
            } while (0);
            delete w;
    });
    if (ret != 0) {
        LOGE("Failed to execute libuv work queue, ret: %{public}d", ret);
        delete work;
    }
}

void CloudSyncCallbackImpl::OnSyncStateChanged(SyncType type, SyncPromptState state)
{
    return;
}

napi_value GallerySyncNapi::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        NError(Convert2JsErrNum(E_INVAL_ARG)).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    return funcArg.GetThisVar();
}

napi_value GallerySyncNapi::OnCallback(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        NError(Convert2JsErrNum(E_INVAL_ARG)).ThrowErr(env, "Number of arguments unmatched");
        LOGE("OnCallback Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, type, ignore] = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!(succ && (type.get() == std::string("progress")))) {
        NError(Convert2JsErrNum(E_INVAL_ARG)).ThrowErr(env);
        return nullptr;
    }

    auto callback = make_shared<CloudSyncCallbackImpl>(env, NVal(env, funcArg[(int)NARG_POS::SECOND]).val_);
    int32_t errNum = CloudSyncManager::GetInstance().RegisterCallback(callback);
    if (errNum != E_OK) {
        LOGE("OnCallback Register error, result: %{public}d", errNum);
        NError(Convert2JsErrNum(E_BROKEN_IPC)).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value GallerySyncNapi::OffCallback(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        NError(Convert2JsErrNum(E_INVAL_ARG)).ThrowErr(env, "Number of arguments unmatched");
        LOGE("OffCallback Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, type, ignore] = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!(succ && (type.get() == std::string("progress")))) {
        NError(Convert2JsErrNum(E_INVAL_ARG)).ThrowErr(env);
        return nullptr;
    }

    int32_t errNum = CloudSyncManager::GetInstance().UnRegisterCallback();
    if (errNum != E_OK) {
        LOGE("OffCallback UnRegister error, result: %{public}d", errNum);
        NError(Convert2JsErrNum(E_BROKEN_IPC)).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value GallerySyncNapi::Start(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(Convert2JsErrNum(E_INVAL_ARG)).ThrowErr(env);
    }

    auto cbExec = []() -> NError {
        int32_t result = CloudSyncManager::GetInstance().StartSync();
        if (result != E_OK) {
            LOGE("Start Sync error, result: %{public}d", result);
            return NError(Convert2JsErrNum(E_BROKEN_IPC));
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUndefined(env);
    };

    std::string PROCEDURE_NAME = "Start";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }
}

napi_value GallerySyncNapi::Stop(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(Convert2JsErrNum(E_INVAL_ARG)).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = []() -> NError {
        int32_t result = CloudSyncManager::GetInstance().StopSync();
        if (result != E_OK) {
            LOGE("Stop Sync error, result: %{public}d", result);
            return NError(Convert2JsErrNum(E_BROKEN_IPC));
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return NVal::CreateUndefined(env);
    };

    std::string PROCEDURE_NAME = "Stop";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ZERO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::FIRST]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(PROCEDURE_NAME, cbExec, cbComplete).val_;
    }
}

std::string GallerySyncNapi::GetClassName()
{
    return GallerySyncNapi::className_;
}

bool GallerySyncNapi::Export()
{
    std::vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("on", OnCallback),
        NVal::DeclareNapiFunction("off", OffCallback),
        NVal::DeclareNapiFunction("start", Start),
        NVal::DeclareNapiFunction("stop", Stop),
    };

    std::string className = GetClassName();
    auto [succ, classValue] = NClass::DefineClass(exports_.env_, className,
        GallerySyncNapi::Constructor, std::move(props));
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
