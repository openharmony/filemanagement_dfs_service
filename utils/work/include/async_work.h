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

#ifndef OHOS_FILEMGMT_ASYNC_WORK_H
#define OHOS_FILEMGMT_ASYNC_WORK_H

#include "filemgmt_libn.h"
#include "n_func_arg.h"
#include "n_async_context.h"

namespace OHOS {
namespace FileManagement {
namespace LibN {
    std::unique_ptr<NAsyncWork> GetPromiseOrCallBackWork(napi_env env, const NFuncArg& funcArg, size_t maxArgSize);
} // namespace LibN
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_FILEMGMT_ASYNC_WORK_H