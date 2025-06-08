/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cloud_sync_manager_core.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "uri.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudSync;

class CloudSyncManagerCoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncManagerCoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudSyncManagerCoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudSyncManagerCoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudSyncManagerCoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: DoChangeAppCloudSwitch
 * @tc.desc: Verify the CloudSyncManagerCore::DoChangeAppCloudSwitch function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncManagerCoreTest, DoChangeAppCloudSwitchTest1, TestSize.Level1)
{
    std::string accoutId = "100";
    std::string bundleName = "com.example.test";
    bool status = true;
    auto data = CloudSyncManagerCore::DoChangeAppCloudSwitch(accoutId, bundleName, status);
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: DoChangeAppCloudSwitch
 * @tc.desc: Verify the CloudSyncManagerCore::DoChangeAppCloudSwitch function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncManagerCoreTest, DoChangeAppCloudSwitchTest2, TestSize.Level1)
{
    std::string accoutId = "100";
    std::string bundleName = "com.example.test";
    bool status = false;
    auto data = CloudSyncManagerCore::DoChangeAppCloudSwitch(accoutId, bundleName, status);
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: DoNotifyEventChange
 * @tc.desc: Verify the CloudSyncManagerCore::DoNotifyEventChange function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncManagerCoreTest, DoNotifyEventChangeTest1, TestSize.Level1)
{
    int32_t userId = 100;
    std::string eventId = "testId";
    std::string extraData = "testData";
    auto data = CloudSyncManagerCore::DoNotifyEventChange(userId, eventId, extraData);
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: DoNotifyDataChange
 * @tc.desc: Verify the CloudSyncManagerCore::DoNotifyDataChange function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncManagerCoreTest, DoNotifyDataChangeTest1, TestSize.Level1)
{
    std::string accoutId = "100";
    std::string bundleName = "com.example.test";
    auto data = CloudSyncManagerCore::DoNotifyDataChange(accoutId, bundleName);
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: DoDisableCloud
 * @tc.desc: Verify the CloudSyncManagerCore::DoDisableCloud function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncManagerCoreTest, DoDisableCloudTest1, TestSize.Level1)
{
    std::string accoutId = "100";
    auto data = CloudSyncManagerCore::DoDisableCloud(accoutId);
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: DoEnableCloud
 * @tc.desc: Verify the CloudSyncManagerCore::DoEnableCloud function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncManagerCoreTest, DoEnableCloudTest1, TestSize.Level1)
{
    std::string accoutId = "100";
    SwitchDataObj switchData;
    auto data = CloudSyncManagerCore::DoEnableCloud(accoutId, switchData);
    EXPECT_TRUE(data.IsSuccess());
}

/**
 * @tc.name: DoClean
 * @tc.desc: Verify the CloudSyncManagerCore::DoClean function
 * @tc.type: FUNC
 */
HWTEST_F(CloudSyncManagerCoreTest, DoCleanTest1, TestSize.Level1)
{
    std::string accoutId = "100";
    CleanOptions cleanOptions {};
    auto data = CloudSyncManagerCore::DoClean(accoutId, cleanOptions);
    EXPECT_TRUE(data.IsSuccess());
}
} // namespace OHOS::FileManagement::CloudDisk::Test