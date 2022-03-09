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

#include "trans_event.h"

#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
napi_value TransEvent::ToJsObject(napi_env env)
{
    napi_value te;
    napi_status status = napi_create_object(env, &te);
    if (napi_ok == status) {
        napi_value err;
        status = napi_create_int32(env, errorCode_, &err);
        if (napi_ok == status) {
            status = napi_set_named_property(env, te, "errorCode", err);
            if (napi_ok != status) {
                LOGE("ToJsObject: package error code failed.\n");
                return nullptr;
            }
        } else {
            LOGE("ToJsObject: create error code failed.\n");
            return nullptr;
        }

        napi_value fname;
        status = napi_create_string_utf8(env, fileName_.c_str(), fileName_.size()+1, &fname);
        if (napi_ok == status) {
            status = napi_set_named_property(env, te, "fileName", fname);
            if (napi_ok != status) {
                LOGE("ToJsObject: package filename failed.\n");
                return nullptr;
            }
        } else {
            LOGE("ToJsObject: create filename failed.\n");
            return nullptr;
        }

        napi_value fcount;
        status = napi_create_int32(env, fileCount_, &fcount);
        if (napi_ok == status) {
            status = napi_set_named_property(env, te, "fileCount", fcount);
            if (napi_ok != status) {
                LOGE("ToJsObject: package file counts failed.\n");
                return nullptr;
            }
        } else {
            LOGE("ToJsObject: create file counts failed.\n");
            return nullptr;
        }
    } else {
        LOGE("ToJsObject: create object failed.\n");
        return nullptr;
    }

    return te;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS