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
 * @tc.name: AddOccupyToMemTest002
 * @tc.desc: Verify AddOccupyToMem with existing user
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest002 Start";
    try {
        int32_t userId = 100;
        int64_t initialNum = 100;
        int64_t addNum = 50;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->occupyNums_[userId] = initialNum;
        dfsOccupyManager->AddOccupyToMem(userId, addNum);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], initialNum + addNum);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest002 End";
}

/**
 * @tc.name: AddOccupyToMemTest003
 * @tc.desc: Verify AddOccupyToMem with new user
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest003 Start";
    try {
        int32_t userId = 150;
        int64_t addNum = 200;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->AddOccupyToMem(userId, addNum);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], addNum);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest003 End";
}

/**
 * @tc.name: AddOccupyToMemTest004
 * @tc.desc: Verify AddOccupyToMem with zero value
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest004 Start";
    try {
        int32_t userId = 200;
        int64_t initialNum = 100;
        int64_t addNum = 0;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->occupyNums_[userId] = initialNum;
        dfsOccupyManager->AddOccupyToMem(userId, addNum);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], initialNum);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest004 End";
}


/**
 * @tc.name: AddOccupyToMemTest005
 * @tc.desc: Verify AddOccupyToMem with overflow at boundary
 * @tc.type: FUNC
* @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest005 Start";
    try {
        int32_t userId = 650;
        int64_t initialNum = INT64_MAX;
        int64_t addNum = 1;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->occupyNums_[userId] = initialNum;
        dfsOccupyManager->AddOccupyToMem(userId, addNum);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest005 End";
}

/**
 * @tc.name: AddOccupyToMemTest006
 * @tc.desc: Verify AddOccupyToMem with overflow scenario
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest006 Start";
    try {
        int32_t userId = 300;
        int64_t initialNum = INT64_MAX - 100;
        int64_t addNum = 200;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->occupyNums_[userId] = initialNum;
        dfsOccupyManager->AddOccupyToMem(userId, addNum);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest006 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest006 End";
}

/**
 * @tc.name: AddOccupyToMemTest007
 * @tc.desc: Verify AddOccupyToMem with INT64_MAX
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest007 Start";
    try {
        int32_t userId = 350;
        int64_t initialNum = 0;
        int64_t addNum = INT64_MAX;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->occupyNums_[userId] = initialNum;
        dfsOccupyManager->AddOccupyToMem(userId, addNum);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], INT64_MAX);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest007 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest007 End";
}

/**
 * @tc.name: AddOccupyToMemTest008
 * @tc.desc: Verify AddOccupyToMem with minimum user id
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest008 Start";
    try {
        int32_t userId = INT32_MIN;
        int64_t addNum = 100;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->AddOccupyToMem(userId, addNum);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], addNum);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest008 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest008 End";
}

/**
 * @tc.name: AddOccupyToMemTest009
 * @tc.desc: Verify AddOccupyToMem with maximum user id
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest009 Start";
    try {
        int32_t userId = INT32_MAX;
        int64_t addNum = 100;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->AddOccupyToMem(userId, addNum);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], addNum);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest009 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest009 End";
}

/**
 * @tc.name: AddOccupyToMemTest010
 * @tc.desc: Verify AddOccupyToMem with zero user id
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest010 Start";
    try {
        int32_t userId = 0;
        int64_t addNum = 100;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->AddOccupyToMem(userId, addNum);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], addNum);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest010 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest010 End";
}

/**
 * @tc.name: AddOccupyToMemTest011
 * @tc.desc: Verify AddOccupyToMem with multiple additions to same user
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest011 Start";
    try {
        int32_t userId = 400;
        int64_t addNum1 = 50;
        int64_t addNum2 = 30;
        int64_t addNum3 = 20;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->AddOccupyToMem(userId, addNum1);
        dfsOccupyManager->AddOccupyToMem(userId, addNum2);
        dfsOccupyManager->AddOccupyToMem(userId, addNum3);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], addNum1 + addNum2 + addNum3);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest011 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest011 End";
}

/**
 * @tc.name: AddOccupyToMemTest012
 * @tc.desc: Verify AddOccupyToMem with multiple different users
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest012 Start";
    try {
        int32_t userId1 = 500;
        int32_t userId2 = 501;
        int32_t userId3 = 502;
        int64_t addNum1 = 100;
        int64_t addNum2 = 200;
        int64_t addNum3 = 300;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->AddOccupyToMem(userId1, addNum1);
        dfsOccupyManager->AddOccupyToMem(userId2, addNum2);
        dfsOccupyManager->AddOccupyToMem(userId3, addNum3);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId1], addNum1);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId2], addNum2);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId3], addNum3);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest012 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest012 End";
}

/**
 * @tc.name: AddOccupyToMemTest013
 * @tc.desc: Verify AddOccupyToMem with large positive value
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest013 Start";
    try {
        int32_t userId = 550;
        int64_t initialNum = 1000;
        int64_t addNum = 1000000000000LL;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->occupyNums_[userId] = initialNum;
        dfsOccupyManager->AddOccupyToMem(userId, addNum);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], initialNum + addNum);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest013 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest013 End";
}

/**
 * @tc.name: AddOccupyToMemTest014
 * @tc.desc: Verify AddOccupyToMem with negative initial value
 * @tc.type: FUNC
 * @tc.require: issues2770
 */
HWTEST_F(DfsOccupyManagerTest, AddOccupyToMemTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddOccupyToMemTest014 Start";
    try {
        int32_t userId = 600;
        int64_t initialNum = -100;
        int64_t addNum = 50;
        auto dfsOccupyManager = make_shared<DfsOccupyManager>();
        dfsOccupyManager->occupyNums_[userId] = initialNum;
        dfsOccupyManager->AddOccupyToMem(userId, addNum);
        EXPECT_EQ(dfsOccupyManager->occupyNums_[userId], initialNum + addNum);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddOccupyToMemTest014 ERROR";
    }
    GTEST_LOG_(INFO) << "AddOccupyToMemTest014 End";
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