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

#include "ani_utils.h"
#include "dfs_error.h"
#include "error_handler.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

using namespace arkts::ani_signature;

const int32_t E_IPCSS = 13600001;
const int32_t E_PERMISSION = 201;

void CloudSyncManagerAni::ChangeAppCloudSwitch(
    ani_env *env, ani_class clazz, ani_string accoutId, ani_string bundleName, ani_boolean status)
{
    std::string accoutIdStr;
    ani_status ret = ANIUtils::AniString2String(env, accoutId, accoutIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_PERMISSION);
        return;
    }

    std::string bundleNameStr;
    ret = ANIUtils::AniString2String(env, bundleName, bundleNameStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_PERMISSION);
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
    ani_status ret = ANIUtils::AniString2String(env, accoutId, accoutIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_PERMISSION);
        return;
    }

    std::string bundleNameStr;
    ret = ANIUtils::AniString2String(env, bundleName, bundleNameStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_PERMISSION);
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
        ret = ANIUtils::AniString2String(env, static_cast<ani_string>(key_value), bundleId);
        if (ret != ANI_OK) {
            return ret;
        }
        data.switchData.emplace(bundleId, static_cast<bool>(value_obj));
    }

    return ANI_OK;
}

void CloudSyncManagerAni::EnableCloud(ani_env *env, ani_class clazz, ani_string accoutId, ani_object record)
{
    std::string accoutIdStr;
    ani_status ret = ANIUtils::AniString2String(env, accoutId, accoutIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_PERMISSION);
        return;
    }

    ani_class recordCls;
    Type clsName = Builder::BuildClass("std.core.Record");
    ret = env->FindClass(clsName.Descriptor().c_str(), &recordCls);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_PERMISSION);
        return;
    }

    SwitchDataObj switchData;
    ret = ParseSwitches(env, record, switchData);
    if (ret != ANI_OK) {
        LOGE("ParseSwitches failed. ret = %{public}d", ret);
        ErrorHandler::Throw(env, E_PARAMS);
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
    ani_status ret = ANIUtils::AniString2String(env, accoutId, accoutIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_PERMISSION);
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
        ret = ANIUtils::AniString2String(env, static_cast<ani_string>(key_value), bundleName);
        if (ret != ANI_OK) {
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
    ani_status ret = ANIUtils::AniString2String(env, accoutId, accoutIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_PERMISSION);
        return;
    }

    ani_class recordCls;
    Type clsName = Builder::BuildClass("std.core.Record");
    ret = env->FindClass(clsName.Descriptor().c_str(), &recordCls);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_PERMISSION);
        return;
    }

    CleanOptions cleanOptions {};
    ret = ParseAppActions(env, record, cleanOptions);
    if (ret != ANI_OK) {
        LOGE("ParseAppActions failed. ret = %{public}d", ret);
        ErrorHandler::Throw(env, E_PARAMS);
        return;
    }

    auto data = CloudSyncManagerCore::DoClean(accoutIdStr, cleanOptions);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync manager do clean failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudSyncManagerAni::NotifyEventChange(ani_env *env, ani_class clazz, ani_int userId, ani_object extraData)
{
    ani_ref eventId;
    ani_ref extraDataRef;
    ani_status ret = env->Object_GetPropertyByName_Ref(extraData, "eventId", &eventId);
    if (ret != ANI_OK) {
        LOGE("get event id failed. ret = %{public}d", ret);
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }
    ret = env->Object_GetPropertyByName_Ref(extraData, "extraData", &extraDataRef);
    if (ret != ANI_OK) {
        LOGE("get extraData failed. ret = %{public}d", ret);
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }

    std::string eventIdStr;
    ret = ANIUtils::AniString2String(env, static_cast<ani_string>(eventId), eventIdStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }

    std::string extraDataStr;
    ret = ANIUtils::AniString2String(env, static_cast<ani_string>(extraDataRef), extraDataStr);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, E_IPCSS);
        return;
    }

    auto data = CloudSyncManagerCore::DoNotifyEventChange(userId, eventIdStr, extraDataStr);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync manager notify Event change failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

