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

#include "cloud_file_download_napi.h"

#include <sys/types.h>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "uv.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;

CloudFileDownloadNapi::CloudFileDownloadNapi(napi_env env, napi_value exports) : NExporter(env, exports) {}

CloudFileDownloadNapi::~CloudFileDownloadNapi() {}

napi_value CloudFileDownloadNapi::Constructor(napi_env env, napi_callback_info info)
{
    LOGI("CloudFileDownloadNapi::Constructor begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        LOGE("Start Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    LOGI("CloudFileDownloadNapi::Constructor end");
    return funcArg.GetThisVar();
}

napi_value CloudFileDownloadNapi::Start(napi_env env, napi_callback_info info)
{
    LOGI("Start begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        LOGE("Start Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto [succUri, uri, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succUri) {
        LOGE("Start get uri parameter failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [uri = string(uri.get()), env = env]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().StartDownloadFile(uri);
        if (ret != E_OK) {
            LOGE("Start Download failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        LOGI("Start Download Success!");
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    string procedureName = "cloudFileDownload";
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }
}

CloudDownloadCallbackImpl::CloudDownloadCallbackImpl(napi_env env, napi_value fun) : env_(env)
{
    if (fun != nullptr) {
        napi_create_reference(env_, fun, 1, &cbOnRef_);
    }
}

CloudDownloadCallbackImpl::~CloudDownloadCallbackImpl()
{
    napi_delete_reference(env_, cbOnRef_);
}

static void DownloadCallbackComplete(uv_work_t *work, int stat)
{
    if (work == nullptr) {
        return;
    }
    CloudDownloadCallbackImpl::UvChangeMsg *msg =
        reinterpret_cast<CloudDownloadCallbackImpl::UvChangeMsg *>(work->data);
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
        obj.AddProp("state", NVal::CreateInt32(msg->env_, (int32_t)msg->downloadProgress_.state).val_);
        obj.AddProp("processed", NVal::CreateInt64(msg->env_, (int64_t)msg->downloadProgress_.downloadedSize).val_);
        obj.AddProp("size", NVal::CreateInt64(msg->env_, (int64_t)msg->downloadProgress_.totalSize).val_);
        obj.AddProp("uri", NVal::CreateUTF8String(msg->env_, msg->downloadProgress_.path).val_);
        napi_value retVal = nullptr;
        status = napi_call_function(msg->env_, nullptr, jsCallback, 1, &(obj.val_), &retVal);
        if (status != napi_ok) {
            break;
        }
    } while (0);
    delete work;
}

void CloudDownloadCallbackImpl::OnDownloadProcess(DownloadProgressObj &progress)
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

    UvChangeMsg *msg = new (std::nothrow) UvChangeMsg(env_, cbOnRef_, progress);
    if (msg == nullptr) {
        delete work;
        return;
    }
    work->data = reinterpret_cast<void *>(msg);

    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
                            reinterpret_cast<uv_after_work_cb>(DownloadCallbackComplete));
    if (ret != 0) {
        HILOGE("Failed to execute libuv work queue, ret: %{public}d", ret);
        delete work;
    }
}

napi_value CloudFileDownloadNapi::On(napi_env env, napi_callback_info info)
{
    LOGI("On begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        LOGE("On Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto [succProgress, progress, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!(succProgress && std::string(progress.get()) == "progress")) {
        LOGE("On get progress failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto callback = make_shared<CloudDownloadCallbackImpl>(env, NVal(env, funcArg[(int)NARG_POS::SECOND]).val_);
    int32_t ret = CloudSyncManager::GetInstance().RegisterDownloadFileCallback(callback);
    if (ret != E_OK) {
        LOGE("RegisterDownloadFileCallback error, ret: %{public}d", ret);
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value CloudFileDownloadNapi::Off(napi_env env, napi_callback_info info)
{
    LOGI("Off begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        LOGE("Off Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto [succProgress, progress, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!(succProgress && std::string(progress.get()) == "progress")) {
        LOGE("Off get progress failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    int32_t ret = CloudSyncManager::GetInstance().UnregisterDownloadFileCallback();
    if (ret != E_OK) {
        LOGE("UnregisterDownloadFileCallback error, ret: %{public}d", ret);
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value CloudFileDownloadNapi::Stop(napi_env env, napi_callback_info info)
{
    LOGI("Stop begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        LOGE("Stop Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto [succUri, uri, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succUri) {
        LOGE("Stop get uri parameter failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [uri = string(uri.get()), env = env]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().StopDownloadFile(uri);
        if (ret != E_OK) {
            LOGE("Stop Download failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        LOGI("Stop Download Success!");
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };

    NVal thisVar(env, funcArg.GetThisVar());
    string procedureName = "cloudFileDownload";
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbCompl).val_;
    } else {
        NVal cb(env, funcArg[NARG_POS::SECOND]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbCompl).val_;
    }
}

bool CloudFileDownloadNapi::Export()
{
    LOGI("CloudFileDownloadNapi::Export begin");
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("start", CloudFileDownloadNapi::Start),
        NVal::DeclareNapiFunction("on", CloudFileDownloadNapi::On),
        NVal::DeclareNapiFunction("off", CloudFileDownloadNapi::Off),
        NVal::DeclareNapiFunction("stop", CloudFileDownloadNapi::Stop),
    };

    auto [succ, classValue] =
        NClass::DefineClass(exports_.env_, className, CloudFileDownloadNapi::Constructor, std::move(props));
    if (!succ) {
        LOGE("Failed to define Download class");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        LOGE("Failed to save Download class");
        NError(EIO).ThrowErr(exports_.env_);
        return false;
    }

    LOGI("CloudFileDownloadNapi::Export end");
    return exports_.AddProp(className, classValue);
}

string CloudFileDownloadNapi::GetClassName()
{
    return CloudFileDownloadNapi::className;
}
} // namespace OHOS::FileManagement::CloudSync