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

#ifndef DFSU_ACCESS_TOKEN_HELPER_MOCK
#define DFSU_ACCESS_TOKEN_HELPER_MOCK

#include <gmock/gmock.h>
#include "dfsu_access_token_helper.h"
#include <string>

namespace OHOS::FileManagement::CloudSync {

class DfsuAccessToken {
public:
    virtual bool CheckCallerPermission(const std::string &permissionName);
    virtual int32_t GetCallerBundleName(std::string &bundleName);
    virtual bool CheckPermission(uint32_t tokenId, const std::string &permissionName);
    virtual bool CheckUriPermission(const std::string &uri);
    virtual int32_t GetBundleNameByToken(uint32_t tokenId, std::string &bundleName);
    virtual bool IsSystemApp();
    virtual int32_t GetUserId();
    virtual int32_t GetPid();

public:
   DfsuAccessToken() = default;
   virtual ~DfsuAccessToken() = default;
   static inline std::shared_ptr<DfsuAccessToken> dfsuAccessToken = nullptr;
};

class DfsuAccessTokenMock : public DfsuAccessToken {
public:
    MOCK_METHOD1(CheckCallerPermission, bool(const std::string &permissionName));
    MOCK_METHOD1(GetCallerBundleName, int32_t(std::string &bundleName));
    MOCK_METHOD2(CheckPermission, bool(uint32_t tokenId, const std::string &permissionName));
    MOCK_METHOD1(CheckUriPermission, bool(const std::string &uri));
    MOCK_METHOD2(GetBundleNameByToken, int32_t(uint32_t tokenId, std::string &bundleName));
    MOCK_METHOD0(IsSystemApp, bool());
    MOCK_METHOD0(GetUserId, int32_t());
    MOCK_METHOD0(GetPid, int32_t());
};

}
#endif