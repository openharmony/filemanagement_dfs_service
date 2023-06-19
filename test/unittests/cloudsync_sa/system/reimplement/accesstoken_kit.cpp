/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "accesstoken_kit.h"

namespace OHOS {
namespace Security {
namespace AccessToken {
AccessTokenIDEx AccessTokenKit::AllocHapToken(const HapInfoParams& info, const HapPolicyParams& policy)
{
    AccessTokenIDEx token;
    token.tokenIDEx = 1;
    token.tokenIdExStruct.tokenID = 1;
    token.tokenIdExStruct.tokenAttr = 1;
    return token;
}

int AccessTokenKit::UpdateHapToken(AccessTokenIDEx &tokenIDEx, bool isSystemApp, const std::string& appIDDesc,
    int32_t apiVersion, const HapPolicyParams& policy)
{
    if (isSystemApp) {
        tokenIDEx.tokenIdExStruct.tokenAttr = 1;
    } else {
        tokenIDEx.tokenIdExStruct.tokenAttr = 0;
    }
    return 0;
}

#ifdef BUNDLE_PERMISSION_START_FULL_FALSE
int AccessTokenKit::GetDefPermissions(AccessTokenID tokenID, std::vector<PermissionDef>& permList)
{
    return -1;
}
#else
int AccessTokenKit::GetDefPermissions(AccessTokenID tokenID, std::vector<PermissionDef>& permList)
{
    return 0;
}
#endif


#ifdef BUNDLE_PERMISSION_DEF_LIST
int AccessTokenKit::GetReqPermissions(AccessTokenID tokenID, std::vector<PermissionStateFull>& reqPermList,
    bool isSystemGrant)
{
    return -1;
}

int AccessTokenKit::GrantPermission(AccessTokenID tokenID, const std::string& permissionName, int flag)
{
    return 1;
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    if (tokenID == 0) {
        return PermissionState::PERMISSION_DENIED;
    } else {
        return 0;
    }
}

int AccessTokenKit::GetDefPermission(const std::string& permissionName, PermissionDef& permissionDefResult)
{
    return 0;
}

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    switch (tokenID) {
        case 0: {
            return TOKEN_HAP;
        }
        case 1: {
            return TOKEN_NATIVE;
        }
        case 2: {
            return TOKEN_SHELL;
        }
        default: {
            return TOKEN_INVALID;
        }
    }
}
#else
int AccessTokenKit::GetReqPermissions(AccessTokenID tokenID, std::vector<PermissionStateFull>& reqPermList,
    bool isSystemGrant)
{
    return 0;
}

int AccessTokenKit::GrantPermission(AccessTokenID tokenID, const std::string& permissionName, int flag)
{
    return 0;
}

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    if (tokenID == 0) {
        return PermissionState::PERMISSION_DENIED;
    } else {
        return 0;
    }
}

int AccessTokenKit::GetDefPermission(const std::string& permissionName, PermissionDef& permissionDefResult)
{
    return -1;
}

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
#ifdef BUNDLE_FRAMEWORK_SYSTEM_APP_FALSE
    switch (tokenID) {
        case 0: {
            return TOKEN_HAP;
        }
        case 1: {
            return TOKEN_NATIVE;
        }
        case 2: {
            return TOKEN_SHELL;
        }
        default: {
            return TOKEN_INVALID;
        }
    }
#else
    switch (tokenID) {
        case 0: {
            return TOKEN_HAP;
        }
        case 1: {
            return TOKEN_NATIVE;
        }
        case 2: {
            return TOKEN_SHELL;
        }
        default: {
            return TOKEN_INVALID;
        }
    }
#endif
}
#endif

int AccessTokenKit::VerifyAccessToken(
    AccessTokenID callerTokenID, AccessTokenID firstTokenID, const std::string& permissionName)
{
    return 0;
}

int AccessTokenKit::DeleteToken(AccessTokenID tokenID)
{
    return 0;
}

int AccessTokenKit::ClearUserGrantedPermissionState(AccessTokenID tokenID)
{
    return 0;
}

AccessTokenID AccessTokenKit::GetHapTokenID(int userID, const std::string& bundleName, int instIndex)
{
    return 0;
}

AccessTokenIDEx AccessTokenKit::GetHapTokenIDEx(int userID, const std::string& bundleName, int instIndex)
{
    AccessTokenIDEx tokenIdEx;
    return tokenIdEx;
}

int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo &nativeTokenInfo)
{
    if (tokenID == 2) {
        return 1;
    } else {
        nativeTokenInfo.processName = "foundation";
        return 0;
    }
}
}
}
}