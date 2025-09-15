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

#include "uuid_helper.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudDiskService {
namespace Test {
using namespace testing::ext;
using namespace std;

class UuidHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UuidHelperTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void UuidHelperTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void UuidHelperTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void UuidHelperTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GenerateUuidTest001
 * @tc.desc: Verify the GenerateUuid function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UuidHelperTest, GenerateUuidTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateUuidTest001 start";
    try {
        string uuid = UuidHelper::GenerateUuid();
        EXPECT_EQ(uuid.length(), 36);
        EXPECT_TRUE(uuid.find('-') != string::npos);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateUuidTest001 failed";
    }
    GTEST_LOG_(INFO) << "GenerateUuidTest001 end";
}

/**
 * @tc.name: GenerateUuidWithoutDelimTest001
 * @tc.desc: Verify the GenerateUuidWithoutDelim function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UuidHelperTest, GenerateUuidWithoutDelimTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateUuidWithoutDelimTest001 start";
    try {
        string uuid = UuidHelper::GenerateUuidWithoutDelim();
        EXPECT_EQ(uuid.length(), 32);
        EXPECT_TRUE(uuid.find('-') == string::npos);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateUuidWithoutDelimTest001 failed";
    }
    GTEST_LOG_(INFO) << "GenerateUuidWithoutDelimTest001 end";
}

/**
 * @tc.name: GenerateUuidOnlyTest001
 * @tc.desc: Verify the GenerateUuidOnly function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(UuidHelperTest, GenerateUuidOnlyTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateUuidOnlyTest001 start";
    try {
        string uuid = UuidHelper::GenerateUuidOnly();
        EXPECT_EQ(uuid.length(), 16);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateUuidOnlyTest001 failed";
    }
    GTEST_LOG_(INFO) << "GenerateUuidOnlyTest001 end";
}
} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS