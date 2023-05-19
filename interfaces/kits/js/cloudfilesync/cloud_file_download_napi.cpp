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

#include "utils_log.h"

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

    auto cbExec = [env]() -> NError {
        LOGE("Start Download failed!");
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

    auto cbExec = [env]() -> NError {
        LOGE("Stop Download failed!");
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