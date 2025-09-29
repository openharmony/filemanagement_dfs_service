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
#include "cloud_disk_service_access_token.h"

#include "accesstoken_kit.h"
#include "cloud_disk_service_error.h"
#include "ipc_skeleton.h"
#include "os_account_manager.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;
using namespace Security::AccessToken;
constexpr int32_t ROOT_UID = 0;
constexpr int32_t BASE_USER_RANGE = 200000;
bool CloudDiskServiceAccessToken::CheckCallerPermission(const std::string &permissionName)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto uid = IPCSkeleton::GetCallingUid();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == TOKEN_HAP || tokenType == TOKEN_NATIVE) {
        bool isGranted = CheckPermission(tokenId, permissionName);
        if (!isGranted) {
            LOGE("Token Type is %{public}d", tokenType);
        }
        return isGranted;
    } else if ((tokenType == TOKEN_SHELL) && (uid == ROOT_UID)) {
        LOGI("Token type is shell");
        return false;
    } else {
        LOGE("Unsupported token type:%{public}d", tokenType);
        return false;
    }
}

bool CloudDiskServiceAccessToken::CheckPermission(uint32_t tokenId, const std::string &permissionName)
{
    int32_t ret = AccessTokenKit::VerifyAccessToken(tokenId, permissionName);
    if (ret == PermissionState::PERMISSION_DENIED) {
        LOGE("permission %{private}s: PERMISSION_DENIED", permissionName.c_str());
        return false;
    }
    return true;
}

int32_t CloudDiskServiceAccessToken::GetCallerBundleName(std::string &bundleName)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    return GetBundleNameByToken(tokenId, bundleName);
}

int32_t CloudDiskServiceAccessToken::GetBundleNameByToken(uint32_t tokenId, std::string &bundleName)
{
    int32_t tokenType = AccessTokenKit::GetTokenTypeFlag(tokenId);
    switch (tokenType) {
        case TOKEN_HAP: {
            HapTokenInfo hapInfo;
            if (AccessTokenKit::GetHapTokenInfo(tokenId, hapInfo) != 0) {
                LOGE("[Permission Check] get hap token info fail");
                return E_GET_TOKEN_INFO_ERROR;
            }
            if (hapInfo.instIndex != 0) {
                LOGE("[Permission Check] APP twin is not supported.");
                break;
            }
            bundleName = hapInfo.bundleName;
            break;
        }
        case TOKEN_NATIVE:
        // fall-through
        case TOKEN_SHELL: {
            NativeTokenInfo tokenInfo;
            if (AccessTokenKit::GetNativeTokenInfo(tokenId, tokenInfo) != 0) {
                LOGE("[Permission Check] get native token info fail");
                return E_GET_TOKEN_INFO_ERROR;
            }
            bundleName = tokenInfo.processName;
            break;
        }
        default: {
            LOGE("[Permission Check] token type not match");
            return E_GET_TOKEN_INFO_ERROR;
        }
    }
    if (bundleName.empty()) {
        LOGE("[Permission Check] package name is empty");
        return E_INVALID_ARG;
    }
    return E_OK;
}

int32_t CloudDiskServiceAccessToken::GetUserId()
{
    auto uid = IPCSkeleton::GetCallingUid();
    return uid / BASE_USER_RANGE;
}

int32_t CloudDiskServiceAccessToken::GetAccountId(int32_t &userId)
{
    vector<int32_t> activeUsers;
    if (AccountSA::OsAccountManager::QueryActiveOsAccountIds(activeUsers) != E_OK || activeUsers.empty()) {
        LOGE("query active user failed");
        return E_OSACCOUNT;
    }
    userId = activeUsers.front();
    if (!IsUserVerifyed(userId)) {
        LOGE("userId is invalid");
        return E_INVALID_ARG;
    }
    LOGI("GetAccountId ok, UserId: %{public}d", userId);
    return E_OK;
}

bool CloudDiskServiceAccessToken::IsUserVerifyed(const int32_t userId)
{
    bool isVerified = false;
    if (AccountSA::OsAccountManager::IsOsAccountVerified(userId, isVerified) != E_OK) {
        LOGE("check user verified failed");
        return false;
    }
    return isVerified;
}
} // OHOS::FileManagement::CloudDiskService