/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "setting_data_helper.h"
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

/**
 * @tc.name: OnAddSystemAbilityTest002
 * @tc.desc: Verify the OnAddSystemAbility function
 * @tc.type: FUNC
 */
HWTEST_F(CloudDaemonTest, OnAddSystemAbilityTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAddSystemAbilityTest002 start";
    try {
        const int32_t systemAbilityId = DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID;
        const std::string deviceId = "device_test";
        SettingDataHelper::GetInstance().isBundleInited_ = true;
        cloudDaemon_->OnAddSystemAbility(systemAbilityId, deviceId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnAddSystemAbilityTest002 failed";
    }
    GTEST_LOG_(INFO) << "OnAddSystemAbilityTest002 end";
}
} // namespace OHOS::FileManagement::CloudSync::Test