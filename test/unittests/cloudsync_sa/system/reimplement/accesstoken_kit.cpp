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
#ifdef BUNDLE_PERMISSION_DEF_LIST

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    if (tokenID == 0) {
        return PermissionState::PERMISSION_DENIED;
    } else {
        return 0;
    }
}

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
    switch (tokenID) {
        case TOKEN_HAP: {
            return TOKEN_HAP;
        }
        case TOKEN_NATIVE: {
            return TOKEN_NATIVE;
        }
        case TOKEN_SHELL: {
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

int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenID, const std::string& permissionName)
{
    if (tokenID == 0) {
        return PermissionState::PERMISSION_DENIED;
    } else {
        return 0;
    }
}

ATokenTypeEnum AccessTokenKit::GetTokenTypeFlag(AccessTokenID tokenID)
{
#ifdef BUNDLE_FRAMEWORK_SYSTEM_APP_FALSE
    switch (tokenID) {
        case TOKEN_HAP: {
            return TOKEN_HAP;
        }
        case TOKEN_NATIVE: {
            return TOKEN_NATIVE;
        }
        case TOKEN_SHELL: {
            return TOKEN_SHELL;
        }
        default: {
            return TOKEN_INVALID;
        }
    }
#else
    switch (tokenID) {
        case TOKEN_HAP: {
            return TOKEN_HAP;
        }
        case TOKEN_NATIVE: {
            return TOKEN_NATIVE;
        }
        case TOKEN_SHELL: {
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
int AccessTokenKit::GetNativeTokenInfo(AccessTokenID tokenID, NativeTokenInfo &nativeTokenInfo)
{
    if (tokenID == TOKEN_SHELL) {
        return TOKEN_NATIVE;
    } else {
        nativeTokenInfo.processName = "foundation";
        return 0;
    }
}
}
}
}