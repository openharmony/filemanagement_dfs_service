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

#include "cloud_file_kit_mock.cpp"
#include "cycle_task.h"
#include "cycle_task_runner.h"
#include "cloud_status_mock.h"
#include "dfs_error.h"
#include "data_syncer_rdb_store.h"
#include "gmock/gmock.h"
#include "result_set_mock.h"
#include "system_load.h"
#include "tasks/database_backup_task.h"
#include "tasks/optimize_cache_task.h"
#include "tasks/optimize_storage_task.h"
#include "tasks/periodic_check_task.h"
#include "tasks/save_subscription_task.h"
#include "tasks/report_statistics_task.h"
#include "system_func_mock.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;
using namespace testing;

static std::shared_ptr<CloudFile::DataSyncManager> g_dataSyncManagerPtr_;
static const string START_CONDITION = "persist.kernel.cloudsync.screen_off_enable_download";
static const string TEST_BUNDLE_NAME = "com.ohos.test";
static const int32_t USER_ID = 100;

class SubCycleTask : public CycleTask {
public:
    SubCycleTask(std::string taskName,
                 std::set<std::string> bundleNames,
                 int32_t intervalTime,
                 std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
        :CycleTask(taskName, bundleNames, intervalTime, dataSyncManager)
    {}
    int32_t RunTaskForBundle(int32_t userId, std::string bundleName) override
    {
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
    static inline std::shared_ptr<DataSyncerRdbStoreMock> dataSyncerRdbStore_ = nullptr;
    static inline std::shared_ptr<ResultSetMock> rset_ = nullptr;
    static inline std::unique_ptr<CloudPrefImpl> cloudPrefImpl_ = nullptr;
    static inline std::shared_ptr<CloudStatusMethodMock> cloudStatus_ = nullptr;
    static inline std::shared_ptr<CycleTaskRunner> taskRunner_ = nullptr;
};

void CloudSyncServiceCycleTaskTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
    if (g_dataSyncManagerPtr_ == nullptr) {
        g_dataSyncManagerPtr_ = make_shared<CloudFile::DataSyncManager>();
    }
    rset_ = std::make_shared<ResultSetMock>();
    cloudPrefImpl_ = std::make_unique<CloudPrefImpl>(USER_ID, TEST_BUNDLE_NAME,  CycleTask::FILE_PATH);
    dataSyncerRdbStore_ = make_shared<DataSyncerRdbStoreMock>();
    DataSyncerRdbStoreMock::proxy_ = dataSyncerRdbStore_;
    cloudStatus_ = make_shared<CloudStatusMethodMock>();
    CloudStatusMethod::proxy_ = cloudStatus_;
    EXPECT_CALL(*dataSyncerRdbStore_, QueryDataSyncer(_, _)).WillOnce(DoAll(
        SetArgReferee<1>(nullptr),
        Return(E_OK)));
    taskRunner_ = make_shared<CycleTaskRunner>(g_dataSyncManagerPtr_);
}

void CloudSyncServiceCycleTaskTest::TearDownTestCase(void)
{
    ability = nullptr;
    g_dataSyncManagerPtr_ = nullptr;
    rset_ = nullptr;
    cloudPrefImpl_ = nullptr;
    dataSyncerRdbStore_ = nullptr;
    cloudStatus_ = nullptr;
    taskRunner_ = nullptr;
    CloudStatusMethodMock::proxy_ = nullptr;
    DataSyncerRdbStoreMock::proxy_ = nullptr;
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
        SystemFuncMock::proxy_ = std::make_shared<SystemFuncMock>();
        EXPECT_CALL(*SystemFuncMock::proxy_, access(_, _)).WillOnce(Return(-1));
        errno = 123456;

        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        shared_ptr<DatabaseBackupTask> task = make_shared<DatabaseBackupTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_NE(ret, 0);

        SystemFuncMock::proxy_ = nullptr;
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
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
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
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
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
        taskRunner_->cycleTasks_.clear();
        taskRunner_->InitTasks();
        EXPECT_EQ(taskRunner_->cycleTasks_.size(), 7);
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
        GTEST_LOG_(INFO) << "StartTaskTest001 userId_:" << taskRunner_->userId_;
        taskRunner_->StartTask();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartTaskTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "StartTaskTest001 end";
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
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
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
        EXPECT_CALL(*ability, GetAppConfigParams(_, _, _)).WillOnce(Return(E_OK));
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
        shared_ptr<OptimizeStorageTask> task = make_shared<OptimizeStorageTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);