static ani_status ConvertAniObj(ani_env *env, LocalFilePresentStatus &status,
    ani_class &cls, ani_method &ctor, ani_object &obj)
{
    auto [succ, bundleNameAni] = ANIUtils::ToAniString(env, status.bundleName);
    if (!succ) {
        LOGE("bundleName std string to ani string failed.");
        return ANI_ERROR;
    }
    ani_boolean isLocalFilePresentsAni = status.isLocalFilePresents;
    ani_status ret = env->Object_New(cls, ctor, &obj, bundleNameAni, isLocalFilePresentsAni);
    if (ret != ANI_OK) {
        LOGE("LocalFilePresentStatus obj new failed, ret = %{public}d", ret);
        return ret;
    }

    return ANI_OK;
}

static ani_status GetAniArrayObject(ani_env *env, const std::vector<LocalFilePresentStatus> &localFilePresentStatusList,
    ani_object &obj, ani_class &cls, ani_method &ctor)
{
    ani_class arrayCls = nullptr;
    Type arrSign = Builder::BuildClass("escompat.Array");
    ani_status ret = env->FindClass(arrSign.Descriptor().c_str(), &arrayCls);
    if (ret != ANI_OK) {
        LOGE("find ani array failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    ani_method arrayCtor;
    std::string ct = Builder::BuildConstructorName();
    std::string ctSign = Builder::BuildSignatureDescriptor({Builder::BuildInt()});
    ret = env->Class_FindMethod(arrayCls, ct.c_str(), ctSign.c_str(), &arrayCtor);
    if (ret != ANI_OK) {
        LOGE("array find method failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    ret = env->Object_New(arrayCls, arrayCtor, &obj, localFilePresentStatusList.size());
    if (ret != ANI_OK) {
        LOGE("set array localFilePresentStatus new object failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    ani_size index = 0;
    for (auto item : localFilePresentStatusList) {
        ani_object pg;
        if (ConvertAniObj(env, item, cls, ctor, pg) != ANI_OK) {
            LOGE("convert ani obj fail.");
            continue;
        }
        std::string setSign = Builder::BuildSignatureDescriptor({Builder::BuildInt(), Builder::BuildNull()});
        ret = env->Object_CallMethodByName_Void(obj, "$_set", setSign.c_str(), index, pg);
        if (ret != ANI_OK) {
            LOGE("set array localFilePresentStatus value failed. ret = %{public}d", static_cast<int32_t>(ret));
            return ret;
        }
        index++;
    }
    return ANI_OK;
}

static ani_status CreateLocalFilePresentStatusArray(ani_env *env,
    const std::vector<LocalFilePresentStatus> &localFilePresentStatusList, ani_object &obj)
{
    Type clsName = Builder::BuildClass("@ohos.file.cloudSyncManager.cloudSyncManager.LocalFilePresentStatus");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }
    ani_method ctor;
    std::string ct = Builder::BuildConstructorName();
    std::string argSign = Builder::BuildSignatureDescriptor({Builder::BuildClass("std.core.String"),
        Builder::BuildBoolean()});
    ret = env->Class_FindMethod(cls, ct.c_str(), argSign.c_str(), &ctor);
    if (ret != ANI_OK) {
        LOGE("find ctor method failed. ret = %{public}d", ret);
        return ret;
    }

    return GetAniArrayObject(env, localFilePresentStatusList, obj, cls, ctor);
}

ani_ref CloudSyncManagerAni::GetBundlesLocalFilePresentStatus(ani_env *env, ani_class clazz, ani_array bundleNames)
{
    auto [ret, bundleNameInputArray] = ANIUtils::AniToStringArray(env, bundleNames);
    if (!ret) {
        ErrorHandler::Throw(env, JsErrCode::E_IPCSS);
        return nullptr;
    }

    auto data = CloudSyncManagerCore::DoGetLocalFilePresentStatus(bundleNameInputArray);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync manager get local file present status failed, err is %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    
    auto localFilePresentStatusList = data.GetData().value();
    ani_object result = nullptr;
    if (CreateLocalFilePresentStatusArray(env, localFilePresentStatusList, result) != ANI_OK) {
        LOGE("create LocalFilePresentStatus array failed");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }

    return result;
}
} // namespace OHOS::FileManagement::CloudSync