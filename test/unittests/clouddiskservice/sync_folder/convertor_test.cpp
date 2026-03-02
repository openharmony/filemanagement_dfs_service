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
 * @tc.desc: Verify the ConvertFromHex function with valid inputs
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
        EXPECT_EQ(Convertor::ConvertFromHex(""), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("123456789ABCDEF0"), 0x123456789ABCDEF0ULL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertFromHexTest001 failed";
    }
    GTEST_LOG_(INFO) << "ConvertFromHexTest001 end";
}

/**
 * @tc.name: ConvertToHexTest002
 * @tc.desc: Verify the ConvertToHex function with boundary values
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertToHexTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToHexTest002 start";
    try {
        EXPECT_EQ(Convertor::ConvertToHex(9), "0000000000000009");
        EXPECT_EQ(Convertor::ConvertToHex(10), "000000000000000a");
        EXPECT_EQ(Convertor::ConvertToHex(15), "000000000000000f");
        EXPECT_EQ(Convertor::ConvertToHex(16), "0000000000000010");
        EXPECT_EQ(Convertor::ConvertToHex(255), "00000000000000ff");
        EXPECT_EQ(Convertor::ConvertToHex(256), "0000000000000100");
        EXPECT_EQ(Convertor::ConvertToHex(4095), "0000000000000fff");
        EXPECT_EQ(Convertor::ConvertToHex(4096), "0000000000001000");
        EXPECT_EQ(Convertor::ConvertToHex(65535), "000000000000ffff");
        EXPECT_EQ(Convertor::ConvertToHex(65536), "0000000000010000");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertToHexTest002 failed";
    }
    GTEST_LOG_(INFO) << "ConvertToHexTest002 end";
}

/**
 * @tc.name: ConvertToHexTest003
 * @tc.desc: Verify the ConvertToHex function with maximum and large values
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertToHexTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToHexTest003 start";
    try {
        EXPECT_EQ(Convertor::ConvertToHex(UINT64_MAX), "ffffffffffffffff");
        EXPECT_EQ(Convertor::ConvertToHex(0xFFFFFFFFFFFFFFFFULL), "ffffffffffffffff");
        EXPECT_EQ(Convertor::ConvertToHex(0x8000000000000000ULL), "8000000000000000");
        EXPECT_EQ(Convertor::ConvertToHex(0x7FFFFFFFFFFFFFFFULL), "7fffffffffffffff");
        EXPECT_EQ(Convertor::ConvertToHex(0x123456789ABCDEF0ULL), "123456789abcdef0");
        EXPECT_EQ(Convertor::ConvertToHex(0x0123456789ABCDEFULL), "0123456789abcdef");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertToHexTest003 failed";
    }
    GTEST_LOG_(INFO) << "ConvertToHexTest003 end";
}

/**
 * @tc.name: ConvertFromHexTest002
 * @tc.desc: Verify the ConvertFromHex function with invalid characters
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertFromHexTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertFromHexTest002 start";
    try {
        EXPECT_EQ(Convertor::ConvertFromHex("G"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("g"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("Z"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("z"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("x"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("X"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("!"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("@"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("#"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex(" "), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("12G34"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("abg"), 0ULL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertFromHexTest002 failed";
    }
    GTEST_LOG_(INFO) << "ConvertFromHexTest002 end";
}

/**
 * @tc.name: ConvertFromHexTest003
 * @tc.desc: Verify the ConvertFromHex function with mixed case
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertFromHexTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertFromHexTest003 start";
    try {
        EXPECT_EQ(Convertor::ConvertFromHex("aBcDeF"), 0xABCDEFULL);
        EXPECT_EQ(Convertor::ConvertFromHex("AbCdEf"), 0xABCDEFULL);
        EXPECT_EQ(Convertor::ConvertFromHex("ABCDEF"), 0xABCDEFULL);
        EXPECT_EQ(Convertor::ConvertFromHex("abcdef"), 0xABCDEFULL);
        EXPECT_EQ(Convertor::ConvertFromHex("0a1B2c3D"), 0x0A1B2C3DULL);
        EXPECT_EQ(Convertor::ConvertFromHex("123abcABC"), 0x123ABCABCULL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertFromHexTest003 failed";
    }
    GTEST_LOG_(INFO) << "ConvertFromHexTest003 end";
}

/**
 * @tc.name: ConvertFromHexTest004
 * @tc.desc: Verify the ConvertFromHex function with full length strings
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertFromHexTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertFromHexTest004 start";
    try {
        EXPECT_EQ(Convertor::ConvertFromHex("ffffffffffffffff"), 0xFFFFFFFFFFFFFFFFULL);
        EXPECT_EQ(Convertor::ConvertFromHex("0000000000000000"), 0ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("0000000000000001"), 1ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("7fffffffffffffff"), 0x7FFFFFFFFFFFFFFFULL);
        EXPECT_EQ(Convertor::ConvertFromHex("8000000000000000"), 0x8000000000000000ULL);
        EXPECT_EQ(Convertor::ConvertFromHex("123456789abcdef0"), 0x123456789ABCDEF0ULL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertFromHexTest004 failed";
    }
    GTEST_LOG_(INFO) << "ConvertFromHexTest004 end";
}

/**
 * @tc.name: ConvertorRoundTripTest001
 * @tc.desc: Verify round-trip conversion (value -> hex -> value)
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(ConvertorTest, ConvertorRoundTripTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertorRoundTripTest001 start";
    try {
        std::vector<uint64_t> testValues = {
            0, 1, 9, 10, 15, 16, 255, 256, 4095, 4096, 65535, 65536,
            0x123456789ABCDEF0ULL, 0x8000000000000000ULL, 0x7FFFFFFFFFFFFFFFULL,
            UINT64_MAX
        };
        
        for (uint64_t value : testValues) {
            std::string hex = Convertor::ConvertToHex(value);
            uint64_t result = Convertor::ConvertFromHex(hex);
            EXPECT_EQ(result, value);
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertorRoundTripTest001 failed";
    }
    GTEST_LOG_(INFO) << "ConvertorRoundTripTest001 end";
}

} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS