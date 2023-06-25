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

#include "cloud_sync_manager_impl.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing::ext;
using namespace testing;
using namespace std;

class CloudSyncManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncManagerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudSyncManagerTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncManagerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudSyncManagerTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(CloudSyncManagerTest, GetInstanceTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest Begin";
    try {
        CloudSyncManager::GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest ERROR";
    }

    GTEST_LOG_(INFO) << "GetInstanceTest End";
}

} // namespace OHOS::FileManagement::CloudSync::Test