        EXPECT_EQ(ret, E_OK);
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

        EXPECT_EQ(ret, E_OK);
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

/**
 * @tc.name: RunTaskForBundleTest010
 * @tc.desc: 温控后不触发核查任务
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest010 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::OVERHEATED);
        shared_ptr<PeriodicCheckTask> task = make_shared<PeriodicCheckTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest010 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest010 end";
}

/**
 * @tc.name: RunTaskForBundleTest011
 * @tc.desc: 温控后不触发老化任务
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest011 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::OVERHEATED);
        shared_ptr<OptimizeStorageTask> task = make_shared<OptimizeStorageTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest011 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest011 end";
}

/**
 * @tc.name: RunTaskForBundleTest012
 * @tc.desc: 温控后不触发清理任务
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest012 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::OVERHEATED);
        shared_ptr<OptimizeCacheTask> task = make_shared<OptimizeCacheTask>(g_dataSyncManagerPtr_);
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest012 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest012 end";
}

/**
 * @tc.name: RunTaskForBundleTest013
 * @tc.desc: Verify the RunTaskForBundle function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest013 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
        shared_ptr<OptimizeStorageTask> task = make_shared<OptimizeStorageTask>(g_dataSyncManagerPtr_);
        CloudFile::CloudFileKit::instance_ = ability.get();
        EXPECT_CALL(*ability, GetAppConfigParams(_, _, _)).WillOnce([](const int32_t userId,
            const std::string &bundleName, std::map<std::string, std::string> &param) {
            param["validDays"] = "30";
            param["dataAgingPolicy"] = "1";
            return E_OK;
        });
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest013 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest013 end";
}

/**
 * @tc.name: RunTaskForBundleTest014
 * @tc.desc: Verify the RunTaskForBundle function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest014 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
        shared_ptr<OptimizeStorageTask> task = make_shared<OptimizeStorageTask>(g_dataSyncManagerPtr_);
        CloudFile::CloudFileKit::instance_ = ability.get();
        EXPECT_CALL(*ability, GetAppConfigParams(_, _, _)).WillOnce(Return(E_CLOUD_SDK));
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_CLOUD_SDK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest014 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest014 end";
}

/**
 * @tc.name: RunTaskForBundleTest015
 * @tc.desc: Verify the RunTaskForBundle function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest015 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
        shared_ptr<OptimizeStorageTask> task = make_shared<OptimizeStorageTask>(g_dataSyncManagerPtr_);
        CloudFile::CloudFileKit::instance_ = ability.get();
        EXPECT_CALL(*ability, GetAppConfigParams(_, _, _)).WillOnce([](const int32_t userId,
            const std::string &bundleName, std::map<std::string, std::string> &param) {
            param["dataAgingPolicy"] = "0";
            return E_OK;
        });
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest015 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest015 end";
}

/**
 * @tc.name: RunTaskForBundleTest016
 * @tc.desc: Verify the RunTaskForBundle function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest016 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
        shared_ptr<OptimizeStorageTask> task = make_shared<OptimizeStorageTask>(g_dataSyncManagerPtr_);
        CloudFile::CloudFileKit::instance_ = ability.get();
        EXPECT_CALL(*ability, GetAppConfigParams(_, _, _)).WillOnce([](const int32_t userId,
            const std::string &bundleName, std::map<std::string, std::string> &param) {
            param["validDays"] = "30";
            return E_OK;
        });
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest016 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest016 end";
}

/**
 * @tc.name: RunTaskForBundleTest017
 * @tc.desc: Verify the RunTaskForBundle function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, RunTaskForBundleTest017, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest017 start";
    try {
        string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        SystemLoadStatus::Setload(PowerMgr::ThermalLevel::NORMAL);
        shared_ptr<OptimizeStorageTask> task = make_shared<OptimizeStorageTask>(g_dataSyncManagerPtr_);
        CloudFile::CloudFileKit::instance_ = ability.get();
        EXPECT_CALL(*ability, GetAppConfigParams(_, _, _)).WillOnce([](const int32_t userId,
            const std::string &bundleName, std::map<std::string, std::string> &param) {
            param["validDays"] = "invaid int32_t value";
            return E_OK;
        });
        int32_t ret = task->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RunTaskForBundleTest017 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest017 end";
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
        EXPECT_CALL(*dataSyncerRdbStore_, QueryDataSyncer(_, _)).WillOnce(DoAll(
            SetArgReferee<1>(nullptr),
            Return(E_OK)));
        taskRunner_->SetRunableBundleNames();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetRunableBundleNamesTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest001 end";
}

/**
 * @tc.name: SetRunableBundleNamesTest002
 * @tc.desc: Verify the SetRunableBundleNames function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, SetRunableBundleNamesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest002 start";
    try {
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        EXPECT_CALL(*dataSyncerRdbStore_, QueryDataSyncer(_, _)).WillOnce(DoAll(
            SetArgReferee<1>(nullptr),
            Return(E_RDB)));
        taskRunner_->SetRunableBundleNames();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetRunableBundleNamesTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest002 end";
}

/**
 * @tc.name: SetRunableBundleNamesTest003
 * @tc.desc: Verify the SetRunableBundleNames function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, SetRunableBundleNamesTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest003 start";
    try {
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        cloudPrefImpl_->SetLong("lastCheckTime", 0);
        EXPECT_CALL(*dataSyncerRdbStore_, QueryDataSyncer(_, _)).WillOnce(DoAll(
            SetArgReferee<1>(rset_),
            Return(E_OK)));
        EXPECT_CALL(*rset_, GoToNextRow()).WillOnce(Return(E_OK)).WillOnce(Return(E_RDB));
        EXPECT_CALL(*rset_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*rset_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>(TEST_BUNDLE_NAME), Return(E_OK)));
        EXPECT_CALL(*cloudStatus_, IsCloudStatusOkay(_, _)).WillOnce(Return(true));
        taskRunner_->SetRunableBundleNames();
        
        EXPECT_EQ(system::GetParameter(START_CONDITION, ""), "true");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetRunableBundleNamesTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest003 end";
}

/**
 * @tc.name: SetRunableBundleNamesTest004
 * @tc.desc: Verify the SetRunableBundleNames function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, SetRunableBundleNamesTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest004 start";
    try {
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        EXPECT_CALL(*dataSyncerRdbStore_, QueryDataSyncer(_, _)).WillOnce(DoAll(
            SetArgReferee<1>(rset_),
            Return(E_OK)));
        EXPECT_CALL(*rset_, GoToNextRow()).WillOnce(Return(E_RDB));
        taskRunner_->SetRunableBundleNames();
        EXPECT_EQ(system::GetParameter(START_CONDITION, ""), "false");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetRunableBundleNamesTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest004 end";
}

/**
 * @tc.name: SetRunableBundleNamesTest005
 * @tc.desc: Verify the SetRunableBundleNames function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, SetRunableBundleNamesTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest005 start";
    try {
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        std::time_t currentTime = std::time(nullptr);
        cloudPrefImpl_->SetLong("lastCheckTime", currentTime);

        EXPECT_CALL(*dataSyncerRdbStore_, QueryDataSyncer(_, _)).WillOnce(DoAll(
            SetArgReferee<1>(rset_),
            Return(E_OK)));
        EXPECT_CALL(*rset_, GoToNextRow()).WillOnce(Return(E_OK)).WillOnce(Return(E_RDB));
        EXPECT_CALL(*rset_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*rset_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>(TEST_BUNDLE_NAME), Return(E_OK)));

        taskRunner_->SetRunableBundleNames();
        EXPECT_EQ(system::GetParameter(START_CONDITION, ""), "false");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetRunableBundleNamesTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest005 end";
}

/**
 * @tc.name: SetRunableBundleNamesTest006
 * @tc.desc: Verify the SetRunableBundleNames function
 * @tc.type: FUNC
 * @tc.require: IB3SWZ
 */
HWTEST_F(CloudSyncServiceCycleTaskTest, SetRunableBundleNamesTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest006 start";
    try {
        EXPECT_NE(g_dataSyncManagerPtr_, nullptr);
        cloudPrefImpl_->SetLong("lastCheckTime", 0);
        EXPECT_CALL(*dataSyncerRdbStore_, QueryDataSyncer(_, _)).WillOnce(DoAll(
            SetArgReferee<1>(rset_),
            Return(E_OK)));
        EXPECT_CALL(*rset_, GoToNextRow()).WillOnce(Return(E_OK)).WillOnce(Return(E_RDB));
        EXPECT_CALL(*rset_, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*rset_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>(TEST_BUNDLE_NAME), Return(E_OK)));
        EXPECT_CALL(*cloudStatus_, IsCloudStatusOkay(_, _)).WillOnce(Return(false));

        taskRunner_->SetRunableBundleNames();
        EXPECT_EQ(system::GetParameter(START_CONDITION, ""), "false");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetRunableBundleNamesTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "SetRunableBundleNamesTest006 end";
}
}
}
}