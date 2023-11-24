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
        NVal::DeclareNapiFunction("stop", CloudFileCacheNapi::Stop),
        NVal::DeclareNapiFunction("cleanCache", CloudFileCacheNapi::CleanCloudFileCache),
    };

    SetClassName("CloudFileCache");
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
} // namespace OHOS::FileManagement::CloudSync