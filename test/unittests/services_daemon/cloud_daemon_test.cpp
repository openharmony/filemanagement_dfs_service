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

#include "ipc/cloud_daemon.h"

#include "iremote_object.h"
#include "system_ability_definition.h"

#include "dfs_error.h"
#include "fuse_manager/fuse_manager.h"
#include "utils_log.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDaemonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<CloudDaemon> cloudDaemon_;
};

void CloudDaemonTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDaemonTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDaemonTest::SetUp(void)
{
    int32_t saID = FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID;
    bool runOnCreate = true;
    cloudDaemon_ = std::make_shared<CloudDaemon>(saID, runOnCreate);
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDaemonTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: OnStopTest
 * @tc.desc: Verify the OnStop function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonTest, OnStopTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStop Start";
    try {
        cloudDaemon_->state_ = ServiceRunningState::STATE_NOT_START;
        cloudDaemon_->registerToService_ = false;
        cloudDaemon_->OnStop();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStop  ERROR";
    }
    GTEST_LOG_(INFO) << "OnStop End";
}

/**
 * @tc.name: OnStartTest1
 * @tc.desc: Verify the OnStart function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonTest, OnStartTest1, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStart1 Start";
    try {
        cloudDaemon_->state_ = ServiceRunningState::STATE_RUNNING;
        cloudDaemon_->OnStart();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStart1  ERROR";
    }
    GTEST_LOG_(INFO) << "OnStart1 End";
}

/**
 * @tc.name: OnStartTest2
 * @tc.desc: Verify the OnStart function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDaemonTest, OnStartTest2, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnStart2 Start";
    try {
        cloudDaemon_->state_ = ServiceRunningState::STATE_NOT_START;
        cloudDaemon_->OnStart();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStart2  ERROR";
    }
    GTEST_LOG_(INFO) << "OnStart2 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test