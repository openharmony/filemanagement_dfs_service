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

#include "dfs_error.h"
#include "dfs_occupy_manager.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DfsOccupyManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DfsOccupyManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DfsOccupyManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DfsOccupyManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DfsOccupyManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetInstanceTest001
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, GetInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest001 Start";
    try {
        DfsOccupyManager::GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest001 End";
}

/**
 * @tc.name: AddOccupyToMemTest001
 * @tc.desc: Verify the AddOccupyToMem function
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest001 Start";
    try {
        int32_t userId = 200;
        int64_t num = 101;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->occupyNums_[userId] = num;
        dfsOccupyManager->AddOccupyToMem(userId, num);

        userId = 300;
        dfsOccupyManager->AddOccupyToMem(userId, num);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest001 End";
}

/**
 * @tc.name: SubOccupyToMemTest001
 * @tc.desc: Verify the SubOccupyToMem function
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, SubOccupyToMemTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SubOccupyToMemTest001 Start";
    try {
        int32_t userId = 200;
        int64_t num = 101;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->occupyNums_[userId] = num;
        dfsOccupyManager->SubOccupyToMem(userId, num);

        userId = 400;
        dfsOccupyManager->SubOccupyToMem(userId, num);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SubOccupyToMemTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "SubOccupyToMemTest001 End";
}

/**
 * @tc.name: SaveOccupyToFileTest001
 * @tc.desc: Verify the SaveOccupyToFile function
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, SaveOccupyToFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveOccupyToFileTest001 Start";
    try {
        int32_t userId = 200;
        int64_t num = 101;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->occupyNums_[userId] = num;
        dfsOccupyManager->SaveOccupyToFile(userId);

        userId = 500;
        dfsOccupyManager->SaveOccupyToFile(userId);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SaveOccupyToFileTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "SaveOccupyToFileTest001 End";
}

/**
 * @tc.name: SaveOccupyToFileMemByNumTest001
 * @tc.desc: Verify the SaveOccupyToFileMemByNum function
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, SaveOccupyToFileMemByNumTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SaveOccupyToFileMemByNumTest001 Start";
    try {
        int32_t userId = 200;
        int64_t num = 101;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->occupyNums_[userId] = num;
        dfsOccupyManager->SaveOccupyToFileMemByNum(userId, num);

        userId = 600;
        dfsOccupyManager->SaveOccupyToFileMemByNum(userId, num);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SaveOccupyToFileMemByNumTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "SaveOccupyToFileMemByNumTest001 End";
}

/**
 * @tc.name: GetOccupyFromFileTest001
 * @tc.desc: Verify the GetOccupyFromFile function
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, GetOccupyFromFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetOccupyFromFileTest001 Start";
    try {
        int32_t userId = 700;
        int64_t num = 101;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->GetOccupyFromFile(userId, num);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetOccupyFromFileTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetOccupyFromFileTest001 End";
}
}