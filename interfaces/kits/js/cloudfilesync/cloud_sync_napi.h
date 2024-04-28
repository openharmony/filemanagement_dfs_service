/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_NAPI_H
#define OHOS_FILEMGMT_CLOUD_SYNC_NAPI_H

#include "cloud_sync_callback.h"
#include "data_ability_observer_interface.h"
#include "data_ability_observer_stub.h"
#include "dataobs_mgr_client.h"
#include "filemgmt_libn.h"
#include "uv.h"

namespace OHOS::FileManagement::CloudSync {
const int32_t ARGS_ONE = 1;

struct BundleEntity {
    std::string bundleName_;
    explicit BundleEntity(const std::string bundleName): bundleName_(bundleName){};
};

struct CloudChangeListener {
    NotifyType notifyType;
    AAFwk::ChangeInfo changeInfo;
    std::string strUri;
};

struct RegisterParams {
    std::string uri;
    bool recursion;
    napi_ref cbOnRef;
};

class CloudSyncCallbackImpl;
class ChangeListenerNapi;
class CloudNotifyObserver;
class CloudSyncNapi : public LibN::NExporter {
public:
    void SetClassName(const std::string classname);
    std::string GetClassName() override;
    bool Export() override;
    bool ToExport(std::vector<napi_property_descriptor> props);
    static napi_value Constructor(napi_env env, napi_callback_info info);

    static napi_value Start(napi_env env, napi_callback_info info);
    static napi_value Stop(napi_env env, napi_callback_info info);
    static napi_value OnCallback(napi_env env, napi_callback_info info);
    static napi_value OffCallback(napi_env env, napi_callback_info info);
    static napi_value RegisterChange(napi_env env, napi_callback_info info);
    static napi_value UnregisterChange(napi_env env, napi_callback_info info);

    static std::string GetBundleName(const napi_env &env, const LibN::NFuncArg &funcArg);
    CloudSyncNapi(napi_env env, napi_value exports) : NExporter(env, exports) {};
    ~CloudSyncNapi() = default;

private:
    static bool CheckRef(napi_env env, napi_ref ref, ChangeListenerNapi &listObj, const std::string &uri);
    static int32_t RegisterToObs(napi_env env, const RegisterParams &registerParams);
    static napi_value UnregisterFromObs(napi_env env, const std::string &uri);
    static inline std::shared_ptr<CloudSyncCallbackImpl> callback_;
    std::string className_;
    static std::mutex sOnOffMutex_;
};

class CloudSyncCallbackImpl : public CloudSyncCallback, public std::enable_shared_from_this<CloudSyncCallbackImpl> {
public:
    CloudSyncCallbackImpl(napi_env env, napi_value fun);
    ~CloudSyncCallbackImpl() = default;
    void OnSyncStateChanged(SyncType type, SyncPromptState state) override;
    void OnSyncStateChanged(CloudSyncState state, ErrorType error) override;
    void DeleteReference();

    class UvChangeMsg {
    public:
        UvChangeMsg(std::shared_ptr<CloudSyncCallbackImpl> cloudSyncCallbackIn, CloudSyncState state, ErrorType error)
            : cloudSyncCallback_(cloudSyncCallbackIn), state_(state), error_(error)
        {
        }
        ~UvChangeMsg() {}
        std::weak_ptr<CloudSyncCallbackImpl> cloudSyncCallback_;
        CloudSyncState state_;
        ErrorType error_;
    };

private:
    static void OnComplete(UvChangeMsg *msg);
    napi_env env_;
    napi_ref cbOnRef_ = nullptr;
};

class ChangeListenerNapi {
public:
    class UvChangeMsg {
    public:
        UvChangeMsg(napi_env env, napi_ref ref, AAFwk::ChangeInfo &changeInfo, std::string strUri)
            : env_(env), ref_(ref), changeInfo_(changeInfo), strUri_(std::move(strUri))
        {
        }
        ~UvChangeMsg() {}
        napi_env env_;
        napi_ref ref_;
        AAFwk::ChangeInfo changeInfo_;
        uint8_t *data_;
        std::string strUri_;
    };

    explicit ChangeListenerNapi(napi_env env) : env_(env) {}

    ChangeListenerNapi(const ChangeListenerNapi &listener)
    {
        this->env_ = listener.env_;
        this->cbOnRef_ = listener.cbOnRef_;
        this->cbOffRef_ = listener.cbOffRef_;
    }

    ChangeListenerNapi &operator=(const ChangeListenerNapi &listener)
    {
        this->env_ = listener.env_;
        this->cbOnRef_ = listener.cbOnRef_;
        this->cbOffRef_ = listener.cbOffRef_;
        return *this;
    }

    ~ChangeListenerNapi(){};
    void OnChange(CloudChangeListener &listener, const napi_ref cbRef);
    int32_t UvQueueWork(uv_loop_s *loop, uv_work_t *work);
    static napi_value SolveOnChange(napi_env env, UvChangeMsg *msg);
    napi_ref cbOnRef_ = nullptr;
    napi_ref cbOffRef_ = nullptr;
    std::vector<std::shared_ptr<CloudNotifyObserver>> observers_;

private:
    napi_env env_ = nullptr;
};

class CloudNotifyObserver {
public:
    CloudNotifyObserver(const ChangeListenerNapi &listObj, std::string uri, napi_ref ref)
        : listObj_(listObj), uri_(uri), ref_(ref)
    {
    }
    ~CloudNotifyObserver() = default;

    void OnChange();
    void OnchangeExt(const AAFwk::ChangeInfo &changeInfo);

    ChangeListenerNapi listObj_;
    std::string uri_;
    napi_ref ref_;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_SYNC_NAPI_H
