/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "account_utils.h"
#include "os_account_manager_mock.h"

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
namespace OHOS {
namespace FileManagement {
using namespace testing;
using namespace testing::ext;
using namespace std;

class AccountUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<OsAccountManagerMethodMock> OsAccountMethodMock_ = nullptr;
};

void AccountUtilsTest::SetUpTestCase(void)
{
    OsAccountMethodMock_ = make_shared<OsAccountManagerMethodMock>();
    OsAccountManagerMethod::osMethod_ = OsAccountMethodMock_;
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void AccountUtilsTest::TearDownTestCase(void)
{
    OsAccountMethodMock_ = nullptr;
    OsAccountManagerMethod::osMethod_ = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void AccountUtilsTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void AccountUtilsTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: IsAccountAvailable001
 * @tc.desc: Verify the IsAccountAvailable function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(AccountUtilsTest, IsAccountAvailable001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAccountAvailable001 Start";
    try {
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountTypeFromProcess(_))
            .WillOnce(DoAll(SetArgReferee<0>(AccountSA::OsAccountType::NORMAL), Return(0)));
        bool result = AccountUtils::IsAccountAvailable();
        EXPECT_TRUE(result);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " IsAccountAvailable001 ERROR";
    }
}

/**
 * @tc.name: IsAccountAvailable002
 * @tc.desc: Verify the IsAccountAvailable function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(AccountUtilsTest, IsAccountAvailable002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAccountAvailable002 Start";
    try {
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountTypeFromProcess(_))
            .WillOnce(DoAll(SetArgReferee<0>(AccountSA::OsAccountType::PRIVATE), Return(0)));
        bool result = AccountUtils::IsAccountAvailable();
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " IsAccountAvailable002 ERROR";
    }
}

/**
 * @tc.name: IsAccountAvailable003
 * @tc.desc: Verify the IsAccountAvailable function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(AccountUtilsTest, IsAccountAvailable003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAccountAvailable003 Start";
    try {
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountTypeFromProcess(_))
            .WillOnce(DoAll(SetArgReferee<0>(AccountSA::OsAccountType::PRIVATE), Return(0)));
        bool result = AccountUtils::IsAccountAvailable();
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " IsAccountAvailable003 ERROR";
    }
}

/**
 * @tc.name: IsAccountAvailable004
 * @tc.desc: Verify the IsAccountAvailable function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(AccountUtilsTest, IsAccountAvailable004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsAccountAvailable004 Start";
    try {
        EXPECT_CALL(*OsAccountMethodMock_, GetOsAccountTypeFromProcess(_))
            .WillOnce(Return(-1));
        bool result = AccountUtils::IsAccountAvailable();
        EXPECT_TRUE(result);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " IsAccountAvailable004 ERROR";
    }
}
} // namespace FileManagement
} // namespace OHOS