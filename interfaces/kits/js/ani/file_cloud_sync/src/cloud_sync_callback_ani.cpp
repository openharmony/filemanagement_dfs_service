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

#include "cloud_sync_callback_ani.h"

#include "ani_utils.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

constexpr int32_t ANI_SCOPE_SIZE = 16;
static const unsigned int READ_SIZE = 1024;
static mutex obsMutex_;

ConcurrentMap<CloudNotifyObserver *, ObserverImpl::ObserverParam> ObserverImpl::observers_;

void ObserverImpl::OnChange() {}

void ObserverImpl::OnChangeExt(const AAFwk::ChangeInfo &info)
{
    if (cloudNotifyObserver_ == nullptr) {
        LOGE("cloudNotifyObserver_ is null!");
        return;
    }
    cloudNotifyObserver_->OnChangeExt(info);
}

sptr<ObserverImpl> ObserverImpl::GetObserver(const Uri &uri, const shared_ptr<CloudNotifyObserver> &observer)
{
    lock_guard<mutex> lock(obsMutex_);
    sptr<ObserverImpl> result = nullptr;
    observers_.Compute(observer.get(), [&result, &uri, &observer](const auto &key, auto &value) {
        if (value.obs_ == nullptr) {
            value.obs_ = new (nothrow) ObserverImpl(observer);
            value.uris_.push_back(uri);
        } else {
            auto it = find(value.uris_.begin(), value.uris_.end(), uri);
            if (it == value.uris_.end()) {
                value.uris_.push_back(uri);
            }
        }

        result = value.obs_;
        return result != nullptr;
    });

    return result;
}

bool ObserverImpl::FindObserver(const Uri &uri, const shared_ptr<CloudNotifyObserver> &observer)
{
    lock_guard<mutex> lock(obsMutex_);
    auto result = observers_.Find(observer.get());
    if (result.first) {
        auto it = std::find(result.second.uris_.begin(), result.second.uris_.end(), uri);
        if (it == result.second.uris_.end()) {
            return false;
        }
    }
    return result.first;
}

bool ObserverImpl::DeleteObserver(const Uri &uri, const shared_ptr<CloudNotifyObserver> &observer)
{
    lock_guard<mutex> lock(obsMutex_);
    return observers_.ComputeIfPresent(observer.get(), [&uri](auto &key, auto &value) {
        value.uris_.remove_if([&uri](const auto &value) { return uri == value; });
        return !value.uris_.empty();
    });
}

CloudSyncCallbackAniImpl::CloudSyncCallbackAniImpl(ani_env *env, ani_ref fun)
{
    env_ = env;
    if (fun != nullptr) {
        cbOnRef_ = fun;
    }
}

void CloudSyncCallbackAniImpl::GetSyncProgress(
    CloudSyncState state, ErrorType error, const ani_class &cls, ani_object &pg)
{
    ani_method ctor;
    ani_status ret = env_->Class_FindMethod(
        cls, "<ctor>", "L@ohos/file/cloudSync/cloudSync/SyncState;L@ohos/file/cloudSync/cloudSync/ErrorType;:V", &ctor);
    if (ret != ANI_OK) {
        LOGE("find ctor method failed. ret = %{public}d", ret);
        return;
    }

    // Please replace it with GetEnumItemByValue_Int if suppored in the future.
    ani_enum stateEnum;
    env_->FindEnum("L@ohos/file/cloudSync/cloudSync/SyncState;", &stateEnum);
    ani_enum errorEnum;
    env_->FindEnum("L@ohos/file/cloudSync/cloudSync/ErrorType;", &errorEnum);

    ani_enum_item stateEnumItem;
    env_->Enum_GetEnumItemByIndex(stateEnum, state, &stateEnumItem);
    ani_enum_item errorEnumItem;
    env_->Enum_GetEnumItemByIndex(errorEnum, error, &errorEnumItem);

    ret = env_->Object_New(cls, ctor, &pg, stateEnumItem, errorEnumItem);
    if (ret != ANI_OK) {
        LOGE("create new object failed. ret = %{public}d", ret);
    }
}

