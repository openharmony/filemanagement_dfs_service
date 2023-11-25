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

#ifndef OHOS_FILEMGMT_DFSU_ACCESS_TOKEN_HELPER_H
#define OHOS_FILEMGMT_DFSU_ACCESS_TOKEN_HELPER_H

#include <string>

namespace OHOS::FileManagement {
inline const std::string PERM_CLOUD_SYNC_MANAGER = "ohos.permission.CLOUDFILE_SYNC_MANAGER";
inline const std::string PERM_CLOUD_SYNC = "ohos.permission.CLOUDFILE_SYNC";
inline const std::string PERM_AUTH_URI = "ohos.permission.PROXY_AUTHORIZATION_URI";
class DfsuAccessTokenHelper final {
public:
    static bool CheckCallerPermission(const std::string &permissionName);
    static int32_t GetCallerBundleName(std::string &bundleName);
    static bool CheckPermission(uint32_t tokenId, const std::string &permissionName);
    static bool CheckUriPermission(const std::string &uri);
    static int32_t GetBundleNameByToken(uint32_t tokenId, std::string &bundleName);
    static bool IsSystemApp();
    static int32_t GetUserId();
};
} // namespace OHOS::FileManagement

#endif // OHOS_FILEMGMT_DFSU_ACCESS_TOKEN_HELPER_H