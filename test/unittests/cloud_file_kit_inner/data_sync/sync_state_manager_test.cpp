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
        EXPECT_CALL(mockSyncStateManager, UpdateSyncState(SyncState::SYNC_FAILED)).Times(1);
        EXPECT_CALL(mockSyncStateManager, UpdateSyncState(SyncState::SYNC_SUCCEED)).Times(1);
        EXPECT_CALL(mockSyncStateManager, SetCleaningFlag()).Times(1);
        EXPECT_CALL(mockSyncStateManager, ClearCleaningFlag()).Times(1);
        EXPECT_CALL(mockSyncStateManager, SetDisableCloudFlag).Times(1);
        EXPECT_CALL(mockSyncStateManager, ClearDisableCloudFlag).Times(1);

        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::INIT), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::SYNCING), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::SYNC_FAILED), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.UpdateSyncState(SyncState::SYNC_SUCCEED), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.ClearCleaningFlag(), Action::STOP);
        EXPECT_EQ(mockSyncStateManager.ClearDisableCloudFlag(), Action::STOP);
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

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_009";
    try {
        SyncStateManager syncStateManager;
        int32_t ret = (int32_t)syncStateManager.UpdateSyncState(SyncState::INIT);
        EXPECT_EQ(ret, 0);
        ret = (int32_t)syncStateManager.UpdateSyncState(SyncState::SYNCING);
        EXPECT_EQ(ret, 0);
        ret = (int32_t)syncStateManager.UpdateSyncState(SyncState::SYNC_FAILED);
        EXPECT_EQ(ret, 0);
        ret = (int32_t)syncStateManager.UpdateSyncState(SyncState::SYNC_SUCCEED);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_009 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_009 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_010";
    try {
        SyncStateManager syncStateManager;
        bool forceFlag = true;
        SyncState syncState = syncStateManager.GetSyncState();
        EXPECT_EQ(static_cast<int32_t>(syncState), 0);
        bool ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::APP_TRIGGER);
        EXPECT_FALSE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_010 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_010 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_011";
    try {
        SyncStateManager syncStateManager;
        bool forceFlag = true;

        Action action = syncStateManager.UpdateSyncState(SyncState::SYNCING);
        EXPECT_EQ(static_cast<int32_t>(action), static_cast<int32_t>(Action::STOP));
        SyncState syncState = syncStateManager.GetSyncState();
        EXPECT_EQ(static_cast<int32_t>(syncState), static_cast<int32_t>(SyncState::SYNCING));

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret);
        syncStateManager.SetCleaningFlag();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_011 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_011 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_012";
    try {
        SyncStateManager syncStateManager;
        bool forceFlag = false;

        Action action = syncStateManager.UpdateSyncState(SyncState::SYNCING);
        EXPECT_EQ(static_cast<int32_t>(action), static_cast<int32_t>(Action::STOP));
        SyncState syncState = syncStateManager.GetSyncState();
        EXPECT_EQ(static_cast<int32_t>(syncState), static_cast<int32_t>(SyncState::SYNCING));
        
        bool ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::TASK_TRIGGER);
        EXPECT_TRUE(ret);

        ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret);

        forceFlag = true;
        ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret);
        syncStateManager.SetCleaningFlag();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_012 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_012 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_013";
    try {
        SyncStateManager syncStateManager;
        bool forceFlag = false;

        syncStateManager.SetCleaningFlag();
        EXPECT_TRUE(syncStateManager.CheckCleaningFlag());
        
        bool ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::TASK_TRIGGER);
        EXPECT_TRUE(ret);

        ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret);

        forceFlag = true;
        ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret);
        syncStateManager.SetCleaningFlag();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_013 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_013 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_014";
    try {
        SyncStateManager syncStateManager;
        bool forceFlag = false;

        syncStateManager.SetDisableCloudFlag();
        EXPECT_TRUE(syncStateManager.CheckDisableCloudFlag());
        
        bool ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::TASK_TRIGGER);
        EXPECT_TRUE(ret);

        ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret);

        forceFlag = true;
        ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret);
        syncStateManager.SetCleaningFlag();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_014 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_014 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_015";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_015 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_015 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_016";
    try {
        SyncStateManager syncStateManager;
        EXPECT_FALSE(syncStateManager.GetStopSyncFlag());
        syncStateManager.SetStopSyncFlag();
        EXPECT_TRUE(syncStateManager.GetStopSyncFlag());

        Action action = syncStateManager.UpdateSyncState(SyncState::INIT);
        EXPECT_EQ(static_cast<int32_t>(action), static_cast<int32_t>(Action::STOP));
        EXPECT_FALSE(syncStateManager.GetStopSyncFlag());

        SyncState syncState = syncStateManager.GetSyncState();
        EXPECT_EQ(static_cast<int32_t>(syncState), static_cast<int32_t>(SyncState::INIT));
        syncStateManager.SetStopSyncFlag();
        EXPECT_TRUE(syncStateManager.GetStopSyncFlag());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_016 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_016 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_017";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetStopSyncFlag();
        EXPECT_TRUE(syncStateManager.GetStopSyncFlag());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_017 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_017 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_018, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_018";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetDisableCloudFlag();
        EXPECT_TRUE(syncStateManager.CheckDisableCloudFlag());

        syncStateManager.SetCleaningFlag();
        EXPECT_TRUE(syncStateManager.CheckCleaningFlag());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_018 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_018 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_019, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_019";
    try {
        SyncStateManager syncStateManager;
        SyncState syncState;
        bool forceFlag = true;
        EXPECT_FALSE(syncStateManager.GetForceFlag());

        Action action = syncStateManager.ClearDisableCloudFlag();
        EXPECT_EQ(static_cast<int32_t>(action), static_cast<int32_t>(Action::STOP));
        EXPECT_TRUE(!syncStateManager.CheckDisableCloudFlag())

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, SyncTriggerType::TASK_TRIGGER);
        syncState = syncStateManager.GetSyncState();
        EXPECT_EQ(static_cast<int32_t>(syncState), static_cast<int32_t>(SyncState::SYNCING));
        EXPECT_FALSE(ret);
        EXPECT_TRUE(syncStateManager.GetForceFlag());
        
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_019 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_019 End";
}
}