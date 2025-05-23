/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ani_utils.h"

#include "fs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

ani_status ANIUtils::AniString2String(ani_env *env, ani_string str, std::string &res)
{
    ani_size strSize;
    ani_status ret = env->String_GetUTF8Size(str, &strSize);
    if (ret != ANI_OK) {
        LOGE("ani string get size failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }
    std::vector<char> buffer(strSize + 1);
    char *utf8Buffer = buffer.data();
    ani_size byteWrite = 0;
    ret = env->String_GetUTF8(str, utf8Buffer, strSize + 1, &byteWrite);
    if (ret != ANI_OK) {
        LOGE("ani string to string failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }
    utf8Buffer[byteWrite] = '\0';
    res = std::string(utf8Buffer);
    return ANI_OK;
}

bool ANIUtils::SendEventToMainThread(const std::function<void()> func)
{
    if (func == nullptr) {
        LOGE("func is nullptr!");
        return false;
    }

    if (!mainHandler) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (!runner) {
            LOGE("get main event runner failed!");
            return false;
        }
        mainHandler = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    mainHandler->PostTask(func, "", 0, OHOS::AppExecFwk::EventQueue::Priority::HIGH, {});
    return true;
}
}