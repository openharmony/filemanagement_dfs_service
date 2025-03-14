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

#ifndef OHOS_FILEMGMT_ERROR_HANDLER_H
#define OHOS_FILEMGMT_ERROR_HANDLER_H

#include <ani.h>
#include <cstdint>
#include <string>

#include "fs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace ModuleFileIO;

class ErrorHandler {
public:
    static ani_status Throw(ani_env *env, int32_t code, const std::string &errMsg);

    static ani_status Throw(ani_env *env, int32_t code)
    {
        if (env == nullptr) {
            LOGE("Invalid parameter env");
            return ANI_INVALID_ARGS;
        }
        FsError err(code);
        return Throw(env, std::move(err));
    }

    static ani_status Throw(ani_env *env, const FsError &err)
    {
        if (env == nullptr) {
            LOGE("Invalid parameter env");
            return ANI_INVALID_ARGS;
        }
        auto code = err.GetErrNo();
        const auto &errMsg = err.GetErrMsg();
        return Throw(env, code, errMsg);
    }

private:
    static ani_status Throw(ani_env *env, const ani_class &cls, int32_t code, const std::string &errMsg)
    {
        ani_method ctor;
        if (ANI_OK != env->Class_FindMethod(cls, "<ctor>", nullptr, &ctor)) {
            LOGE("Cannot find constructor for class");
            return ANI_NOT_FOUND;
        }

        ani_string msg = nullptr;
        if (ANI_OK != env->String_NewUTF8(errMsg.c_str(), errMsg.size(), &msg)) {
            LOGE("Convert string to ani string failed.");
            return ANI_ERROR;
        }

        ani_object obj;
        if (ANI_OK != env->Object_New(cls, ctor, &obj, static_cast<double>(code), msg)) {
            LOGE("Cannot create ani error object");
            return ANI_ERROR;
        }
        auto status = env->ThrowError(static_cast<ani_error>(obj));
        if (status != ANI_OK) {
            LOGE("Throw ani error object failed!");
            return status;
        }
        return ANI_OK;
    }
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_ERROR_HANDLER_H