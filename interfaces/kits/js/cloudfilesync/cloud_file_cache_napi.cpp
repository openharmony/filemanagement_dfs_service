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

#include "cloud_file_cache_napi.h"

#include <cstddef>
#include <set>
#include <memory>
#include <mutex>
#include <sys/types.h>

#include "async_work.h"
#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "multi_download_progress_napi.h"
#include "register_callback_manager_napi.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;

static const std::set<int32_t> errForSingleFileSync = { ENOENT, EACCES, EAGAIN, EINTR, ENOSYS, E_ILLEGAL_URI };

int32_t CloudFileCacheCallbackImplNapi::StartDownloadInner(const std::string &uri, int32_t fieldKey)
{
    int64_t downloadId = 0;
    std::lock_guard<std::mutex> lock(downloadInfoMtx_);
    int32_t ret = CloudSyncManager::GetInstance().StartFileCache({uri}, downloadId, fieldKey, shared_from_this());
    if (ret != E_OK) {
        LOGE("Start single download failed! ret = %{public}d", ret);
        return ret;
    }
    downloadInfos_[downloadId] = std::make_shared<SingleProgressNapi>(downloadId);
    return ret;
}

int32_t CloudFileCacheCallbackImplNapi::StopDownloadInner(const std::string &uri, bool needClean)
{
    auto downloadIdList = GetDownloadIdsByUri(uri);
    int32_t ret = E_OK;
    int32_t resErr = E_OK;
    LOGI("Stop Download downloadId list size: %{public}zu", downloadIdList.size());
    for (auto taskId : downloadIdList) {
        resErr = CloudSyncManager::GetInstance().StopFileCache(taskId, needClean);
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

int32_t CloudFileCacheCallbackImplNapi::StartDownloadInner(const std::vector<std::string> &uriVec,
                                                           int64_t &downloadId,
                                                           int32_t fieldKey)
{
    std::unique_lock<std::mutex> lock(downloadInfoMtx_);
    int32_t ret = CloudSyncManager::GetInstance().StartFileCache(uriVec, downloadId, fieldKey, shared_from_this());
    if (ret != E_OK) {
        LOGE("Start batch download failed! ret = %{public}d", ret);
        return ret;
    }
    downloadInfos_[downloadId] = std::make_shared<BatchProgressNapi>(downloadId);
    return ret;
}

int32_t CloudFileCacheCallbackImplNapi::StopDownloadInner(int64_t downloadId, bool needClean)
{
    int32_t ret = CloudSyncManager::GetInstance().StopFileCache(downloadId, needClean);
    if (ret != E_OK) {
        LOGE("Batch stop file cache failed! ret = %{public}d", ret);
    }
    return ret;
}

napi_value CloudFileCacheNapi::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        LOGE("Start Number of arguments unmatched");
        NError(JsErrCode::E_INNER_FAILED).ThrowErr(env);
        return nullptr;
    }

    auto fileCacheEntity = make_unique<FileCacheEntity>();
    if (!NClass::SetEntityFor<FileCacheEntity>(env, funcArg.GetThisVar(), move(fileCacheEntity))) {
        LOGE("Failed to set file cache entity.");
        NError(JsErrCode::E_INNER_FAILED).ThrowErr(env);
        return nullptr;
    }
    return funcArg.GetThisVar();
}

bool CloudFileCacheNapi::ToExport(std::vector<napi_property_descriptor> props)
{
    std::string className = GetClassName();
    auto [succ, classValue] = NClass::DefineClass(exports_.env_, className, Constructor, std::move(props));
    if (!succ) {
        NError(JsErrCode::E_INNER_FAILED).ThrowErr(exports_.env_);
        LOGE("Failed to define GallerySync class");
        return false;
    }

    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        NError(JsErrCode::E_INNER_FAILED).ThrowErr(exports_.env_);
        LOGE("Failed to save GallerySync class");
        return false;
    }

    return exports_.AddProp(className, classValue);
}

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

