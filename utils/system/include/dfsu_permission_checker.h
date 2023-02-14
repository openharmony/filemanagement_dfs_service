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

#ifndef OHOS_FILEMGMT_DFSU_PERMISSION_CHECKER_H
#define OHOS_FILEMGMT_DFSU_PERMISSION_CHECKER_H

#include <string>

namespace OHOS::FileManagement {
inline const std::string PERM_CLOUD_SYNC = "ohos.permission.cloud_sync";
class DfsuPermissionChecker final {
public:
    static bool CheckCallerPermission(const std::string &permissionName);
    static bool CheckPermission(const std::string &permissionName);
};
} // namespace OHOS::FileManagement

#endif // OHOS_FILEMGMT_DFSU_PERMISSION_CHECKER_H