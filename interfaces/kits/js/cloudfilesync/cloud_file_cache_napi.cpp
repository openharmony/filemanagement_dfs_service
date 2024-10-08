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

#include "cloud_file_cache_napi.h"

#include <cstddef>
#include <sys/types.h>

#include "async_work.h"
#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "uv.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;

napi_value CloudFileCacheNapi::CleanCloudFileCache(napi_env env, napi_callback_info info)
{
    LOGI("CleanCache start");
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto [succ, uri, ignore] = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        LOGE("Get uri error");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    int32_t ret = CloudSyncManager::GetInstance().CleanCache(uri.get());
    if (ret != E_OK) {
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        return nullptr;
    }
    return NVal::CreateUndefined(env).val_;
}

bool CloudFileCacheNapi::Export()
{
    std::vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("on", CloudFileCacheNapi::On),
        NVal::DeclareNapiFunction("off", CloudFileCacheNapi::Off),
        NVal::DeclareNapiFunction("start", CloudFileCacheNapi::StartFileCache),
        NVal::DeclareNapiFunction("startBatch", CloudFileCacheNapi::StartBatchFileCache),
        NVal::DeclareNapiFunction("stop", CloudFileCacheNapi::StopFileCache),
        NVal::DeclareNapiFunction("stopBatch", CloudFileCacheNapi::StopBatchFileCache),
        NVal::DeclareNapiFunction("cleanCache", CloudFileCacheNapi::CleanCloudFileCache),
    };

    return ToExport(props);
}

napi_value CloudFileCacheNapi::StartFileCache(napi_env env, napi_callback_info info)
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

    auto cbExec = [uri = string(uri.get())]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().StartFileCache(uri);
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

    string procedureName = "cloudFileCache";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

napi_value CloudFileCacheNapi::StopFileCache(napi_env env, napi_callback_info info)
{
    LOGI("Stop begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        LOGE("Stop Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto [succ, uri, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        LOGE("Stop get uri parameter failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    bool needClean = false;
    size_t maxArgSize = static_cast<size_t>(NARG_CNT::TWO);
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        NVal option(env, funcArg[NARG_POS::SECOND]);
        if (!option.TypeIs(napi_function)) {
            tie(succ, needClean) = option.ToBool();
            if (!succ) {
                LOGE("Failed to get clean flag!");
                NError(E_PARAMS).ThrowErr(env);
                return nullptr;
            }
            maxArgSize = static_cast<size_t>(NARG_CNT::THREE);
        }
    }
    auto cbExec = [uri = string(uri.get()), env = env, needClean]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().StopDownloadFile(uri, needClean);
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

    string procedureName = "cloudFileCache";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, maxArgSize);
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

struct FileCacheArg {
    vector<string> uriList;
    int64_t downloadId;
};

napi_value CloudFileCacheNapi::StartBatchFileCache(napi_env env, napi_callback_info info)
{
    LOGI("Batch start file cache start.");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        LOGE("Start Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto fileUris = std::make_shared<FileCacheArg>();
    auto [resGetUris, uriArray, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToStringArray();
    if (!resGetUris) {
        LOGE("Start get uri array parameter failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    fileUris->uriList = uriArray;
    auto cbExec = [fileUris]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().StartFileCache(fileUris->uriList, fileUris->downloadId);
        if (ret != E_OK) {
            LOGE("Batch start file cache failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        LOGI("Batch start file cache successful!");
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [fileUris](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateInt64(env, fileUris->downloadId);
    };

    string procedureName = "cloudFileCache";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

napi_value CloudFileCacheNapi::StopBatchFileCache(napi_env env, napi_callback_info info)
{
    LOGI("Batch stop file cache start.");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        LOGE("Start Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto [succ, downloadId] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt64();
    if (!succ) {
        LOGE("Start get download ID parameter failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    bool needClean = false;
    size_t maxArgSize = static_cast<size_t>(NARG_CNT::TWO);
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        NVal option(env, funcArg[NARG_POS::SECOND]);
        if (!option.TypeIs(napi_function)) {
            tie(succ, needClean) = option.ToBool();
            if (!succ) {
                LOGE("Failed to get clean flag!");
                NError(E_PARAMS).ThrowErr(env);
                return nullptr;
            }
            maxArgSize = static_cast<size_t>(NARG_CNT::THREE);
        }
    }

    auto cbExec = [downloadId = downloadId, needClean]() -> NError {
        int32_t ret = CloudSyncManager::GetInstance().StopFileCache(downloadId, needClean);
        if (ret != E_OK) {
            LOGE("Batch stop file cache failed! ret = %{public}d", ret);
            return NError(Convert2JsErrNum(ret));
        }
        LOGI("Batch stop file cache successful!");
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateUndefined(env);
    };

    string procedureName = "cloudFileCache";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, maxArgSize);
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

napi_value CloudFileCacheNapi::On(napi_env env, napi_callback_info info)
{
    LOGI("Batch-On begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        LOGE("Batch-On Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto [succProgress, progress, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    string eventType(progress.get());
    if (!succProgress || (eventType != "progress" && eventType != "multiProgress")) {
        LOGE("Batch-On get progress failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    if (!NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) {
        LOGE("Batch-On argument type mismatch");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    if (callback_ != nullptr) {
        LOGI("Batch-On callback already exist");
        return NVal::CreateUndefined(env).val_;
    }

    if (eventType == "progress") {
        callback_ = make_shared<CloudDownloadCallbackImpl>(env, NVal(env, funcArg[(int)NARG_POS::SECOND]).val_);
    } else if (eventType == "multiProgress") {
        callback_ = make_shared<CloudDownloadCallbackImpl>(env, NVal(env, funcArg[(int)NARG_POS::SECOND]).val_, true);
    }

    int32_t ret = CloudSyncManager::GetInstance().RegisterDownloadFileCallback(callback_);
    if (ret != E_OK) {
        LOGE("Failed to register callback, error: %{public}d", ret);
        NError(Convert2JsErrNum(ret)).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value CloudFileCacheNapi::Off(napi_env env, napi_callback_info info)
{
    LOGI("Off begin");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        LOGE("Off Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto [succProgress, progress, ignore] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    string eventType(progress.get());
    if (!succProgress || (eventType != "progress" && eventType != "multiProgress")) {
        LOGE("Batch-Off get progress failed!");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO && !NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) {
        LOGE("Batch-Off argument type mismatch");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    /* callback_ may be nullptr */
    int32_t ret = CloudSyncManager::GetInstance().UnregisterDownloadFileCallback();
    if (ret != E_OK) {
        LOGE("Failed to unregister callback, error: %{public}d", ret);
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

string CloudFileCacheNapi::GetClassName()
{
    return className_;
}
} // namespace OHOS::FileManagement::CloudSync