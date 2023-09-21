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

#include "async_work.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {

std::unique_ptr<NAsyncWork> GetPromiseOrCallBackWork(napi_env env, const NFuncArg &funcArg, size_t maxArgSize)
{
    std::unique_ptr<NAsyncWork> asyncWork = nullptr;
    NVal thisVar(env, funcArg.GetThisVar());
    if (funcArg.GetArgc() != maxArgSize) {
        asyncWork = std::make_unique<NAsyncWorkPromise>(env, thisVar);
    } else {
        NVal cb(env, funcArg[(int)maxArgSize - 1]);
        if (cb.TypeIs(napi_function)) {
            asyncWork = std::make_unique<NAsyncWorkCallback>(env, thisVar, cb);
        } else {
            LOGE("Argument type mismatch");
            NError(E_PARAMS).ThrowErr(env);
        }
    }
    return asyncWork;
}
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS