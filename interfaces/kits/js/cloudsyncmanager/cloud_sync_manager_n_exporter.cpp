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

#include <map>

#include <sys/types.h>

#include "cloud_sync_common.h"
#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "async_work.h"

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
        if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
            return result == E_PERMISSION_DENIED? NError(Convert2JsErrNum(E_PERMISSION_DENIED)) :
            NError(Convert2JsErrNum(E_PERMISSION_SYSTEM));
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
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::FOUR));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbComplete).val_;
}

napi_value NotifyDataChangeInner(napi_env env, NFuncArg &funcArg)
{
    LOGI("NotifyDataChangeInner entrance");
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
        if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
            return result == E_PERMISSION_DENIED? NError(Convert2JsErrNum(E_PERMISSION_DENIED)) :
            NError(Convert2JsErrNum(E_PERMISSION_SYSTEM));
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
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::THREE));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbComplete).val_;
}

static bool IsNotifyEventChange(napi_env env, NFuncArg &funcArg)
{
    auto options = NVal(env, funcArg[NARG_POS::SECOND]);
    if (options.TypeIs(napi_object)) {
        return true;
    }
    return false;
}

napi_value NotifyEventChange(napi_env env, NFuncArg &funcArg)
{
    LOGI("NotifyEventChange entrance");
    bool succ = false;
    int32_t userId;
    tie(succ, userId) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToInt32();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    auto argv = NVal(env, funcArg[NARG_POS::SECOND]);
    if (!argv.HasProp("eventId") || !argv.HasProp("extraData")) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    std::unique_ptr<char []> eventId;
    std::unique_ptr<char []> extraData;

    tie(succ, eventId, std::ignore) = argv.GetProp("eventId").ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    tie(succ, extraData, std::ignore) = argv.GetProp("extraData").ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    
    string eventIdStr(eventId.get());
    string extraDataStr(extraData.get());
    
    auto cbExec = [userId, eventIdStr, extraDataStr]() -> NError {
        int32_t result = CloudSyncManager::GetInstance().NotifyEventChange(userId, eventIdStr, extraDataStr);
        if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
            return result == E_PERMISSION_DENIED? NError(Convert2JsErrNum(E_PERMISSION_DENIED)) :
            NError(Convert2JsErrNum(E_PERMISSION_SYSTEM));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    std::string procedureName = "NotifyEventChange";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::THREE));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbComplete).val_;
}

napi_value NotifyDataChange(napi_env env, napi_callback_info info)
{
    LOGI("NotifyDataChange entrance");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<size_t>(NARG_CNT::TWO), static_cast<size_t>(NARG_CNT::THREE))) {
        NError(E_PARAMS).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    if (IsNotifyEventChange(env, funcArg)) {
        return NotifyEventChange(env, funcArg);
    }
    return NotifyDataChangeInner(env, funcArg);
}

napi_value DisableCloud(napi_env env, napi_callback_info info)
{
    LOGI("DisableCloud");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<size_t>(NARG_CNT::ONE), static_cast<size_t>(NARG_CNT::TWO))) {
        NError(E_PARAMS).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> accoutId;

    tie(succ, accoutId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    string accoutIdStr(accoutId.get());

    auto cbExec = [accoutIdStr]() -> NError {
        int32_t result = CloudSyncManager::GetInstance().DisableCloud(accoutIdStr);
        if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
            return result == E_PERMISSION_DENIED? NError(Convert2JsErrNum(E_PERMISSION_DENIED)) :
            NError(Convert2JsErrNum(E_PERMISSION_SYSTEM));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    std::string procedureName = "DisableCloud";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::TWO));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbComplete).val_;
}


bool ParseSwitches(napi_env env, napi_value object, SwitchDataObj &data)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, object, &valueType);
    if (valueType != napi_object) {
        LOGE("ParseSwitches failed, not napi object");
        return false;
    }
    napi_value bundleNameArr = nullptr;
    napi_get_property_names(env, object, &bundleNameArr);
    uint32_t bundleNameNum = 0;
    napi_get_array_length(env, bundleNameArr, &bundleNameNum);
    for (uint32_t i = 0; i < bundleNameNum; ++i) {
        napi_value item = nullptr;
        napi_get_element(env, bundleNameArr, i, &item);
        std::unique_ptr<char []> bundleId;
        bool succ = false;
        tie(succ, bundleId, std::ignore) = NVal(env, item).ToUTF8String();
        if (!succ) {
            LOGE("fail to get string");
            return false;
        }

        napi_value val = nullptr;
        napi_get_named_property(env, object, bundleId.get(), &val);
        bool switchVal = false;
        tie(succ, switchVal) = NVal(env, val).ToBool();
        if (!succ) {
            LOGE("fail to get switchVal");
            return false;
        }
        data.switchData.emplace(bundleId.get(), switchVal);
    }
    return true;
}

