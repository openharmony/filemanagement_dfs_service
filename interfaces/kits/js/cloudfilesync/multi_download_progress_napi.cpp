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

#include "multi_download_progress_napi.h"

#include "cloud_sync_common.h"
#include "dfs_error.h"
#include "download_progress_napi.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;
napi_value MultiDlProgressNapi::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);
    auto progressEntity = make_unique<MultiDlProgressEntity>();
    if (progressEntity == nullptr) {
        LOGE("Failed to request heap memory.");
        return nullptr;
    }
    if (!NClass::SetEntityFor<MultiDlProgressEntity>(env, funcArg.GetThisVar(), move(progressEntity))) {
        LOGE("Failed to set progressEntity.");
        return nullptr;
    }
    return funcArg.GetThisVar();
}

napi_value MultiDlProgressNapi::GetStatus(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);
    int32_t state = static_cast<int32_t>(DownloadProgressObj::Status::FAILED);
    auto progressEntity = NClass::GetEntityOf<MultiDlProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->downloadProgress == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        state = progressEntity->downloadProgress->GetState();
    }

    return NVal::CreateInt32(env, state).val_;
}

napi_value MultiDlProgressNapi::GetTaskId(napi_env env, napi_callback_info info)
{
    int64_t taskId = 0;
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);
    auto progressEntity = NClass::GetEntityOf<MultiDlProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->downloadProgress == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        taskId = progressEntity->downloadProgress->GetTaskId();
    }

    return NVal::CreateInt64(env, taskId).val_;
}

napi_value MultiDlProgressNapi::GetDownloadedNum(napi_env env, napi_callback_info info)
{
    int64_t succNum = -1;
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);
    auto progressEntity = NClass::GetEntityOf<MultiDlProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->downloadProgress == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        succNum = static_cast<int64_t>(progressEntity->downloadProgress->GetSuccNum());
    }

    return NVal::CreateInt64(env, succNum).val_;
}

napi_value MultiDlProgressNapi::GetFailedNum(napi_env env, napi_callback_info info)
{
    int64_t failedNum = -1;
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);
    auto progressEntity = NClass::GetEntityOf<MultiDlProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->downloadProgress == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        failedNum = static_cast<int64_t>(progressEntity->downloadProgress->GetFailedNum());
    }

    return NVal::CreateInt64(env, failedNum).val_;
}

napi_value MultiDlProgressNapi::GetTotalNum(napi_env env, napi_callback_info info)
{
    int64_t totalNum = -1;
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);
    auto progressEntity = NClass::GetEntityOf<MultiDlProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->downloadProgress == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        totalNum = progressEntity->downloadProgress->GetTotalNum();
    }

    return NVal::CreateInt64(env, totalNum).val_;
}

napi_value MultiDlProgressNapi::GetDownloadedSize(napi_env env, napi_callback_info info)
{
    int64_t downloadSize = INT64_MAX;
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);
    auto progressEntity = NClass::GetEntityOf<MultiDlProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->downloadProgress == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        downloadSize = progressEntity->downloadProgress->GetDownloadedSize();
    }

    return NVal::CreateInt64(env, downloadSize).val_;
}

napi_value MultiDlProgressNapi::GetTotalSize(napi_env env, napi_callback_info info)
{
    int64_t totalSize = INT64_MAX;
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);
    auto progressEntity = NClass::GetEntityOf<MultiDlProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->downloadProgress == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        totalSize = progressEntity->downloadProgress->GetTotalSize();
    }

    return NVal::CreateInt64(env, totalSize).val_;
}

napi_value MultiDlProgressNapi::GetErrorType(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);
    int32_t errorType = static_cast<int32_t>(DownloadProgressObj::DownloadErrorType::UNKNOWN_ERROR);
    auto progressEntity = NClass::GetEntityOf<MultiDlProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->downloadProgress == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
    } else {
        errorType = progressEntity->downloadProgress->GetErrorType();
    }

    return NVal::CreateInt32(env, errorType).val_;
}

napi_value MultiDlProgressNapi::GetFailedFileList(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        LOGE("Failed to get param.");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
        return nullptr;
    }
    auto progressEntity = NClass::GetEntityOf<MultiDlProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->downloadProgress == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
        return nullptr;
    }
    napi_value res = nullptr;
    napi_status status = napi_create_array(env, &res);
    if (status != napi_ok) {
        HILOGE("Failed to creat array");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
        return nullptr;
    }

    size_t index = 0;
    auto failedFiles = progressEntity->downloadProgress->GetFailedFiles();
    for (const auto &iter : failedFiles) {
        NVal obj = NVal::CreateObject(env);
        obj.AddProp("uri", NVal::CreateUTF8String(env, iter.first).val_);
        obj.AddProp("error", NVal::CreateInt32(env, iter.second).val_);
        status = napi_set_element(env, res, index, obj.val_);
        if (status != napi_ok) {
            HILOGE("Failed to set element on data, %{public}s", GetAnonyString(iter.first).c_str());
            NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
            return nullptr;
        }
        index++;
    }

    return res;
}

napi_value MultiDlProgressNapi::GetDownloadedFileList(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    if (!funcArg.InitArgs(NARG_CNT::ZERO)) {
        LOGE("Failed to get param.");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
        return nullptr;
    }
    auto progressEntity = NClass::GetEntityOf<MultiDlProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->downloadProgress == nullptr) {
        LOGE("Failed to get MultiDlProgressEntity.");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
        return nullptr;
    }
    napi_value res = nullptr;
    napi_status status = napi_create_array(env, &res);
    if (status != napi_ok) {
        HILOGE("Failed to creat array");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
        return nullptr;
    }

    size_t index = 0;
    auto downloadedFiles = progressEntity->downloadProgress->GetDownloadedFiles();
    for (const auto &item : downloadedFiles) {
        status = napi_set_element(env, res, index, NVal::CreateUTF8String(env, item).val_);
        if (status != napi_ok) {
            HILOGE("Failed to set element on data, %{public}s", GetAnonyString(item).c_str());
            NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(env);
            return nullptr;
        }
        index++;
    }

    return res;
}

std::string MultiDlProgressNapi::GetClassName()
{
    return className_;
}

bool MultiDlProgressNapi::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiGetter("state", GetStatus),
        NVal::DeclareNapiGetter("taskId", GetTaskId),
        NVal::DeclareNapiGetter("successfulCount", GetDownloadedNum),
        NVal::DeclareNapiGetter("failedCount", GetFailedNum),
        NVal::DeclareNapiGetter("totalCount", GetTotalNum),
        NVal::DeclareNapiGetter("downloadedSize", GetDownloadedSize),
        NVal::DeclareNapiGetter("totalSize", GetTotalSize),
        NVal::DeclareNapiGetter("errType", GetErrorType),
        NVal::DeclareNapiFunction("getFailedFiles", GetFailedFileList),
        NVal::DeclareNapiFunction("getSuccessfulFiles", GetDownloadedFileList),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) =
        NClass::DefineClass(exports_.env_, className, MultiDlProgressNapi::Constructor, move(props));
    if (!succ) {
        LOGE("Define class exceptions");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(exports_.env_);
        return false;
    }
    succ = NClass::SaveClass(exports_.env_, className, classValue);
    if (!succ) {
        LOGE("Save class exceptions");
        NError(Convert2JsErrNum(E_SERVICE_INNER_ERROR)).ThrowErr(exports_.env_);
        return false;
    }

    return exports_.AddProp(className, classValue);
}

} // namespace OHOS::FileManagement::CloudSync