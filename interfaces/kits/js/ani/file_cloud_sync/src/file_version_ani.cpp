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

#include "file_version_ani.h"

#include "ani_utils.h"
#include "dfs_error.h"
#include "error_handler.h"
#include "file_version_core.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {

using namespace arkts::ani_signature;

static const int32_t TOP_NUM = 10000;
static const int DEC = 10;

static FileVersionCore *FileVersionUnwrap(ani_env *env, ani_object object)
{
    ani_long nativePtr;
    auto ret = env->Object_GetFieldByName_Long(object, "nativePtr", &nativePtr);
    if (ret != ANI_OK) {
        LOGE("Unwrap FileVersionCore err: %{public}d", static_cast<int32_t>(ret));
        return nullptr;
    }
    std::uintptr_t ptrValue = static_cast<std::uintptr_t>(nativePtr);
    FileVersionCore *fileVersion = reinterpret_cast<FileVersionCore *>(ptrValue);
    return fileVersion;
}

static ani_status ConvertAniObj(ani_env *env, HistoryVersion &version,
    ani_class &cls, ani_method &ctor, ani_object &obj)
{
    std::string versionId = std::to_string(version.versionId);
    auto [succ, versionIdAni] = ANIUtils::ToAniString(env, versionId);
    if (!succ) {
        LOGE("versionId std string to ani string failed.");
        return ANI_ERROR;
    }
    ani_string oriFileName;
    std::tie(succ, oriFileName) = ANIUtils::ToAniString(env, version.originalFileName);
    if (!succ) {
        LOGE("oldFileName string to ani string failed.");
        return ANI_ERROR;
    }
    ani_string sha256;
    std::tie(succ, sha256) = ANIUtils::ToAniString(env, version.sha256);
    if (!succ) {
        LOGE("sha256 string to ani string failed.");
        return ANI_ERROR;
    }
    ani_status ret = env->Object_New(cls, ctor, &obj, static_cast<ani_double>(version.editedTime),
        static_cast<ani_double>(version.fileSize), versionIdAni, oriFileName, sha256, version.resolved);
    if (ret != ANI_OK) {
        LOGE("history obj new failed, ret = %{public}d", ret);
        return ret;
    }

    return ANI_OK;
}

static ani_status GetAniArrayObject(ani_env *env, const std::vector<HistoryVersion> &verList,
    ani_object &obj, ani_class &cls, ani_method &ctor)
{
    ani_class arrayCls = nullptr;
    Type arrSign = Builder::BuildClass("std.core.Array");
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

    ret = env->Object_New(arrayCls, arrayCtor, &obj, verList.size());
    if (ret != ANI_OK) {
        LOGE("set array uri new object failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    ani_size index = 0;
    for (auto version : verList) {
        ani_object pg;
        if (ConvertAniObj(env, version, cls, ctor, pg) != ANI_OK) {
            LOGE("convert ani obj fail.");
            continue;
        }
        std::string setSign = Builder::BuildSignatureDescriptor({Builder::BuildInt(), Builder::BuildNull()});
        ret = env->Object_CallMethodByName_Void(obj, "$_set", setSign.c_str(), index, pg);
        if (ret != ANI_OK) {
            LOGE("set array uri value failed. ret = %{public}d", static_cast<int32_t>(ret));
            return ret;
        }
        index++;
    }
    return ANI_OK;
}

static ani_status GetHistoryArray(ani_env *env, const std::vector<HistoryVersion> &verList, ani_object &obj)
{
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.HistoryVersionInner");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", ret);
        return ret;
    }
    ani_method ctor;
    std::string ct = Builder::BuildConstructorName();
    std::string argSign = Builder::BuildSignatureDescriptor({Builder::BuildLong(), Builder::BuildLong(),
        Builder::BuildClass("std.core.String"), Builder::BuildClass("std.core.String"),
        Builder::BuildClass("std.core.String"), Builder::BuildBoolean()});
    ret = env->Class_FindMethod(cls, ct.c_str(), argSign.c_str(), &ctor);
    if (ret != ANI_OK) {
        LOGE("find ctor method failed. ret = %{public}d", ret);
        return ret;
    }

    return GetAniArrayObject(env, verList, obj, cls, ctor);
}

