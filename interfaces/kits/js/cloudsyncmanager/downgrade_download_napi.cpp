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

#include "downgrade_download_napi.h"

#include <cstddef>
#include <sys/types.h>

#include "async_work.h"
#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;
static int32_t CheckPermissions(const string &permission, bool isSystemApp)
{
    if (!permission.empty() && !DfsuAccessTokenHelper::CheckCallerPermission(permission)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (isSystemApp && !DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    return E_OK;
}

DowngradeDlCallbackImpl::DowngradeDlCallbackImpl(napi_env env, napi_value func) : env_(env)
{
    napi_status status = napi_create_reference(env_, func, 1, &cbOnRef_);
    if (status != napi_ok) {
        LOGE("Failed to create napi ref, %{public}d", status);
    }
}

DowngradeDlCallbackImpl::~DowngradeDlCallbackImpl()
{
    if (cbOnRef_ != nullptr) {
        napi_status status = napi_delete_reference(env_, cbOnRef_);
        if (status != napi_ok) {
            LOGE("Failed to delete napi ref, %{public}d", status);
        }
        cbOnRef_ = nullptr;
    }
}

napi_value DowngradeDlCallbackImpl::ConvertToValue()
{
    napi_value progressVal = NClass::InstantiateClass(env_, DowngradeProgressNapi::className_, {});
    if (progressVal == nullptr) {
        LOGE("Failed to instantiate class");
        return nullptr;
    }
    auto progressEntity = NClass::GetEntityOf<DowngradeProgressEntity>(env_, progressVal);
    if (progressEntity == nullptr) {
        LOGE("Failed to get progressEntity.");
        return nullptr;
    }

    progressEntity->progress = dlProgress_;
    return progressVal;
}

void DowngradeDlCallbackImpl::UpdateDownloadProgress(const DowngradeProgress &progress)
{
    if (dlProgress_ == nullptr) {
        dlProgress_ = std::make_shared<SingleBundleProgress>();
    }

    dlProgress_->state = static_cast<int32_t>(progress.state);
    dlProgress_->downloadedSize = progress.downloadedSize;
    dlProgress_->totalSize = progress.totalSize;
    dlProgress_->successfulCount = progress.successfulCount;
    dlProgress_->failedCount = progress.failedCount;
    dlProgress_->totalCount = progress.totalCount;
    dlProgress_->stopReason = static_cast<int32_t>(progress.stopReason);
}

void DowngradeDlCallbackImpl::OnDownloadProcess(const DowngradeProgress &progress)
{
    UpdateDownloadProgress(progress);
    std::shared_ptr<DowngradeDlCallbackImpl> callbackImpl = shared_from_this();
    napi_status status = napi_send_event(
        callbackImpl->env_,
        [callbackImpl]() mutable {
            auto env = callbackImpl->env_;
            auto ref = callbackImpl->cbOnRef_;
            if (env == nullptr || ref == nullptr) {
                LOGE("The env context is invalid");
                return;
            }
            napi_handle_scope scope = nullptr;
            napi_status status = napi_open_handle_scope(env, &scope);
            if (status != napi_ok) {
                LOGE("Failed to open handle scope, status: %{public}d", status);
                return;
            }
            napi_value jsCallback = nullptr;
            status = napi_get_reference_value(env, ref, &jsCallback);
            if (status != napi_ok) {
                LOGE("Create reference failed, status: %{public}d", status);
                napi_close_handle_scope(env, scope);
                return;
            }
            napi_value jsProgress = callbackImpl->ConvertToValue();
            if (jsProgress == nullptr) {
                napi_close_handle_scope(env, scope);
                return;
            }
            napi_value jsResult = nullptr;
            status = napi_call_function(env, nullptr, jsCallback, 1, &jsProgress, &jsResult);
            if (status != napi_ok) {
                LOGE("napi call function failed, status: %{public}d", status);
            }
            napi_close_handle_scope(env, scope);
        },
        napi_eprio_immediate);
    if (status != napi_ok) {
        LOGE("Failed to execute libuv work queue, status: %{public}d", status);
    }
}

napi_value DowngradeDownloadNapi::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        LOGE("Start Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    int32_t ret = CheckPermissions(PERM_CLOUD_SYNC_MANAGER, true);
    if (ret != E_OK) {
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        return nullptr;
    }
    auto [succ, bundle, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    std::string bundleName(bundle.get());
    if (!succ || bundleName.empty()) {
        LOGE("Failed to get bundleName.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto downgradeEntity = make_unique<DowngradeEntity>(bundleName);
    if (!NClass::SetEntityFor<DowngradeEntity>(env, funcArg.GetThisVar(), move(downgradeEntity))) {
        LOGE("Failed to set file cache entity.");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    return funcArg.GetThisVar();
}

bool DowngradeDownloadNapi::ToExport(std::vector<napi_property_descriptor> props)
{
    std::string className = GetClassName();
    auto [succ, classValue] = NClass::DefineClass(exports_.env_, className, Constructor, std::move(props));
    if (!succ) {
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(exports_.env_);
        LOGE("Failed to define GallerySync class");
        return false;
    }

    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(exports_.env_);
        LOGE("Failed to save GallerySync class");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

bool DowngradeDownloadNapi::Export()
{
    std::vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("startDownload", DowngradeDownloadNapi::StartDownload),
        NVal::DeclareNapiFunction("stopDownload", DowngradeDownloadNapi::StopDownload),
        NVal::DeclareNapiFunction("getCloudFileInfo", DowngradeDownloadNapi::GetCloudFileInfo),
    };

    return ToExport(props);
}

napi_value DowngradeDownloadNapi::StartDownload(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        LOGE("Start Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    NVal callbackVal(env, funcArg[NARG_POS::FIRST]);
    if (!callbackVal.TypeIs(napi_function)) {
        LOGE("Batch-On argument type mismatch");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto downgradeEntity = NClass::GetEntityOf<DowngradeEntity>(env, funcArg.GetThisVar());
    if (!downgradeEntity) {
        LOGE("Failed to get downgrade entity.");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
        return nullptr;
    }

    if (downgradeEntity->callbackImpl == nullptr) {
        downgradeEntity->callbackImpl = make_shared<DowngradeDlCallbackImpl>(env, callbackVal.val_);
    }

    auto cbExec = [callbackImpl{downgradeEntity->callbackImpl}, bundleName{downgradeEntity->bundleName}]() -> NError {
        if (callbackImpl == nullptr) {
            LOGE("Failed to get download callback");
            return NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR));
        }
        int32_t ret = CloudSyncManager::GetInstance().StartDowngrade(bundleName, callbackImpl);
        if (ret != E_OK) {
            LOGE("Start downgrade failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }

        LOGI("Start downgrade success!");
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };

    string procedureName = "downgradeDownload";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

napi_value DowngradeDownloadNapi::StopDownload(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);
    auto downgradeEntity = NClass::GetEntityOf<DowngradeEntity>(env, funcArg.GetThisVar());
    if (!downgradeEntity || downgradeEntity->callbackImpl == nullptr) {
        LOGE("Failed to get downgrade entity.");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [bundleName{downgradeEntity->bundleName}]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().StopDowngrade(bundleName);
        if (ret != E_OK) {
            LOGE("Stop downgrade failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        LOGI("Stop downgrade success!");
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };

    string procedureName = "downgradeDownload";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

napi_value DowngradeDownloadNapi::GetCloudFileInfo(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);
    auto downgradeEntity = NClass::GetEntityOf<DowngradeEntity>(env, funcArg.GetThisVar());
    if (!downgradeEntity) {
        LOGE("Failed to get downgrade entity.");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
        return nullptr;
    }

    auto fileInfo = std::make_shared<CloudFileInfo>();
    std::string bundleName = downgradeEntity->bundleName;
    auto cbExec = [bundleName{downgradeEntity->bundleName}, fileInfo]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().GetCloudFileInfo(bundleName, *fileInfo);
        if (ret != E_OK) {
            LOGE("Start Download failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        LOGI("Start Download Success!");
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [fileInfo](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        NVal obj = NVal::CreateObject(env);
        obj.AddProp("cloudfileCount", NVal::CreateInt32(env, fileInfo->cloudfileCount).val_);
        obj.AddProp("cloudFileTotalSize", NVal::CreateInt64(env, fileInfo->cloudFileTotalSize).val_);
        obj.AddProp("localFileCount", NVal::CreateInt32(env, fileInfo->localFileCount).val_);
        obj.AddProp("localFileTotalSize", NVal::CreateInt64(env, fileInfo->localFileTotalSize).val_);
        obj.AddProp("bothFileCount", NVal::CreateInt32(env, fileInfo->bothFileCount).val_);
        obj.AddProp("bothFileTotalSize", NVal::CreateInt64(env, fileInfo->bothFileTotalSize).val_);
        return obj;
    };

    string procedureName = "downgradeDownload";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::ONE));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

string DowngradeDownloadNapi::GetClassName()
{
    return className_;
}
} // namespace OHOS::FileManagement::CloudSync