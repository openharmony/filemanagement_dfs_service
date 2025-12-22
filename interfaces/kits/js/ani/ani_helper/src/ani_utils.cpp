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

#include <ani_signature_builder.h>
#include "fs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace arkts::ani_signature;
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

std::tuple<bool, ani_string> ANIUtils::ToAniString(ani_env *env, const std::string &str)
{
    ani_string result;
    bool succ = (ANI_OK == env->String_NewUTF8(str.c_str(), str.size(), &result));
    return {succ, std::move(result)};
}

std::tuple<bool, int32_t> ANIUtils::EnumToInt32(ani_env *env, ani_enum_item enumItem)
{
    ani_int result;
    bool succ = (ANI_OK == env->EnumItem_GetValue_Int(enumItem, &result));
    return {succ, result};
}

std::tuple<bool, std::vector<std::string>> ANIUtils::AniToStringArray(ani_env *env, ani_array strArray)
{
    ani_size arraySize;
    ani_status ret;
    if ((ret = env->Array_GetLength(strArray, &arraySize)) != ANI_OK) {
        LOGE("Failed to get array size, %{public}d", ret);
        return {false, {}};
    }
    std::vector<std::string> res;
    for (ani_size i = 0; i < arraySize; ++i) {
        ani_ref strRef;
        if ((ret = env->Array_Get(strArray, i, &strRef)) != ANI_OK) {
            LOGE("Failed to get string item, %{public}d", ret);
            return {false, {}};
        }
        std::string strItem;
        if ((ret = ANIUtils::AniString2String(env, static_cast<ani_string>(strRef), strItem)) != ANI_OK) {
            LOGE("Failed to get string item, %{public}d", ret);
            return {false, {}};
        }
        res.emplace_back(strItem);
    }
    return {true, res};
}

std::tuple<bool, ani_array> ANIUtils::ToAniStringArray(ani_env *env, const std::vector<std::string> &strList)
{
    size_t length = strList.size();
    const std::string *strArray = strList.data();
    std::string classDesc = Builder::BuildClass("std.core.String").Descriptor();
    ani_class cls = nullptr;
    if (env->FindClass(classDesc.c_str(), &cls) != ANI_OK) {
        return {false, nullptr};
    }
    ani_array result = nullptr;
    if (env->Array_New(length, nullptr, &result) != ANI_OK) {
        return {false, nullptr};
    }
    for (size_t i = 0; i < length; ++i) {
        auto [succ, item] = ANIUtils::ToAniString(env, strArray[i]);
        if (!succ) {
            LOGE("Failed to conver to ani string");
            return {false, nullptr};
        }
        if (env->Array_Set(result, i, item) != ANI_OK) {
            LOGE("Failed to set element for array");
            return {false, nullptr};
        }
    }
    return {true, result};
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
} // namespace OHOS::FileManagement::CloudSync