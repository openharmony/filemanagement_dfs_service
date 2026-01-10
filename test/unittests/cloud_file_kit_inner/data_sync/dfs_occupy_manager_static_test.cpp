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

#include "dfs_occupy_manager.cpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dfs_error.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DfsOccupyManagerStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DfsOccupyManagerStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DfsOccupyManagerStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DfsOccupyManagerStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DfsOccupyManagerStaticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CheckAddTest001
 * @tc.desc: Verify the CheckAdd function
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerStaticTest, CheckAddTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAddTest001 Start";
    try {
        int64_t data = INT64_MAX;
        int addValue = 1;
        CheckAdd(data, addValue);

        data = 10;
        CheckAdd(data, addValue);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAddTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAddTest001 End";
}

/**
 * @tc.name: CheckSubTest001
 * @tc.desc: Verify the CheckSub function
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerStaticTest, CheckSubTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSubTest001 Start";
    try {
        int64_t data = 12;
        int addValue = 11;
        CheckSub(data, addValue);

        data = 10;
        CheckSub(data, addValue);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckSubTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckSubTest001 End";
}
}