napi_value EnableCloud(napi_env env, napi_callback_info info)
{
    LOGI("EnableCloud");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<size_t>(NARG_CNT::TWO), static_cast<size_t>(NARG_CNT::THREE))) {
        NError(E_PARAMS).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }

    bool succ = false;
    std::unique_ptr<char []> accoutId;

    tie(succ, accoutId, std::ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    SwitchDataObj switchData;
    if (!ParseSwitches(env, funcArg[(int)NARG_POS::SECOND], switchData)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    string accoutIdStr(accoutId.get());

    auto cbExec = [accoutIdStr, switchData]() -> NError {
        int32_t result = CloudSyncManager::GetInstance().EnableCloud(accoutIdStr, switchData);
        if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
            return result == E_PERMISSION_DENIED? NError(Convert2JsErrNum(E_PERMISSION_DENIED)) :
            NError(Convert2JsErrNum(E_PERMISSION_SYSTEM));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    std::string procedureName = "EnableCloud";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::THREE));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbComplete).val_;
}

bool ParseAppActions(napi_env env, napi_value object, CleanOptions &cleanOptions)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, object, &valueType);
    if (valueType != napi_object) {
        LOGE("ParseAppActions failed, not napi object");
        return false;
    }
    napi_value bundleNameArr = nullptr;
    napi_get_property_names(env, object, &bundleNameArr);
    uint32_t bundleNameNum = 0;
    napi_get_array_length(env, bundleNameArr, &bundleNameNum);
    for (uint32_t i = 0; i < bundleNameNum; ++i) {
        napi_value item = nullptr;
        napi_get_element(env, bundleNameArr, i, &item);
        std::unique_ptr<char[]> bundleNameStr = nullptr;
        bool resGetKey = false;
        tie(resGetKey, bundleNameStr, ignore) = NVal(env, item).ToUTF8String();
        if (!resGetKey) {
            LOGE("Fail to get string");
            return false;
        }
        napi_value actionObj = nullptr;
        napi_get_named_property(env, object, bundleNameStr.get(), &actionObj);
        int appActionsVal = 0;
        bool resGetValue = false;
        tie(resGetValue, appActionsVal) = NVal(env, actionObj).ToInt32();
        if (!resGetValue) {
            LOGE("Fail to get appActionsVal");
            return false;
        }
        cleanOptions.appActionsData.emplace(string(bundleNameStr.get()).c_str(), appActionsVal);
    }
    return true;
}
napi_value Clean(napi_env env, napi_callback_info info)
{
    LOGI("Clean");
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(static_cast<size_t>(NARG_CNT::TWO), static_cast<size_t>(NARG_CNT::THREE))) {
        NError(E_PARAMS).ThrowErr(env, "Number of arguments unmatched");
        return nullptr;
    }
    std::unique_ptr<char []> accoutId = nullptr;
    bool succ = false;
    tie(succ, accoutId, ignore) = NVal(env, funcArg[(int)NARG_POS::FIRST]).ToUTF8String();
    if (!succ) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }
    CleanOptions cleanOptions {};
    if (!ParseAppActions(env, funcArg[(int)NARG_POS::SECOND], cleanOptions)) {
        NError(E_PARAMS).ThrowErr(env);
        return nullptr;
    }

    std::string accoutIdStr(accoutId.get());
    auto cbExec = [accoutIdStr, cleanOptions]() -> NError {
        int32_t result = CloudSyncManager::GetInstance().Clean(accoutIdStr, cleanOptions);
        if (result == E_PERMISSION_DENIED || result == E_PERMISSION_SYSTEM) {
            return result == E_PERMISSION_DENIED? NError(Convert2JsErrNum(E_PERMISSION_DENIED)) :
            NError(Convert2JsErrNum(E_PERMISSION_SYSTEM));
        }
        return NError(ERRNO_NOERR);
    };
    auto cbComplete = [](napi_env env, NError err) -> NVal {
        if (err) {
            return { env, err.GetNapiErr(env) };
        }
        return { NVal::CreateUndefined(env) };
    };

    std::string procedureName = "Clean";
    auto asyncWork = GetPromiseOrCallBackWork(env, funcArg, static_cast<size_t>(NARG_CNT::THREE));
    return asyncWork == nullptr ? nullptr : asyncWork->Schedule(procedureName, cbExec, cbComplete).val_;
}

} // namespace OHOS::FileManagement::CloudSync
