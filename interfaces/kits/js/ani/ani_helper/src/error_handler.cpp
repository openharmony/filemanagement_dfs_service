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

#include "error_handler.h"

namespace OHOS::FileManagement::CloudSync {

ani_status ErrorHandler::Throw(ani_env *env, int32_t code, const std::string &errMsg)
{
    const char *className = "L@ohos/base/BusinessError;";
    ani_class cls;
    ani_status ret = env->FindClass(className, &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }
    return Throw(env, cls, code, errMsg);
}
} // namespace OHOS::FileManagement::CloudSync