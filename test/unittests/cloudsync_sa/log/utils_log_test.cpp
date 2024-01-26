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

#include <gtest/gtest.h>

#include "utils_log.h"

namespace OHOS::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class UtilsLogTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void UtilsLogTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void UtilsLogTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void UtilsLogTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void UtilsLogTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetFileNameFromFullPathTest
 * @tc.desc: Verify the GetFileNameFromFullPath function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(UtilsLogTest, GetFileNameFromFullPathTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileNameFromFullPathTest Start";
    try {
        const char *str1 = "test_path";
        std::string out = "";
        std::string ret = GetFileNameFromFullPath(str1);
        EXPECT_STREQ(ret.c_str(), out.c_str());
        const char *str2 = "test_path/1.txt";
        out = "1.txt";
        ret = GetFileNameFromFullPath(str2);
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileNameFromFullPathTest FAILED";
    }
    GTEST_LOG_(INFO) << "GetFileNameFromFullPathTest End";
}
} // namespace OHOS::Test
