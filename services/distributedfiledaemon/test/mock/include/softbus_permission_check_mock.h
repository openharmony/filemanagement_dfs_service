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
#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_PERMISSION_CHECK_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_PERMISSION_CHECK_MOCK_H

#include "network/softbus/softbus_permission_check.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class ISoftBusPermissionCheckMock {
public:
    ISoftBusPermissionCheckMock() = default;
    virtual ~ISoftBusPermissionCheckMock() = default;

    virtual bool CheckSrcPermission(const std::string &sinkNetworkId, int32_t userId = INVALID_USER_ID) = 0;
    virtual bool CheckSinkPermission(const AccountInfo &callerAccountInfo) = 0;
    virtual bool GetLocalAccountInfo(AccountInfo &localAccountInfo, int32_t userId = INVALID_USER_ID) = 0;
    virtual int32_t GetCurrentUserId() = 0;
    virtual bool
        TransCallerInfo(SocketAccessInfo *callerInfo, AccountInfo &callerAccountInfo, const std::string &networkId) = 0;
    virtual bool FillLocalInfo(SocketAccessInfo *localInfo) = 0;
    virtual bool SetAccessInfoToSocket(const int32_t sessionId, int32_t userId = INVALID_USER_ID) = 0;
    virtual bool IsSameAccount(const std::string &networkId) = 0;

public:
    static inline std::shared_ptr<ISoftBusPermissionCheckMock> iSoftBusPermissionCheckMock = nullptr;
};

class SoftBusPermissionCheckMock : public ISoftBusPermissionCheckMock {
public:
    MOCK_METHOD2(CheckSrcPermission, bool(const std::string &sinkNetworkId, int32_t userId));
    MOCK_METHOD1(CheckSinkPermission, bool(const AccountInfo &callerAccountInfo));
    MOCK_METHOD2(GetLocalAccountInfo, bool(AccountInfo &localAccountInfo, int32_t userId));
    MOCK_METHOD0(GetCurrentUserId, int32_t());
    MOCK_METHOD3(TransCallerInfo,
                 bool(SocketAccessInfo *callerInfo, AccountInfo &callerAccountInfo, const std::string &networkId));
    MOCK_METHOD1(FillLocalInfo, bool(SocketAccessInfo *localInfo));
    MOCK_METHOD2(SetAccessInfoToSocket, bool(const int32_t sessionId, int32_t userId));
    MOCK_METHOD1(IsSameAccount, bool(const std::string &networkId));
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_PERMISSION_CHECK_MOCK_H
