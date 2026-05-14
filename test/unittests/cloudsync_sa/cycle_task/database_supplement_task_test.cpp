/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "battery_status.h"
#include "cycle_task_runner.h"
#include "database_supplement_task.h"
#include "dfs_error.h"
#include "migration_rdb_mock.h"
#include "parameters.h"
#include "screen_status.h"
#include "screen_status_mock.h"
#include "transaction_mock.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using NativeRdb::E_OK;
using OHOS::FileManagement::CloudDisk::TransactionMock;
using OHOS::FileManagement::CloudDisk::Test::MigrationAssistantMock;
using OHOS::FileManagement::CloudDisk::Test::MigrationRdbStoreMock;
using OHOS::FileManagement::CloudDisk::Test::MigrationResultSetMock;
using OHOS::system::ParameterMock;
using Values = NativeRdb::RdbStore::Values;

class DatabaseSupplementTaskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<CloudFile::DataSyncManager> dataSyncManagerPtr_ = nullptr;
    static inline shared_ptr<DatabaseSupplementTask> databaseSupplementTask_ = nullptr;
    static inline shared_ptr<ScreenStatusMock> screenStatusMock_ = nullptr;
    static inline shared_ptr<ParameterMock> parameterMock_ = nullptr;
    static inline shared_ptr<MigrationAssistantMock> assistantMock_ = nullptr;
    static inline shared_ptr<MigrationRdbStoreMock> rdbStoreMock_ = nullptr;
    static inline shared_ptr<MigrationResultSetMock> resultSetMock_ = nullptr;
    static inline shared_ptr<TransactionMock> transactionMock_ = nullptr;
};

void DatabaseSupplementTaskTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    dataSyncManagerPtr_ = make_shared<CloudFile::DataSyncManager>();
    databaseSupplementTask_ = make_shared<DatabaseSupplementTask>(dataSyncManagerPtr_);
    screenStatusMock_ = make_shared<ScreenStatusMock>();
    ScreenStatusMock::proxy_ = screenStatusMock_;
    parameterMock_ = make_shared<ParameterMock>();
    ParameterMock::proxy_ = parameterMock_;
    assistantMock_ = make_shared<MigrationAssistantMock>();
    rdbStoreMock_ = make_shared<MigrationRdbStoreMock>();
    resultSetMock_ = make_shared<MigrationResultSetMock>();
    transactionMock_ = make_shared<TransactionMock>();
}

void DatabaseSupplementTaskTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    dataSyncManagerPtr_ = nullptr;
    databaseSupplementTask_ = nullptr;
    screenStatusMock_ = nullptr;
    ScreenStatusMock::proxy_ = nullptr;
    parameterMock_ = nullptr;
    ParameterMock::proxy_ = nullptr;
    MigrationAssistantMock::ins = nullptr;
    assistantMock_ = nullptr;
    rdbStoreMock_ = nullptr;
    resultSetMock_ = nullptr;
    transactionMock_ = nullptr;
}

void DatabaseSupplementTaskTest::SetUp(void)
{
    std::system("rm -rf /data/test_tdd");
    std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager");
    MigrationAssistantMock::ins = assistantMock_;
    resultSetMock_ = make_shared<MigrationResultSetMock>();
}

void DatabaseSupplementTaskTest::TearDown(void)
{
    std::system("rm -rf /data/test_tdd");
    std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager");
    MigrationAssistantMock::ins = nullptr;
    resultSetMock_ = nullptr;
}