void CloudSyncCallbackAniImpl::OnSyncStateChanged(CloudSyncState state, ErrorType error)
{
    auto task = [this, state, error]() {
        LOGI("OnSyncStateChanged state is %{public}d, error is %{public}d", state, error);
        ani_env *tmpEnv = env_;
        ani_size nr_refs = ANI_SCOPE_SIZE;
        ani_status ret = tmpEnv->CreateLocalScope(nr_refs);
        if (ret != ANI_OK) {
            LOGE("crete local scope failed. ret = %{public}d", ret);
            return;
        }
        ani_namespace ns {};
        ret = tmpEnv->FindNamespace("L@ohos/file/cloudSync/cloudSync;", &ns);
        if (ret != ANI_OK) {
            LOGE("find namespace failed. ret = %{public}d", ret);
            return;
        }
        static const char *className = "LSyncProgressInner;";
        ani_class cls;
        ret = tmpEnv->Namespace_FindClass(ns, className, &cls);
        if (ret != ANI_OK) {
            LOGE("find class failed. ret = %{public}d", ret);
            return;
        }
        ani_object pg;
        GetSyncProgress(state, error, cls, pg);
        ani_ref ref_;
        ani_fn_object etsCb = reinterpret_cast<ani_fn_object>(cbOnRef_);
        std::vector<ani_ref> vec = { pg };
        ret = tmpEnv->FunctionalObject_Call(etsCb, 1, vec.data(), &ref_);
        if (ret != ANI_OK) {
            LOGE("ani call function failed. ret = %{public}d", ret);
            return;
        }
        ret = tmpEnv->DestroyLocalScope();
        if (ret != ANI_OK) {
            LOGE("failed to DestroyLocalScope. ret = %{public}d", ret);
        }
    };
    if (!ANIUtils::SendEventToMainThread(task)) {
        LOGE("failed to send event");
    }
}

void CloudSyncCallbackAniImpl::DeleteReference()
{
    if (cbOnRef_ != nullptr) {
        env_->GlobalReference_Delete(cbOnRef_);
        cbOnRef_ = nullptr;
    }
}

void CloudSyncCallbackAniImpl::OnSyncStateChanged(SyncType type, SyncPromptState state)
{
    return;
}

