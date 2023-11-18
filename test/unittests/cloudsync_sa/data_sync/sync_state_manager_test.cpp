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

#include "sync_state_manager.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

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

/**
 * @tc.name: UpdateSyncState
 * @tc.desc: Verify the UpdateSyncState function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SyncStateManagerTest, UpdateSyncState, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateSyncState Start";
    SyncState newState = SyncState::SYNCING;
    SyncStateManager syncStateMgr;
    syncStateMgr.UpdateSyncState(newState);
    EXPECT_EQ(syncStateMgr.state_, newState);
    GTEST_LOG_(INFO) << "UpdateSyncState End";
}

/**
 * @tc.name: CheckAndSetPending001
 * @tc.desc: Verify the CheckAndSetPending function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SyncStateManagerTest, CheckAndSetPending001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPending001 Start";
    SyncStateManager syncStateMgr;
    syncStateMgr.state_ = SyncState::SYNC_FAILED;
    auto ret = syncStateMgr.CheckAndSetPending(false, SyncTriggerType::CLOUD_TRIGGER);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "CheckAndSetPending001 End";
}

/**
 * @tc.name: CheckAndSetPending002
 * @tc.desc: Verify the CheckAndSetPending function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SyncStateManagerTest, CheckAndSetPending002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPending002 Start";
    SyncStateManager syncStateMgr;
    syncStateMgr.state_ = SyncState::SYNCING;
    auto ret = syncStateMgr.CheckAndSetPending(false, SyncTriggerType::CLOUD_TRIGGER);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "CheckAndSetPending002 End";
}

/**
 * @tc.name: CheckAndSetPending003
 * @tc.desc: Verify the CheckAndSetPending function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SyncStateManagerTest, CheckAndSetPending003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckAndSetPending003 Start";
    SyncStateManager syncStateMgr;
    syncStateMgr.state_ = SyncState::SYNCING;
    auto ret = syncStateMgr.CheckAndSetPending(true, SyncTriggerType::CLOUD_TRIGGER);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "CheckAndSetPending003 End";
}

/**
 * @tc.name: GetStopSyncFlag
 * @tc.desc: Verify the GetStopSyncFlag function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SyncStateManagerTest, GetStopSyncFlag, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetStopSyncFlag Start";
    SyncStateManager syncStateMgr;
    auto ret = syncStateMgr.GetStopSyncFlag();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "GetStopSyncFlag End";
}

/**
 * @tc.name: SetStopSyncFlag
 * @tc.desc: Verify the SetStopSyncFlag function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SyncStateManagerTest, SetStopSyncFlag, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetStopSyncFlag Start";
    SyncStateManager syncStateMgr;
    syncStateMgr.SetStopSyncFlag();
    EXPECT_EQ(syncStateMgr.stopSyncFlag_, true);
    GTEST_LOG_(INFO) << "SetStopSyncFlag End";
}

/**
 * @tc.name: GetSyncState
 * @tc.desc: Verify the GetSyncState function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SyncStateManagerTest, GetSyncState, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncState Start";
    SyncStateManager syncStateMgr;
    auto ret = syncStateMgr.GetSyncState();
    EXPECT_EQ(ret, SyncState::INIT);
    GTEST_LOG_(INFO) << "GetSyncState End";
}

/**
 * @tc.name: GetForceFlag
 * @tc.desc: Verify the GetForceFlag function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(SyncStateManagerTest, GetForceFlag, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetForceFlag Start";
    SyncStateManager syncStateMgr;
    auto ret = syncStateMgr.GetForceFlag();
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "GetForceFlag End";
}
} // namespace OHOS::FileManagement::CloudSync::Test