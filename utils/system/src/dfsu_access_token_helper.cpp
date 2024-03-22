/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dfsu_access_token_helper.h"
#include "accesstoken_kit.h"
#include "dfs_error.h"
#include "ipc_skeleton.h"
#include "tokenid_kit.h"
#include "uri.h"
#include "uri_permission_manager_client.h"
#include "utils_log.h"
#include "want.h"

namespace OHOS::FileManagement {
using namespace std;
using namespace Security::AccessToken;
constexpr int32_t ROOT_UID = 0;
constexpr int32_t BASE_USER_RANGE = 200000;
bool DfsuAccessTokenHelper::CheckCallerPermission(const std::string &permissionName)
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

bool DfsuAccessTokenHelper::CheckPermission(uint32_t tokenId, const std::string &permissionName)
{
    int32_t ret = AccessTokenKit::VerifyAccessToken(tokenId, permissionName);
    if (ret == PermissionState::PERMISSION_DENIED) {
        LOGE("permission %{private}s: PERMISSION_DENIED", permissionName.c_str());
        return false;
    }
    return true;
}

int32_t DfsuAccessTokenHelper::GetCallerBundleName(std::string &bundleName)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    return GetBundleNameByToken(tokenId, bundleName);
}

int32_t DfsuAccessTokenHelper::GetBundleNameByToken(uint32_t tokenId, std::string &bundleName)
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
        return E_INVAL_ARG;
    }
    return E_OK;
}
bool DfsuAccessTokenHelper::IsSystemApp()
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if (tokenType == TOKEN_HAP) {
        uint64_t fullTokenId = IPCSkeleton::GetCallingFullTokenID();
        return TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
    }
    return true;
}

int32_t DfsuAccessTokenHelper::GetUserId()
{
    auto uid = IPCSkeleton::GetCallingUid();
    return uid / BASE_USER_RANGE;
}

bool DfsuAccessTokenHelper::CheckUriPermission(const std::string &uriStr)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    string bundleName;
    if (GetBundleNameByToken(tokenId, bundleName) != E_OK) {
        LOGE("get caller bundle name failed");
        return false;
    }
    Uri uri(uriStr);
    auto &uriPermissionClient = AAFwk::UriPermissionManagerClient::GetInstance();
    if (bundleName != uri.GetAuthority() &&
        !uriPermissionClient.VerifyUriPermission(uri, AAFwk::Want::FLAG_AUTH_READ_URI_PERMISSION, tokenId)) {
        LOGE("uri permission denied");
        return false;
    }
    return true;
}
} // namespace OHOS::FileManagement