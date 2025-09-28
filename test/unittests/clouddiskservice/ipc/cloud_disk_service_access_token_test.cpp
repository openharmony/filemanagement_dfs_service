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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "accesstoken_kit.h"
#include "cloud_disk_service_access_token.h"
#include "cloud_disk_service_error.h"
#include "ipc_skeleton.h"

extern void MockGetTokenTypeFlag(int32_t mockRet);
extern void MockVerifyAccessToken(int32_t mockRet);
extern void MockGetHapTokenInfo(int32_t mockRet);
extern void MockGetNativeTokenInfo(int32_t mockRet);
extern void MockQueryActiveOsAccountIds(bool mockRet);
extern void MockIsOsAccountVerified(bool mockRet);

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif
pid_t IPCSkeleton::GetCallingUid()
{
    return 0;
}

uint32_t IPCSkeleton::GetCallingTokenID()
{
    return 0;
}
}

namespace OHOS {
namespace FileManagement::CloudDiskService {
namespace Test {
using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS::Security::AccessToken;

class CloudDiskServiceAccessTokenTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskServiceAccessTokenTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskServiceAccessTokenTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskServiceAccessTokenTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDiskServiceAccessTokenTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CheckCallerPermission001
 * @tc.desc: Verify the CheckCallerPermission function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, CheckCallerPermission001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckCallerPermission001 start";
    try {
        MockGetTokenTypeFlag(TOKEN_HAP);
        MockVerifyAccessToken(PermissionState::PERMISSION_DENIED);
        std::string permissionName = "";
        bool res = CloudDiskServiceAccessToken::CheckCallerPermission(permissionName);
        EXPECT_EQ(res, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckCallerPermission001 failed";
    }
    GTEST_LOG_(INFO) << "CheckCallerPermission001 end";
}

/**
 * @tc.name: CheckCallerPermission002
 * @tc.desc: Verify the CheckCallerPermission function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, CheckCallerPermission002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckCallerPermission002 start";
    try {
        MockGetTokenTypeFlag(TOKEN_SHELL);
        std::string permissionName = "";
        bool res = CloudDiskServiceAccessToken::CheckCallerPermission(permissionName);
        EXPECT_EQ(res, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckCallerPermission002 failed";
    }
    GTEST_LOG_(INFO) << "CheckCallerPermission002 end";
}

/**
 * @tc.name: CheckCallerPermission003
 * @tc.desc: Verify the CheckCallerPermission function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, CheckCallerPermission003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckCallerPermission003 start";
    try {
        MockGetTokenTypeFlag(TOKEN_INVALID);
        std::string permissionName = "";
        bool res = CloudDiskServiceAccessToken::CheckCallerPermission(permissionName);
        EXPECT_EQ(res, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckCallerPermission003 failed";
    }
    GTEST_LOG_(INFO) << "CheckCallerPermission003 end";
}

/**
 * @tc.name: CheckPermission001
 * @tc.desc: Verify the CheckPermission function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, CheckPermission001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission001 start";
    try {
        MockVerifyAccessToken(PermissionState::PERMISSION_DENIED);
        uint32_t tokenId = 0;
        std::string permissionName = "";
        bool res = CloudDiskServiceAccessToken::CheckPermission(tokenId, permissionName);
        EXPECT_EQ(res, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckPermission001 failed";
    }
    GTEST_LOG_(INFO) << "CheckPermission001 end";
}

/**
 * @tc.name: CheckPermission002
 * @tc.desc: Verify the CheckPermission function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, CheckPermission002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermission002 start";
    try {
        MockVerifyAccessToken(PermissionState::PERMISSION_GRANTED);
        uint32_t tokenId = 0;
        std::string permissionName = "";
        bool res = CloudDiskServiceAccessToken::CheckPermission(tokenId, permissionName);
        EXPECT_EQ(res, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckPermission002 failed";
    }
    GTEST_LOG_(INFO) << "CheckPermission002 end";
}

/**
 * @tc.name: GetCallerBundleName001
 * @tc.desc: Verify the GetCallerBundleName function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, GetCallerBundleName001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallerBundleName001 start";
    try {
        MockGetTokenTypeFlag(TOKEN_INVALID);
        std::string bundleName = "";
        int32_t res = CloudDiskServiceAccessToken::GetCallerBundleName(bundleName);
        EXPECT_EQ(res, E_GET_TOKEN_INFO_ERROR);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCallerBundleName001 failed";
    }
    GTEST_LOG_(INFO) << "GetCallerBundleName001 end";
}

/**
 * @tc.name: GetBundleNameByToken001
 * @tc.desc: Verify the GetBundleNameByToken function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, GetBundleNameByToken001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleNameByToken001 start";
    try {
        MockGetTokenTypeFlag(TOKEN_HAP);
        MockGetHapTokenInfo(1);
        uint32_t tokenId = 0;
        std::string bundleName = "";
        int32_t res = CloudDiskServiceAccessToken::GetBundleNameByToken(tokenId, bundleName);
        EXPECT_EQ(res, E_GET_TOKEN_INFO_ERROR);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBundleNameByToken001 failed";
    }
    GTEST_LOG_(INFO) << "GetBundleNameByToken001 end";
}

/**
 * @tc.name: GetBundleNameByToken002
 * @tc.desc: Verify the GetBundleNameByToken function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, GetBundleNameByToken002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleNameByToken002 start";
    try {
        MockGetTokenTypeFlag(TOKEN_HAP);
        MockGetHapTokenInfo(0);
        uint32_t tokenId = 0;
        std::string bundleName = "";
        int32_t res = CloudDiskServiceAccessToken::GetBundleNameByToken(tokenId, bundleName);
        EXPECT_EQ(res, E_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBundleNameByToken002 failed";
    }
    GTEST_LOG_(INFO) << "GetBundleNameByToken002 end";
}

/**
 * @tc.name: GetBundleNameByToken003
 * @tc.desc: Verify the GetBundleNameByToken function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, GetBundleNameByToken003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleNameByToken003 start";
    try {
        MockGetTokenTypeFlag(TOKEN_NATIVE);
        MockGetNativeTokenInfo(1);
        uint32_t tokenId = 0;
        std::string bundleName = "";
        int32_t res = CloudDiskServiceAccessToken::GetBundleNameByToken(tokenId, bundleName);
        EXPECT_EQ(res, E_GET_TOKEN_INFO_ERROR);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBundleNameByToken003 failed";
    }
    GTEST_LOG_(INFO) << "GetBundleNameByToken003 end";
}

/**
 * @tc.name: GetBundleNameByToken004
 * @tc.desc: Verify the GetBundleNameByToken function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, GetBundleNameByToken004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleNameByToken004 start";
    try {
        MockGetTokenTypeFlag(TOKEN_NATIVE);
        MockGetNativeTokenInfo(0);
        uint32_t tokenId = 0;
        std::string bundleName = "";
        int32_t res = CloudDiskServiceAccessToken::GetBundleNameByToken(tokenId, bundleName);
        EXPECT_EQ(res, E_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBundleNameByToken004 failed";
    }
    GTEST_LOG_(INFO) << "GetBundleNameByToken004 end";
}

/**
 * @tc.name: GetBundleNameByToken005
 * @tc.desc: Verify the GetBundleNameByToken function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, GetBundleNameByToken005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleNameByToken005 start";
    try {
        MockGetTokenTypeFlag(TOKEN_INVALID);
        uint32_t tokenId = 0;
        std::string bundleName = "";
        int32_t res = CloudDiskServiceAccessToken::GetBundleNameByToken(tokenId, bundleName);
        EXPECT_EQ(res, E_GET_TOKEN_INFO_ERROR);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetBundleNameByToken005 failed";
    }
    GTEST_LOG_(INFO) << "GetBundleNameByToken005 end";
}

/**
 * @tc.name: GetUserId001
 * @tc.desc: Verify the GetUserId function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, GetUserId001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUserId001 start";
    try {
        int32_t res = CloudDiskServiceAccessToken::GetUserId();
        EXPECT_EQ(res, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetUserId001 failed";
    }
    GTEST_LOG_(INFO) << "GetUserId001 end";
}

/**
 * @tc.name: GetAccountId001
 * @tc.desc: Verify the GetAccountId function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, GetAccountId001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAccountId001 start";
    try {
        MockQueryActiveOsAccountIds(false);
        int32_t userId = 0;
        int32_t res = CloudDiskServiceAccessToken::GetAccountId(userId);
        EXPECT_EQ(res, E_OSACCOUNT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAccountId001 failed";
    }
    GTEST_LOG_(INFO) << "GetAccountId001 end";
}

/**
 * @tc.name: GetAccountId002
 * @tc.desc: Verify the GetAccountId function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, GetAccountId002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAccountId002 start";
    try {
        MockQueryActiveOsAccountIds(true);
        MockIsOsAccountVerified(false);
        int32_t userId = 0;
        int32_t res = CloudDiskServiceAccessToken::GetAccountId(userId);
        EXPECT_EQ(res, E_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAccountId002 failed";
    }
    GTEST_LOG_(INFO) << "GetAccountId002 end";
}

/**
 * @tc.name: GetAccountId003
 * @tc.desc: Verify the GetAccountId function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, GetAccountId003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAccountId003 start";
    try {
        MockQueryActiveOsAccountIds(true);
        MockIsOsAccountVerified(true);
        int32_t userId = 0;
        int32_t res = CloudDiskServiceAccessToken::GetAccountId(userId);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAccountId003 failed";
    }
    GTEST_LOG_(INFO) << "GetAccountId003 end";
}

/**
 * @tc.name: IsUserVerifyed001
 * @tc.desc: Verify the IsUserVerifyed function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, IsUserVerifyed001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsUserVerifyed001 start";
    try {
        MockIsOsAccountVerified(true);
        int32_t userId = 0;
        bool res = CloudDiskServiceAccessToken::IsUserVerifyed(userId);
        EXPECT_EQ(res, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsUserVerifyed001 failed";
    }
    GTEST_LOG_(INFO) << "IsUserVerifyed001 end";
}

/**
 * @tc.name: IsUserVerifyed002
 * @tc.desc: Verify the IsUserVerifyed function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceAccessTokenTest, IsUserVerifyed002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsUserVerifyed002 start";
    try {
        MockIsOsAccountVerified(false);
        int32_t userId = 0;
        bool res = CloudDiskServiceAccessToken::IsUserVerifyed(userId);
        EXPECT_EQ(res, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsUserVerifyed002 failed";
    }
    GTEST_LOG_(INFO) << "IsUserVerifyed002 end";
}
} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS