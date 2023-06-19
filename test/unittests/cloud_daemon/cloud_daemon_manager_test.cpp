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
#include "cloud_daemon_manager_impl.h"

namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDaemonManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDaemonManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDaemonManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDaemonManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDaemonManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonManagerTest, GetInstanceTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest Start";
    try {
        CloudDaemonManager::GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest  ERROR";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest End";
}
} // namespace OHOS::FileManagement::CloudFile::Test