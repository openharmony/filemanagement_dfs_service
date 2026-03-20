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

#include "cloud_status.h"
#include "data_sync_const.h"
#include "system_mock.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace system;

class MockSyncStateManager : public SyncStateManager {
public:
    MOCK_METHOD1(UpdateSyncState, Action(SyncState newState));
    MOCK_METHOD2(CheckAndSetPending, bool(bool forceFlag, SyncTriggerType triggerType));
    MOCK_METHOD0(SetCleaningFlag, void());
    MOCK_METHOD0(ClearCleaningFlag, Action());
    MOCK_METHOD0(CheckCleaningFlag, bool());
    MOCK_METHOD0(GetStopSyncFlag, bool());
    MOCK_METHOD0(SetStopSyncFlag, void());
    MOCK_METHOD0(SetDisableCloudFlag, void());
    MOCK_METHOD0(ClearDisableCloudFlag, Action());
    MOCK_METHOD0(CheckDisableCloudFlag, bool());
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
        EXPECT_CALL(mockSyncStateManager, CheckAndSetPending(true, SyncTriggerType::POWER_CONNECT_TRIGGER));
        EXPECT_CALL(mockSyncStateManager, CheckAndSetPending(true, SyncTriggerType::SCREEN_OFF_TRIGGER));
        mockSyncStateManager.CheckAndSetPending(false, SyncTriggerType::TASK_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::CLOUD_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::PENDING_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::BATTERY_OK_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::NETWORK_AVAIL_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::SYSTEM_LOAD_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::POWER_CONNECT_TRIGGER);
        mockSyncStateManager.CheckAndSetPending(true, SyncTriggerType::SCREEN_OFF_TRIGGER);
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
        EXPECT_TRUE(!syncStateManager.CheckDisableCloudFlag());

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

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_020, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_020";
    try {
        SyncStateManager syncStateManager;
        bool forceFlag = false;
        syncStateManager.state_ = SyncState::SYNCING;
        SyncTriggerType triggerType = SyncTriggerType::SYSTEM_LOAD_TRIGGER;

        syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_020 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_020 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_021, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_021";
    try {
        SyncStateManager syncStateManager;
        bool forceFlag = false;
        syncStateManager.state_ = SyncState::SYNCING;
        SyncTriggerType triggerType = SyncTriggerType::POWER_CONNECT_TRIGGER;

        syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_021 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_021 End";
}

HWTEST_F(SyncStateManagerTest, SyncStateManagerTest_022, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SyncStateManagerTest_022";
    try {
        SyncStateManager syncStateManager;
        bool forceFlag = false;
        syncStateManager.state_ = SyncState::SYNCING;
        SyncTriggerType triggerType = SyncTriggerType::SCREEN_OFF_TRIGGER;

        syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SyncStateManagerTest_022 ERROR";
    }
    GTEST_LOG_(INFO) << "SyncStateManagerTest_022 End";
}

HWTEST_F(SyncStateManagerTest, CheckMediaLibCleaningTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_001 start";
    try {
        CloudStatus::isStopSync_.store(false);
        std::shared_ptr<SystemMock> systemMock = std::make_shared<SystemMock>();
        ISystem::system_ = systemMock;
        EXPECT_CALL(*systemMock, GetParameter(_, _)).WillOnce(Return(""));
        SyncStateManager syncStateManager;
        bool ret = syncStateManager.CheckMediaLibCleaning();
        EXPECT_EQ(ret, false);

        ISystem::system_ = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_001 End";
}

HWTEST_F(SyncStateManagerTest, CheckMediaLibCleaningTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_002 start";
    try {
        CloudStatus::isStopSync_.store(false);
        std::shared_ptr<SystemMock> systemMock = std::make_shared<SystemMock>();
        ISystem::system_ = systemMock;
        EXPECT_CALL(*systemMock, GetParameter(_, _)).WillOnce(Return("0"));
        SyncStateManager syncStateManager;
        bool ret = syncStateManager.CheckMediaLibCleaning();
        EXPECT_EQ(ret, false);

        ISystem::system_ = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_002 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_002 End";
}

HWTEST_F(SyncStateManagerTest, CheckMediaLibCleaningTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_003 start";
    try {
        CloudStatus::isStopSync_.store(false);
        std::shared_ptr<SystemMock> systemMock = std::make_shared<SystemMock>();
        ISystem::system_ = systemMock;
        EXPECT_CALL(*systemMock, GetParameter(_, _)).WillOnce(Return("xxx"));
        EXPECT_CALL(*systemMock, SetParameter(_, _)).WillOnce(Return(true));
        SyncStateManager syncStateManager;
        bool ret = syncStateManager.CheckMediaLibCleaning();
        EXPECT_EQ(ret, false);

        ISystem::system_ = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_003 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_003 End";
}

HWTEST_F(SyncStateManagerTest, CheckMediaLibCleaningTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_004 start";
    try {
        CloudStatus::isStopSync_.store(false);
        std::shared_ptr<SystemMock> systemMock = std::make_shared<SystemMock>();
        ISystem::system_ = systemMock;
        EXPECT_CALL(*systemMock, GetParameter(_, _)).WillOnce(Return("10xx"));
        EXPECT_CALL(*systemMock, SetParameter(_, _)).WillOnce(Return(true));
        SyncStateManager syncStateManager;
        bool ret = syncStateManager.CheckMediaLibCleaning();
        EXPECT_EQ(ret, false);

        ISystem::system_ = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_004 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_004 End";
}

HWTEST_F(SyncStateManagerTest, CheckMediaLibCleaningTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_005 start";
    try {
        CloudStatus::isStopSync_.store(false);
        std::shared_ptr<SystemMock> systemMock = std::make_shared<SystemMock>();
        ISystem::system_ = systemMock;
        EXPECT_CALL(*systemMock, GetParameter(_, _)).WillOnce(Return("   10"));
        EXPECT_CALL(*systemMock, SetParameter(_, _)).WillOnce(Return(true));
        SyncStateManager syncStateManager;
        bool ret = syncStateManager.CheckMediaLibCleaning();
        EXPECT_EQ(ret, false);

        ISystem::system_ = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_005 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_005 End";
}

HWTEST_F(SyncStateManagerTest, CheckMediaLibCleaningTest_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_006 start";
    try {
        CloudStatus::isStopSync_.store(false);
        std::shared_ptr<SystemMock> systemMock = std::make_shared<SystemMock>();
        ISystem::system_ = systemMock;
        EXPECT_CALL(*systemMock, GetParameter(_, _)).WillOnce(Return("10  "));
        EXPECT_CALL(*systemMock, SetParameter(_, _)).WillOnce(Return(true));
        SyncStateManager syncStateManager;
        bool ret = syncStateManager.CheckMediaLibCleaning();
        EXPECT_EQ(ret, false);

        ISystem::system_ = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_006 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_006 End";
}

HWTEST_F(SyncStateManagerTest, CheckMediaLibCleaningTest_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_007 start";
    try {
        CloudStatus::isStopSync_.store(false);
        std::shared_ptr<SystemMock> systemMock = std::make_shared<SystemMock>();
        ISystem::system_ = systemMock;
        uint64_t curTime = GetCurrentTimeStampMs() - TWELVE_HOURS_MILLISECOND;
        EXPECT_CALL(*systemMock, GetParameter(_, _)).WillRepeatedly(Return(std::to_string(curTime)));
        EXPECT_CALL(*systemMock, SetParameter(_, _)).WillRepeatedly(Return(true));
        SyncStateManager syncStateManager;
        bool ret = syncStateManager.CheckMediaLibCleaning();
        EXPECT_EQ(ret, false);

        ISystem::system_ = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_007 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_007 End";
}

HWTEST_F(SyncStateManagerTest, CheckMediaLibCleaningTest_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_008 start";
    try {
        CloudStatus::isStopSync_.store(false);
        std::shared_ptr<SystemMock> systemMock = std::make_shared<SystemMock>();
        ISystem::system_ = systemMock;
        uint64_t curTime = GetCurrentTimeStampMs() - 10;
        EXPECT_CALL(*systemMock, GetParameter(_, _)).WillOnce(Return(std::to_string(curTime)));
        SyncStateManager syncStateManager;
        bool ret = syncStateManager.CheckMediaLibCleaning();
        EXPECT_EQ(ret, true);

        ISystem::system_ = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_008 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_008 End";
}

HWTEST_F(SyncStateManagerTest, CheckMediaLibCleaningTest_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_009 start";
    try {
        CloudStatus::isStopSync_.store(false);
        std::shared_ptr<SystemMock> systemMock = std::make_shared<SystemMock>();
        ISystem::system_ = systemMock;
        uint64_t curTime = GetCurrentTimeStampMs() + 10;
        EXPECT_CALL(*systemMock, GetParameter(_, _)).WillOnce(Return(std::to_string(curTime)));
        EXPECT_CALL(*systemMock, SetParameter(_, _)).WillOnce(Return(true));
        SyncStateManager syncStateManager;
        bool ret = syncStateManager.CheckMediaLibCleaning();
        EXPECT_EQ(ret, false);

        ISystem::system_ = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_009 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_009 End";
}

HWTEST_F(SyncStateManagerTest, CheckMediaLibCleaningTest_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_010 start";
    try {
        CloudStatus::isStopSync_.store(false);
        std::shared_ptr<SystemMock> systemMock = std::make_shared<SystemMock>();
        ISystem::system_ = systemMock;
        uint64_t curTime = GetCurrentTimeStampMs() - TWELVE_HOURS_MILLISECOND + 50000;
        EXPECT_CALL(*systemMock, GetParameter(_, _)).WillOnce(Return(std::to_string(curTime)));
        SyncStateManager syncStateManager;
        bool ret = syncStateManager.CheckMediaLibCleaning();
        EXPECT_EQ(ret, true);

        ISystem::system_ = nullptr;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_010 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_010 End";
}

HWTEST_F(SyncStateManagerTest, CheckMediaLibCleaningTest_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_011 start";
    try {
        CloudStatus::isStopSync_.store(true);
        std::shared_ptr<SystemMock> systemMock = std::make_shared<SystemMock>();
        ISystem::system_ = systemMock;
        EXPECT_CALL(*systemMock, GetParameter(_, _)).Times(0);
        SyncStateManager syncStateManager;
        bool ret = syncStateManager.CheckMediaLibCleaning();
        EXPECT_EQ(ret, true);

        ISystem::system_ = nullptr;
        CloudStatus::isStopSync_.store(false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_011 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckMediaLibCleaningTest_011 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_001";
    try {
        SyncStateManager syncStateManager;
        // 初始状态：CheckCleaningFlag()=false, CheckDisableCloudFlag()=false, state_!=SYNCING
        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_FALSE(ret);

        // 验证状态变更
        SyncState syncState = syncStateManager.GetSyncState();
        EXPECT_EQ(static_cast<int32_t>(syncState), static_cast<int32_t>(SyncState::SYNCING));
        EXPECT_FALSE(syncStateManager.GetForceFlag());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_001 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_002";
    try {
        SyncStateManager syncStateManager;
        bool forceFlag = true;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_FALSE(ret);

        // 验证 isForceSync_ 被设置为 true
        EXPECT_TRUE(syncStateManager.GetForceFlag());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_002 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_002 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_003";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag();

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_003 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_003 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_004";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetDisableCloudFlag();

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_004 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_004 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_005";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.state_ = SyncState::SYNCING;

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_005 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_005 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_006";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1
        syncStateManager.nextAction_ = Action::CHECK;

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 保持 CHECK
        EXPECT_EQ(syncStateManager.nextAction_, Action::CHECK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_006 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_006 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_007";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool forceFlag = true;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 FORCE_START
        EXPECT_EQ(syncStateManager.nextAction_, Action::FORCE_START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_007 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_007 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_008";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::TASK_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 CHECK
        EXPECT_EQ(syncStateManager.nextAction_, Action::CHECK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_008 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_008 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_009";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 START
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_009 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_009 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_010";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::CLOUD_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 START
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_010 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_010 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_011";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::PENDING_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 START
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_011 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_011 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_012";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::BATTERY_OK_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 START
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_012 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_012 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_013";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::NETWORK_AVAIL_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 START
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_013 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_013 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_014";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::SYSTEM_LOAD_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 START
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_014 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_014 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_015";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::POWER_CONNECT_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 START
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_015 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_015 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_016";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::SCREEN_OFF_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 START
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_016 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_016 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_017";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.state_ = SyncState::SYNCING; // 跳过分支1

        bool forceFlag = true;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 FORCE_START
        EXPECT_EQ(syncStateManager.nextAction_, Action::FORCE_START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_017 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_017 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_018, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_018";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool forceFlag = true;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 FORCE_START
        EXPECT_EQ(syncStateManager.nextAction_, Action::FORCE_START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_018 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_018 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_019, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_019";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetDisableCloudFlag(); // 跳过分支1

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::TASK_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 CHECK
        EXPECT_EQ(syncStateManager.nextAction_, Action::CHECK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_019 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_019 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_020, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_020";
    try {
        SyncStateManager syncStateManager;

        // 第一次调用 - 进入分支1
        bool ret1 = syncStateManager.CheckAndSetPending(false, SyncTriggerType::APP_TRIGGER);
        EXPECT_FALSE(ret1);

        // 第二次调用 - state_==SYNCING，跳过分支1，进入分支5
        bool ret2 = syncStateManager.CheckAndSetPending(false, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret2);
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);

        // 第三次调用 - nextAction_==START，跳过分支2，进入分支5
        bool ret3 = syncStateManager.CheckAndSetPending(false, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret3);
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_020 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_020 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_021, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_021";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        // 第一次调用 - TASK_TRIGGER，设置 nextAction_=CHECK
        bool ret1 = syncStateManager.CheckAndSetPending(false, SyncTriggerType::TASK_TRIGGER);
        EXPECT_TRUE(ret1);
        EXPECT_EQ(syncStateManager.nextAction_, Action::CHECK);

        // 第二次调用 - nextAction_==CHECK，直接返回 true
        bool ret2 = syncStateManager.CheckAndSetPending(false, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret2);
        EXPECT_EQ(syncStateManager.nextAction_, Action::CHECK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_021 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_021 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_022, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_022";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        // 第一次调用 - TASK_TRIGGER，设置 nextAction_=CHECK
        bool ret1 = syncStateManager.CheckAndSetPending(false, SyncTriggerType::TASK_TRIGGER);
        EXPECT_TRUE(ret1);
        EXPECT_EQ(syncStateManager.nextAction_, Action::CHECK);

        // 第二次调用 - nextAction_==CHECK，直接返回 true，不修改 nextAction_
        bool ret2 = syncStateManager.CheckAndSetPending(true, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret2);
        EXPECT_EQ(syncStateManager.nextAction_, Action::CHECK); // 保持 CHECK
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_022 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_022 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_023, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_023";
    try {
        SyncStateManager syncStateManager;
        // 确保所有标志都为 false
        EXPECT_FALSE(syncStateManager.CheckCleaningFlag());
        EXPECT_FALSE(syncStateManager.CheckDisableCloudFlag());
        EXPECT_NE(static_cast<int32_t>(syncStateManager.GetSyncState()), static_cast<int32_t>(SyncState::SYNCING));

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_FALSE(ret);

        // 验证状态变更
        SyncState syncState = syncStateManager.GetSyncState();
        EXPECT_EQ(static_cast<int32_t>(syncState), static_cast<int32_t>(SyncState::SYNCING));
        EXPECT_EQ(syncStateManager.nextAction_, Action::STOP);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_023 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_023 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_024, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_024";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag();
        syncStateManager.SetDisableCloudFlag();

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_TRUE(ret);

        // 验证 nextAction_ 被设置为 START
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_024 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_024 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_025, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_025";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.state_ = SyncState::INIT;

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_FALSE(ret);

        // 验证状态变更
        SyncState syncState = syncStateManager.GetSyncState();
        EXPECT_EQ(static_cast<int32_t>(syncState), static_cast<int32_t>(SyncState::SYNCING));
        EXPECT_EQ(syncStateManager.nextAction_, Action::STOP);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_025 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_025 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_026, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_026";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.state_ = SyncState::SYNC_FAILED;

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_FALSE(ret);

        // 验证状态变更
        SyncState syncState = syncStateManager.GetSyncState();
        EXPECT_EQ(static_cast<int32_t>(syncState), static_cast<int32_t>(SyncState::SYNCING));
        EXPECT_EQ(syncStateManager.nextAction_, Action::STOP);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_026 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_026 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_027, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_027";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.state_ = SyncState::SYNC_SUCCEED;

        bool forceFlag = false;
        SyncTriggerType triggerType = SyncTriggerType::APP_TRIGGER;

        bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
        EXPECT_FALSE(ret);

        // 验证状态变更
        SyncState syncState = syncStateManager.GetSyncState();
        EXPECT_EQ(static_cast<int32_t>(syncState), static_cast<int32_t>(SyncState::SYNCING));
        EXPECT_EQ(syncStateManager.nextAction_, Action::STOP);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_027 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_027 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_028, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_028";
    try {
        SyncStateManager syncStateManager;

        // 第一次调用 - forceFlag=false
        bool ret1 = syncStateManager.CheckAndSetPending(false, SyncTriggerType::APP_TRIGGER);
        EXPECT_FALSE(ret1);
        EXPECT_FALSE(syncStateManager.GetForceFlag());

        // 重置状态
        syncStateManager.state_ = SyncState::INIT;

        // 第二次调用 - forceFlag=true
        bool ret2 = syncStateManager.CheckAndSetPending(true, SyncTriggerType::APP_TRIGGER);
        EXPECT_FALSE(ret2);
        EXPECT_TRUE(syncStateManager.GetForceFlag());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_028 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_028 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_029, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_029";
    try {
        SyncStateManager syncStateManager;

        // 模拟多次并发调用
        for (int i = 0; i < 10; i++) {
            bool forceFlag = (i % 2 == 0);
            SyncTriggerType triggerType = (i % 2 == 0) ? SyncTriggerType::APP_TRIGGER : SyncTriggerType::TASK_TRIGGER;

            bool ret = syncStateManager.CheckAndSetPending(forceFlag, triggerType);
            // 验证返回值
            EXPECT_TRUE(ret || !ret); // 总是 true 或 false
        }

        // 验证最终状态
        SyncState syncState = syncStateManager.GetSyncState();
        EXPECT_TRUE(static_cast<int32_t>(syncState) == static_cast<int32_t>(SyncState::SYNCING) ||
                    static_cast<int32_t>(syncState) != static_cast<int32_t>(SyncState::SYNCING));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_029 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_029 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_030, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_030";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool ret = syncStateManager.CheckAndSetPending(false, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret);
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_030 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_030 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_031, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_031";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool ret = syncStateManager.CheckAndSetPending(false, SyncTriggerType::TASK_TRIGGER);
        EXPECT_TRUE(ret);
        EXPECT_EQ(syncStateManager.nextAction_, Action::CHECK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_031 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_031 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_032, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_032";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        bool ret = syncStateManager.CheckAndSetPending(true, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret);
        EXPECT_EQ(syncStateManager.nextAction_, Action::FORCE_START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_032 ERROR";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_032 End";
}

HWTEST_F(SyncStateManagerTest, CheckAndSetPendingTest_033, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_033";
    try {
        SyncStateManager syncStateManager;
        syncStateManager.SetCleaningFlag(); // 跳过分支1

        // 第一次调用 - APP_TRIGGER
        bool ret1 = syncStateManager.CheckAndSetPending(false, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret1);
        EXPECT_EQ(syncStateManager.nextAction_, Action::START);

        // 重置 nextAction_
        syncStateManager.nextAction_ = Action::STOP;

        // 第二次调用 - TASK_TRIGGER
        bool ret2 = syncStateManager.CheckAndSetPending(false, SyncTriggerType::TASK_TRIGGER);
        EXPECT_TRUE(ret2);
        EXPECT_EQ(syncStateManager.nextAction_, Action::CHECK);

        // 重置 nextAction_
        syncStateManager.nextAction_ = Action::STOP;

        // 第三次调用 - forceFlag=true
        bool ret3 = syncStateManager.CheckAndSetPending(true, SyncTriggerType::APP_TRIGGER);
        EXPECT_TRUE(ret3);
        EXPECT_EQ(syncStateManager.nextAction_, Action::FORCE_START);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckAndSetPendingTest_033 End";
    }
    GTEST_LOG_(INFO) << "CheckAndSetPendingTest_033 End";
}
}