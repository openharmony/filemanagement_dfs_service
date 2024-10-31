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

#include "dfs_error.h"

namespace OHOS::FileManagement::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DfsErrorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DfsErrorTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DfsErrorTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DfsErrorTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DfsErrorTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: Convert2JsErrNumTest
 * @tc.desc: Verify the Convert2JsErrNum function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKAs
 */
HWTEST_F(DfsErrorTest, Convert2JsErrNumTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Convert2JsErrNumTest Start";
    try {
        int32_t errNum = -1;
        int32_t ret = Convert2JsErrNum(errNum);
        EXPECT_EQ(ret, errNum);
        errNum = E_PERMISSION_DENIED;
        ret = Convert2JsErrNum(errNum);
        EXPECT_EQ(ret, E_PERMISSION);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "Convert2JsErrNumTest FAILED";
    }
    GTEST_LOG_(INFO) << "Convert2JsErrNumTest End";
}
} // namespace OHOS::FileManagement::Test
