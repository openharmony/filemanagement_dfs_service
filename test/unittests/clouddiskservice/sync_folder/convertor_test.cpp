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

#include "convertor.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudDiskService {
namespace Test {
using namespace testing::ext;
using namespace std;

class ConvertorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void ConvertorTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void ConvertorTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void ConvertorTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void ConvertorTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ConvertToHexTest001
 * @tc.desc: Verify the ConvertToHex function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertToHexTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToHexTest001 start";
    try {
        EXPECT_EQ(Convertor::ConvertToHex(0), "0000000000000000");
        EXPECT_EQ(Convertor::ConvertToHex(1), "0000000000000001");
        EXPECT_EQ(Convertor::ConvertToHex(255), "00000000000000ff");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertToHexTest001 failed";
    }
    GTEST_LOG_(INFO) << "ConvertToHexTest001 end";
}

/**
 * @tc.name: ConvertFromHexTest001
 * @tc.desc: Verify the ConvertFromHex function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertFromHexTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertFromHexTest001 start";
    try {
        EXPECT_EQ(Convertor::ConvertFromHex("0"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("a"), 10ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("F"), 15ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("G"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex(""), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("123456789ABCDEF0"), 0x123456789ABCDEF0ULL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertFromHexTest001 failed";
    }
    GTEST_LOG_(INFO) << "ConvertFromHexTest001 end";
}

} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS