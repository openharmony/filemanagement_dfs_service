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

#include "cloud_download_ani.h"
#include "utils_log.h"

using namespace OHOS;
using namespace OHOS::FileManagement::CloudSync;

static CloudFileCore *CloudDownloadUnwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        LOGE("Unwrap cloudsyncCore err: %{public}d", ret);
        return nullptr;
    }
    std::uintptr_t ptrValue = static_cast<std::uintptr_t>(nativePtr);
    CloudFileCore *cloudDwonload = reinterpret_cast<CloudFileCore *>(ptrValue);
    return cloudDwonload;
}

static ani_status AniString2String(ani_env *env, ani_string str, std::string &res)
{
    ani_size strSize;
    ani_status ret = env->String_GetUTF8Size(str, &strSize);
    if (ret != ANI_OK) {
        LOGE("ani string get size failed. ret = %{public}d", ret);
        return ret;
    }
    std::vector<char> buffer(strSize + 1);
    char *utf8Buffer = buffer.data();
    ani_size byteWrite = 0;
    ret = env->String_GetUTF8(str, utf8Buffer, strSize + 1, &byteWrite);
    if (ret != ANI_OK) {
        LOGE("ani string to string failed. ret = %{public}d", ret);
        return ret;
    }
    utf8Buffer[byteWrite] = '\0';
    res = std::string(utf8Buffer);
    return ANI_OK;
}

void CloudDownloadAni::DownloadConstructor(ani_env *env, ani_object object)
{
    ani_namespace ns {};
    ani_status ret = env->FindNamespace("Lani_cloud_sync/cloudSync;", &ns);
    if (ret != ANI_OK) {
        LOGE("find namespace failed. ret = %{public}d", ret);
        return;
    }
    static const char *className = "LDownload;";
    ani_class cls;
    ret = env->Namespace_FindClass(ns, className, &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return;
    }

    ani_method bindNativePtr;
    ret = env->Class_FindMethod(cls, "bindNativePtr", "J:V", &bindNativePtr);
    if (ret != ANI_OK) {
        LOGE("find class ctor. ret = %{public}d", ret);
        return;
    }

    auto cloudFile = CloudFileCore::Constructor().GetData().value();
    if (cloudFile == nullptr) {
        LOGE("cloudsync constructor failed.");
    }

    ret = env->Object_CallMethod_Void(object, bindNativePtr, reinterpret_cast<ani_long>(cloudFile));
    if (ret != ANI_OK) {
        LOGE("bindNativePtr failed.");
        delete cloudFile;
    }
}

void CloudDownloadAni::DownloadOn(ani_env *env, ani_object object, ani_string evt, ani_object fun)
{
    auto callback = std::make_shared<CloudDownloadCallbackAniImpl>(env, fun);

    std::string event;
    ani_status ret = AniString2String(env, evt, event);
    if (ret != ANI_OK) {
        return;
    }

    auto cloudDownload = CloudDownloadUnwrap(env, object);
    if (cloudDownload == nullptr) {
        LOGE("Cannot wrap cloudDownload.");
        return;
    }
    cloudDownload->DoOn(event, callback);
}

void CloudDownloadAni::DownloadOff0(ani_env *env, ani_object object, ani_string evt, ani_object fun)
{
    auto callback = std::make_shared<CloudDownloadCallbackAniImpl>(env, fun);

    std::string event;
    ani_status ret = AniString2String(env, evt, event);
    if (ret != ANI_OK) {
        return;
    }

    auto cloudDownload = CloudDownloadUnwrap(env, object);
    if (cloudDownload == nullptr) {
        LOGE("Cannot wrap cloudDownload.");
        return;
    }
    cloudDownload->DoOff(event, callback);
}

void CloudDownloadAni::DownloadOff1(ani_env *env, ani_object object, ani_string evt)
{
    std::string event;
    ani_status ret = AniString2String(env, evt, event);
    if (ret != ANI_OK) {
        return;
    }

    auto cloudDownload = CloudDownloadUnwrap(env, object);
    if (cloudDownload == nullptr) {
        LOGE("Cannot wrap cloudDownload.");
        return;
    }
    cloudDownload->DoOff(event);
}

void CloudDownloadAni::DownloadStart(ani_env *env, ani_object object, ani_string uri)
{
    std::string uriInput;
    ani_status ret = AniString2String(env, uri, uriInput);
    if (ret != ANI_OK) {
        return;
    }

    auto cloudDownload = CloudDownloadUnwrap(env, object);
    if (cloudDownload == nullptr) {
        LOGE("Cannot wrap cloudDownload.");
        return;
    }
    cloudDownload->DoStart(uriInput);
}

void CloudDownloadAni::DownloadStop(ani_env *env, ani_object object, ani_string uri)
{
    std::string uriInput;
    ani_status ret = AniString2String(env, uri, uriInput);
    if (ret != ANI_OK) {
        return;
    }

    auto cloudDownload = CloudDownloadUnwrap(env, object);
    if (cloudDownload == nullptr) {
        LOGE("Cannot wrap cloudDownload.");
        return;
    }
    cloudDownload->DoStop(uriInput);
}