ani_status ChangeListenerAni::SetValueArray(ani_env *env, const std::list<Uri> listValue, ani_object &uris)
{
    ani_class arrayCls = nullptr;
    ani_status ret = env->FindClass("Lescompat/Array;", &arrayCls);
    if (ret != ANI_OK) {
        LOGE("find ani array failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    ani_method arrayCtor;
    ret = env->Class_FindMethod(arrayCls, "<ctor>", "I:V", &arrayCtor);
    if (ret != ANI_OK) {
        LOGE("array find method failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    std::vector<std::string> strings(listValue.size());
    int elementIndex = 0;
    for (auto uri : listValue) {
        strings[elementIndex++] = uri.ToString();
    }

    ret = env->Object_New(arrayCls, arrayCtor, &uris, strings.size());
    if (ret != ANI_OK) {
        LOGE("set array uri new object failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    ani_size index = 0;
    for (auto str : strings) {
        ani_string ani_str;
        ret = env->String_NewUTF8(str.c_str(), str.size(), &ani_str);
        if (ret != ANI_OK) {
            LOGE("std string to ani string failed. ret = %{public}d", static_cast<int32_t>(ret));
            return ret;
        }
        ret = env->Object_CallMethodByName_Void(uris, "$_set", "ILstd/core/Object;:V", index, ani_str);
        if (ret != ANI_OK) {
            LOGE("set array uri value failed. ret = %{public}d", static_cast<int32_t>(ret));
            return ret;
        }
        index++;
    }
    return ANI_OK;
}

ani_status ChangeListenerAni::SetIsDir(ani_env *env, const shared_ptr<MessageParcel> parcel, ani_object &isDirectory)
{
    uint32_t len = 0;
    if (!parcel->ReadUint32(len)) {
        LOGE("Failed to read sub uri list length");
        return ANI_INVALID_ARGS;
    }

    if (len > READ_SIZE) {
        return ANI_INVALID_ARGS;
    }

    ani_class arrayCls = nullptr;
    ani_status ret = env->FindClass("Lescompat/Array;", &arrayCls);
    if (ret != ANI_OK) {
        LOGE("find ani array failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    ani_method arrayCtor;
    ret = env->Class_FindMethod(arrayCls, "<ctor>", "I:V", &arrayCtor);
    if (ret != ANI_OK) {
        LOGE("array find method failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    std::vector<bool> isDirs(len);
    for (uint32_t i = 0; i < len; i++) {
        isDirs[i] = parcel->ReadBool();
    }

    ret = env->Object_New(arrayCls, arrayCtor, &isDirectory, isDirs.size());
    if (ret != ANI_OK) {
        LOGE("set array uri new object failed. ret = %{public}d", static_cast<int32_t>(ret));
        return ret;
    }

    ani_size index = 0;
    for (auto isDir : isDirs) {
        ret = env->Object_CallMethodByName_Void(
            isDirectory, "$_set", "ILstd/core/Object;:V", index, static_cast<ani_boolean>(isDir));
        if (ret != ANI_OK) {
            LOGE("set array uri value failed. ret = %{public}d", static_cast<int32_t>(ret));
            return ret;
        }
        index++;
    }
    return ANI_OK;
}

ani_status ChangeListenerAni::GetChangeDataObject(
    ani_env *env, CloudChangeListener &listener, ani_class cls, ani_object &changeData)
{
    ani_object uris;
    ani_status ret = SetValueArray(env, listener.changeInfo.uris_, uris);
    if (ret != ANI_OK) {
        LOGE("set array uris failed. ret = %{public}d", ret);
        return ret;
    }

    ani_object isDirectory;
    if (listener.changeInfo.size_ > 0) {
        shared_ptr<MessageParcel> parcel = make_shared<MessageParcel>();
        if (parcel->ParseFrom(reinterpret_cast<uintptr_t>(listener.changeInfo.data_), listener.changeInfo.size_)) {
            ret = SetIsDir(env, parcel, isDirectory);
            if (ret != ANI_OK) {
                LOGE("Set subArray named property error! field: subUris");
                return ret;
            }
        }
    }

    ani_enum notifyTypeEnum;
    env->FindEnum("L@ohos/file/cloudSync/cloudSync/NotifyType;", &notifyTypeEnum);
    ani_enum_item notifyTypeEnumItem;
    env->Enum_GetEnumItemByIndex(notifyTypeEnum, listener.changeInfo.changeType_, &notifyTypeEnumItem);

    ani_method ctor;
    ret = env->Class_FindMethod(
        cls, "<ctor>", "L@ohos/file/cloudSync/cloudSync/NotifyType;Lescompat/Array;Lescompat/Array;:V", &ctor);
    if (ret != ANI_OK) {
        LOGE("find ctor method failed. ret = %{public}d", ret);
        return ret;
    }

    ret = env->Object_New(cls, ctor, &changeData, notifyTypeEnumItem, isDirectory, uris);
    if (ret != ANI_OK) {
        LOGE("create new object failed. ret = %{public}d", ret);
        return ret;
    }
    return ANI_OK;
}

void ChangeListenerAni::OnChange(CloudChangeListener &listener, const ani_ref cbRef)
{
    auto task = [this, listener, cbRef]() {
        LOGI("ChangeListenerAni OnChange");
        ani_env *tmpEnv = env_;
        ani_size nr_refs = ANI_SCOPE_SIZE;
        ani_status ret = tmpEnv->CreateLocalScope(nr_refs);
        if (ret != ANI_OK) {
            LOGE("crete local scope failed. ret = %{public}d", ret);
            return;
        }
        CloudChangeListener tmpListener = listener;
        if (tmpListener.changeInfo.uris_.empty()) {
            return;
        }
        ani_namespace ns {};
        ret = tmpEnv->FindNamespace("L@ohos/file/cloudSync/cloudSync;", &ns);
        if (ret != ANI_OK) {
            LOGE("find namespace failed. ret = %{public}d", ret);
            return;
        }
        static const char *className = "LChangeDataInner;";
        ani_class cls;
        ret = tmpEnv->Namespace_FindClass(ns, className, &cls);
        if (ret != ANI_OK) {
            LOGE("find class failed. ret = %{public}d", ret);
            return;
        }
        ani_object changeData;
        ret = GetChangeDataObject(tmpEnv, tmpListener, cls, changeData);
        ani_ref ref_;
        ani_fn_object etsCb = reinterpret_cast<ani_fn_object>(cbRef);
        std::vector<ani_ref> vec = { changeData };
        ret = tmpEnv->FunctionalObject_Call(etsCb, 1, vec.data(), &ref_);
        if (ret != ANI_OK) {
            LOGE("ani call function failed. ret = %{public}d", ret);
            return;
        }
        ret = tmpEnv->DestroyLocalScope();
        if (ret != ANI_OK) {
            LOGE("failed to DestroyLocalScope. ret = %{public}d", ret);
        }
    };
    if (!ANIUtils::SendEventToMainThread(task)) {
        LOGE("failed to send event");
    }
}

void CloudNotifyObserver::OnChange() {}

void CloudNotifyObserver::OnChangeExt(const AAFwk::ChangeInfo &changeInfo)
{
    CloudChangeListener listener;
    listener.changeInfo = changeInfo;
    listener.strUri = uri_;
    listObj_.OnChange(listener, ref_);
}
} // namespace OHOS::FileManagement::CloudSync