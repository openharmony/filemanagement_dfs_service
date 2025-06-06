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
#include <sys/types.h>

#include "async_work.h"
#include "cloud_sync_manager.h"
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
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == NARG_CNT::ONE) {
        if (!NVal(env, funcArg[NARG_POS::FIRST]).TypeIs(napi_number)) {
            NVal cb(env, funcArg[NARG_POS::FIRST]);
            return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
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
    if (!FileSyncNapi::InitArgsOnCallback(env, funcArg)) {
        return nullptr;
    }

    string bundleName = GetBundleName(env, funcArg);
    callback_ = make_shared<CloudSyncCallbackImpl>(env, NVal(env, funcArg[(int)NARG_POS::SECOND]).val_);
    int32_t ret = CloudSyncManager::GetInstance().RegisterFileSyncCallback(callback_, bundleName);
    if (ret != E_OK) {
        LOGE("OnCallback Register error, result: %{public}d", ret);
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value FileSyncNapi::OffCallback(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!FileSyncNapi::InitArgsOffCallback(env, funcArg)) {
        return nullptr;
    }

    string bundleName = GetBundleName(env, funcArg);
    int32_t ret = CloudSyncManager::GetInstance().UnRegisterFileSyncCallback(bundleName);
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
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
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
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbComplete).val_;
}

struct SyncStateArg {
    vector<int32_t> stateList {};
};

bool FileSyncNapi::Export()
{
    std::vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("on", FileSyncNapi::OnCallback),
        NVal::DeclareNapiFunction("off", FileSyncNapi::OffCallback),
        NVal::DeclareNapiFunction("start", FileSyncNapi::Start),
        NVal::DeclareNapiFunction("stop", FileSyncNapi::Stop),
        NVal::DeclareNapiFunction("getLastSyncTime", FileSyncNapi::GetLastSyncTime),
    };

    SetClassName("FileSync");
    return ToExport(props);
}
} // namespace OHOS::FileManagement::CloudSync