static int32_t GetCleanCacheErrPublic(int32_t result)
{
    LOGE("cleanCache failed, errno : %{public}d", result);
    if (errForSingleFileSync.find(result) != errForSingleFileSync.end()) {
        return result;
    }
    return E_SERVICE_INNER_ERROR;
}

napi_value CloudFileCacheNapi::CleanFileCache(napi_env env, napi_callback_info info)
{
    LOGI("CleanCache start");
    NFuncArg funcArg(env, info);

    if (!funcArg.InitArgs(NARG_CNT::ONE)) {
        NError(EINVAL).ThrowErr(env);
        LOGE("Number of arguments unmatched");
        return nullptr;
    }

    auto [succ, uri, ignore] = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        LOGE("CleanFileCache get uri parameter failed!");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    int32_t ret = CloudSyncManager::GetInstance().CleanFileCache(uri.get());
    if (ret != E_OK) {
        int32_t err = GetCleanCacheErrPublic(ret);
        NError(Convert2JsErrNum(err)).ThrowErr(env);
        LOGE("CleanFileCache failed, ret:%{public}d", err);
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
        NVal::DeclareNapiFunction("cleanFileCache", CloudFileCacheNapi::CleanFileCache),
    };

    return ToExport(props);
}

static std::shared_ptr<CloudFileCacheCallbackImplNapi> GetCallbackImpl(napi_env env, NFuncArg &funcArg,
    const std::string &eventType, bool isInit)
{
    auto fileCacheEntity = NClass::GetEntityOf<FileCacheEntity>(env, funcArg.GetThisVar());
    if (!fileCacheEntity) {
        LOGE("Failed to get file cache entity.");
        return nullptr;
    }

    std::shared_ptr<CloudFileCacheCallbackImplNapi> callbackImpl = nullptr;
    auto iter = fileCacheEntity->registerMap.find(eventType);
    if (iter == fileCacheEntity->registerMap.end() || iter->second == nullptr) {
        if (isInit) {
            callbackImpl = std::make_shared<CloudFileCacheCallbackImplNapi>(env);
            fileCacheEntity->registerMap.insert(make_pair(eventType, callbackImpl));
        }
    } else {
        callbackImpl = iter->second;
    }
    return callbackImpl;
}

static std::tuple<int32_t, std::string> ParseUriFromParam(napi_env env, NFuncArg &funcArg)
{
    auto [succ, uri, size] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succ || size == 0) {
        LOGE("Off get progress failed!");
        return {E_PARAMS, ""};
    }
    return {E_OK, string(uri.get())};
}