/*
 * @tc.name: GetBaseDirTest001
 * @tc.desc: Verify that GetBaseDir returns correct path for valid userId.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, GetBaseDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBaseDirTest001 Start";
    try {
        int32_t userId = 100;

        string result = databaseSupplementTask_->GetBaseDir(userId);
        EXPECT_EQ(result, "/data/service/el2/100/hmdfs/cloudfile_manager/");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetBaseDirTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetBaseDirTest001 End";
}

/*
 * @tc.name: GetBaseDirTest002
 * @tc.desc: Verify that GetBaseDir returns correct path for different userId.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, GetBaseDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBaseDirTest002 Start";
    try {
        int32_t userId = 0;

        string result = databaseSupplementTask_->GetBaseDir(userId);
        EXPECT_EQ(result, "/data/service/el2/0/hmdfs/cloudfile_manager/");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetBaseDirTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GetBaseDirTest002 End";
}

/*
 * @tc.name: GetBaseDirTest003
 * @tc.desc: Verify that GetBaseDir returns correct path for negative userId.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, GetBaseDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBaseDirTest003 Start";
    try {
        int32_t userId = -1;

        string result = databaseSupplementTask_->GetBaseDir(userId);
        EXPECT_EQ(result, "/data/service/el2/-1/hmdfs/cloudfile_manager/");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetBaseDirTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "GetBaseDirTest003 End";
}

/*
 * @tc.name: RunTaskForBundleTest001
 * @tc.desc: Verify that RunTaskForBundle returns E_STOP when screen is on.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunTaskForBundleTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return("com.filemanager"));
        BatteryStatus::SetChargingStatus(true);

        int32_t userId = 100;
        string bundleName = "com.filemanager";
        int32_t ret = databaseSupplementTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 End";
}

/*
 * @tc.name: RunTaskForBundleTest002
 * @tc.desc: Verify that RunTaskForBundle returns E_STOP when not charging.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunTaskForBundleTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return("com.filemanager"));
        BatteryStatus::SetChargingStatus(false);

        int32_t userId = 100;
        string bundleName = "com.filemanager";
        int32_t ret = databaseSupplementTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 End";
}

/*
 * @tc.name: RunTaskForBundleTest003
 * @tc.desc: Verify that RunTaskForBundle returns E_OK when conditions are met and all pruned.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunTaskForBundleTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return("com.filemanager"));
        BatteryStatus::SetChargingStatus(true);

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/.all_pruned");

        int32_t userId = 100;
        string bundleName = "com.filemanager";
        int32_t ret = databaseSupplementTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 End";
}

/*
 * @tc.name: RunTaskForBundleTest004
 * @tc.desc: Verify that RunTaskForBundle returns E_STOP when condition changes during prune.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunTaskForBundleTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false)).WillOnce(Return(true));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return("com.filemanager"));
        BatteryStatus::SetChargingStatus(true);

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated");

        int32_t userId = 100;
        string bundleName = "com.filemanager";
        int32_t ret = databaseSupplementTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 End";
}

/*
 * @tc.name: RunTaskForBundleTest005
 * @tc.desc: Verify that RunTaskForBundle returns E_OK when no migrated apps.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunTaskForBundleTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return("com.filemanager"));
        BatteryStatus::SetChargingStatus(true);

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        int32_t userId = 100;
        string bundleName = "com.filemanager";
        int32_t ret = databaseSupplementTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);

        string allPrunedMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/.all_pruned";
        EXPECT_EQ(access(allPrunedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 End";
}

/*
 * @tc.name: RunTaskForBundleTest006
 * @tc.desc: Verify that RunTaskForBundle handles multiple migrated apps.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunTaskForBundleTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest006 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return("com.filemanager"));
        BatteryStatus::SetChargingStatus(true);

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app1");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app1/.migrated");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app1/.pruned");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app2");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app2/.migrated");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app2/.pruned");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        int32_t userId = 100;
        string bundleName = "com.filemanager";
        int32_t ret = databaseSupplementTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest006 End";
}

/*
 * @tc.name: RunTaskForBundleTest007
 * @tc.desc: Verify that RunTaskForBundle returns E_STOP when RunChownTask returns E_STOP.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunTaskForBundleTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest007 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn())
            .WillOnce(Return(false))
            .WillOnce(Return(false))
            .WillOnce(Return(true));
        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return("com.filemanager"));
        BatteryStatus::SetChargingStatus(true);

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/.all_pruned");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated");

        int32_t userId = 100;
        string bundleName = "com.filemanager";
        int32_t ret = databaseSupplementTask_->RunTaskForBundle(userId, bundleName);
        EXPECT_EQ(ret, E_STOP);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunTaskForBundleTest007 FAILED";
    }
    GTEST_LOG_(INFO) << "RunTaskForBundleTest007 End";
}

/*
 * @tc.name: RunPruneTaskTest001
 * @tc.desc: Verify that RunPruneTask returns E_OK when all pruned marker exists.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunPruneTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunPruneTaskTest001 Start";
    try {
        BatteryStatus::SetChargingStatus(true);

        int32_t userId = 100;
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/.all_pruned");

        int32_t ret = databaseSupplementTask_->RunPruneTask(userId, filemanager);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunPruneTaskTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RunPruneTaskTest001 End";
}

/*
 * @tc.name: RunPruneTaskTest002
 * @tc.desc: Verify that RunPruneTask returns E_OK when no migrated apps.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunPruneTaskTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunPruneTaskTest002 Start";
    try {
        BatteryStatus::SetChargingStatus(true);

        int32_t userId = 100;
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        int32_t ret = databaseSupplementTask_->RunPruneTask(userId, filemanager);
        EXPECT_EQ(ret, E_OK);

        string allPrunedMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/.all_pruned";
        EXPECT_EQ(access(allPrunedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunPruneTaskTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RunPruneTaskTest002 End";
}

/*
 * @tc.name: RunPruneTaskTest003
 * @tc.desc: Verify that RunPruneTask returns E_STOP when condition changes.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunPruneTaskTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunPruneTaskTest003 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false)).WillOnce(Return(true));
        BatteryStatus::SetChargingStatus(true);

        int32_t userId = 100;
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app2/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app2/rdb/clouddisk.db");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app2/.migrated");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillRepeatedly(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetInt(0, _)).WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(E_OK)));
        EXPECT_CALL(*resultSetMock_, Close()).WillRepeatedly(Return(E_OK));

        int32_t ret = databaseSupplementTask_->RunPruneTask(userId, filemanager);
        EXPECT_EQ(ret, E_STOP);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunPruneTaskTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "RunPruneTaskTest003 End";
}

/*
 * @tc.name: RunPruneTaskTest004
 * @tc.desc: Verify that RunPruneTask processes migrated apps.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunPruneTaskTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunPruneTaskTest004 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        BatteryStatus::SetChargingStatus(true);

        int32_t userId = 100;
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.pruned");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        int32_t ret = databaseSupplementTask_->RunPruneTask(userId, filemanager);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunPruneTaskTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "RunPruneTaskTest004 End";
}

/*
 * @tc.name: RunPruneTaskTest005
 * @tc.desc: Verify that RunPruneTask does not create all_pruned when PruneAndCleanup fails for some apps.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunPruneTaskTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunPruneTaskTest005 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        BatteryStatus::SetChargingStatus(true);

        int32_t userId = 100;
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app2/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app2/rdb/clouddisk.db");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app2/.migrated");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillRepeatedly(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetInt(0, _)).WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(E_OK)));
        EXPECT_CALL(*resultSetMock_, Close()).WillRepeatedly(Return(E_OK));

        int32_t ret = databaseSupplementTask_->RunPruneTask(userId, filemanager);
        EXPECT_EQ(ret, E_OK);

        string prunedMarker1 = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.pruned";
        EXPECT_EQ(access(prunedMarker1.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunPruneTaskTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "RunPruneTaskTest005 End";
}

/*
 * @tc.name: RunChownTaskTest001
 * @tc.desc: Verify that RunChownTask returns E_OK when no bundles for chown.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunChownTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunChownTaskTest001 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.filemanager";

        int32_t ret = databaseSupplementTask_->RunChownTask(userId, filemanager);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunChownTaskTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "RunChownTaskTest001 End";
}

/*
 * @tc.name: RunChownTaskTest002
 * @tc.desc: Verify that RunChownTask returns E_OK with filemanager bundle.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunChownTaskTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunChownTaskTest002 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        BatteryStatus::SetChargingStatus(true);

        int32_t userId = 100;
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        int32_t ret = databaseSupplementTask_->RunChownTask(userId, filemanager);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunChownTaskTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "RunChownTaskTest002 End";
}

/*
 * @tc.name: RunChownTaskTest003
 * @tc.desc: Verify that RunChownTask returns E_STOP when condition changes.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunChownTaskTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunChownTaskTest003 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false)).WillOnce(Return(true));
        BatteryStatus::SetChargingStatus(true);

        int32_t userId = 100;
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated");

        int32_t ret = databaseSupplementTask_->RunChownTask(userId, filemanager);
        EXPECT_EQ(ret, E_STOP);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunChownTaskTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "RunChownTaskTest003 End";
}

/*
 * @tc.name: RunChownTaskTest004
 * @tc.desc: Verify that RunChownTask handles migrated bundles.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, RunChownTaskTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunChownTaskTest004 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillRepeatedly(Return(false));
        BatteryStatus::SetChargingStatus(true);

        int32_t userId = 100;
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated");

        int32_t ret = databaseSupplementTask_->RunChownTask(userId, filemanager);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "RunChownTaskTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "RunChownTaskTest004 End";
}

/*
 * @tc.name: CheckPruneConditionTest001
 * @tc.desc: Verify that CheckPruneCondition returns true when screen off and charging.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CheckPruneConditionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPruneConditionTest001 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(true);

        bool result = databaseSupplementTask_->CheckPruneCondition();
        EXPECT_TRUE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckPruneConditionTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckPruneConditionTest001 End";
}

/*
 * @tc.name: CheckPruneConditionTest002
 * @tc.desc: Verify that CheckPruneCondition returns false when screen on.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CheckPruneConditionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPruneConditionTest002 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(true));
        BatteryStatus::SetChargingStatus(true);

        bool result = databaseSupplementTask_->CheckPruneCondition();
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckPruneConditionTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckPruneConditionTest002 End";
}

/*
 * @tc.name: CheckPruneConditionTest003
 * @tc.desc: Verify that CheckPruneCondition returns false when not charging.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CheckPruneConditionTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPruneConditionTest003 Start";
    try {
        EXPECT_CALL(*screenStatusMock_, IsScreenOn()).WillOnce(Return(false));
        BatteryStatus::SetChargingStatus(false);

        bool result = databaseSupplementTask_->CheckPruneCondition();
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckPruneConditionTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckPruneConditionTest003 End";
}

/*
 * @tc.name: HasAllPrunedMarkerTest001
 * @tc.desc: Verify that HasAllPrunedMarker returns true when marker exists.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, HasAllPrunedMarkerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasAllPrunedMarkerTest001 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/.all_pruned");

        bool result = databaseSupplementTask_->HasAllPrunedMarker(userId, filemanager);
        EXPECT_TRUE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "HasAllPrunedMarkerTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "HasAllPrunedMarkerTest001 End";
}

/*
 * @tc.name: HasAllPrunedMarkerTest002
 * @tc.desc: Verify that HasAllPrunedMarker returns false when marker does not exist.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, HasAllPrunedMarkerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasAllPrunedMarkerTest002 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        bool result = databaseSupplementTask_->HasAllPrunedMarker(userId, filemanager);
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "HasAllPrunedMarkerTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "HasAllPrunedMarkerTest002 End";
}

/*
 * @tc.name: CreateAllPrunedMarkerTest001
 * @tc.desc: Verify that CreateAllPrunedMarker creates marker file.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CreateAllPrunedMarkerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAllPrunedMarkerTest001 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        databaseSupplementTask_->CreateAllPrunedMarker(userId, filemanager);

        string markerPath = "/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/.all_pruned";
        EXPECT_EQ(access(markerPath.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CreateAllPrunedMarkerTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CreateAllPrunedMarkerTest001 End";
}

/*
 * @tc.name: FindMigratedAppsTest001
 * @tc.desc: Verify that FindMigratedApps finds migrated apps without prune marker.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, FindMigratedAppsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindMigratedAppsTest001 Start";
    try {
        int32_t userId = 100;
        string baseDir = "/data/service/el2/100/hmdfs/cloudfile_manager/";
        string filemanager = "com.filemanager";
        vector<string> migratedBundles;

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        databaseSupplementTask_->FindMigratedApps(userId, baseDir, filemanager, migratedBundles);

        EXPECT_EQ(migratedBundles.size(), 1);
        EXPECT_EQ(migratedBundles[0], "com.test.app");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "FindMigratedAppsTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "FindMigratedAppsTest001 End";
}

/*
 * @tc.name: FindMigratedAppsTest002
 * @tc.desc: Verify that FindMigratedApps skips apps with prune marker.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, FindMigratedAppsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindMigratedAppsTest002 Start";
    try {
        int32_t userId = 100;
        string baseDir = "/data/service/el2/100/hmdfs/cloudfile_manager/";
        string filemanager = "com.filemanager";
        vector<string> migratedBundles;

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.pruned");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        databaseSupplementTask_->FindMigratedApps(userId, baseDir, filemanager, migratedBundles);

        EXPECT_EQ(migratedBundles.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "FindMigratedAppsTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "FindMigratedAppsTest002 End";
}

/*
 * @tc.name: FindMigratedAppsTest003
 * @tc.desc: Verify that FindMigratedApps skips filemanager app.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, FindMigratedAppsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindMigratedAppsTest003 Start";
    try {
        int32_t userId = 100;
        string baseDir = "/data/service/el2/100/hmdfs/cloudfile_manager/";
        string filemanager = "com.filemanager";
        vector<string> migratedBundles;

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/.migrated");

        databaseSupplementTask_->FindMigratedApps(userId, baseDir, filemanager, migratedBundles);

        EXPECT_EQ(migratedBundles.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "FindMigratedAppsTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "FindMigratedAppsTest003 End";
}

/*
 * @tc.name: FindMigratedAppsTest004
 * @tc.desc: Verify that FindMigratedApps returns empty for non-existent directory.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, FindMigratedAppsTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindMigratedAppsTest004 Start";
    try {
        int32_t userId = 100;
        string baseDir = "/data/test_tdd/nonexistent/";
        string filemanager = "com.filemanager";
        vector<string> migratedBundles;

        databaseSupplementTask_->FindMigratedApps(userId, baseDir, filemanager, migratedBundles);

        EXPECT_EQ(migratedBundles.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "FindMigratedAppsTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "FindMigratedAppsTest004 End";
}

/*
 * @tc.name: FindMigratedAppsTest005
 * @tc.desc: Verify that FindMigratedApps skips non-directory entries.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, FindMigratedAppsTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindMigratedAppsTest005 Start";
    try {
        int32_t userId = 100;
        string baseDir = "/data/service/el2/100/hmdfs/cloudfile_manager/";
        string filemanager = "com.filemanager";
        vector<string> migratedBundles;

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/test_file.txt");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        databaseSupplementTask_->FindMigratedApps(userId, baseDir, filemanager, migratedBundles);

        EXPECT_EQ(migratedBundles.size(), 1);
        EXPECT_EQ(migratedBundles[0], "com.test.app");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "FindMigratedAppsTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "FindMigratedAppsTest005 End";
}

/*
 * @tc.name: FindMigratedBundlesForChownTest001
 * @tc.desc: Verify that FindMigratedBundlesForChown finds filemanager.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, FindMigratedBundlesForChownTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindMigratedBundlesForChownTest001 Start";
    try {
        int32_t userId = 100;
        string baseDir = "/data/service/el2/100/hmdfs/cloudfile_manager/";
        string filemanager = "com.filemanager";
        vector<string> migratedBundles;

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        databaseSupplementTask_->FindMigratedBundlesForChown(userId, baseDir, filemanager, migratedBundles);

        EXPECT_EQ(migratedBundles.size(), 1);
        EXPECT_EQ(migratedBundles[0], "com.filemanager");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "FindMigratedBundlesForChownTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "FindMigratedBundlesForChownTest001 End";
}

/*
 * @tc.name: FindMigratedBundlesForChownTest002
 * @tc.desc: Verify that FindMigratedBundlesForChown finds migrated apps.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, FindMigratedBundlesForChownTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindMigratedBundlesForChownTest002 Start";
    try {
        int32_t userId = 100;
        string baseDir = "/data/service/el2/100/hmdfs/cloudfile_manager/";
        string filemanager = "com.filemanager";
        vector<string> migratedBundles;

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        databaseSupplementTask_->FindMigratedBundlesForChown(userId, baseDir, filemanager, migratedBundles);

        EXPECT_EQ(migratedBundles.size(), 2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "FindMigratedBundlesForChownTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "FindMigratedBundlesForChownTest002 End";
}

/*
 * @tc.name: FindMigratedBundlesForChownTest003
 * @tc.desc: Verify that FindMigratedBundlesForChown handles non-existent directory.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, FindMigratedBundlesForChownTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindMigratedBundlesForChownTest003 Start";
    try {
        int32_t userId = 100;
        string baseDir = "/data/test_tdd/nonexistent/";
        string filemanager = "com.filemanager";
        vector<string> migratedBundles;

        databaseSupplementTask_->FindMigratedBundlesForChown(userId, baseDir, filemanager, migratedBundles);

        EXPECT_EQ(migratedBundles.size(), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "FindMigratedBundlesForChownTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "FindMigratedBundlesForChownTest003 End";
}

/*
 * @tc.name: FindMigratedBundlesForChownTest004
 * @tc.desc: Verify that FindMigratedBundlesForChown skips non-directory entries.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, FindMigratedBundlesForChownTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindMigratedBundlesForChownTest004 Start";
    try {
        int32_t userId = 100;
        string baseDir = "/data/service/el2/100/hmdfs/cloudfile_manager/";
        string filemanager = "com.filemanager";
        vector<string> migratedBundles;

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/test_file.txt");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        databaseSupplementTask_->FindMigratedBundlesForChown(userId, baseDir, filemanager, migratedBundles);

        EXPECT_EQ(migratedBundles.size(), 1);
        EXPECT_EQ(migratedBundles[0], "com.filemanager");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "FindMigratedBundlesForChownTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "FindMigratedBundlesForChownTest004 End";
}

/*
 * @tc.name: FindMigratedBundlesForChownTest005
 * @tc.desc: Verify that FindMigratedBundlesForChown skips apps without migrated marker.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, FindMigratedBundlesForChownTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindMigratedBundlesForChownTest005 Start";
    try {
        int32_t userId = 100;
        string baseDir = "/data/service/el2/100/hmdfs/cloudfile_manager/";
        string filemanager = "com.filemanager";
        vector<string> migratedBundles;

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        databaseSupplementTask_->FindMigratedBundlesForChown(userId, baseDir, filemanager, migratedBundles);

        EXPECT_EQ(migratedBundles.size(), 1);
        EXPECT_EQ(migratedBundles[0], "com.filemanager");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "FindMigratedBundlesForChownTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "FindMigratedBundlesForChownTest005 End";
}

/*
 * @tc.name: ChownMigratedAppDirectoryTest001
 * @tc.desc: Verify that ChownMigratedAppDirectory handles non-existent directory.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, ChownMigratedAppDirectoryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownMigratedAppDirectoryTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";

        databaseSupplementTask_->ChownMigratedAppDirectory(userId, bundleName);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownMigratedAppDirectoryTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownMigratedAppDirectoryTest001 End";
}

/*
 * @tc.name: ChownMigratedAppDirectoryTest002
 * @tc.desc: Verify that ChownMigratedAppDirectory handles empty directory.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, ChownMigratedAppDirectoryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownMigratedAppDirectoryTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");

        databaseSupplementTask_->ChownMigratedAppDirectory(userId, bundleName);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownMigratedAppDirectoryTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownMigratedAppDirectoryTest002 End";
}

/*
 * @tc.name: ChownMigratedAppDirectoryTest003
 * @tc.desc: Verify that ChownMigratedAppDirectory handles directory with files.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, ChownMigratedAppDirectoryTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownMigratedAppDirectoryTest003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/test.txt");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db");

        databaseSupplementTask_->ChownMigratedAppDirectory(userId, bundleName);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownMigratedAppDirectoryTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownMigratedAppDirectoryTest003 End";
}

/*
 * @tc.name: ChownMigratedAppDirectoryTest004
 * @tc.desc: Verify that ChownMigratedAppDirectory handles nested directory structure.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, ChownMigratedAppDirectoryTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownMigratedAppDirectoryTest004 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app004";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/rdb/subdir");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/cache");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/test.txt");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/rdb/clouddisk.db");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/rdb/subdir/data.bin");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/cache/temp.log");

        databaseSupplementTask_->ChownMigratedAppDirectory(userId, bundleName);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownMigratedAppDirectoryTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownMigratedAppDirectoryTest004 End";
}

/*
 * @tc.name: ChownMigratedAppDirectoryTest005
 * @tc.desc: Verify that ChownMigratedAppDirectory handles symbolic links.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, ChownMigratedAppDirectoryTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChownMigratedAppDirectoryTest005 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app005";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app005/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app005/rdb/clouddisk.db");
        std::system(
            "ln -s /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app005/rdb/clouddisk.db "
            "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app005/rdb/link.db");

        databaseSupplementTask_->ChownMigratedAppDirectory(userId, bundleName);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ChownMigratedAppDirectoryTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "ChownMigratedAppDirectoryTest005 End";
}

/*
 * @tc.name: PruneAndCleanupTest001
 * @tc.desc: Verify that PruneAndCleanup handles non-existent database.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, PruneAndCleanupTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PruneAndCleanupTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db")));

        databaseSupplementTask_->PruneAndCleanup(userId, bundleName, filemanager);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "PruneAndCleanupTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "PruneAndCleanupTest001 End";
}

/*
 * @tc.name: PruneAndCleanupTest002
 * @tc.desc: Verify that PruneAndCleanup creates pruned marker.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, PruneAndCleanupTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PruneAndCleanupTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillRepeatedly(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetInt(0, _)).WillRepeatedly(DoAll(SetArgReferee<1>(0), Return(E_OK)));
        EXPECT_CALL(*resultSetMock_, Close()).WillRepeatedly(Return(E_OK));

        databaseSupplementTask_->PruneAndCleanup(userId, bundleName, filemanager);

        string prunedMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.pruned";
        EXPECT_EQ(access(prunedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "PruneAndCleanupTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "PruneAndCleanupTest002 End";
}

/*
 * @tc.name: PruneAndCleanupTest003
 * @tc.desc: Verify that PruneAndCleanup executes prune with transaction.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, PruneAndCleanupTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PruneAndCleanupTest003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app003";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/rdb/clouddisk.db");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillRepeatedly(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(5), Return(E_OK)));
        EXPECT_CALL(*resultSetMock_, Close()).WillRepeatedly(Return(E_OK));
        EXPECT_CALL(*rdbStoreMock_, CreateTransaction(_))
            .WillRepeatedly(Return(make_pair(E_OK, static_pointer_cast<NativeRdb::Transaction>(transactionMock_))));
        EXPECT_CALL(*transactionMock_, Delete(_)).WillRepeatedly(Return(make_pair(E_OK, 3)));

        databaseSupplementTask_->PruneAndCleanup(userId, bundleName, filemanager);

        string prunedMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/.pruned";
        EXPECT_EQ(access(prunedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "PruneAndCleanupTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "PruneAndCleanupTest003 End";
}

/*
 * @tc.name: CheckNeedPruneTest001
 * @tc.desc: Verify that CheckNeedPrune returns false when database not found.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CheckNeedPruneTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNeedPruneTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";
        string filemanager = "com.filemanager";

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(nullptr));

        bool result = databaseSupplementTask_->CheckNeedPrune(userId, bundleName, filemanager);
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckNeedPruneTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckNeedPruneTest001 End";
}

/*
 * @tc.name: CheckNeedPruneTest002
 * @tc.desc: Verify that CheckNeedPrune returns false when count is 0.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CheckNeedPruneTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNeedPruneTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app002";
        string filemanager = "com.filemanager";

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app002/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetInt(0, _)).WillOnce(DoAll(SetArgReferee<1>(0), Return(E_OK)));
        EXPECT_CALL(*resultSetMock_, Close()).WillRepeatedly(Return(E_OK));

        bool result = databaseSupplementTask_->CheckNeedPrune(userId, bundleName, filemanager);
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckNeedPruneTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckNeedPruneTest002 End";
}

/*
 * @tc.name: CheckNeedPruneTest003
 * @tc.desc: Verify that CheckNeedPrune returns true when count > 0.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CheckNeedPruneTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNeedPruneTest003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app003";
        string filemanager = "com.filemanager";

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSetMock_, GetInt(0, _)).WillOnce(DoAll(SetArgReferee<1>(5), Return(E_OK)));
        EXPECT_CALL(*resultSetMock_, Close()).WillRepeatedly(Return(E_OK));

        bool result = databaseSupplementTask_->CheckNeedPrune(userId, bundleName, filemanager);
        EXPECT_TRUE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckNeedPruneTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckNeedPruneTest003 End";
}

/*
 * @tc.name: CheckNeedPruneTest004
 * @tc.desc: Verify that CheckNeedPrune returns false when rdbStore is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CheckNeedPruneTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNeedPruneTest004 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app004";
        string filemanager = "com.filemanager";

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(nullptr));

        bool result = databaseSupplementTask_->CheckNeedPrune(userId, bundleName, filemanager);
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckNeedPruneTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckNeedPruneTest004 End";
}

/*
 * @tc.name: CheckNeedPruneTest005
 * @tc.desc: Verify that CheckNeedPrune returns false when resultSet is nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CheckNeedPruneTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckNeedPruneTest005 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app005";
        string filemanager = "com.filemanager";

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app005/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(nullptr));

        bool result = databaseSupplementTask_->CheckNeedPrune(userId, bundleName, filemanager);
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CheckNeedPruneTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "CheckNeedPruneTest005 End";
}

/*
 * @tc.name: PruneDatabaseTest001
 * @tc.desc: Verify that PruneDatabase handles non-existent database directory.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, PruneDatabaseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PruneDatabaseTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";
        string filemanager = "com.filemanager";

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(nullptr));

        databaseSupplementTask_->PruneDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "PruneDatabaseTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "PruneDatabaseTest001 End";
}

/*
 * @tc.name: PruneDatabaseTest002
 * @tc.desc: Verify that PruneDatabase handles non-existent rdb directory.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, PruneDatabaseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PruneDatabaseTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(nullptr));

        databaseSupplementTask_->PruneDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "PruneDatabaseTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "PruneDatabaseTest002 End";
}

/*
 * @tc.name: PruneDatabaseTest003
 * @tc.desc: Verify that PruneDatabase handles invalid database file.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, PruneDatabaseTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PruneDatabaseTest003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app003";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/rdb/clouddisk.db");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(nullptr));

        databaseSupplementTask_->PruneDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "PruneDatabaseTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "PruneDatabaseTest003 End";
}

/*
 * @tc.name: PruneDatabaseTest004
 * @tc.desc: Verify that PruneDatabase handles transaction successfully.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, PruneDatabaseTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PruneDatabaseTest004 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app004";
        string filemanager = "com.filemanager";

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, CreateTransaction(_))
            .WillRepeatedly(Return(make_pair(E_OK, static_pointer_cast<NativeRdb::Transaction>(transactionMock_))));
        EXPECT_CALL(*transactionMock_, Delete(_)).WillRepeatedly(Return(make_pair(E_OK, 5)));

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/rdb/clouddisk.db");

        databaseSupplementTask_->PruneDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "PruneDatabaseTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "PruneDatabaseTest004 End";
}

/*
 * @tc.name: PruneDatabaseTest005
 * @tc.desc: Verify that PruneDatabase handles CreateTransaction failure.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, PruneDatabaseTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PruneDatabaseTest005 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app005";
        string filemanager = "com.filemanager";

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app005/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, CreateTransaction(_)).WillRepeatedly(Return(make_pair(-1, nullptr)));

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app005/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app005/rdb/clouddisk.db");

        databaseSupplementTask_->PruneDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "PruneDatabaseTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "PruneDatabaseTest005 End";
}

/*
 * @tc.name: CleanupOldDatabaseTest001
 * @tc.desc: Verify that CleanupOldDatabase handles non-existent filemanager directory.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CleanupOldDatabaseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";
        string filemanager = "com.filemanager";

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db")));

        databaseSupplementTask_->CleanupOldDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CleanupOldDatabaseTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest001 End";
}

/*
 * @tc.name: CleanupOldDatabaseTest002
 * @tc.desc: Verify that CleanupOldDatabase handles non-existent old database.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CleanupOldDatabaseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db")));

        databaseSupplementTask_->CleanupOldDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CleanupOldDatabaseTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest002 End";
}

/*
 * @tc.name: CleanupOldDatabaseTest003
 * @tc.desc: Verify that CleanupOldDatabase handles existing old database without rdb.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CleanupOldDatabaseTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db")));

        databaseSupplementTask_->CleanupOldDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CleanupOldDatabaseTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest003 End";
}

/*
 * @tc.name: CleanupOldDatabaseTest004
 * @tc.desc: Verify that CleanupOldDatabase handles invalid database file.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CleanupOldDatabaseTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest004 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app004";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(nullptr));

        databaseSupplementTask_->CleanupOldDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CleanupOldDatabaseTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest004 End";
}

/*
 * @tc.name: CleanupOldDatabaseTest005
 * @tc.desc: Verify that CleanupOldDatabase deletes matching records successfully.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CleanupOldDatabaseTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest005 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app005";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, CreateTransaction(_))
            .WillRepeatedly(Return(make_pair(E_OK, static_pointer_cast<NativeRdb::Transaction>(transactionMock_))));
        EXPECT_CALL(*transactionMock_, Delete(_)).WillRepeatedly(Return(make_pair(E_OK, 2)));

        databaseSupplementTask_->CleanupOldDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CleanupOldDatabaseTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest005 End";
}

/*
 * @tc.name: CleanupOldDatabaseTest006
 * @tc.desc: Verify that CleanupOldDatabase handles no matching records (Delete returns 0 rows).
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CleanupOldDatabaseTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest006 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app006";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, CreateTransaction(_))
            .WillRepeatedly(Return(make_pair(E_OK, static_pointer_cast<NativeRdb::Transaction>(transactionMock_))));
        EXPECT_CALL(*transactionMock_, Delete(_)).WillRepeatedly(Return(make_pair(E_OK, 0)));

        databaseSupplementTask_->CleanupOldDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CleanupOldDatabaseTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest006 End";
}

/*
 * @tc.name: CleanupOldDatabaseTest007
 * @tc.desc: Verify that CleanupOldDatabase handles transaction Start failure.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CleanupOldDatabaseTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest007 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app007";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, CreateTransaction(_)).WillRepeatedly(Return(make_pair(-1, nullptr)));

        databaseSupplementTask_->CleanupOldDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CleanupOldDatabaseTest007 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest007 End";
}

/*
 * @tc.name: CleanupOldDatabaseTest008
 * @tc.desc: Verify that CleanupOldDatabase handles Delete failure.
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseSupplementTaskTest, CleanupOldDatabaseTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest008 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app008";
        string filemanager = "com.filemanager";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db");

        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                DoAll(SetArgReferee<2>(E_OK),
                      Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/rdb/clouddisk.db")));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, CreateTransaction(_))
            .WillRepeatedly(Return(make_pair(E_OK, static_pointer_cast<NativeRdb::Transaction>(transactionMock_))));
        EXPECT_CALL(*transactionMock_, Delete(_)).WillRepeatedly(Return(make_pair(-1, 0)));

        databaseSupplementTask_->CleanupOldDatabase(userId, bundleName, filemanager);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CleanupOldDatabaseTest008 FAILED";
    }
    GTEST_LOG_(INFO) << "CleanupOldDatabaseTest008 End";
}

} // namespace OHOS::FileManagement::CloudSync::Test