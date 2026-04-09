/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef CLOUD_FILE_IF_SYSTEM_ABILITY_MANAGER_MOCK_H
#define CLOUD_FILE_IF_SYSTEM_ABILITY_MANAGER_MOCK_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "if_system_ability_manager.h"

namespace OHOS::FileManagement::CloudSync::Test {
class IRemoteObjectMock : public IRemoteObject {
public:
    MOCK_METHOD(int32_t, GetObjectRefCount, (), (override));
    MOCK_METHOD(int, SendRequest, (uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option),
        (override));
    MOCK_METHOD(bool, IsProxyObject, (), (const, override));
    MOCK_METHOD(bool, AddDeathRecipient, (const sptr<DeathRecipient> &recipient), (override));
    MOCK_METHOD(bool, RemoveDeathRecipient, (const sptr<DeathRecipient> &recipient), (override));
    MOCK_METHOD(int, Dump, (int fd, const std::vector<std::u16string> &args), (override));
    IRemoteObjectMock() : IRemoteObject(u"mock_IRemoteObject") {}
};

class IfSystemAbilityManagerMock : public ISystemAbilityManager {
public:
    IfSystemAbilityManagerMock() {}
    ~IfSystemAbilityManagerMock() {}
    MOCK_METHOD(sptr<IRemoteObject>, AsObject, (), (override));
    MOCK_METHOD(std::vector<std::u16string>, ListSystemAbilities, (unsigned int dumpFlags), (override));
    MOCK_METHOD(sptr<IRemoteObject>, GetSystemAbility, (int32_t systemAbilityId), (override));
    MOCK_METHOD(sptr<IRemoteObject>, CheckSystemAbility, (int32_t systemAbilityId), (override));
    MOCK_METHOD(int32_t, RemoveSystemAbility, (int32_t systemAbilityId), (override));
    MOCK_METHOD(int32_t, SubscribeSystemAbility,
        (int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange> &listener), (override));
    MOCK_METHOD(int32_t, UnSubscribeSystemAbility,
        (int32_t systemAbilityId, const sptr<ISystemAbilityStatusChange> &listener), (override));
    MOCK_METHOD(
        sptr<IRemoteObject>, GetSystemAbility, (int32_t systemAbilityId, const std::string &deviceId), (override));
    MOCK_METHOD(
        sptr<IRemoteObject>, CheckSystemAbility, (int32_t systemAbilityId, const std::string &deviceId), (override));
    MOCK_METHOD(int32_t, AddOnDemandSystemAbilityInfo,
        (int32_t systemAbilityId, const std::u16string &localAbilityManagerName), (override));
    MOCK_METHOD(sptr<IRemoteObject>, CheckSystemAbility, (int32_t systemAbilityId, bool &isExist), (override));
    MOCK_METHOD(int32_t, AddSystemAbility,
        (int32_t systemAbilityId, const sptr<IRemoteObject> &ability, const SAExtraProp &extraProp), (override));
    MOCK_METHOD(
        int32_t, AddSystemProcess, (const std::u16string &procName, const sptr<IRemoteObject> &procObject), (override));
    MOCK_METHOD(sptr<IRemoteObject>, LoadSystemAbility, (int32_t systemAbilityId, int32_t timeout), (override));
    MOCK_METHOD(int32_t, LoadSystemAbility, (int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback> &callback),
        (override));
    MOCK_METHOD(int32_t, LoadSystemAbility,
        (int32_t systemAbilityId, const std::string &deviceId, const sptr<ISystemAbilityLoadCallback> &callback),
        (override));
    MOCK_METHOD(int32_t, UnloadSystemAbility, (int32_t systemAbilityId), (override));
    MOCK_METHOD(int32_t, CancelUnloadSystemAbility, (int32_t systemAbilityId), (override));
    MOCK_METHOD(int32_t, UnloadAllIdleSystemAbility, (), (override));

    MOCK_METHOD(
        int32_t, GetSystemProcessInfo, (int32_t systemAbilityId, SystemProcessInfo &systemProcessInfo), (override));
    MOCK_METHOD(int32_t, GetRunningSystemProcess, (std::list<SystemProcessInfo> & systemProcessInfos), (override));
    MOCK_METHOD(int32_t, SubscribeSystemProcess, (const sptr<ISystemProcessStatusChange> &listener), (override));
    MOCK_METHOD(int32_t, SendStrategy,
        (int32_t type, std::vector<int32_t> &systemAbilityIds, int32_t level, std::string &action), (override));
    MOCK_METHOD(int32_t, UnSubscribeSystemProcess, (const sptr<ISystemProcessStatusChange> &listener), (override));
    MOCK_METHOD(int32_t, GetExtensionSaIds, (const std::string &extension, std::vector<int32_t> &saIds), (override));
    MOCK_METHOD(int32_t, GetExtensionRunningSaList,
        (const std::string &extension, std::vector<sptr<IRemoteObject>> &saList), (override));
    MOCK_METHOD(int32_t, GetRunningSaExtensionInfoList,
        (const std::string &extension, std::vector<SaExtensionInfo> &infoList), (override));
    MOCK_METHOD(int32_t, GetCommonEventExtraDataIdlist,
        (int32_t saId, std::vector<int64_t> &extraDataIdList, const std::string &eventName), (override));
    MOCK_METHOD(int32_t, GetOnDemandReasonExtraData, (int64_t extraDataId, MessageParcel &extraDataParcel), (override));
    MOCK_METHOD(int32_t, GetOnDemandPolicy,
        (int32_t systemAbilityId, OnDemandPolicyType type,
            std::vector<SystemAbilityOnDemandEvent> &abilityOnDemandEvents),
        (override));
    MOCK_METHOD(int32_t, UpdateOnDemandPolicy,
        (int32_t systemAbilityId, OnDemandPolicyType type,
            const std::vector<SystemAbilityOnDemandEvent> &abilityOnDemandEvents),
        (override));
    MOCK_METHOD(int32_t, GetOnDemandSystemAbilityIds, (std::vector<int32_t> & systemAbilityIds), (override));
};
} // OHOS::FileManagement::CloudSync::Test
#endif