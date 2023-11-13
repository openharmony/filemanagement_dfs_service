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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "dfsu_access_token_helper.h"
#include "tokenInfo.h"

namespace OHOS::FileManagement::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
class DfsuAccessTokenHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DfsuAccessTokenHelperTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DfsuAccessTokenHelperTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DfsuAccessTokenHelperTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DfsuAccessTokenHelperTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CheckCallerPermissionTest
 * @tc.desc: Verify the CheckCallerPermission function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DfsuAccessTokenHelperTest, CheckCallerPermissionTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckCallerPermissionTest Begin";
    try {
        DfsuAccessTokenHelper dfsuAccessTokenHelper;
        string permissionName = "permissionName";
        TokenInfo tokenInfo;
        tokenInfo.SetTokenId(0);
        auto res = dfsuAccessTokenHelper.CheckCallerPermission(permissionName);
        EXPECT_FALSE(res == true);
        tokenInfo.SetTokenId(2);
        res = dfsuAccessTokenHelper.CheckCallerPermission(permissionName);
        EXPECT_TRUE(res == true);
        tokenInfo.SetTokenId(3);
        res = dfsuAccessTokenHelper.CheckCallerPermission(permissionName);
        EXPECT_TRUE(res == false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CheckCallerPermissionTest ERROR";
    }

    GTEST_LOG_(INFO) << "CheckCallerPermissionTest End";
}

/**
 * @tc.name: CheckPermissionTest
 * @tc.desc: Verify the CheckPermission function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DfsuAccessTokenHelperTest, CheckPermissionTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionTest Begin";
    try {
        DfsuAccessTokenHelper dfsuAccessTokenHelper;
        uint32_t tokenId = 0;
        string permissionName = "permissionName";
        auto res = dfsuAccessTokenHelper.CheckPermission(tokenId, permissionName);
        EXPECT_FALSE(res);
        res = dfsuAccessTokenHelper.CheckPermission(tokenId, permissionName);
        EXPECT_FALSE(res);
        tokenId = 1;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " CheckPermissionTest ERROR";
    }

    GTEST_LOG_(INFO) << "CheckPermissionTest End";
}

/**
 * @tc.name: GetCallerBundleNameTest
 * @tc.desc: Verify the GetCallerBundleName function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DfsuAccessTokenHelperTest, GetCallerBundleNameTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCallerBundleNameTest Begin";
    try {
        DfsuAccessTokenHelper dfsuAccessTokenHelper;
        string bundleName = "bundleName";
        auto res = dfsuAccessTokenHelper.GetCallerBundleName(bundleName);
        EXPECT_EQ(res, 9);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetCallerBundleNameTest ERROR";
    }

    GTEST_LOG_(INFO) << "GetCallerBundleNameTest End";
}

/**
 * @tc.name: GetBundleNameByTokenTest
 * @tc.desc: Verify the GetBundleNameByToken function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DfsuAccessTokenHelperTest, GetBundleNameByTokenTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBundleNameByTokenTest Begin";
    try {
        DfsuAccessTokenHelper dfsuAccessTokenHelper;
        uint32_t tokenId = 0;
        string bundleName = "bundleName";
        auto res = dfsuAccessTokenHelper.GetBundleNameByToken(tokenId, bundleName);
        EXPECT_EQ(res, 9);
        tokenId = 1;
        res = dfsuAccessTokenHelper.GetBundleNameByToken(tokenId, bundleName);
        EXPECT_EQ(res, 0);
        tokenId = 2;
        res = dfsuAccessTokenHelper.GetBundleNameByToken(tokenId, bundleName);
        EXPECT_EQ(res, 9);
        tokenId = 3;
        res = dfsuAccessTokenHelper.GetBundleNameByToken(tokenId, bundleName);
        EXPECT_EQ(res, 9);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetBundleNameByTokenTest ERROR";
    }

    GTEST_LOG_(INFO) << "GetBundleNameByTokenTest End";
}

/**
 * @tc.name: IsSystemAppTest
 * @tc.desc: Verify the IsSystemApp function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DfsuAccessTokenHelperTest, IsSystemAppTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsSystemAppTest Begin";
    try {
        DfsuAccessTokenHelper dfsuAccessTokenHelper;
        TokenInfo tokenInfo;
        tokenInfo.SetTokenId(1);
        auto res = dfsuAccessTokenHelper.IsSystemApp();
        EXPECT_TRUE(res);
        tokenInfo.SetTokenId(0);
        res = dfsuAccessTokenHelper.IsSystemApp();
        EXPECT_TRUE(res == false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsSystemAppTest ERROR";
    }

    GTEST_LOG_(INFO) << "IsSystemAppTest End";
}

/**
 * @tc.name: GetUserIdTest
 * @tc.desc: Verify the GetUserId function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DfsuAccessTokenHelperTest, GetUserIdTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetUserIdTest Begin";
    try {
        DfsuAccessTokenHelper dfsuAccessTokenHelper;
        auto res = dfsuAccessTokenHelper.GetUserId();
        EXPECT_EQ(res, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetUserIdTest ERROR";
    }

    GTEST_LOG_(INFO) << "GetUserIdTest End";
}
} // namespace OHOS::FileManagement::Test
