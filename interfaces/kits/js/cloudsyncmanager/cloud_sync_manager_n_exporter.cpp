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

#include "cloud_sync_manager_n_exporter.h"

#include <sys/types.h>

#include "cloud_sync_manager.h"
namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;

napi_value ChangeAppCloudSwitch(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<size_t>(NARG_CNT::THREE), static_cast<size_t>(NARG_CNT::FOUR))) {
        NError(E_PARAMS).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> accoutId;
    std::unique_ptr<char []> bundleName;
    bool status;

    tie(succ, accoutId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    tie(succ, bundleName, std::ignore) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    tie(succ, status) = NVal(env, funcArg[(int)NARG_POS::THIRD]).ToBool();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    string accoutIdStr(accoutId.get());
    string bundleNameStr(bundleName.get());

    auto cbExec = [accoutIdStr, bundleNameStr, status]() -> NError {
        int32_t result = CloudSyncManager::GetInstance().ChangeAppSwitch(accoutIdStr, bundleNameStr, status);
        if (result != 0) {
            return NError(result);
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    std::string procedureName = "ChangeAppCloudSwitch";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::THREE) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::FOURTH]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

napi_value NotifyDataChange(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<size_t>(NARG_CNT::TWO), static_cast<size_t>(NARG_CNT::THREE))) {
        NError(E_PARAMS).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> accoutId;
    std::unique_ptr<char []> bundleName;

    tie(succ, accoutId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    tie(succ, bundleName, std::ignore) = NVal(env, funcArg[(int)NARG_POS::SECOND]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    string accoutIdStr(accoutId.get());
    string bundleNameStr(bundleName.get());

    auto cbExec = [accoutIdStr, bundleNameStr]() -> NError {
        int32_t result = CloudSyncManager::GetInstance().NotifyDataChange(accoutIdStr, bundleNameStr);
        if (result != 0) {
            return NError(result);
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    std::string procedureName = "NotifyDataChange";
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() == (uint)NARG_CNT::TWO) {
        return NAsyncWorkPromise(env, thisVar).Schedule(procedureName, cbExec, cbComplete).val_;
    } else {
        NVal cb(env, funcArg[(int)NARG_POS::THIRD]);
        return NAsyncWorkCallback(env, thisVar, cb).Schedule(procedureName, cbExec, cbComplete).val_;
    }
}

} // namespace OHOS::FileManagement::CloudSync
