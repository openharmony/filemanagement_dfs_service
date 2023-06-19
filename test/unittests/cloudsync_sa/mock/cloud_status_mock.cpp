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

#include "sync_rule/cloud_status.h"

#include "dfs_error.h"

namespace OHOS::FileManagement::CloudSync {
int32_t CloudStatus::GetCurrentCloudInfo(const std::string &bundleName, const int32_t userId)
{
    return E_OK;
}

bool CloudStatus::IsCloudStatusOkay(const std::string &bundleName, const int32_t userId)
{
    return true;
}

int32_t CloudStatus::ChangeAppSwitch(const std::string &bundleName, const int32_t userId, bool appSwitchStatus)
{
    return E_OK;
}

bool CloudStatus::IsAccountIdChanged(const std::string &accountId)
{
    return false;
}
} // namespace OHOS::FileManagement::CloudSync