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

#include "softbus_permission_check_mock.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

bool SoftBusPermissionCheck::CheckSrcPermission(const std::string &sinkNetworkId, int32_t userId)
{
    if (ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock == nullptr) {
        return false;
    }
    return ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock->CheckSrcPermission(sinkNetworkId, userId);
}

bool SoftBusPermissionCheck::CheckSinkPermission(const AccountInfo &callerAccountInfo)
{
    if (ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock == nullptr) {
        return false;
    }
    return ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock->CheckSinkPermission(callerAccountInfo);
}

bool SoftBusPermissionCheck::GetLocalAccountInfo(AccountInfo &localAccountInfo, int32_t userId)
{
    if (ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock == nullptr) {
        return false;
    }
    return ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock->GetLocalAccountInfo(localAccountInfo, userId);
}

int32_t SoftBusPermissionCheck::GetCurrentUserId()
{
    if (ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock == nullptr) {
        return -1; // -1: default userId
    }
    return ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock->GetCurrentUserId();
}

bool SoftBusPermissionCheck::TransCallerInfo(SocketAccessInfo *callerInfo,
                                             AccountInfo &callerAccountInfo,
                                             const std::string &networkId)
{
    if (ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock == nullptr) {
        return false;
    }
    return ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock->TransCallerInfo(callerInfo, callerAccountInfo,
                                                                                     networkId);
}

bool SoftBusPermissionCheck::FillLocalInfo(SocketAccessInfo *localInfo)
{
    if (ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock == nullptr) {
        return false;
    }
    return ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock->FillLocalInfo(localInfo);
}

bool SoftBusPermissionCheck::SetAccessInfoToSocket(const int32_t sessionId, int32_t userId)
{
    if (ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock == nullptr) {
        return false;
    }
    return ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock->SetAccessInfoToSocket(sessionId, userId);
}

bool SoftBusPermissionCheck::IsSameAccount(const std::string &networkId)
{
    if (ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock == nullptr) {
        return false;
    }
    return ISoftBusPermissionCheckMock::iSoftBusPermissionCheckMock->IsSameAccount(networkId);
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
