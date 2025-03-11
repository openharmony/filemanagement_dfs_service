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
#include "tasks/optimize_cache_task.h"
#include "tasks/optimize_storage_task.h"
#include "tasks/periodic_check_task.h"
#include "tasks/save_subscription_task.h"
#include "tasks/report_statistics_task.h"
#include "utils_log.h"
#include "cloud_file_kit_mock.cpp"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;
using namespace testing;

std::shared_ptr<CloudFile::DataSyncManager> g_dataSyncManagerPtr_;

class SubCycleTask : public CycleTask {
public:
    SubCycleTask(std::string taskName,
                 std::set<std::string> bundleNames,
                 int32_t intervalTime,
                 std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
    :CycleTask(taskName, bundleNames, intervalTime, dataSyncManager)
    {}
    int32_t RunTaskForBundle(int32_t userId, std::string bundleName) override {
        return 0;
    }
};

class CloudSyncServiceCycleTaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<CloudFileKitMock> ability = make_shared<CloudFileKitMock>();
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
    ability = nullptr;
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

/*
 * @tc.name: RunTaskForBundlTest
 * @tc.desc: Verify the RunTaskForBundl function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest006 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        shared_ptr<OptimizeCacheTask> task = make_shared<OptimizeCacheTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);

        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest006 end";
}
  
 /*
  * @tc.name: CleanCacheTest
  * @tc.desc: Verify the CleanCache function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest007 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        CloudFile::CloudFileKit::instance_ = ability.get();
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        shared_ptr<OptimizeStorageTask> task = make_shared<OptimizeStorageTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);

        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest007 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest007 end";
}
  
 /*
  * @tc.name: CleanCacheTest
  * @tc.desc: Verify the CleanCache function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest008 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        CloudFile::CloudFileKit::instance_ = ability.get();
        EXPECT_CALL(*ability, GetAppConfigParams(_, _, _)).WillOnce(Return(E_OK));
        shared_ptr<OptimizeStorageTask> task = make_shared<OptimizeStorageTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);

        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest008 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest008 end";
}
  
 /*
  * @tc.name: CleanCacheTest
  * @tc.desc: Verify the CleanCache function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest009 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        CloudFile::CloudFileKit::instance_ = ability.get();
        shared_ptr<SaveSubscriptionTask> task = make_shared<SaveSubscriptionTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);

        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest009 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest009 end";
}

/*
  * @tc.name: CycleTaskRunnerTest
  * @tc.desc: Verify the CycleTaskRunner function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncServiceCycleTaskTest, CycleTaskRunnerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CycleTaskRunnerTest001 start";
    try {
        shared_ptr<CloudFile::DataSyncManager> dataSyncManager = nullptr;
        CycleTaskRunner cycleTaskRunner(dataSyncManager);
        auto res = cycleTaskRunner.dataSyncManager_;
        EXPECT_EQ(res, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CycleTaskRunnerTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CycleTaskRunnerTest001 end";
}

 /*
  * @tc.name: SetLastRunTimeTest001
  * @tc.desc: Verify the SetLastRunTime function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncServiceCycleTaskTest, SetLastRunTimeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetLastRunTimeTest001 start";
    try {
        std::string taskName;
        std::set<std::string> bundleNames;
        int32_t intervalTime = 0;
        std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager = nullptr;

        CycleTask *sycTask = new SubCycleTask(taskName, bundleNames, intervalTime, dataSyncManager);
        
        std::time_t time = std::time(nullptr);
        sycTask->SetLastRunTime(time);
        EXPECT_EQ(sycTask->cloudPrefImpl_, nullptr);
        delete sycTask;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetLastRunTimeTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetLastRunTimeTest001 end";
}

 /*
  * @tc.name: SetLastRunTimeTest002
  * @tc.desc: Verify the SetLastRunTime function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncServiceCycleTaskTest, SetLastRunTimeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetLastRunTimeTest002 start";
    try {
        std::string taskName;
        std::set<std::string> bundleNames;
        int32_t intervalTime = 0;
        std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager = nullptr;

        CycleTask *sycTask = new SubCycleTask(taskName, bundleNames, intervalTime, dataSyncManager);
        
        sycTask->cloudPrefImpl_ = std::make_unique<CloudPrefImpl>("");
        std::time_t time = std::time(nullptr);
        sycTask->SetLastRunTime(time);
        EXPECT_NE(sycTask->cloudPrefImpl_, nullptr);
        delete sycTask;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetLastRunTimeTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "SetLastRunTimeTest002 end";
}

 /*
  * @tc.name: GetLastRunTimeTest001
  * @tc.desc: Verify the GetLastRunTime function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncServiceCycleTaskTest, GetLastRunTimeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLastRunTimeTest001 start";
    try {
        std::string taskName;
        std::set<std::string> bundleNames;
        int32_t intervalTime = 0;
        std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager = nullptr;

        CycleTask *sycTask = new SubCycleTask(taskName, bundleNames, intervalTime, dataSyncManager);
        
        std::time_t time = std::time(nullptr);
        sycTask->GetLastRunTime(time);
        EXPECT_EQ(sycTask->cloudPrefImpl_, nullptr);
        delete sycTask;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLastRunTimeTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetLastRunTimeTest001 end";
}

 /*
  * @tc.name: GetLastRunTimeTest002
  * @tc.desc: Verify the GetLastRunTime function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncServiceCycleTaskTest, GetLastRunTimeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLastRunTimeTest002 start";
    try {
        std::string taskName;
        std::set<std::string> bundleNames;
        int32_t intervalTime = 0;
        std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager = nullptr;

        CycleTask *sycTask = new SubCycleTask(taskName, bundleNames, intervalTime, dataSyncManager);
        
        sycTask->cloudPrefImpl_ = std::make_unique<CloudPrefImpl>("");
        std::time_t time = std::time(nullptr);
        sycTask->GetLastRunTime(time);
        EXPECT_NE(sycTask->cloudPrefImpl_, nullptr);
        delete sycTask;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLastRunTimeTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GetLastRunTimeTest002 end";
}

 /*
  * @tc.name: IsEligibleToRunTest001
  * @tc.desc: Verify the IsEligibleToRun function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncServiceCycleTaskTest, IsEligibleToRunTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEligibleToRunTest001 start";
    try {
        std::string taskName;
        std::set<std::string> bundleNames;
        int32_t intervalTime = 0;
        std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager = nullptr;
        CycleTask *sycTask = new SubCycleTask(taskName, bundleNames, intervalTime, dataSyncManager);

        std::time_t currentTime = std::time(nullptr);
        std::string bundleName;
        bool res = sycTask->IsEligibleToRun(currentTime, bundleName);
        EXPECT_EQ(res, true);
        delete sycTask;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsEligibleToRunTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "IsEligibleToRunTest001 end";
}

 /*
  * @tc.name: IsEligibleToRunTest002
  * @tc.desc: Verify the IsEligibleToRun function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncServiceCycleTaskTest, IsEligibleToRunTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsEligibleToRunTest002 start";
    try {
        std::string taskName;
        std::set<std::string> bundleNames = {"test"};
        int32_t intervalTime = 0;
        std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager = nullptr;
        CycleTask *sycTask = new SubCycleTask(taskName, bundleNames, intervalTime, dataSyncManager);

        std::time_t currentTime = std::time(nullptr);
        std::string bundleName = "";
        bool res = sycTask->IsEligibleToRun(currentTime, bundleName);
        EXPECT_EQ(res, false);
        delete sycTask;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsEligibleToRunTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "IsEligibleToRunTest002 end";
}

 /*
  * @tc.name: RunTaskTest001
  * @tc.desc: Verify the RunTask function.
  * @tc.type: FUNC
  * @tc.require: I6H5MH
  */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskTest001 start";
    try {
        std::string taskName;
        std::set<std::string> bundleNames = {"test"};
        int32_t intervalTime = 0;
        std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager = nullptr;
        CycleTask *sycTask = new SubCycleTask(taskName, bundleNames, intervalTime, dataSyncManager);

        int32_t userId = 0;
        sycTask->RunTask(userId);
        EXPECT_EQ(sycTask->runnableBundleNames_, nullptr);
        delete sycTask;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskTest001 end";
}
}
}
}