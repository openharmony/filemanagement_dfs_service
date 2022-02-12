/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef SENDFILE_NAPI_H
#define SENDFILE_NAPI_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
    enum DFS_ARG_CNT {
        ZERO = 0,
        ONE = 1,
        TWO = 2,
        THREE = 3,
        FOUR = 4,
        FIVE = 5
    };
    enum DFS_ARG_POS {
        FIRST = 0,
        SECOND = 1,
        THIRD = 2,
        FOURTH = 3,
        FIFTH = 4,
        SIXTH = 5
    };
    napi_value JS_Constructor(napi_env env, napi_callback_info cbinfo);
    napi_value SendFile(napi_env env, napi_callback_info info);
    napi_value JS_On(napi_env env, napi_callback_info cbinfo);
    napi_value JS_Off(napi_env env, napi_callback_info cbinfo);
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // SENDFILE_NAPI_H