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

#include "cloud_file_napi.h"

#include <memory>
#include <mutex>
#include <sys/types.h>

#include "async_work.h"
#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "filemgmt_libn.h"
#include "n_napi.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;
int32_t CloudDownloadCallbackImplNapi::StartDownloadInner(const std::string &uri)
{
    int64_t downloadId = 0;
    std::lock_guard<std::mutex> lock(downloadInfoMtx_);
    int32_t ret = CloudSyncManager::GetInstance().StartDownloadFile(uri, shared_from_this(), downloadId);
    if (ret != E_OK) {
        LOGE("Start batch download failed! ret = %{public}d", ret);
        return ret;
    }

    downloadInfos_[downloadId] = std::make_shared<SingleProgressNapi>(downloadId);
    return ret;
}

int32_t CloudDownloadCallbackImplNapi::StopDownloadInner(const std::string &uri)
{
    auto downloadIdList = GetDownloadIdsByUri(uri);
    int32_t ret = E_OK;
    int32_t resErr = E_OK;
    LOGI("Stop Download downloadId list size: %{public}zu", downloadIdList.size());
    for (auto taskId : downloadIdList) {
        resErr = CloudSyncManager::GetInstance().StopDownloadFile(taskId, true);
        if (resErr != E_OK) {
            ret = resErr;
            continue;
        }
    }
    if (ret != E_OK) {
        LOGE("Stop Download failed! ret = %{public}d", ret);
    }
    return ret;
}

napi_value CloudFileDownloadNapi::Constructor(napi_env env, napi_callback_info info)
{
    LOGI("CloudFileDownloadNapi::Constructor begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        LOGE("Start Number of arguments unmatched");
        NError(JsErrCode::E_IPCSS).ThrowErr(env);
        return nullptr;
    }
    auto downloadEntity = make_unique<DownloadEntity>();
    if (!NClass::SetEntityFor<DownloadEntity>(env, funcArg.GetThisVar(), move(downloadEntity))) {
        LOGE("Failed to set download entity.");
        NError(JsErrCode::E_IPCSS).ThrowErr(env);
        return nullptr;
    }
    LOGI("CloudFileDownloadNapi::Constructor end");
    return funcArg.GetThisVar();
}

static std::shared_ptr<CloudDownloadCallbackImplNapi> GetCallbackImpl(napi_env env, NFuncArg &funcArg, bool isInit)
{
    auto downloadEntity = NClass::GetEntityOf<DownloadEntity>(env, funcArg.GetThisVar());
    if (!downloadEntity) {
        LOGE("Failed to get file cache entity.");
        return nullptr;
    }

    if (downloadEntity->callbackImpl == nullptr && isInit) {
        downloadEntity->callbackImpl = std::make_shared<CloudDownloadCallbackImplNapi>(env);
    }
    return downloadEntity->callbackImpl;
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
    auto [succUri, uri, size] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succUri || size == 0) {
        LOGE("Start get uri parameter failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto callbackImpl = GetCallbackImpl(env, funcArg, true);
    auto cbExec = [uri{string(uri.get())}, callbackImpl{callbackImpl}]() -> NError {
        if (callbackImpl == nullptr) {
            LOGE("Failed to get download callback");
            return NError(E_PARAMS);
        }
        int32_t ret = callbackImpl->StartDownloadInner(uri);
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

static int32_t CheckPermissions(const string &permission, bool isSystemApp)
{
    if (!permission.empty() && !DfsuAccessTokenHelper::CheckCallerPermission(permission)) {
        LOGE("permission denied");
        return JsErrCode::E_PERMISSION;
    }
    if (isSystemApp && !DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return JsErrCode::E_PERMISSION_SYS;
    }
    return E_OK;
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
    int32_t ret = CheckPermissions(PERM_CLOUD_SYNC, true);
    if (ret != E_OK) {
        LOGE("On get progress failed!");
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    auto [succProgress, progress, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succProgress || std::string(progress.get()) != "progress") {
        LOGE("On get progress failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    if (!NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) {
        LOGE("Argument type mismatch");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto callbackImpl = GetCallbackImpl(env, funcArg, true);
    ret = callbackImpl->RegisterCallback(funcArg[NARG_POS::SECOND]);
    if (ret != napi_ok) {
        LOGE("On register callback fail, ret: %{public}d", ret);
        NError(JsErrCode::E_IPCSS).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value CloudFileDownloadNapi::Off(napi_env env, napi_callback_info info)
{
    LOGI("Off begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        LOGE("Off Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    int32_t ret = CheckPermissions(PERM_CLOUD_SYNC, true);
    if (ret != E_OK) {
        LOGE("On get progress failed!");
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    auto [succProgress, progress, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succProgress || std::string(progress.get()) != "progress") {
        LOGE("Off get progress failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    napi_value callbackVel = nullptr;
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        if (!NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) {
            LOGE("Off argument type mismatch");
            NError(E_PARAMS).ThrowErr(env);
            return nullptr;
        }
        callbackVel = funcArg[NARG_POS::SECOND];
    }

    auto callbackImpl = GetCallbackImpl(env, funcArg, false);
    if (callbackImpl == nullptr || callbackImpl->UnregisterCallback(callbackVel) != napi_ok) {
        LOGE("Off no callback is registered for this event type");
        NError(JsErrCode::E_IPCSS).ThrowErr(env);
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

    auto [succUri, uri, size] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succUri || size == 0) {
        LOGE("Stop get uri parameter failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto callbackImpl = GetCallbackImpl(env, funcArg, false);
    auto cbExec = [uri{string(uri.get())}, callbackImpl{callbackImpl}]() -> NError {
        if (callbackImpl == nullptr) {
            LOGE("Failed to get download callback");
            return NError(E_PARAMS);
        }
        int32_t ret = callbackImpl->StopDownloadInner(uri);
        if (ret != E_OK) {
            LOGE("Stop Download failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
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

bool CloudFileDownloadNapi::ToExport(std::vector<napi_property_descriptor> props)
{
    std::string className = GetClassName();
    auto [succ, classValue] = NClass::DefineClass(exports_.env_, className, Constructor, std::move(props));
    if (!succ) {
        NError(JsErrCode::E_IPCSS).ThrowErr(exports_.env_);
        LOGE("Failed to define GallerySync class");
        return false;
    }

    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        NError(JsErrCode::E_IPCSS).ThrowErr(exports_.env_);
        LOGE("Failed to save GallerySync class");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

bool CloudFileDownloadNapi::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("start", CloudFileDownloadNapi::Start),
        NVal::DeclareNapiFunction("on", CloudFileDownloadNapi::On),
        NVal::DeclareNapiFunction("off", CloudFileDownloadNapi::Off),
        NVal::DeclareNapiFunction("stop", CloudFileDownloadNapi::Stop),
    };
    return ToExport(props);
}

void CloudFileDownloadNapi::SetClassName(std::string classname)
{
    className_ = classname;
}

string CloudFileDownloadNapi::GetClassName()
{
    return className_;
}
} // namespace OHOS::FileManagement::CloudSync