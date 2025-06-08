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

#include "downgrade_progress_napi.h"

#include <cstddef>
#include <cstdint>
#include <sys/types.h>

#include "async_work.h"
#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "uv.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;
napi_value DowngradeProgressNapi::Constructor(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);

    auto progressEntity = make_unique<DowngradeProgressEntity>();
    if (progressEntity == nullptr) {
        LOGE("Failed to request heap memory.");
        return nullptr;
    }
    if (!NClass::SetEntityFor<DowngradeProgressEntity>(env, funcArg.GetThisVar(), move(progressEntity))) {
        LOGE("Failed to set progressEntity.");
        return nullptr;
    }
    return funcArg.GetThisVar();
}

napi_value DowngradeProgressNapi::GetState(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);

    int32_t state = static_cast<int32_t>(DowngradeProgress::State::STOPPED);
    auto progressEntity = NClass::GetEntityOf<DowngradeProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->progress == nullptr) {
        LOGE("Failed to get DowngradeProgressEntity.");
    } else {
        state = progressEntity->progress->state;
    }

    return NVal::CreateInt32(env, state).val_;
}

napi_value DowngradeProgressNapi::GetSuccessfulCount(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);

    int32_t succNum = -1;
    auto progressEntity = NClass::GetEntityOf<DowngradeProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->progress == nullptr) {
        LOGE("Failed to get DowngradeProgressEntity.");
    } else {
        succNum = progressEntity->progress->successfulCount;
    }

    return NVal::CreateInt32(env, succNum).val_;
}

napi_value DowngradeProgressNapi::GetFailedCount(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);

    int32_t failedNum = -1;
    auto progressEntity = NClass::GetEntityOf<DowngradeProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->progress == nullptr) {
        LOGE("Failed to get DowngradeProgressEntity.");
    } else {
        failedNum = progressEntity->progress->failedCount;
    }

    return NVal::CreateInt32(env, failedNum).val_;
}

napi_value DowngradeProgressNapi::GetTotalCount(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);

    int32_t totalNum = -1;
    auto progressEntity = NClass::GetEntityOf<DowngradeProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->progress == nullptr) {
        LOGE("Failed to get DowngradeProgressEntity.");
    } else {
        totalNum = progressEntity->progress->totalCount;
    }

    return NVal::CreateInt32(env, totalNum).val_;
}

napi_value DowngradeProgressNapi::GetDownloadedSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);

    int64_t downloadSize = INT64_MAX;
    auto progressEntity = NClass::GetEntityOf<DowngradeProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->progress == nullptr) {
        LOGE("Failed to get DowngradeProgressEntity.");
    } else {
        downloadSize = progressEntity->progress->downloadedSize;
    }

    return NVal::CreateInt64(env, downloadSize).val_;
}

napi_value DowngradeProgressNapi::GetTotalSize(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);

    int64_t totalSize = INT64_MAX;
    auto progressEntity = NClass::GetEntityOf<DowngradeProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->progress == nullptr) {
        LOGE("Failed to get DowngradeProgressEntity.");
    } else {
        totalSize = progressEntity->progress->totalSize;
    }

    return NVal::CreateInt64(env, totalSize).val_;
}

napi_value DowngradeProgressNapi::GetStopReason(napi_env env, napi_callback_info info)
{
    NFuncArg funcArg(env, info);
    funcArg.InitArgs(NARG_CNT::ZERO);

    int32_t stopReason = DowngradeProgress::StopReason::OTHER_REASON;
    auto progressEntity = NClass::GetEntityOf<DowngradeProgressEntity>(env, funcArg.GetThisVar());
    if (progressEntity == nullptr || progressEntity->progress == nullptr) {
        LOGE("Failed to get DowngradeProgressEntity.");
    } else {
        stopReason = progressEntity->progress->stopReason;
    }

    return NVal::CreateInt32(env, stopReason).val_;
}

std::string DowngradeProgressNapi::GetClassName()
{
    return className_;
}

bool DowngradeProgressNapi::Export()
{
    vector<napi_property_descriptor> props = {
        NVal::DeclareNapiGetter("state", GetState),
        NVal::DeclareNapiGetter("successfulCount", GetSuccessfulCount),
        NVal::DeclareNapiGetter("failedCount", GetFailedCount),
        NVal::DeclareNapiGetter("totalCount", GetTotalCount),
        NVal::DeclareNapiGetter("downloadedSize", GetDownloadedSize),
        NVal::DeclareNapiGetter("totalSize", GetTotalSize),
        NVal::DeclareNapiGetter("stopReason", GetStopReason),
    };

    string className = GetClassName();
    bool succ = false;
    napi_value classValue = nullptr;
    tie(succ, classValue) =
        NClass::DefineClass(exports_.env_, className, DowngradeProgressNapi::Constructor, move(props));
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