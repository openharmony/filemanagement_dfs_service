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
}

void DatabaseManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DatabaseManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    databaseManager_ = make_shared<DatabaseManager>();
}

void DatabaseManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    databaseManager_ = nullptr;
}

/**
 * @tc.name: GetRdbStoreTest001
 * @tc.desc: Verify the GetRdbStore function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(DatabaseManagerTest, GetRdbStoreTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRdbStoreTest001 start";
    try {
        const std::string bundleName = "GetRdbStoreTest";
        const int32_t userId = 123456789;
        databaseManager_->GetRdbStore(bundleName, userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRdbStoreTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetRdbStoreTest001 end";
}

/**
 * @tc.name: GetRdbStoreTest002
 * @tc.desc: Verify GetRdbStore returns non-null pointer
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerTest, GetRdbStoreTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRdbStoreTest002 start";
    try {
        auto result = databaseManager_->GetRdbStore("com.test.bundle", 100);
        EXPECT_NE(result, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRdbStoreTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetRdbStoreTest002 end";
}

/**
 * @tc.name: GetRdbStoreTest003
 * @tc.desc: Verify same key returns same shared_ptr
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerTest, GetRdbStoreTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRdbStoreTest003 start";
    try {
        auto result1 = databaseManager_->GetRdbStore("com.test.samekey", 200);
        auto result2 = databaseManager_->GetRdbStore("com.test.samekey", 200);
        EXPECT_EQ(result1, result2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRdbStoreTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetRdbStoreTest003 end";
}

/**
 * @tc.name: GetRdbStoreTest004
 * @tc.desc: Verify different bundleName returns different shared_ptr
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerTest, GetRdbStoreTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRdbStoreTest004 start";
    try {
        auto result1 = databaseManager_->GetRdbStore("com.test.bundleA", 300);
        auto result2 = databaseManager_->GetRdbStore("com.test.bundleB", 300);
        EXPECT_NE(result1, result2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRdbStoreTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetRdbStoreTest004 end";
}

/**
 * @tc.name: GetRdbStoreTest005
 * @tc.desc: Verify different userId returns different shared_ptr
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerTest, GetRdbStoreTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRdbStoreTest005 start";
    try {
        auto result1 = databaseManager_->GetRdbStore("com.test.diffuser", 400);
        auto result2 = databaseManager_->GetRdbStore("com.test.diffuser", 500);
        EXPECT_NE(result1, result2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRdbStoreTest005 failed";
    }
    GTEST_LOG_(INFO) << "GetRdbStoreTest005 end";
}

/**
 * @tc.name: GetRdbStoreTest006
 * @tc.desc: Verify empty bundleName does not crash
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerTest, GetRdbStoreTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRdbStoreTest006 start";
    try {
        auto result = databaseManager_->GetRdbStore("", 600);
        EXPECT_NE(result, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRdbStoreTest006 failed";
    }
    GTEST_LOG_(INFO) << "GetRdbStoreTest006 end";
}

/**
 * @tc.name: GetInstanceTest001
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(DatabaseManagerTest, GetInstanceTest001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetInstanceTest001 start";
    try{
        DatabaseManager &instance = databaseManager_->GetInstance();
        EXPECT_NE(&instance, nullptr);
    }
    catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest001 end";
}

/**
 * @tc.name: GetInstanceTest002
 * @tc.desc: Verify singleton consistency - two calls return same address
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerTest, GetInstanceTest002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetInstanceTest002 start";
    try {
        DatabaseManager &instance1 = databaseManager_->GetInstance();
        DatabaseManager &instance2 = databaseManager_->GetInstance();
        EXPECT_EQ(&instance1, &instance2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest002 end";
}

/**
 * @tc.name: GetInstanceTest003
 * @tc.desc: Verify multiple calls return stable same address
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerTest, GetInstanceTest003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "GetInstanceTest003 start";
    try {
        DatabaseManager *ptr1 = &databaseManager_->GetInstance();
        DatabaseManager *ptr2 = &databaseManager_->GetInstance();
        DatabaseManager *ptr3 = &databaseManager_->GetInstance();
        EXPECT_EQ(ptr1, ptr2);
        EXPECT_EQ(ptr2, ptr3);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest003 end";
}

/**
 * @tc.name: ClearRdbStoreTest001
 * @tc.desc: Verify the ClearRdbStore function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(DatabaseManagerTest, ClearRdbStoreTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearRdbStore001 start";
    try{
        databaseManager_->ClearRdbStore();
        EXPECT_TRUE(true);
    }
    catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ClearRdbStoreTest001 failed";
    }
    GTEST_LOG_(INFO) << "ClearRdbStoreTest001 end";
}

/**
 * @tc.name: ClearRdbStoreTest002
 * @tc.desc: Verify after clear, GetRdbStore creates new instance
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerTest, ClearRdbStoreTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearRdbStoreTest002 start";
    try {
        auto before = databaseManager_->GetRdbStore("com.test.clear", 700);
        databaseManager_->ClearRdbStore();
        auto after = databaseManager_->GetRdbStore("com.test.clear", 700);
        EXPECT_NE(before, after);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ClearRdbStoreTest002 failed";
    }
    GTEST_LOG_(INFO) << "ClearRdbStoreTest002 end";
}

/**
 * @tc.name: ClearRdbStoreTest003
 * @tc.desc: Verify multiple clears do not crash
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerTest, ClearRdbStoreTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ClearRdbStoreTest003 start";
    try {
        databaseManager_->GetRdbStore("com.test.multiclear", 800);
        databaseManager_->ClearRdbStore();
        databaseManager_->ClearRdbStore();
        auto result = databaseManager_->GetRdbStore("com.test.multiclear", 800);
        EXPECT_NE(result, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ClearRdbStoreTest003 failed";
    }
    GTEST_LOG_(INFO) << "ClearRdbStoreTest003 end";
}
} // namespace OHOS::FileManagement::CloudDisk::Test
