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
#include "cloud_file_napi.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "async_work.h"
#include "uv.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;
const int32_t ARGS_ONE = 1;

CloudFileNapi::CloudFileNapi(napi_env env, napi_value exports) : NExporter(env, exports) {}

napi_value CloudFileNapi::Constructor(napi_env env, napi_callback_info info)
{
    LOGI("CloudFileNapi::Constructor begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        LOGE("Start Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    LOGI("CloudFileNapi::Constructor end");
    return funcArg.GetThisVar();
}

napi_value CloudFileNapi::Start(napi_env env, napi_callback_info info)
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

    string procedureName = "cloudFileDownload";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

CloudDownloadCallbackImpl::CloudDownloadCallbackImpl(napi_env env, napi_value fun) : env_(env)
{
    if (fun != nullptr) {
        napi_create_reference(env_, fun, 1, &cbOnRef_);
    }
}

void CloudDownloadCallbackImpl::OnComplete(UvChangeMsg *msg)
{
    auto downloadcCallback = msg->CloudDownloadCallback_.lock();
    if (downloadcCallback == nullptr || downloadcCallback->cbOnRef_ == nullptr) {
        LOGE("downloadcCallback->cbOnRef_ is nullptr");
        return;
    }
    auto env = downloadcCallback->env_;
    auto ref = downloadcCallback->cbOnRef_;
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
    obj.AddProp("state", NVal::CreateInt32(env, (int32_t)msg->downloadProgress_.state).val_);
    obj.AddProp("processed", NVal::CreateInt64(env, (int64_t)msg->downloadProgress_.downloadedSize).val_);
    obj.AddProp("size", NVal::CreateInt64(env, (int64_t)msg->downloadProgress_.totalSize).val_);
    obj.AddProp("uri", NVal::CreateUTF8String(env, msg->downloadProgress_.path).val_);
    obj.AddProp("downloadErrorType", NVal::CreateInt32(env, (int32_t)msg->downloadProgress_.downloadErrorType).val_);
    napi_value retVal = nullptr;
    napi_value global = nullptr;
    napi_get_global(env, &global);
    status = napi_call_function(env, global, jsCallback, ARGS_ONE, &(obj.val_), &retVal);
    if (status != napi_ok) {
        LOGE("napi call function failed, status: %{public}d", status);
    }
    napi_close_handle_scope(env, scope);
}

void CloudDownloadCallbackImpl::OnDownloadProcess(const DownloadProgressObj &progress)
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

    UvChangeMsg *msg = new (std::nothrow) UvChangeMsg(shared_from_this(), progress);
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

void CloudDownloadCallbackImpl::DeleteReference()
{
    if (cbOnRef_ != nullptr) {
        napi_delete_reference(env_, cbOnRef_);
        cbOnRef_ = nullptr;
    }
}

napi_value CloudFileNapi::On(napi_env env, napi_callback_info info)
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

    if (!NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) {
        LOGE("Argument type mismatch");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    if (callback_ != nullptr) {
        LOGI("callback already exist");
        return NVal::CreateUndefined(env).val_;
    }

    callback_ = make_shared<CloudDownloadCallbackImpl>(env, NVal(env, funcArg[(int)NARG_POS::SECOND]).val_);
    int32_t ret = CloudSyncManager::GetInstance().RegisterDownloadFileCallback(callback_);
    if (ret != E_OK) {
        LOGE("RegisterDownloadFileCallback error, ret: %{public}d", ret);
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value CloudFileNapi::Off(napi_env env, napi_callback_info info)
{
    LOGI("Off begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
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

    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO &&!NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) {
        LOGE("Argument type mismatch");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    int32_t ret = CloudSyncManager::GetInstance().UnregisterDownloadFileCallback();
    if (ret != E_OK) {
        LOGE("UnregisterDownloadFileCallback error, ret: %{public}d", ret);
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

napi_value CloudFileNapi::Stop(napi_env env, napi_callback_info info)
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

    string procedureName = "cloudFileDownload";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

bool CloudFileNapi::ToExport(std::vector<napi_property_descriptor> props)
{
    std::string className = GetClassName();
    auto [succ, classValue] =
        NClass::DefineClass(exports_.env_, className, Constructor, std::move(props));
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

bool CloudFileNapi::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("start", CloudFileNapi::Start),
        NVal::DeclareNapiFunction("on", CloudFileNapi::On),
        NVal::DeclareNapiFunction("off", CloudFileNapi::Off),
        NVal::DeclareNapiFunction("stop", CloudFileNapi::Stop),
    };
    return ToExport(props);
}

void CloudFileNapi::SetClassName(std::string classname)
{
    className_ = classname;
}

string CloudFileNapi::GetClassName()
{
    return className_;
}
} // namespace OHOS::FileManagement::CloudSync