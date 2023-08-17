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

#include "dfs_error.h"
#include "ipc/daemon.h"
#include "iremote_object.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DaemonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<Daemon> daemon_;
};

void DaemonTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DaemonTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DaemonTest::SetUp(void)
{
    int32_t saID = FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID;
    bool runOnCreate = true;
    daemon_ = std::make_shared<Daemon>(saID, runOnCreate);
    GTEST_LOG_(INFO) << "SetUp";
}

void DaemonTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: OnStopTest
 * @tc.desc: Verify the OnStop function
 * @tc.type: FUNC
 * @tc.require: issueI7M6L1
 */
HWTEST_F(DaemonTest, OnStopTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStop Start";
    try {
        daemon_->state_ = ServiceRunningState::STATE_NOT_START;
        daemon_->registerToService_ = false;
        daemon_->OnStop();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStop  ERROR";
    }
    GTEST_LOG_(INFO) << "OnStop End";
}

/**
 * @tc.name: DaemonTest_PublishSA_0100
 * @tc.desc: Verify the PublishSA function.
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DaemonTest, DaemonTest_PublishSA_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_PublishSA_0100 start";
    try {
        daemon_->registerToService_ = false;
        daemon_->PublishSA();
        EXPECT_TRUE(daemon_->registerToService_);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "DaemonTest_PublishSA_0100 end";
}

/**
 * @tc.name: DaemonTest_OnStart_0100
 * @tc.desc: Verify the OnStart function.
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DaemonTest, DaemonTest_OnStart_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnStart_0100 start";
    try {
        daemon_->state_ = ServiceRunningState::STATE_RUNNING;
        daemon_->OnStart();
        EXPECT_EQ(daemon_->state_, ServiceRunningState::STATE_RUNNING);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "DaemonTest_OnStart_0100 end";
}

/**
 * @tc.name: DaemonTest_OnRemoveSystemAbility_0100
 * @tc.desc: Verify the OnRemoveSystemAbility function.
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DaemonTest, DaemonTest_OnRemoveSystemAbility_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnRemoveSystemAbility_0100 start";
    try {
        daemon_->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID + 1, 0);
        EXPECT_TRUE(true);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "DaemonTest_OnRemoveSystemAbility_0100 end";
}

/**
 * @tc.name: DaemonTest_OnRemoveSystemAbility_0200
 * @tc.desc: Verify the OnRemoveSystemAbility function.
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DaemonTest, DaemonTest_OnRemoveSystemAbility_0200, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DaemonTest_OnRemoveSystemAbility_0200 start";
    try {
        daemon_->subScriber_ = nullptr;
        daemon_->OnRemoveSystemAbility(COMMON_EVENT_SERVICE_ID, 0);
        EXPECT_TRUE(true);
    } catch (const exception &e) {
        LOGE("Error:%{public}s", e.what());
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "DaemonTest_OnRemoveSystemAbility_0200 end";
}
} // namespace OHOS::Storage::DistributedFile::Test