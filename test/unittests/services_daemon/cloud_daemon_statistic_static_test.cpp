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

#include "cloud_daemon_statistic.cpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudFile::test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDaemonStatisticStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDaemonStatisticStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDaemonStatisticStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDaemonStatisticStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDaemonStatisticStaticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(CloudDaemonStatisticStaticTest, CheckOverflow_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckOverflow_001 Start";
    uint32_t data = 0;

    try {
        CheckOverflow(data, 1);
        EXPECT_EQ(data, 1);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckOverflow_001 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckOverflow_001 End";
}

HWTEST_F(CloudDaemonStatisticStaticTest, CheckOverflow_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckOverflow_002 Start";
    uint32_t data = UINT32_MAX;

    try {
        CheckOverflow(data, 1);
        EXPECT_EQ(data, UINT32_MAX);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckOverflow_002 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckOverflow_002 End";
}
}