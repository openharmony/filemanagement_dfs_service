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

#include "os_account_manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vector>

#include "cloud_disk_service_error.h"

namespace {
bool g_queryActiveOsAccountIds = 0;
bool g_isOsAccountVerified = 0;
int32_t TEST_ID = 100;
int32_t INVALID_USER_ID = -1;
}

void MockQueryActiveOsAccountIds(bool mockRet)
{
    g_queryActiveOsAccountIds = mockRet;
}

void MockIsOsAccountVerified(bool mockRet)
{
    g_isOsAccountVerified = mockRet;
}

namespace OHOS::AccountSA {
using namespace OHOS::FileManagement::CloudDiskService;

ErrCode OsAccountManager::QueryActiveOsAccountIds(std::vector<int32_t> &ids)
{
    if (g_queryActiveOsAccountIds) {
        ids.push_back(TEST_ID);
        return E_OK;
    }

    return INVALID_USER_ID;
}

ErrCode OsAccountManager::IsOsAccountVerified(const int32_t id, bool &isVerified)
{
    if (g_isOsAccountVerified) {
        isVerified = true;
        return E_OK;
    }

    isVerified = false;
    return INVALID_USER_ID;
}

}