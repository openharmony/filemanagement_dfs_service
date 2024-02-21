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

#include "database_manager.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DatabaseManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<DatabaseManager> databaseManager_ = nullptr;
};

void DatabaseManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    databaseManager_ = make_shared<DatabaseManager>();
}

void DatabaseManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    databaseManager_ = nullptr;
}

void DatabaseManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DatabaseManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetRdbStoreTest001
 * @tc.desc: Verify the GetRdbStore function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(DatabaseManagerTest, GetRdbStoreTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRdbStoreTest001 Start";
    try {
        const std::string bundleName = "GetRdbStoreTest";
        const int32_t userId = 123456789;
        databaseManager_->GetRdbStore(bundleName, userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRdbStoreTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "GetRdbStoreTest001 End";
}

} // namespace OHOS::FileManagement::CloudDisk::Test