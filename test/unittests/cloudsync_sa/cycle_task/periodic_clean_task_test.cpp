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

#include "cycle_task_runner.h"
#include "parameters.h"
#include "periodic_clean_task.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

static const std::string PHOTOS_KEY = "persist.kernel.bundle_name.photos";

class PeriodicCleanTaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<CloudFile::DataSyncManager> dataSyncManagerPtr_ = nullptr;
    static inline std::shared_ptr<PeriodicCleanTask> periodicCleanTask_ = nullptr;
};

void PeriodicCleanTaskTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    dataSyncManagerPtr_ = std::make_shared<CloudFile::DataSyncManager>();
    periodicCleanTask_ = std::make_shared<PeriodicCleanTask>(dataSyncManagerPtr_);
}

void PeriodicCleanTaskTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    dataSyncManagerPtr_ = nullptr;
    periodicCleanTask_ = nullptr;
}

void PeriodicCleanTaskTest::SetUp(void)
{
}

void PeriodicCleanTaskTest::TearDown(void)
{
}

/*
  * @tc.name: RunTaskForBundleTest001
  * @tc.desc: Verify the RunTaskForBundle function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskForBundleTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 Start";
    try {
        int32_t userId = -1;
        std::string bundleName = "xxx";
        int32_t ret = periodicCleanTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 End";
}

/**
 * @tc.name: PeriodicCleanTempDirTest001
 * @tc.desc: Verify PeriodicCleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, PeriodicCleanTempDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest001 Start.";
    try {
        int32_t userId = -1;
        periodicCleanTask_->PeriodicCleanTempDir(userId);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest001 ERROR.";
    }
}

/**
 * @tc.name: PeriodicCleanTempDirTest002
 * @tc.desc: Verify PeriodicCleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, PeriodicCleanTempDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest002 Start.";
    try {
        std::string prevPhotoBundleName = system::GetParameter(PHOTOS_KEY, "");
        system::SetParameter(PHOTOS_KEY, "");
        int32_t userId = -1;
        periodicCleanTask_->PeriodicCleanTempDir(userId);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        system::SetParameter(PHOTOS_KEY, prevPhotoBundleName);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PeriodicCleanTempDirTest002 ERROR.";
    }
}

/**
 * @tc.name: CleanTempDirTest001
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, CleanTempDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanTempDirTest001 Start.";
    try {
        const std::string dir = "/data/test_tdd";
        const std::string prefix = "tmp_test";
        periodicCleanTask_->CleanTempDir(dir, prefix);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanTempDirTest001 ERROR.";
    }
}

/**
 * @tc.name: CleanTempDirTest002
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, CleanTempDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanTempDirTest002 Start.";
    try {
        const std::string dir = "/data/test_tdd";
        const std::string prefix = "";
        periodicCleanTask_->CleanTempDir(dir, prefix);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanTempDirTest002 ERROR.";
    }
}

/**
 * @tc.name: CleanTempDirTest003
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, CleanTempDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanTempDirTest003 Start.";
    try {
        std::system("touch /data/test_tdd");
        const std::string dir = "/data/test_tdd";
        const std::string prefix = "tmp_test";
        periodicCleanTask_->CleanTempDir(dir, prefix);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        std::system("rm /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanTempDirTest003 ERROR.";
    }
}

/**
 * @tc.name: CleanTempDirTest004
 * @tc.desc: Verify CleanTempDir function.
 * @tc.type: FUNC
 */
HWTEST_F(PeriodicCleanTaskTest, CleanTempDirTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanTempDirTest004 Start.";
    try {
        const std::string dir = "/data/test_tdd";
        const std::string prefix = "tmp_test";
        std::system("mkdir /data/test_tdd");
        std::system("mkdir /data/test_tdd/tmp_test_123456");
        std::system("mkdir /data/test_tdd/test");
        std::system("touch /data/test_tdd/file1.txt");
        std::system("touch /data/test_tdd/tmp_test_123456/file2.txt");
        periodicCleanTask_->CleanTempDir(dir, prefix);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        std::system("rm -rf /data/test_tdd");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanTempDirTest004 ERROR.";
    }
}

/*
  * @tc.name: RunTaskTest001
  * @tc.desc: Verify the RunTask function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskTest001 Start";
    try {
        int32_t userId = -1;
        periodicCleanTask_->runTaskForSwitchOff_ = true;
        periodicCleanTask_->RunTask(userId);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, true);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskTest001 End";
}

/*
  * @tc.name: RunTaskTest002
  * @tc.desc: Verify the RunTask function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskTest002 Start";
    try {
        int32_t userId = -1;
        periodicCleanTask_->runTaskForSwitchOff_ = false;
        periodicCleanTask_->RunTask(userId);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, false);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_EQ(periodicCleanTask_->runnableBundleNames_, nullptr);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskTest002 End";
}

/*
  * @tc.name: RunTaskTest003
  * @tc.desc: Verify the RunTask function.
  * @tc.type: FUNC
  */
HWTEST_F(PeriodicCleanTaskTest, RunTaskTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskTest003 Start";
    try {
        int32_t userId = -1;
        std::shared_ptr<std::set<std::string>> runnableBundleNames = make_shared<std::set<std::string>>();
        runnableBundleNames->insert("xxxx");
        periodicCleanTask_->runTaskForSwitchOff_ = false;
        periodicCleanTask_->SetRunnableBundleNames(runnableBundleNames);
        periodicCleanTask_->RunTask(userId);
        EXPECT_EQ(periodicCleanTask_->runTaskForSwitchOff_, false);
        EXPECT_EQ(periodicCleanTask_->bundleNames_.size(), 1);
        EXPECT_EQ(periodicCleanTask_->taskName_, "periodic_clean_task");
        EXPECT_NE(periodicCleanTask_->runnableBundleNames_, nullptr);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskTest003 End";
}
}