static bool SafeStringToUInt(const string &str, uint64_t &value)
{
    char *endPtr = nullptr;
    errno = 0;

    value = strtoull(str.c_str(), &endPtr, DEC);
    if (str.c_str() == endPtr || *endPtr != '\0' || errno == ERANGE) {
        LOGE("string to uint64 failed.");
        return false;
    }

    return true;
}

void FileVersionAni::FileVersionConstructor(ani_env *env, ani_object object)
{
    Type clsName = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.FileVersion");
    ani_class cls;
    ani_status ret = env->FindClass(clsName.Descriptor().c_str(), &cls);
    if (ret != ANI_OK) {
        LOGE("find class failed. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    ani_method bindNativePtr;
    std::string bindSign = Builder::BuildSignatureDescriptor({Builder::BuildLong()});
    ret = env->Class_FindMethod(cls, "bindNativePtr", bindSign.c_str(), &bindNativePtr);
    if (ret != ANI_OK) {
        LOGE("find class ctor. ret = %{public}d", static_cast<int32_t>(ret));
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    FsResult<FileVersionCore *> data = FileVersionCore::Constructor();
    if (!data.IsSuccess()) {
        LOGE("FileVersion constructor failed.");
        const auto &err = data.GetError();
        ErrorHandler::Throw(env, err);
        return;
    }

    const FileVersionCore *fileVersion = data.GetData().value();
    ret = env->Object_CallMethod_Void(object, bindNativePtr, reinterpret_cast<ani_long>(fileVersion));
    if (ret != ANI_OK) {
        LOGE("bindNativePtr failed.");
        delete fileVersion;
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
    }
}

ani_ref FileVersionAni::FileVersionGetHistoryVersionList(ani_env *env, ani_object object,
    ani_string uri, ani_int numLimit)
{
    if (numLimit <= 0) {
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }
    std::string uriIn;
    ani_status ret = ANIUtils::AniString2String(env, uri, uriIn);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }

    if (uriIn == "") {
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto fileVersion = FileVersionUnwrap(env, object);
    if (fileVersion == nullptr) {
        LOGE("Cannot wrap fileVersion.");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }

    int32_t num = static_cast<int32_t>(numLimit);
    num = num >= TOP_NUM ? TOP_NUM : num;

    auto data = fileVersion->GetHistoryVersionList(uriIn, num);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("fileVersion GetHistoryVersionList failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return nullptr;
    }
    auto verList = data.GetData().value();
    ani_object obj = nullptr;
    if (GetHistoryArray(env, verList, obj) != ANI_OK) {
        LOGE("get history array obj failed");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }

    return obj;
}

static tuple<bool, std::string, uint64_t> GetDownloadVersionParam(ani_env *env, ani_string uri, ani_string versionId)
{
    std::string uriIn;
    ani_status ret = ANIUtils::AniString2String(env, uri, uriIn);
    if (ret != ANI_OK) {
        LOGE("unavailable uri.");
        return {false, "", 0};
    }

    std::string versionIdIn;
    ret = ANIUtils::AniString2String(env, versionId, versionIdIn);
    if (ret != ANI_OK) {
        LOGE("unavailable version id.");
        return {false, "", 0};
    }

    if (uriIn == "" || versionIdIn == "") {
        LOGE("unavailable version id or uri.");
        return {false, "", 0};
    }

    bool isDigit = std::all_of(versionIdIn.begin(), versionIdIn.end(), ::isdigit);
    if (!isDigit) {
        LOGE("unavailable version id.");
        return {false, "", 0};
    }
    uint64_t value = 0;
    if (!SafeStringToUInt(versionIdIn, value)) {
        LOGE("unavailable version id.");
        return {false, "", 0};
    }

    return make_tuple(true, move(uriIn), value);
}

ani_string FileVersionAni::FileVersionDownloadHistoryVersion(ani_env *env, ani_object object,
    ani_string uri, ani_string versionId, ani_object fun)
{
    auto [succ, uriIn, versionIdIn] = GetDownloadVersionParam(env, uri, versionId);
    if (!succ) {
        ErrorHandler::Throw(env, EINVAL);
        return nullptr;
    }

    auto fileVersion = FileVersionUnwrap(env, object);
    if (fileVersion == nullptr) {
        LOGE("Cannot wrap fileVersion.");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }
    ani_ref cbOnRef;
    ani_status ret = env->GlobalReference_Create(reinterpret_cast<ani_ref>(fun), &cbOnRef);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }
    auto callback = std::make_shared<VersionDownloadCallbackAniImpl>(env, cbOnRef);
    auto data = fileVersion->DownloadHistoryVersion(uriIn, versionIdIn, callback);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("fileVersion DownloadHistoryVersion failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return nullptr;
    }

    string res = data.GetData().value();
    ani_string versionIdAni;
    std::tie(succ, versionIdAni) = ANIUtils::ToAniString(env, res);
    if (!succ) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return nullptr;
    }

    return versionIdAni;
}

void FileVersionAni::FileVersionReplaceFileWithHistoryVersion(ani_env *env, ani_object object,
    ani_string originalUri, ani_string versionUri)
{
    std::string oriUri;
    ani_status ret = ANIUtils::AniString2String(env, originalUri, oriUri);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    std::string versionUriIn;
    ret = ANIUtils::AniString2String(env, versionUri, versionUriIn);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    if (oriUri == "" || versionUriIn == "") {
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto fileVersion = FileVersionUnwrap(env, object);
    if (fileVersion == nullptr) {
        LOGE("Cannot wrap fileVersion.");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    auto data = fileVersion->ReplaceFileWithHistoryVersion(oriUri, versionUriIn);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("fileVersion ReplaceFileWithHistoryVersion failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return;
    }
}

ani_boolean FileVersionAni::FileVersionIsFileConflict(ani_env *env, ani_object object, ani_string uri)
{
    std::string uriIn;
    ani_status ret = ANIUtils::AniString2String(env, uri, uriIn);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return false;
    }

    if (uriIn == "") {
        ErrorHandler::Throw(env, EINVAL);
        return false;
    }

    auto fileVersion = FileVersionUnwrap(env, object);
    if (fileVersion == nullptr) {
        LOGE("Cannot wrap fileVersion.");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return false;
    }

    auto data = fileVersion->IsConflict(uriIn);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("fileVersion IsConflict failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return false;
    }

    return data.GetData().value();
}

void FileVersionAni::FileVersionClearFileConflict(ani_env *env, ani_object object, ani_string uri)
{
    std::string uriIn;
    ani_status ret = ANIUtils::AniString2String(env, uri, uriIn);
    if (ret != ANI_OK) {
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    if (uriIn == "") {
        ErrorHandler::Throw(env, EINVAL);
        return;
    }

    auto fileVersion = FileVersionUnwrap(env, object);
    if (fileVersion == nullptr) {
        LOGE("Cannot wrap fileVersion.");
        ErrorHandler::Throw(env, JsErrCode::E_INNER_FAILED);
        return;
    }

    auto data = fileVersion->ClearFileConflict(uriIn);
    if (!data.IsSuccess()) {
        const auto &err = data.GetError();
        LOGE("fileVersion ClearFileConflict failed, ret = %{public}d", err.GetErrNo());
        ErrorHandler::Throw(env, err);
        return;
    }
}
} // namespace OHOS::FileManagement::CloudSync
