/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "sync_state_manager.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MockSyncStateManager : public SyncStateManager {
public:
    MOCK_METHOD1(UpdateSyncState, Action(SyncState newState));
    MOCK_METHOD2(CheckAndSetPending, bool(bool forceFlag, SyncTriggerType triggerType));
    MOCK_METHOD0(SetCleaningFlag, void());
    MOCK_METHOD0(GetStopSyncFlag, bool());
    MOCK_METHOD0(SetStopSyncFlag, void());
    MOCK_METHOD0(SetDisableCloudFlag, void());
    MOCK_METHOD0(GetSyncState, void());
    MOCK_METHOD0(GetForceFlag, bool());
};

class SyncStateManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SyncStateManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void SyncStateManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void SyncStateManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void SyncStateManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_001";
    MockSyncStateManager mockSyncStateManager;
    try {
        EXPECT_CALL(mockSyncStateManager, SetCleaningFlag());
        mockSyncStateManager.SetCleaningFlag();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_001 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_002";
    MockSyncStateManager mockSyncStateManager;
    try {
        EXPECT_CALL(mockSyncStateManager, UpdateSyncState(SyncState::INIT)).Times(1);
        EXPECT_CALL(mockSyncStateManager, UpdateSyncState(SyncState::SYNCING)).Times(1);
        EXPECT_CALL(mockSyncStateManager, UpdateSyncState(SyncState::CLEANING)).Times(1);
        EXPECT_CALL(mockSyncStateManager, UpdateSyncState(SyncState::SYNC_FAILED)).Times(1);
        EXPECT_CALL(mockSyncStateManager, UpdateSyncState(SyncState::SYNC_SUCCEED)).Times(1);
        EXPECT_CALL(mockSyncStateManager, UpdateSyncState(SyncState::CLEAN_SUCCEED)).Times(1);
        EXPECT_CALL(mockSyncStateManager, UpdateSyncState(SyncState::DISABLE_CLOUD)).Times(1);
        EXPECT_CALL(mockSyncStateManager, UpdateSyncState(SyncState::DISABLE_CLOUD_SUCCEED)).Times(1);

        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::INIT), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::SYNCING), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::CLEANING), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::SYNC_FAILED), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::SYNC_SUCCEED), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::CLEAN_SUCCEED), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::DISABLE_CLOUD), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::DISABLE_CLOUD_SUCCEED), Action::STOP);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_002 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_002 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_003";
    MockSyncStateManager mockSyncStateManager;
    try {
        EXPECT_CALL(mockSyncStateManager, CheckAndSetPending(true, SyncTriggerType::APP_TRIGGER))
            .WillOnce(Return(true))
            .WillOnce(Return(false));
        EXPECT_CALL(mockSyncStateManager, CheckAndSetPending(false, SyncTriggerType::TASK_TRIGGER))
            .WillOnce(Return(true))
            .WillOnce(Return(false));
        ASSERT_TRUE(mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::APP_TRIGGER));
        ASSERT_FALSE(mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::APP_TRIGGER));
        ASSERT_TRUE(mockSyncStateManager.CheckAndSetPending(false, SyncTriggerType::TASK_TRIGGER));
        ASSERT_FALSE(mockSyncStateManager.CheckAndSetPending(false, SyncTriggerType::TASK_TRIGGER));

        EXPECT_CALL(mockSyncStateManager, CheckAndSetPending(false, SyncTriggerType::TASK_TRIGGER));
        EXPECT_CALL(mockSyncStateManager, CheckAndSetPending(true, SyncTriggerType::CLOUD_TRIGGER));
        EXPECT_CALL(mockSyncStateManager, CheckAndSetPending(true, SyncTriggerType::PENDING_TRIGGER));
        EXPECT_CALL(mockSyncStateManager, CheckAndSetPending(true, SyncTriggerType::BATTERY_OK_TRIGGER));
        EXPECT_CALL(mockSyncStateManager, CheckAndSetPending(true, SyncTriggerType::NETWORK_AVAIL_TRIGGER));
        EXPECT_CALL(mockSyncStateManager, CheckAndSetPending(true, SyncTriggerType::SYSTEM_LOAD_TRIGGER));
        mockSyncStateManager.CheckAndSetPending(false, SyncTriggerType::TASK_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::CLOUD_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::PENDING_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::BATTERY_OK_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::NETWORK_AVAIL_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::SYSTEM_LOAD_TRIGGER);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_003 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_003 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_004";
    MockSyncStateManager mockSyncStateManager;
    try {
        EXPECT_CALL(mockSyncStateManager, GetStopSyncFlag())
            .WillOnce(Return(true))
            .WillOnce(Return(false));
        ASSERT_TRUE(mockSyncStateManager.GetStopSyncFlag());
        ASSERT_FALSE(mockSyncStateManager.GetStopSyncFlag());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_004 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_004 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_005";
    MockSyncStateManager mockSyncStateManager;
    try {
        EXPECT_CALL(mockSyncStateManager, SetStopSyncFlag());
        mockSyncStateManager.SetStopSyncFlag();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_005 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_005 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_006";
    MockSyncStateManager mockSyncStateManager;
    try {
        EXPECT_CALL(mockSyncStateManager, SetDisableCloudFlag());
        mockSyncStateManager.SetDisableCloudFlag();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_006 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_006 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_007";
    MockSyncStateManager mockSyncStateManager;
    try {
        EXPECT_CALL(mockSyncStateManager, GetSyncState());
        mockSyncStateManager.GetSyncState();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_007 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_007 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_008";
    MockSyncStateManager mockSyncStateManager;
    try {
        EXPECT_CALL(mockSyncStateManager, GetForceFlag());
        mockSyncStateManager.GetForceFlag();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_008 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_008 End";
}
}