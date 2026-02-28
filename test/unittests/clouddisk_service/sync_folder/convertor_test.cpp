/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 * @tc.name: ConvertToHexTest002
 * @tc.desc: Verify the ConvertToHex function with max value
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertToHexTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToHexTest002 start";
    try {
        uint64_t maxValue = 0xFFFFFFFFFFFFFFFFULL;
        string hex = Convertor::ConvertToHex(maxValue);
        EXPECT_EQ(hex, "ffffffffffffffff");
        EXPECT_EQ(hex.length(), 16);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertToHexTest002 failed";
    }
    GTEST_LOG_(INFO) << "ConvertToHexTest002 end";
}

/**
 * @tc.name: ConvertToHexTest003
 * @tc.desc: Verify the ConvertToHex function with middle values
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertToHexTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToHexTest003 start";
    try {
        EXPECT_EQ(Convertor::ConvertToHex(0x1234), "0000000000001234");
        EXPECT_EQ(Convertor::ConvertToHex(0xABCD1234), "00000000abcd1234");
        EXPECT_EQ(Convertor::ConvertToHex(0x123456789ABCDEF0ULL), "123456789abcdef0");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertToHexTest003 failed";
    }
    GTEST_LOG_(INFO) << "ConvertToHexTest003 end";
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
        EXPECT_EQ(Convertor::ConvertFromHex("f"), 15ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("g"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex(""), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("123456789ABCDEF0"), 0x123456789ABCDEF0ULL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertFromHexTest001 failed";
    }
    GTEST_LOG_(INFO) << "ConvertFromHexTest001 end";
}

/**
 * @tc.name: ConvertFromHexTest002
 * @tc.desc: Verify the ConvertFromHex function with uppercase letters
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertFromHexTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertFromHexTest002 start";
    try {
        EXPECT_EQ(Convertor::ConvertFromHex("A"), 10ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("B"), 11ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("C"), 12ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("D"), 13ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("E"), 14ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("F"), 15ULL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertFromHexTest002 failed";
    }
    GTEST_LOG_(INFO) << "ConvertFromHexTest002 end";
}

/**
 * @tc.name: ConvertFromHexTest003
 * @tc.desc: Verify the ConvertFromHex function with mixed cases
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertFromHexTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertFromHexTest003 start";
    try {
        EXPECT_EQ(Convertor::ConvertFromHex("AbCd1234"), 0xABCD1234ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("12Ab34Cd"), 0x12AB34CDULL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertFromHexTest003 failed";
    }
    GTEST_LOG_(INFO) << "ConvertFromHexTest003 end";
}

/**
 * @tc.name: ConvertFromHexTest004
 * @tc.desc: Verify the ConvertFromHex function with invalid characters
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertFromHexTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertFromHexTest004 start";
    try {
        EXPECT_EQ(Convertor::ConvertFromHex("G"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("g"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("Z"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("z"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("!"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("@"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("#"), 0ULL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertFromHexTest004 failed";
    }
    GTEST_LOG_(INFO) << "ConvertFromHexTest004 end";
}

/**
 * @tc.name: ConvertRoundTripTest001
 * @tc.desc: Verify round trip conversion
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertRoundTripTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertRoundTripTest001 start";
    try {
        uint64_t testValues[] = {0, 1, 255, 0x1234, 0xABCD1234, 0x123456789ABCDEF0ULL, 0xFFFFFFFFFFFFFFFFULL};
        for (auto value : testValues) {
            string hex = Convertor::ConvertToHex(value);
            uint64_t convertedBack = Convertor::ConvertFromHex(hex);
            EXPECT_EQ(convertedBack, value);
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertRoundTripTest001 failed";
    }
    GTEST_LOG_(INFO) << "ConvertRoundTripTest001 end";
}
} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS