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

#include "cloud_sync_manager_ani.h"

#include "dfs_error.h"
#include "error_handler.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

static ani_status AniString2String(ani_env *env, ani_string str, std::string &res)
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

void CloudSyncManagerAni::ChangeAppCloudSwitch(
    ani_env *env, ani_class clazz, ani_string accoutId, ani_string bundleName, ani_boolean status)
{
    std::string accoutIdStr;
    ani_status ret = AniString2String(env, accoutId, accoutIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    std::string bundleNameStr;
    ret = AniString2String(env, bundleName, bundleNameStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    bool statusInput = static_cast<bool>(status);
    auto data = CloudSyncManagerCore::DoChangeAppCloudSwitch(accoutIdStr, bundleNameStr, statusInput);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync manager change app cloud switch failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudSyncManagerAni::NotifyDataChange(ani_env *env, ani_class clazz, ani_string accoutId, ani_string bundleName)
{
    std::string accoutIdStr;
    ani_status ret = AniString2String(env, accoutId, accoutIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    std::string bundleNameStr;
    ret = AniString2String(env, bundleName, bundleNameStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    auto data = CloudSyncManagerCore::DoNotifyDataChange(accoutIdStr, bundleNameStr);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync manager notify data change failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

static ani_status ParseSwitches(ani_env *env, ani_object map_object, SwitchDataObj &data)
{
    ani_ref keys;
    ani_status ret = env->Object_CallMethodByName_Ref(map_object, "keys", nullptr, &keys);
    if (ret != ANI_OK) {
        LOGE("Failed to get keys iterator, ret = %{public}d", ret);
        return ret;
    }
    bool success = true;
    while (success) {
        ani_ref next;
        ani_boolean done;
        ret = env->Object_CallMethodByName_Ref(static_cast<ani_object>(keys), "next", nullptr, &next);
        if (ret != ANI_OK) {
            LOGE("Failed to get next key, ret = %{public}d", ret);
            return ret;
        }
        ret = env->Object_GetFieldByName_Boolean(static_cast<ani_object>(next), "done", &done);
        if (ret != ANI_OK) {
            LOGE("Failed to check iterator done, ret = %{public}d", ret);
            return ret;
        }
        if (done) {
            success = false;
            break;
        }
        ani_ref key_value;
        ret = env->Object_GetFieldByName_Ref(static_cast<ani_object>(next), "value", &key_value);
        if (ret != ANI_OK) {
            LOGE("Failed to get key value, ret = %{public}d", ret);
            return ret;
        }
        ani_ref value_obj;
        ret = env->Object_CallMethodByName_Ref(map_object, "$_get", nullptr, &value_obj, key_value);
        if (ret != ANI_OK) {
            LOGE("Failed to get value for key, ret = %{public}d", ret);
            return ret;
        }
        std::string bundleId;
        ret = AniString2String(env, static_cast<ani_string>(key_value), bundleId);
        if (ret != ANI_OK) {
            ErrorHandler::Throw(env, static_cast<int32_t>(ret));
            return ret;
        }
        data.switchData.emplace(bundleId, static_cast<bool>(value_obj));
    }

    return ANI_OK;
}

void CloudSyncManagerAni::EnableCloud(ani_env *env, ani_class clazz, ani_string accoutId, ani_object record)
{
    std::string accoutIdStr;
    ani_status ret = AniString2String(env, accoutId, accoutIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    ani_class recordCls;
    const char *recordClassName = "Lescompat/Record;";
    ret = env->FindClass(recordClassName, &recordCls);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    SwitchDataObj switchData;
    ret = ParseSwitches(env, record, switchData);
    if (ret != ANI_OK) {
        LOGE("ParseSwitches failed. ret = %{public}d", ret);
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    auto data = CloudSyncManagerCore::DoEnableCloud(accoutIdStr, switchData);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync manager enable cloud failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudSyncManagerAni::DisableCloud(ani_env *env, ani_class clazz, ani_string accoutId)
{
    std::string accoutIdStr;
    ani_status ret = AniString2String(env, accoutId, accoutIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    auto data = CloudSyncManagerCore::DoDisableCloud(accoutIdStr);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync manager disaable cloud failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

static ani_status ParseAppActions(ani_env *env, ani_object map_object, CleanOptions &cleanOptions)
{
    ani_ref keys;
    ani_status ret = env->Object_CallMethodByName_Ref(map_object, "keys", nullptr, &keys);
    if (ret != ANI_OK) {
        LOGE("Failed to get keys iterator, ret = %{public}d", ret);
        return ret;
    }
    bool success = true;
    while (success) {
        ani_ref next;
        ani_boolean done;
        ret = env->Object_CallMethodByName_Ref(static_cast<ani_object>(keys), "next", nullptr, &next);
        if (ret != ANI_OK) {
            LOGE("Failed to get next key, ret = %{public}d", ret);
            return ret;
        }
        ret = env->Object_GetFieldByName_Boolean(static_cast<ani_object>(next), "done", &done);
        if (ret != ANI_OK) {
            LOGE("Failed to check iterator done, ret = %{public}d", ret);
            return ret;
        }
        if (done) {
            success = false;
            break;
        }
        ani_ref key_value;
        ret = env->Object_GetFieldByName_Ref(static_cast<ani_object>(next), "value", &key_value);
        if (ret != ANI_OK) {
            LOGE("Failed to get key value, ret = %{public}d", ret);
            return ret;
        }
        ani_ref value_obj;
        ret = env->Object_CallMethodByName_Ref(map_object, "$_get", nullptr, &value_obj, key_value);
        if (ret != ANI_OK) {
            LOGE("Failed to get value for key, ret = %{public}d", ret);
            return ret;
        }
        std::string bundleName;
        ret = AniString2String(env, static_cast<ani_string>(key_value), bundleName);
        if (ret != ANI_OK) {
            ErrorHandler::Throw(env, static_cast<int32_t>(ret));
            return ret;
        }
        ani_int enumValue;
        env->EnumItem_GetValue_Int(static_cast<ani_enum_item>(value_obj), &enumValue);
        cleanOptions.appActionsData.emplace(bundleName, static_cast<int32_t>(enumValue));
    }
    return ANI_OK;
}

void CloudSyncManagerAni::Clean(ani_env *env, ani_class clazz, ani_string accoutId, ani_object record)
{
    std::string accoutIdStr;
    ani_status ret = AniString2String(env, accoutId, accoutIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    ani_class recordCls;
    const char *recordClassName = "Lescompat/Record;";
    ret = env->FindClass(recordClassName, &recordCls);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    CleanOptions cleanOptions {};
    ret = ParseAppActions(env, record, cleanOptions);
    if (ret != ANI_OK) {
        LOGE("ParseAppActions failed. ret = %{public}d", ret);
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    auto data = CloudSyncManagerCore::DoClean(accoutIdStr, cleanOptions);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync manager do clean failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudSyncManagerAni::NotifyEventChange(ani_env *env, ani_class clazz, ani_double userId, ani_object extraData)
{
    int32_t userIdInput = static_cast<int32_t>(userId);
    ani_ref eventId;
    ani_ref extraDataRef;
    ani_status ret = env->Object_GetPropertyByName_Ref(extraData, "eventId", &eventId);
    if (ret != ANI_OK) {
        LOGE("get event id failed. ret = %{public}d", ret);
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
    }
    ret = env->Object_GetPropertyByName_Ref(extraData, "extraData", &extraDataRef);
    if (ret != ANI_OK) {
        LOGE("get extraData failed. ret = %{public}d", ret);
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
    }

    std::string eventIdStr;
    ret = AniString2String(env, static_cast<ani_string>(eventId), eventIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
    }

    std::string extraDataStr;
    ret = AniString2String(env, static_cast<ani_string>(extraDataRef), extraDataStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
    }

    auto data = CloudSyncManagerCore::DoNotifyEventChange(userIdInput, eventIdStr, extraDataStr);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync manager notify Event change failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}
} // namespace OHOS::FileManagement::CloudSync