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

#include "cloud_sync_ani.h"
#include "error_handler.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

static CloudSyncCore *CloudSyncUnwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        LOGE("Unwrap cloudsyncCore err: %{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    std::uintptr_t ptrValue = static_cast<std::uintptr_t>(nativePtr);
    CloudSyncCore *cloudSync = reinterpret_cast<CloudSyncCore *>(ptrValue);
    return cloudSync;
}

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

void CloudSyncAni::CloudSyncConstructor(ani_env *env, ani_object object)
{
    ani_namespace ns {};
    ani_status ret = env->FindNamespace("L@ohos/file/cloudSync/cloudSync;", &ns);
    if (ret != ANI_OK) {
        LOGE("find namespace failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }
    static const char *className = "LGallerySync;";
    ani_class cls;
    ret = env->Namespace_FindClass(ns, className, &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    ani_method bindNativePtr;
    ret = env->Class_FindMethod(cls, "bindNativePtr", "J:V", &bindNativePtr);
    if (ret != ANI_OK) {
        LOGE("find class ctor. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    FsResult<CloudSyncCore *> data = CloudSyncCore::Constructor();
    if (!data.IsSuccess()) {
        LOGE("cloudsync constructor failed.");
        const auto &err = data.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }

    const CloudSyncCore *cloudSync = data.GetData().value();
    ret = env->Object_CallMethod_Void(object, bindNativePtr, reinterpret_cast<ani_long>(cloudSync));
    if (ret != ANI_OK) {
        LOGE("bindNativePtr failed.");
        delete cloudSync;
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
    }
}

void CloudSyncAni::CloudyncConstructor1(ani_env *env, ani_object object, ani_string bundleName)
{
    std::string bnm;
    ani_status ret = AniString2String(env, bundleName, bnm);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    ani_namespace ns {};
    ret = env->FindNamespace("L@ohos/file/cloudSync/cloudSync;", &ns);
    if (ret != ANI_OK) {
        LOGE("find namespace failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }
    static const char *className = "LGallerySync;";
    ani_class cls;
    ret = env->Namespace_FindClass(ns, className, &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    ani_method bindNativePtr;
    ret = env->Class_FindMethod(cls, "bindNativePtr", "J:V", &bindNativePtr);
    if (ret != ANI_OK) {
        LOGE("find class ctor. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    FsResult<CloudSyncCore *> data = CloudSyncCore::Constructor(bnm);
    if (!data.IsSuccess()) {
        LOGE("CloudSyncCore constructor failed.");
        const auto &err = data.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }

    const CloudSyncCore *cloudSync = data.GetData().value();
    ret = env->Object_CallMethod_Void(object, bindNativePtr, reinterpret_cast<ani_long>(cloudSync));
    if (ret != ANI_OK) {
        LOGE("bindNativePtr failed.");
        delete cloudSync;
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
    }
}

void CloudSyncAni::CloudSyncOn(ani_env *env, ani_object object, ani_string evt, ani_object fun)
{
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(env, fun);

    std::string event;
    ani_status ret = AniString2String(env, evt, event);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    auto cloudSync = CloudSyncUnwrap(env, object);
    if (cloudSync == nullptr) {
        LOGE("Cannot wrap cloudsync.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto data = cloudSync->DoOn(event, callback);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync do on failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudSyncAni::CloudSyncOff0(ani_env *env, ani_object object, ani_string evt, ani_object fun)
{
    auto callback = std::make_shared<CloudSyncCallbackAniImpl>(env, fun);

    std::string event;
    ani_status ret = AniString2String(env, evt, event);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    auto cloudSync = CloudSyncUnwrap(env, object);
    if (cloudSync == nullptr) {
        LOGE("Cannot wrap cloudsync.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto data = cloudSync->DoOff(event, callback);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync do off failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudSyncAni::CloudSyncOff1(ani_env *env, ani_object object, ani_string evt)
{
    std::string event;
    ani_status ret = AniString2String(env, evt, event);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return;
    }

    auto cloudSync = CloudSyncUnwrap(env, object);
    if (cloudSync == nullptr) {
        LOGE("Cannot wrap cloudsync.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto data = cloudSync->DoOff(event);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync do off failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudSyncAni::CloudSyncStart(ani_env *env, ani_object object)
{
    auto cloudSync = CloudSyncUnwrap(env, object);
    if (cloudSync == nullptr) {
        LOGE("Cannot wrap cloudsync.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto data = cloudSync->DoStart();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync do start failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

void CloudSyncAni::CloudSyncStop(ani_env *env, ani_object object)
{
    auto cloudSync = CloudSyncUnwrap(env, object);
    if (cloudSync == nullptr) {
        LOGE("Cannot wrap cloudsync.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return;
    }
    auto data = cloudSync->DoStop();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync do stop failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
    }
}

ani_int CloudSyncAni::GetFileSyncState(ani_env *env, ani_object object, ani_string path)
{
    string filePath;
    ani_status ret = AniString2String(env, path, filePath);
    if (ret != ANI_OK) {
        LOGE("ani string get size failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, static_cast<int32_t>(ret));
        return static_cast<int32_t>(ret);
    }
    auto data = CloudSyncCore::DoGetFileSyncState(filePath);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync do stop failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return err.GetErrNo();
    }
    return static_cast<ani_int>(data.GetData().value());
}

ani_double CloudSyncAni::CloudyncGetLastSyncTime(ani_env *env, ani_object object)
{
    auto cloudSync = CloudSyncUnwrap(env, object);
    if (cloudSync == nullptr) {
        LOGE("Cannot wrap cloudcynccore.");
        ErrorHandler::Throw(env, UNKNOWN_ERR);
        return 0;
    }

    auto data = cloudSync->CoreGetLastSyncTime();
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("cloud sync do GetLastSyncTime failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return 0;
    }

    const int64_t lastSyncTime = data.GetData().value();
    return static_cast<ani_double>(lastSyncTime);
}
}