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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_PERMISSION_CHECK_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_PERMISSION_CHECK_H

#include <string>

#include "transport/trans_type.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
const int32_t INVALID_USER_ID = -1;
struct AccountInfo {
    int32_t userId_ = INVALID_USER_ID;
    uint64_t tokenId_ = 0;
    std::string accountId_;
    std::string networkId_;
};

class SoftBusPermissionCheck {
public:
    static bool CheckSrcPermission(const std::string &sinkNetworkId, int32_t userId = INVALID_USER_ID);
    static bool CheckSinkPermission(const AccountInfo &callerAccountInfo);
    static bool GetLocalAccountInfo(AccountInfo &localAccountInfo, int32_t userId = INVALID_USER_ID);
    static int32_t GetCurrentUserId();
    static bool TransCallerInfo(SocketAccessInfo *callerInfo,
        AccountInfo &callerAccountInfo, const std::string &networkId);
    static bool FillLocalInfo(SocketAccessInfo *localInfo);
    static bool SetAccessInfoToSocket(const int32_t sessionId, int32_t userId = INVALID_USER_ID);
    static bool IsSameAccount(const std::string &networkId);
private:
    static bool CheckSrcIsSameAccount(const std::string &sinkNetworkId, const AccountInfo &localAccountInfo);
    static bool CheckSinkIsSameAccount(const AccountInfo &callerAccountInfo, const AccountInfo &calleeAccountInfo);
    static bool CheckSrcAccessControl(const std::string &sinkNetworkId, const AccountInfo &localAccountInfo);
    static bool CheckSinkAccessControl(const AccountInfo &callerAccountInfo, const AccountInfo &calleeAccountInfo);
    static bool GetLocalNetworkId(std::string &networkId);
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_PERMISSION_CHECK_H