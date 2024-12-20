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

#include "dfsu_access_token_helper_mock.h"
#include "dfs_error.h"

namespace OHOS::FileManagement {
using namespace std;
bool DfsuAccessTokenHelper::CheckCallerPermission(const std::string &permissionName)
{
    return FileManagement::CloudSync::DfsuAccessToken::dfsuAccessToken->CheckCallerPermission(permissionName);
}

bool DfsuAccessTokenHelper::CheckPermission(uint32_t tokenId, const std::string &permissionName)
{
    return FileManagement::CloudSync::DfsuAccessToken::dfsuAccessToken->CheckPermission(tokenId, permissionName);
}

int32_t DfsuAccessTokenHelper::GetCallerBundleName(std::string &bundleName)
{
    return FileManagement::CloudSync::DfsuAccessToken::dfsuAccessToken->GetCallerBundleName(bundleName);
}

int32_t DfsuAccessTokenHelper::GetBundleNameByToken(uint32_t tokenId, std::string &bundleName)
{
    return FileManagement::CloudSync::DfsuAccessToken::dfsuAccessToken->GetBundleNameByToken(tokenId, bundleName);
}
bool DfsuAccessTokenHelper::IsSystemApp()
{
    return FileManagement::CloudSync::DfsuAccessToken::dfsuAccessToken->IsSystemApp();
}

int32_t DfsuAccessTokenHelper::GetUserId()
{
    return FileManagement::CloudSync::DfsuAccessToken::dfsuAccessToken->GetUserId();
}

int32_t DfsuAccessTokenHelper::GetPid()
{
    return FileManagement::CloudSync::DfsuAccessToken::dfsuAccessToken->GetPid();
}

bool DfsuAccessTokenHelper::CheckUriPermission(const std::string &uri)
{
    return FileManagement::CloudSync::DfsuAccessToken::dfsuAccessToken->CheckUriPermission(uri);
}

} // namespace OHOS::FileManagement