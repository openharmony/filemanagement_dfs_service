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

#include "dfsu_permission_checker.h"
#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "utils_log.h"

namespace OHOS::FileManagement {
using namespace std;
bool DfsuPermissionChecker::CheckCallerPermission(const std::string &permissionName)
{
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    auto tokenType = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(tokenId);
    if ((tokenType == Security::AccessToken::TOKEN_HAP) || (tokenType == Security::AccessToken::TOKEN_NATIVE)) {
        LOGI("Token type is %{public}d", tokenType);
        return CheckPermission(permissionName);
    } else if (tokenType == Security::AccessToken::TOKEN_SHELL) {
        LOGI("Token type is shell");
        return true;
    } else {
        LOGE("Unsupported token type:%{public}d", tokenType);
        return false;
    }
}

bool DfsuPermissionChecker::CheckPermission(const std::string &permissionName)
{
    LOGD("VerifyCallingPermission permission %{private}s", permissionName.c_str());
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    int32_t ret = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken, permissionName);
    if (ret == Security::AccessToken::PermissionState::PERMISSION_DENIED) {
        LOGE("permission %{private}s: PERMISSION_DENIED", permissionName.c_str());
        return false;
    }
    LOGI("verify AccessToken success");
    return true;
}
} // namespace OHOS::FileManagement