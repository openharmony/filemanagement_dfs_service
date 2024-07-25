/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "data_ability_observer_stub.h"

#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "common_utils.h"
#include "string_ex.h"

namespace OHOS {
namespace AAFwk {

const DataAbilityObserverStub::RequestFuncType DataAbilityObserverStub::HANDLES[TRANS_BUTT] = {
    &DataAbilityObserverStub::OnChangeInner,
    &DataAbilityObserverStub::OnChangeExtInner,
    &DataAbilityObserverStub::OnChangePreferencesInner,
};

DataAbilityObserverStub::DataAbilityObserverStub() {}

DataAbilityObserverStub::~DataAbilityObserverStub() {}

int DataAbilityObserverStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    TAG_LOGD(AAFwkTag::DBOBSMGR, "code: %{public}d, flags: %{public}d, callingPid:%{public}d", code, option.GetFlags(),
        IPCSkeleton::GetCallingPid());
    std::u16string descriptor = DataAbilityObserverStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::DBOBSMGR,
            "local descriptor is not equal to remote, descriptor: %{public}s, remoteDescriptor: %{public}s",
            CommonUtils::Anonymous(Str16ToStr8(descriptor)).c_str(),
            CommonUtils::Anonymous(Str16ToStr8(remoteDescriptor)).c_str());
        return ERR_INVALID_STATE;
    }

    if (code < TRANS_HEAD || code >= TRANS_BUTT || HANDLES[code] == nullptr) {
        TAG_LOGE(AAFwkTag::DBOBSMGR, "not support code:%u, BUTT:%d", code, TRANS_BUTT);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*HANDLES[code])(data, reply);
}

/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @return Returns 0 on success, others on failure.
 */
int32_t DataAbilityObserverStub::OnChangeInner(MessageParcel &data, MessageParcel &reply)
{
    OnChange();
    return ERR_NONE;
}

/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @return Returns 0 on success, others on failure.
 */
int32_t DataAbilityObserverStub::OnChangeExtInner(MessageParcel &data, MessageParcel &reply)
{
    ChangeInfo changeInfo;
    if (!ChangeInfo::Unmarshalling(changeInfo, data)) {
        return IPC_STUB_INVALID_DATA_ERR;
    }
    OnChangeExt(changeInfo);
    return ERR_NONE;
}

/**
 * @brief Called back to notify that the data being observed has changed.
 *
 * @return Returns 0 on success, others on failure.
 */
int32_t DataAbilityObserverStub::OnChangePreferencesInner(MessageParcel &data, MessageParcel &reply)
{
    std::string key = data.ReadString();
    if (key.empty()) {
        return IPC_STUB_INVALID_DATA_ERR;
    }
    OnChangePreferences(key);
    return ERR_NONE;
}

void DataObsCallbackRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    if (handler_) {
        handler_(remote);
    }
}

DataObsCallbackRecipient::DataObsCallbackRecipient(RemoteDiedHandler handler) : handler_(handler) {}

DataObsCallbackRecipient::~DataObsCallbackRecipient() {}
}  // namespace AAFwk
}  // namespace OHOS