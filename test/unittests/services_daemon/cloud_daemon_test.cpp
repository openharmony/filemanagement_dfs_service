/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "fuse_assistant.h"
#include "fuse_manager/fuse_manager.h"
#include "ipc/cloud_daemon.h"
#include "iremote_object.h"
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
constexpr int32_t USER_ID = 100;
constexpr int32_t DEV_FD = 10;

class CloudDaemonTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<CloudDaemon> cloudDaemon_;
    static inline shared_ptr<FuseAssistantMock> insMock_ = nullptr;
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
    insMock_ = make_shared<FuseAssistantMock>();
    FuseAssistantMock::ins = insMock_;
    cloudDaemon_ = std::make_shared<CloudDaemon>(saID, runOnCreate);
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDaemonTest::TearDown(void)
{
    FuseAssistantMock::ins = nullptr;
    insMock_ = nullptr;
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
        cloudDaemon_->registerToService_ = true;
        cloudDaemon_->OnStart();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnStart2  ERROR";
    }
    GTEST_LOG_(INFO) << "OnStart2 End";
}

/**
 * @tc.name: StartFuseTest001
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(CloudDaemonTest, StartFuseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest001 start";
    std::cout << "StartFuseTest001 Start" << std::endl;
    try {
        const char* path = "/cloud/test";
        int32_t ret = cloudDaemon_->StartFuse(USER_ID, DEV_FD, path);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest001 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest001 end";
}

/**
 * @tc.name: StartFuseTest002
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(CloudDaemonTest, StartFuseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest002 start";
    try {
        std::string path = "/cloud/test";
        EXPECT_CALL(*insMock_, mkdir(_, _)).WillRepeatedly(Return(-1));
        int32_t ret = cloudDaemon_->StartFuse(USER_ID, DEV_FD, path);
        EXPECT_EQ(ret, E_PATH);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest002 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest002 end";
}

/**
 * @tc.name: StartFuseTest003
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(CloudDaemonTest, StartFuseTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest003 start";
    try {
        std::string path = "/cloud/test";
        EXPECT_CALL(*insMock_, mkdir(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, chmod(_, _)).WillRepeatedly(Return(-1));
        int32_t ret = cloudDaemon_->StartFuse(USER_ID, DEV_FD, path);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest003 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest003 end";
}

/**
 * @tc.name: StartFuseTest004
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(CloudDaemonTest, StartFuseTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest004 start";
    try {
        std::string path = "/cloud/test";
        EXPECT_CALL(*insMock_, mkdir(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, chown(_, _, _)).WillRepeatedly(Return(-1));
        int32_t ret = cloudDaemon_->StartFuse(USER_ID, DEV_FD, path);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest004 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest004 end";
}

/**
 * @tc.name: StartFuseTest007
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(CloudDaemonTest, StartFuseTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest007 start";
    try {
        std::string path = "/cloud_fuse/test";
        EXPECT_CALL(*insMock_, mkdir(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock_, HandleStartMove(_)).WillRepeatedly(Return());
        int32_t ret = cloudDaemon_->StartFuse(USER_ID, DEV_FD, path);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest007 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest007 end";
}

/**
 * @tc.name: OnAddSystemAbilityTest001
 * @tc.desc: Verify the OnStart function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(CloudDaemonTest, OnAddSystemAbility001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAddSystemAbility001 start";
    try {
        const int32_t systemAbilityId = 100;
        const std::string deviceId = "device_test";
        cloudDaemon_->OnAddSystemAbility(systemAbilityId, deviceId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnAddSystemAbilityTest001 failed";
    }
    GTEST_LOG_(INFO) << "OnAddSystemAbilityTest001 end";
}
} // namespace OHOS::FileManagement::CloudSync::Test