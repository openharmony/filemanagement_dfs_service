/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "file_sync_napi.h"

#include <memory>
#include <string>
#include <sys/types.h>

#include "async_work.h"
#include "cloud_file_utils.h"
#include "cloud_sync_common.h"
#include "cloud_sync_constants.h"
#include "cloud_sync_manager.h"
#include "cloud_sync_napi.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "uv.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;

struct SyncTimeArg {
    int64_t time = 0;
};

napi_value FileSyncNapi::GetLastSyncTime(napi_env env, napi_callback_info info)
{
    LOGI("GetLastSyncTime Start");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto arg = make_shared<SyncTimeArg>();
    if (arg == nullptr) {
        HILOGE("Failed to request heap memory.");
        NError(ENOMEM).ThrowErr(env);
        return nullptr;
    }

    string bundleName = GetBundleName(env, funcArg);
    auto cbExec = [arg, bundleName]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().GetSyncTime(arg->time, bundleName);
        if (ret != E_OK) {
            LOGE("GetLastSyncTime error, result: %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        return NError(ERRNO_NOERR);
    };

    auto cbComplete = [cbArg = arg](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateInt64(env, cbArg->time);
    };

    const std::string procedureName = "getLastSyncTime";
    std::string taskName = "cloudSync.FileSync.getLastSyncTime";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        if (!NVal(env, funcArg[NARG_POS::FIRST]).TypeIs(napi_number)) {
            NVal cb(env, funcArg[NARG_POS::FIRST]);
            return NAsyncWorkCallback(env, thisVar, cb, taskName).Schedule(procedureName, cbExec, cbComplete).val_;
        }
        LOGE("Get napi_number Error");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
}

napi_value FileSyncNapi::OnCallback(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!InitArgsOnCallback(env, funcArg)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto bundleEntity = NClass::GetEntityOf<BundleEntity>(env, funcArg.GetThisVar());
    if (bundleEntity == nullptr) {
        LOGE("Argument type mismatch");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    if (bundleEntity->callbackInfo.callback != nullptr) {
        LOGI("callback already exist");
        return nullptr;
    }

    bundleEntity->callbackInfo.callbackId = CloudDisk::CloudFileUtils::GenerateUuid();
    bundleEntity->callbackInfo.callback =
        make_shared<CloudSyncCallbackImpl>(env, NVal(env, funcArg[(int)NARG_POS::SECOND]).val_);
    int32_t ret = CloudSyncManager::GetInstance().RegisterFileSyncCallback(bundleEntity->callbackInfo);
    if (ret != E_OK) {
        LOGE("OnCallback Register error, result: %{public}d", ret);
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        bundleEntity->callbackInfo.callback = nullptr;
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value FileSyncNapi::OffCallback(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!InitArgsOffCallback(env, funcArg)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto bundleEntity = NClass::GetEntityOf<BundleEntity>(env, funcArg.GetThisVar());
    if (bundleEntity == nullptr) {
        LOGE("Argument type mismatch");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    int32_t ret = CloudSyncManager::GetInstance().UnRegisterFileSyncCallback(bundleEntity->callbackInfo);
    if (ret != E_OK) {
        LOGE("OffCallback UnRegister error, result: %{public}d", ret);
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        return nullptr;
    }
    if (bundleEntity->callbackInfo.callback != nullptr) {
        /* napi delete reference */
        bundleEntity->callbackInfo.callback->DeleteReference();
        bundleEntity->callbackInfo.callback = nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

napi_value FileSyncNapi::Start(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        LOGE("Failed to init args");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    string bundleName = GetBundleName(env, funcArg);
    auto cbExec = [bundleName]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().StartFileSync(bundleName);
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
    std::string taskName = "cloudSync.FileSync.start";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO), taskName);
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbComplete).val_;
}

napi_value FileSyncNapi::Stop(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO, NARG_CNT::ONE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    string bundleName = GetBundleName(env, funcArg);
    auto cbExec = [bundleName]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().StopFileSync(bundleName);
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
    std::string taskName = "cloudSync.FileSync.stop";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO), taskName);
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbComplete).val_;
}

static NVal CreateUploadProgressObject(napi_env env, const CloudSync::UploadProgressObj &progress)
{
    NVal progressObj = NVal::CreateObject(env);
    progressObj.AddProp("state", NVal::CreateInt32(env, static_cast<int32_t>(progress.state)).val_);
    progressObj.AddProp("processed", NVal::CreateInt64(env, progress.processed).val_);
    progressObj.AddProp("size", NVal::CreateInt64(env, progress.size).val_);
    progressObj.AddProp("uri", NVal::CreateUTF8String(env, progress.uri).val_);
    progressObj.AddProp("error", NVal::CreateInt32(env, static_cast<int32_t>(progress.error)).val_);
    return progressObj;
}

static NVal CreateUploadListResultArray(napi_env env, const std::vector<CloudSync::UploadProgressObj> &uploadList)
{
    napi_value resultArray = nullptr;
    napi_status status = napi_create_array(env, &resultArray);
    if (status != napi_ok) {
        LOGE("Failed to create array");
        return {env, NError(JsErrCode::E_INNER_FAILED).GetNapiErr(env)};
    }

    for (size_t i = 0; i < uploadList.size(); ++i) {
        auto progressObj = CreateUploadProgressObject(env, uploadList[i]);
        status = napi_set_element(env, resultArray, i, progressObj.val_);
        if (status != napi_ok) {
            LOGE("Failed to set array element");
            return {env, NError(JsErrCode::E_INNER_FAILED).GetNapiErr(env)};
        }
    }

    return {env, resultArray};
}

napi_value FileSyncNapi::GetUploadList(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        LOGE("GetUploadList Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, uriArray, size] = NVal(env, funcArg[NARG_POS::FIRST]).ToStringArray();
    if (!succ || size == 0) {
        LOGE("GetUploadList get uri array parameter failed!");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    struct UploadListArg {
        std::vector<std::string> uriVec;
        std::vector<CloudSync::UploadProgressObj> uploadList;
    };

    auto arg = make_shared<UploadListArg>();
    arg->uriVec.swap(uriArray);

    auto cbExec = [arg]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().GetUploadList(arg->uriVec, arg->uploadList);
        if (ret != E_OK) {
            LOGE("GetUploadList error, result: %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [arg](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return CreateUploadListResultArray(env, arg->uploadList);
    };

    string procedureName = "GetUploadList";
    string taskName = "cloudSync.FileSync.getUploadList";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO), taskName);
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

napi_value FileSyncNapi::RegisterUploadProgress(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto bundleEntity = NClass::GetEntityOf<BundleEntity>(env, funcArg.GetThisVar());
    if (bundleEntity == nullptr) {
        LOGE("Argument type mismatch");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    if (bundleEntity->uploadCallbackInfo.callback != nullptr) {
        LOGI("upload callback already exist");
        return nullptr;
    }

    bundleEntity->uploadCallbackInfo.callbackId = CloudDisk::CloudFileUtils::GenerateUuid();
    bundleEntity->uploadCallbackInfo.callback =
        make_shared<CloudUploadCallbackImpl>(env, NVal(env, funcArg[(int)NARG_POS::FIRST]).val_);
    int32_t ret = CloudSyncManager::GetInstance().RegisterUploadCallback(bundleEntity->uploadCallbackInfo);
    if (ret != E_OK) {
        LOGE("RegisterUploadProgress Register error, result: %{public}d", ret);
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        bundleEntity->uploadCallbackInfo.callback = nullptr;
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value FileSyncNapi::UnRegisterUploadProgress(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto bundleEntity = NClass::GetEntityOf<BundleEntity>(env, funcArg.GetThisVar());
    if (bundleEntity == nullptr) {
        LOGE("Argument type mismatch");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    int32_t ret = CloudSyncManager::GetInstance().UnRegisterUploadCallback(bundleEntity->uploadCallbackInfo);
    if (ret != E_OK) {
        LOGE("UnRegisterUploadProgress UnRegister error, result: %{public}d", ret);
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        return nullptr;
    }
    if (bundleEntity->uploadCallbackInfo.callback != nullptr) {
        bundleEntity->uploadCallbackInfo.callback->DeleteReference();
        bundleEntity->uploadCallbackInfo.callback = nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

CloudUploadCallbackImpl::CloudUploadCallbackImpl(napi_env env, napi_value fun) : env_(env)
{
    if (fun != nullptr) {
        cbOnRef_ = make_shared<AutoRef>();
        unique_lock<mutex> lock(cbOnRef_->refMtx);
        auto status = napi_create_reference(env_, fun, 1, &cbOnRef_->cbRef);
        if (status != napi_ok) {
            cbOnRef_->cbRef = nullptr;
            LOGE("Failed to create reference, status: %{public}d.", status);
        }
    }
}

CloudUploadCallbackImpl::~CloudUploadCallbackImpl()
{
    DeleteReferenceAsync();
}

void CloudUploadCallbackImpl::OnComplete(UvChangeMsg *msg)
{
    auto cloudUploadCallback = msg->cloudUploadCallback_.lock();
    if (cloudUploadCallback == nullptr || cloudUploadCallback->cbOnRef_->cbRef == nullptr) {
        LOGE("cloudUploadCallback->cbOnRef_->cbRef is nullptr");
        return;
    }
    napi_ref ref = nullptr;
    {
        unique_lock<mutex> lock(cloudUploadCallback->cbOnRef_->refMtx);
        ref = cloudUploadCallback->cbOnRef_->cbRef;
    }
    auto env = cloudUploadCallback->env_;
    napi_handle_scope scope = nullptr;
    napi_status status = napi_open_handle_scope(env, &scope);
    if (status != napi_ok || scope == nullptr) {
        LOGE("open handle scope failed, status: %{public}d", status);
        return;
    }
    napi_value jsCallback = nullptr;
    status = napi_get_reference_value(env, ref, &jsCallback);
    if (status != napi_ok) {
        LOGE("Get reference value failed, status: %{public}d", status);
        napi_close_handle_scope(env, scope);
        return;
    }
    const auto &progress = msg->progress_;
    NVal obj = CreateUploadProgressObject(env, progress);
    napi_value retVal = nullptr;
    napi_value global = nullptr;
    napi_get_global(env, &global);
    status = napi_call_function(env, global, jsCallback, ARGS_ONE, &(obj.val_), &retVal);
    if (status != napi_ok) {
        LOGE("napi call function failed, status: %{public}d", status);
    }
    napi_close_handle_scope(env, scope);
}

static void CallUploadProgressCallback(napi_env env, napi_ref callbackRef, const UploadProgressObj &progress)
{
    napi_handle_scope scope = nullptr;
    napi_status status = napi_open_handle_scope(env, &scope);
    if (status != napi_ok || scope == nullptr) {
        LOGE("open handle scope failed, status: %{public}d", status);
        return;
    }

    napi_value jsCallback = nullptr;
    status = napi_get_reference_value(env, callbackRef, &jsCallback);
    if (status != napi_ok) {
        LOGE("Get reference value failed, status: %{public}d", status);
        napi_close_handle_scope(env, scope);
        return;
    }

    NVal obj = CreateUploadProgressObject(env, progress);
    napi_value retVal = nullptr;
    napi_value global = nullptr;
    napi_get_global(env, &global);
    status = napi_call_function(env, global, jsCallback, ARGS_ONE, &(obj.val_), &retVal);
    if (status != napi_ok) {
        LOGE("napi call function failed, status: %{public}d", status);
    }
    napi_close_handle_scope(env, scope);
}

void CloudUploadCallbackImpl::OnUploadProgress(const UploadProgressObj &progress)
{
    UvChangeMsg *msg = new (std::nothrow) UvChangeMsg(shared_from_this(), progress);
    if (msg == nullptr) {
        LOGE("Failed to create uv message object");
        return;
    }
    auto task = [msg, progress]() {
        if (msg->cloudUploadCallback_.expired()) {
            LOGE("cloudUploadCallback_ is expired");
            delete msg;
            return;
        }
        auto cloudUploadCallback = msg->cloudUploadCallback_.lock();
        if (cloudUploadCallback == nullptr || cloudUploadCallback->cbOnRef_->cbRef == nullptr) {
            LOGE("cloudUploadCallback->cbOnRef_->cbRef is nullptr");
            delete msg;
            return;
        }
        napi_ref ref = nullptr;
        {
            unique_lock<mutex> lock(cloudUploadCallback->cbOnRef_->refMtx);
            ref = cloudUploadCallback->cbOnRef_->cbRef;
        }
        CallUploadProgressCallback(cloudUploadCallback->env_, ref, progress);
        delete msg;
    };
    napi_status ret = napi_send_event(env_, task, napi_event_priority::napi_eprio_immediate, taskName_.c_str());
    if (ret != napi_ok) {
        LOGE("Failed to execute libuv work queue, ret: %{public}d", ret);
        delete msg;
        return;
    }
}

void CloudUploadCallbackImpl::DeleteReference()
{
    unique_lock<mutex> lock(cbOnRef_->refMtx);
    if (cbOnRef_->cbRef != nullptr) {
        napi_delete_reference(env_, cbOnRef_->cbRef);
        cbOnRef_->cbRef = nullptr;
    }
}

void CloudUploadCallbackImpl::DeleteReferenceAsync()
{
    if (cbOnRef_->cbRef == nullptr) {
        return;
    }

    auto status = napi_send_event(
        env_,
        [env{env_}, cbOnRef{cbOnRef_}]() mutable {
            unique_lock<mutex> lock(cbOnRef->refMtx);
            if ((env == nullptr) || (cbOnRef->cbRef == nullptr)) {
                LOGE("Failed to callback, is napi_ref null: %{public}d.", (cbOnRef->cbRef == nullptr));
                return;
            }
            auto status = napi_delete_reference(env, cbOnRef->cbRef);
            if (status != napi_ok) {
                LOGE("Delete reference failed, status: %{public}d", status);
                return;
            }
            cbOnRef->cbRef = nullptr;
        },
        napi_event_priority::napi_eprio_immediate, taskName_.c_str());
    if (status != napi_ok) {
        LOGE("Failed to execute libuv work queue, status: %{public}d", status);
    }
}

struct SyncStateArg {
    vector<int32_t> stateList{};
};

bool FileSyncNapi::Export()
{
    std::vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("on", FileSyncNapi::OnCallback),
        NVal::DeclareNapiFunction("off", FileSyncNapi::OffCallback),
        NVal::DeclareNapiFunction("start", FileSyncNapi::Start),
        NVal::DeclareNapiFunction("stop", FileSyncNapi::Stop),
        NVal::DeclareNapiFunction("getLastSyncTime", FileSyncNapi::GetLastSyncTime),
        NVal::DeclareNapiFunction("getUploadList", FileSyncNapi::GetUploadList),
        NVal::DeclareNapiFunction("registerUploadProgress", FileSyncNapi::RegisterUploadProgress),
        NVal::DeclareNapiFunction("unregisterUploadProgress", FileSyncNapi::UnRegisterUploadProgress),
    };

    SetClassName("FileSync");
    return ToExport(props);
}
} // namespace OHOS::FileManagement::CloudSync