napi_value CloudFileCacheNapi::StartFileCache(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        LOGE("Start Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto [ret, uri] = ParseUriFromParam(env, funcArg);
    if (ret != E_OK) {
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    auto callbackImpl = GetCallbackImpl(env, funcArg, PROGRESS, true);
    auto cbExec = [uri{uri}, callbackImpl{callbackImpl}]() -> NError {
        if (callbackImpl == nullptr) {
            LOGE("Failed to get download callback");
            return NError(E_PARAMS);
        }
        int32_t ret = callbackImpl->StartDownloadInner(uri, FieldKey::FIELDKEY_CONTENT);
        if (ret != E_OK) {
            LOGE("Start Download failed! ret = %{public}d", ret);
            if (ret != E_INVAL_ARG) {
                ret = E_BROKEN_IPC;
            }
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

    string procedureName = "cloudFileCache";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

static tuple<int32_t, bool, bool> GetCleanFlagForStop(napi_env env, NFuncArg &funcArg)
{
    bool succ = true;
    bool needClean = false;
    size_t maxArgSize = static_cast<size_t>(NARG_CNT::TWO);
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        NVal option(env, funcArg[NARG_POS::SECOND]);
        if (!option.TypeIs(napi_function)) {
            tie(succ, needClean) = option.ToBool();
            maxArgSize = static_cast<size_t>(NARG_CNT::THREE);
        }
        if (!succ) {
            LOGE("Failed to get clean flag!");
            return {EINVAL, false, false};
        }
    }
    return {E_OK, needClean, maxArgSize};
}

napi_value CloudFileCacheNapi::StopFileCache(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        LOGE("Stop Number of arguments unmatched");
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto [res, uri] = ParseUriFromParam(env, funcArg);
    if (res != E_OK) {
        NError(res).ThrowErr(env);
        return nullptr;
    }

    auto [ret, needClean, maxArgSize] = GetCleanFlagForStop(env, funcArg);
    if (ret != E_OK) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    auto callbackImpl = GetCallbackImpl(env, funcArg, PROGRESS, false);
    auto cbExec = [uri{uri}, needClean{needClean}, callbackImpl{callbackImpl}]() -> NError {
        if (callbackImpl == nullptr) {
            LOGE("Failed to get download callback");
            return NError(E_PARAMS);
        }
        int32_t ret = callbackImpl->StopDownloadInner(uri, needClean);
        if (ret != E_OK) {
            LOGE("Stop Download failed! ret = %{public}d", ret);
            if (ret != E_INVAL_ARG) {
                ret = E_BROKEN_IPC;
            }
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

    string procedureName = "cloudFileCache";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, maxArgSize);
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

struct FileCacheArg {
    std::vector<std::string> uriList;
    int64_t downloadId = 0;
    std::shared_ptr<CloudFileCacheCallbackImplNapi> callbackImpl = nullptr;
    int32_t fieldKey = FieldKey::FIELDKEY_CONTENT;
};

static std::tuple<int32_t, std::shared_ptr<FileCacheArg>, int32_t> FillParamForBatchStart(napi_env env,
                                                                                          NFuncArg &funcArg)
{
    size_t maxArgSize = static_cast<size_t>(NARG_CNT::TWO);
    auto [succ, uriArray, size] = NVal(env, funcArg[NARG_POS::FIRST]).ToStringArray();
    if (!succ || size == 0) {
        LOGE("Start get uri array parameter failed!");
        return {EINVAL, nullptr, maxArgSize};
    }

    int32_t fieldKey = 0;
    if (funcArg.GetArgc() >= NARG_CNT::TWO) {
        NVal option(env, funcArg[NARG_POS::SECOND]);
        if (!option.TypeIs(napi_function)) {
            tie(succ, fieldKey) = option.ToInt32();
            maxArgSize = static_cast<size_t>(NARG_CNT::THREE);
        }
        if (!succ) {
            LOGE("Failed to get fileType!");
            return {EINVAL, nullptr, maxArgSize};
        }
    }

    auto fileCache = std::make_shared<FileCacheArg>();
    fileCache->uriList.swap(uriArray);
    fileCache->fieldKey = fieldKey;
    fileCache->callbackImpl = GetCallbackImpl(env, funcArg, MULTI_PROGRESS, true);
    return {E_OK, fileCache, maxArgSize};
}

napi_value CloudFileCacheNapi::StartBatchFileCache(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        LOGE("Start Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [ret, fileCache, maxArgSize] = FillParamForBatchStart(env, funcArg);
    if (ret != E_OK) {
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    auto cbExec = [fileCache{fileCache}]() -> NError {
        if (fileCache == nullptr || fileCache->callbackImpl == nullptr) {
            LOGE("Failed to get download callback");
            return NError(JsErrCode::E_INNER_FAILED);
        }
        int32_t ret =
            fileCache->callbackImpl->StartDownloadInner(fileCache->uriList, fileCache->downloadId, fileCache->fieldKey);
        if (ret != E_OK) {
            ret = (ret == E_CLOUD_SDK) ? E_SERVICE_INNER_ERROR : ret;
            return NError(Convert2JsErrNum(ret));
        }
        return NError(ERRNO_NOERR);
    };

    auto cbCompl = [fileCache{fileCache}](napi_env env, NError err) -> NVal {
        if (err) {
            return {env, err.GetNapiErr(env)};
        }
        return NVal::CreateInt64(env, fileCache->downloadId);
    };

    string procedureName = "cloudFileCache";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, maxArgSize);
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

napi_value CloudFileCacheNapi::StopBatchFileCache(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::THREE)) {
        LOGE("Start Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [succ, downloadId] = NVal(env, funcArg[NARG_POS::FIRST]).ToInt64();
    if (!succ || downloadId <= 0) {
        LOGE("Start get taskId parameter failed!");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto [ret, needClean, maxArgSize] = GetCleanFlagForStop(env, funcArg);
    if (ret != E_OK) {
        NError(ret).ThrowErr(env);
        return nullptr;
    }
    auto callbackImpl = GetCallbackImpl(env, funcArg, MULTI_PROGRESS, false);
    auto cbExec = [downloadId{downloadId}, needClean{needClean}, callbackImpl{callbackImpl}]() -> NError {
        if (callbackImpl == nullptr) {
            LOGE("Failed to get download callback");
            return NError(JsErrCode::E_INNER_FAILED);
        }
        int32_t ret = callbackImpl->StopDownloadInner(downloadId, needClean);
        if (ret != E_OK) {
            ret = (ret == E_CLOUD_SDK) ? E_SERVICE_INNER_ERROR : ret;
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

    string procedureName = "cloudFileCache";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, maxArgSize);
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbCompl).val_;
}

static std::tuple<int32_t, std::string> ParseEventFromParam(napi_env env, NFuncArg &funcArg)
{
    auto [succProgress, progress, size] = NVal(env, funcArg[NARG_POS::FIRST]).ToUTF8String();
    if (!succProgress || size == 0) {
        LOGE("Parase event from parameter failed");
        NError(EINVAL).ThrowErr(env);
        return {EINVAL, ""};
    }
    string eventType(progress.get());
    if (eventType == "multiProgress") {
        eventType = MULTI_PROGRESS;
    }
    if ((eventType != PROGRESS && eventType != MULTI_PROGRESS)) {
        LOGE("Invalid event type!");
        return {EINVAL, ""};
    }
    return {E_OK, eventType};
}

napi_value CloudFileCacheNapi::On(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::TWO)) {
        LOGE("Batch-On Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [ret, eventType] = ParseEventFromParam(env, funcArg);
    if (ret != E_OK) {
        LOGE("On get event type failed!");
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    if (!NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) {
        LOGE("Batch-On argument type mismatch");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }

    auto callbackImpl = GetCallbackImpl(env, funcArg, eventType, true);
    if (callbackImpl == nullptr) {
        LOGE("Failed to init callback");
        NError(JsErrCode::E_INNER_FAILED).ThrowErr(env);
        return nullptr;
    }
    auto status = callbackImpl->RegisterCallback(funcArg[NARG_POS::SECOND]);
    if (status != napi_ok) {
        LOGE("Failed to register callback, status: %{public}d", status);
        NError(JsErrCode::E_INNER_FAILED).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

napi_value CloudFileCacheNapi::Off(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ONE, NARG_CNT::TWO)) {
        LOGE("Off Number of arguments unmatched");
        NError(EINVAL).ThrowErr(env);
        return nullptr;
    }
    auto [ret, eventType] = ParseEventFromParam(env, funcArg);
    if (ret != E_OK) {
        LOGE("Off get event type failed!");
        NError(ret).ThrowErr(env);
        return nullptr;
    }

    napi_value callbackVel = nullptr;
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        if (!NVal(env, funcArg[NARG_POS::SECOND]).TypeIs(napi_function)) {
            LOGE("Off argument type mismatch");
            NError(EINVAL).ThrowErr(env);
            return nullptr;
        }
        callbackVel = funcArg[NARG_POS::SECOND];
    }

    auto callbackImpl = GetCallbackImpl(env, funcArg, eventType, false);
    if (callbackImpl == nullptr || callbackImpl->UnregisterCallback(callbackVel) != napi_ok) {
        LOGE("Failed to unregister callback, for eventType: %{public}s.", eventType.c_str());
        NError(JsErrCode::E_INNER_FAILED).ThrowErr(env);
        return nullptr;
    }

    return NVal::CreateUndefined(env).val_;
}

string CloudFileCacheNapi::GetClassName()
{
    return className_;
}
} // namespace OHOS::FileManagement::CloudSync