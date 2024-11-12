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

#include <gtest/gtest.h>

#include "cloud_sync_common.h"
#include "dfs_error.h"
#include "cycle_task.h"
#include "cycle_task_runner.h"
#include "tasks/database_backup_task.h"
#include "tasks/optimize_storage_task.h"
#include "tasks/periodic_check_task.h"
#include "tasks/save_subscription_task.h"
#include "tasks/report_statistics_task.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;

std::shared_ptr<CloudFile::DataSyncManager> g_dataSyncManagerPtr_;

class CloudSyncServiceCycleTaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudSyncServiceCycleTaskTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
    if (g_dataSyncManagerPtr_ == nullptr) {
        g_dataSyncManagerPtr_ = make_shared<CloudFile::DataSyncManager>();
    }
}

void CloudSyncServiceCycleTaskTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudSyncServiceCycleTaskTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudSyncServiceCycleTaskTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: RunTaskForBundleTest001
 * @tc.desc: Verify the RunTaskForBundle function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        shared_ptr<DatabaseBackupTask> task = make_shared<DatabaseBackupTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_NE(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 end";
}

/**
 * @tc.name: RunTaskForBundleTest002
 * @tc.desc: Verify the RunTaskForBundle function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        shared_ptr<OptimizeStorageTask> task = make_shared<OptimizeStorageTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_NE(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 end";
}

/**
 * @tc.name: RunTaskForBundleTest003
 * @tc.desc: Verify the RunTaskForBundle function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        shared_ptr<PeriodicCheckTask> task = make_shared<PeriodicCheckTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_NE(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 end";
}

/**
 * @tc.name: RunTaskForBundleTest004
 * @tc.desc: Verify the RunTaskForBundle function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        shared_ptr<ReportStatisticsTask> task = make_shared<ReportStatisticsTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 end";
}

/**
 * @tc.name: RunTaskForBundleTest005
 * @tc.desc: Verify the RunTaskForBundle function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        shared_ptr<SaveSubscriptionTask> task = make_shared<SaveSubscriptionTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_NE(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 end";
}

/**
 * @tc.name: InitTasksTest001
 * @tc.desc: Verify the InitTasks function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, InitTasksTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InitTasksTest001 start";
    try {
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        shared_ptr<CycleTaskRunner> taskRunner = make_shared<CycleTaskRunner>(g_dataSyncManagerPtr_);
        taskRunner->InitTasks();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InitTasksTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "InitTasksTest001 end";
}

/**
 * @tc.name: StartTaskTest001
 * @tc.desc: Verify the StartTask function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, StartTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartTaskTest001 start";
    try {
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        shared_ptr<CycleTaskRunner> taskRunner = make_shared<CycleTaskRunner>(g_dataSyncManagerPtr_);
        GTEST_LOG_(INFO) << "StartTaskTest001 userId_:" << taskRunner->userId_;
        taskRunner->StartTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartTaskTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "StartTaskTest001 end";
}

/**
 * @tc.name: SetRunableBundleNamesTest001
 * @tc.desc: Verify the SetRunableBundleNames function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, SetRunableBundleNamesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest001 start";
    try {
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        shared_ptr<CycleTaskRunner> taskRunner = make_shared<CycleTaskRunner>(g_dataSyncManagerPtr_);
        taskRunner->SetRunableBundleNames();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetRunableBundleNamesTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest001 end";
}
}
}
}