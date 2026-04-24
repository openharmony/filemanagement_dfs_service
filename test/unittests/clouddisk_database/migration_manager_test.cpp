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

#include "dfs_error.h"
#include "migration_manager.h"
#include "migration_rdb_mock.h"
#include "parameters.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using NativeRdb::E_OK;
using Values = NativeRdb::RdbStore::Values;
using OHOS::system::ParameterMock;

class MigrationManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline MigrationManager *migrationManager_ = nullptr;
    static inline shared_ptr<ParameterMock> parameterMock_ = nullptr;
    static inline shared_ptr<MigrationAssistantMock> assistantMock_ = nullptr;
    static inline shared_ptr<MigrationRdbStoreMock> rdbStoreMock_ = nullptr;
    static inline shared_ptr<MigrationResultSetMock> resultSetMock_ = nullptr;
};

void MigrationManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    migrationManager_ = &MigrationManager::GetInstance();
    parameterMock_ = make_shared<ParameterMock>();
    ParameterMock::proxy_ = parameterMock_;
    assistantMock_ = make_shared<MigrationAssistantMock>();
    rdbStoreMock_ = make_shared<MigrationRdbStoreMock>();
    resultSetMock_ = make_shared<MigrationResultSetMock>();
}

void MigrationManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    migrationManager_ = nullptr;
    ParameterMock::proxy_ = nullptr;
    parameterMock_ = nullptr;
    MigrationAssistantMock::ins = nullptr;
    assistantMock_ = nullptr;
    rdbStoreMock_ = nullptr;
    resultSetMock_ = nullptr;
}

void MigrationManagerTest::SetUp(void)
{
    std::system("rm -rf /data/test_tdd");
    std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager");
    {
        lock_guard<mutex> lock(migrationManager_->mutex_);
        migrationManager_->appStates_.clear();
    }
    MigrationAssistantMock::ins = assistantMock_;
}

void MigrationManagerTest::TearDown(void)
{
    std::system("rm -rf /data/test_tdd");
    std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager");
    MigrationAssistantMock::ins = nullptr;
}

/*
 * @tc.name: GetInstanceTest001
 * @tc.desc: Verify that GetInstance returns valid instance.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, GetInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest001 Start";
    try {
        auto &instance = MigrationManager::GetInstance();
        EXPECT_NE(&instance, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetInstanceTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest001 End";
}

/*
 * @tc.name: GetInstanceTest002
 * @tc.desc: Verify that GetInstance returns the same instance every time.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, GetInstanceTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest002 Start";
    try {
        auto &instance1 = MigrationManager::GetInstance();
        auto &instance2 = MigrationManager::GetInstance();
        EXPECT_EQ(&instance1, &instance2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetInstanceTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest002 End";
}

/*
 * @tc.name: GetAppMarkerPathTest001
 * @tc.desc: Verify that GetAppMarkerPath returns correct path for .migrating marker.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, GetAppMarkerPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAppMarkerPathTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";
        string type = ".migrating";

        string result = migrationManager_->GetAppMarkerPath(userId, bundleName, type);
        EXPECT_EQ(result, "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrating");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetAppMarkerPathTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetAppMarkerPathTest001 End";
}

/*
 * @tc.name: GetAppMarkerPathTest002
 * @tc.desc: Verify that GetAppMarkerPath returns correct path for .migrated marker.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, GetAppMarkerPathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAppMarkerPathTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.photos";
        string type = ".migrated";

        string result = migrationManager_->GetAppMarkerPath(userId, bundleName, type);
        EXPECT_EQ(result, "/data/service/el2/100/hmdfs/cloudfile_manager/com.photos/.migrated");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetAppMarkerPathTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "GetAppMarkerPathTest002 End";
}

/*
 * @tc.name: GetAppMarkerPathTest003
 * @tc.desc: Verify that GetAppMarkerPath returns correct path for .migration_checked marker.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, GetAppMarkerPathTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAppMarkerPathTest003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.filemanager";
        string type = ".migration_checked";

        string result = migrationManager_->GetAppMarkerPath(userId, bundleName, type);
        EXPECT_EQ(result, "/data/service/el2/100/hmdfs/cloudfile_manager/com.filemanager/.migration_checked");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetAppMarkerPathTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "GetAppMarkerPathTest003 End";
}

/*
 * @tc.name: GetOldDbDirTest001
 * @tc.desc: Verify that GetOldDbDir returns correct directory path.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, GetOldDbDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetOldDbDirTest001 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager";

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));

        string result = migrationManager_->GetOldDbDir(userId);
        EXPECT_FALSE(result.empty());
        EXPECT_TRUE(result.find("/data/service/el2/100/hmdfs/cloudfile_manager/") != string::npos);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetOldDbDirTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetOldDbDirTest001 End";
}

/*
 * @tc.name: GetOldDbPathTest001
 * @tc.desc: Verify that GetOldDbPath returns correct database path.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, GetOldDbPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetOldDbPathTest001 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager";

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));

        string result = migrationManager_->GetOldDbPath(userId);
        EXPECT_FALSE(result.empty());
        EXPECT_TRUE(result.find("/rdb/clouddisk.db") != string::npos);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetOldDbPathTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetOldDbPathTest001 End";
}

/*
 * @tc.name: GetNewDbDirTest001
 * @tc.desc: Verify that GetNewDbDir returns correct directory path.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, GetNewDbDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNewDbDirTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";

        string result = migrationManager_->GetNewDbDir(userId, bundleName);
        EXPECT_EQ(result, "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetNewDbDirTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetNewDbDirTest001 End";
}

/*
 * @tc.name: GetNewDbPathTest001
 * @tc.desc: Verify that GetNewDbPath returns correct database path.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, GetNewDbPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNewDbPathTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";

        string result = migrationManager_->GetNewDbPath(userId, bundleName);
        EXPECT_EQ(result, "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/rdb/clouddisk.db");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "GetNewDbPathTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "GetNewDbPathTest001 End";
}

/*
 * @tc.name: CreateMarkerFileTest001
 * @tc.desc: Verify that CreateMarkerFile creates marker file successfully.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, CreateMarkerFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateMarkerFileTest001 Start";
    try {
        std::system("mkdir -p /data/test_tdd");
        string markerPath = "/data/test_tdd/.test_marker";

        migrationManager_->CreateMarkerFile(markerPath);

        EXPECT_EQ(access(markerPath.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CreateMarkerFileTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CreateMarkerFileTest001 End";
}

/*
 * @tc.name: DeleteMarkerFileTest001
 * @tc.desc: Verify that DeleteMarkerFile removes marker file successfully.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, DeleteMarkerFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteMarkerFileTest001 Start";
    try {
        std::system("mkdir -p /data/test_tdd");
        string markerPath = "/data/test_tdd/.test_marker";
        std::system(("touch " + markerPath).c_str());

        migrationManager_->DeleteMarkerFile(markerPath);

        EXPECT_NE(access(markerPath.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "DeleteMarkerFileTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "DeleteMarkerFileTest001 End";
}

/*
 * @tc.name: HasMarkerFileTest001
 * @tc.desc: Verify that HasMarkerFile returns true when file exists.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, HasMarkerFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasMarkerFileTest001 Start";
    try {
        std::system("mkdir -p /data/test_tdd");
        string markerPath = "/data/test_tdd/.test_marker";
        std::system(("touch " + markerPath).c_str());

        bool result = migrationManager_->HasMarkerFile(markerPath);
        EXPECT_TRUE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "HasMarkerFileTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "HasMarkerFileTest001 End";
}

/*
 * @tc.name: HasMarkerFileTest002
 * @tc.desc: Verify that HasMarkerFile returns false when file does not exist.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, HasMarkerFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HasMarkerFileTest002 Start";
    try {
        string markerPath = "/data/test_tdd/.nonexistent_marker";

        bool result = migrationManager_->HasMarkerFile(markerPath);
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "HasMarkerFileTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "HasMarkerFileTest002 End";
}

/*
 * @tc.name: StartAsyncMigrationTest001
 * @tc.desc: Verify that StartAsyncMigration returns early when migration already checked.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, StartAsyncMigrationTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest001 Start";
    try {
        int32_t userId = 100;
        string filemanager = "filemanager";

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/filemanager");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/filemanager/.migration_checked");

        migrationManager_->StartAsyncMigration(userId);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager/filemanager");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "StartAsyncMigrationTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest001 End";
}

/*
 * @tc.name: StartAsyncMigrationTest002
 * @tc.desc: Verify that StartAsyncMigration returns early when old database not found.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, StartAsyncMigrationTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest002 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager002";

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager");

        migrationManager_->StartAsyncMigration(userId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "StartAsyncMigrationTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest002 End";
}

/*
 * @tc.name: StartAsyncMigrationTest003
 * @tc.desc: Verify that StartAsyncMigration handles rdbStore nullptr (invalid db file).
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, StartAsyncMigrationTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest003 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager003";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager003/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager003/rdb/clouddisk.db");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));
        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager003/rdb/clouddisk.db"));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(nullptr));

        migrationManager_->StartAsyncMigration(userId);

        string checkedMarker =
            "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager003/.migration_checked";
        EXPECT_NE(access(checkedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "StartAsyncMigrationTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest003 End";
}

/*
 * @tc.name: StartAsyncMigrationTest004
 * @tc.desc: Verify that StartAsyncMigration skips bundle when bundle equals filemanager.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, StartAsyncMigrationTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest004 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager004";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager004/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager004/rdb/clouddisk.db");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));
        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager004/rdb/clouddisk.db"));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(E_OK)).WillRepeatedly(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>(filemanager), Return(E_OK)));

        migrationManager_->StartAsyncMigration(userId);

        string checkedMarker =
            "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager004/.migration_checked";
        EXPECT_EQ(access(checkedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "StartAsyncMigrationTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest004 End";
}

/*
 * @tc.name: StartAsyncMigrationTest005
 * @tc.desc: Verify that StartAsyncMigration skips bundle when migrated marker exists.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, StartAsyncMigrationTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest005 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager005";
        string bundle = "com.test.app005";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager005/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager005/rdb/clouddisk.db");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app005");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app005/.migrated");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));
        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager005/rdb/clouddisk.db"));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(E_OK)).WillRepeatedly(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>(bundle), Return(E_OK)));

        migrationManager_->StartAsyncMigration(userId);

        string checkedMarker =
            "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager005/.migration_checked";
        EXPECT_EQ(access(checkedMarker.c_str(), F_OK), 0);

        string migratingMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app005/.migrating";
        EXPECT_NE(access(migratingMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "StartAsyncMigrationTest005 FAILED";
    }
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest005 End";
}

/*
 * @tc.name: StartAsyncMigrationTest006
 * @tc.desc: Verify that StartAsyncMigration skips bundle when migrating marker exists.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, StartAsyncMigrationTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest006 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager006";
        string bundle = "com.test.app006";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager006/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager006/rdb/clouddisk.db");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app006");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app006/.migrating");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));
        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager006/rdb/clouddisk.db"));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(E_OK)).WillRepeatedly(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>(bundle), Return(E_OK)));

        migrationManager_->StartAsyncMigration(userId);

        string checkedMarker =
            "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager006/.migration_checked";
        EXPECT_EQ(access(checkedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "StartAsyncMigrationTest006 FAILED";
    }
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest006 End";
}

/*
 * @tc.name: StartAsyncMigrationTest007
 * @tc.desc: Verify that StartAsyncMigration returns early when no bundles to migrate.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, StartAsyncMigrationTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest007 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager007";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager007/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager007/rdb/clouddisk.db");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));
        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager007/rdb/clouddisk.db"));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillRepeatedly(Return(-1));

        migrationManager_->StartAsyncMigration(userId);

        string checkedMarker =
            "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager007/.migration_checked";
        EXPECT_EQ(access(checkedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "StartAsyncMigrationTest007 FAILED";
    }
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest007 End";
}

/*
 * @tc.name: StartAsyncMigrationTest008
 * @tc.desc: Verify that StartAsyncMigration creates migrating markers for bundles to migrate.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, StartAsyncMigrationTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest008 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager008";
        string bundle = "com.test.app008";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager008/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager008/rdb/clouddisk.db");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app008");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));
        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager008/rdb/clouddisk.db"));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(E_OK)).WillRepeatedly(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>(bundle), Return(E_OK)));
        EXPECT_CALL(*resultSetMock_, Close()).WillRepeatedly(Return(E_OK));

        migrationManager_->StartAsyncMigration(userId);

        string migratingMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app008/.migrating";
        EXPECT_EQ(access(migratingMarker.c_str(), F_OK), 0);

        string checkedMarker =
            "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager008/.migration_checked";
        EXPECT_NE(access(checkedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "StartAsyncMigrationTest008 FAILED";
    }
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest008 End";
}

/*
 * @tc.name: StartAsyncMigrationTest009
 * @tc.desc: Verify that StartAsyncMigration handles GetDefaultDatabasePath error.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, StartAsyncMigrationTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest009 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager009";
        int errCode = -1;

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager009/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager009/rdb/clouddisk.db");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));
        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(errCode), Return("")));

        migrationManager_->StartAsyncMigration(userId);

        string checkedMarker =
            "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager009/.migration_checked";
        EXPECT_NE(access(checkedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "StartAsyncMigrationTest009 FAILED";
    }
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest009 End";
}

/*
 * @tc.name: StartAsyncMigrationTest010
 * @tc.desc: Verify that StartAsyncMigration handles QuerySql returns nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, StartAsyncMigrationTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest010 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager010";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager010/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager010/rdb/clouddisk.db");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));
        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager010/rdb/clouddisk.db"));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(nullptr));

        migrationManager_->StartAsyncMigration(userId);

        string checkedMarker =
            "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager010/.migration_checked";
        EXPECT_NE(access(checkedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "StartAsyncMigrationTest010 FAILED";
    }
    GTEST_LOG_(INFO) << "StartAsyncMigrationTest010 End";
}

/*
 * @tc.name: WaitForAppMigrationTest001
 * @tc.desc: Verify that WaitForAppMigration returns early when no migrating marker.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, WaitForAppMigrationTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WaitForAppMigrationTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";

        migrationManager_->WaitForAppMigration(userId, bundleName);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "WaitForAppMigrationTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "WaitForAppMigrationTest001 End";
}

/*
 * @tc.name: WaitForAppMigrationTest002
 * @tc.desc: Verify that WaitForAppMigration handles stale migrating marker.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, WaitForAppMigrationTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WaitForAppMigrationTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrating");

        migrationManager_->WaitForAppMigration(userId, bundleName);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "WaitForAppMigrationTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "WaitForAppMigrationTest002 End";
}

/*
 * @tc.name: WaitForAppMigrationTest003
 * @tc.desc: Verify that WaitForAppMigration does not delete marker when migration completed.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, WaitForAppMigrationTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WaitForAppMigrationTest003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app003";
        string appKey = to_string(userId) + "_" + bundleName;

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/.migrating");

        {
            lock_guard<mutex> lock(migrationManager_->mutex_);
            migrationManager_->appStates_[appKey] = make_unique<AppMigrationState>();
            migrationManager_->appStates_[appKey]->completed = true;
        }

        migrationManager_->WaitForAppMigration(userId, bundleName);

        string migratingMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/.migrating";
        EXPECT_EQ(access(migratingMarker.c_str(), F_OK), 0);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "WaitForAppMigrationTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "WaitForAppMigrationTest003 End";
}

/*
 * @tc.name: WaitForAppMigrationTest004
 * @tc.desc: Verify that WaitForAppMigration deletes marker when migration timeout.
 *           Note: This test will wait TIMEOUT_SECONDS (10s) to verify timeout branch.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, WaitForAppMigrationTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WaitForAppMigrationTest004 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app004";
        string appKey = to_string(userId) + "_" + bundleName;

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/.migrating");

        {
            lock_guard<mutex> lock(migrationManager_->mutex_);
            migrationManager_->appStates_[appKey] = make_unique<AppMigrationState>();
            migrationManager_->appStates_[appKey]->completed = false;
        }

        migrationManager_->WaitForAppMigration(userId, bundleName);

        string migratingMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004/.migrating";
        EXPECT_NE(access(migratingMarker.c_str(), F_OK), 0);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app004");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "WaitForAppMigrationTest004 FAILED";
    }
    GTEST_LOG_(INFO) << "WaitForAppMigrationTest004 End";
}

/*
 * @tc.name: OnAppMigrationSuccessTest001
 * @tc.desc: Verify that OnAppMigrationSuccess creates migrated marker and deletes migrating marker.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, OnAppMigrationSuccessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAppMigrationSuccessTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrating");

        string appKey = to_string(userId) + "_" + bundleName;
        {
            lock_guard<mutex> lock(migrationManager_->mutex_);
            migrationManager_->appStates_[appKey] = make_unique<AppMigrationState>();
        }

        migrationManager_->OnAppMigrationSuccess(userId, bundleName);

        string migratedMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrated";
        string migratingMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrating";

        EXPECT_EQ(access(migratedMarker.c_str(), F_OK), 0);
        EXPECT_NE(access(migratingMarker.c_str(), F_OK), 0);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "OnAppMigrationSuccessTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "OnAppMigrationSuccessTest001 End";
}

/*
 * @tc.name: OnAppMigrationSuccessTest002
 * @tc.desc: Verify that OnAppMigrationSuccess returns early when migrating marker not found.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, OnAppMigrationSuccessTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAppMigrationSuccessTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app002";

        string appKey = to_string(userId) + "_" + bundleName;
        {
            lock_guard<mutex> lock(migrationManager_->mutex_);
            migrationManager_->appStates_[appKey] = make_unique<AppMigrationState>();
        }

        migrationManager_->OnAppMigrationSuccess(userId, bundleName);

        {
            lock_guard<mutex> lock(migrationManager_->mutex_);
            EXPECT_EQ(migrationManager_->appStates_.find(appKey), migrationManager_->appStates_.end());
        }

        string migratedMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app002/.migrated";
        EXPECT_NE(access(migratedMarker.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "OnAppMigrationSuccessTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "OnAppMigrationSuccessTest002 End";
}

/*
 * @tc.name: OnAppMigrationFailedTest001
 * @tc.desc: Verify that OnAppMigrationFailed deletes migrating marker.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, OnAppMigrationFailedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAppMigrationFailedTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrating");

        string appKey = to_string(userId) + "_" + bundleName;
        {
            lock_guard<mutex> lock(migrationManager_->mutex_);
            migrationManager_->appStates_[appKey] = make_unique<AppMigrationState>();
        }

        migrationManager_->OnAppMigrationFailed(userId, bundleName);

        string migratingMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app/.migrating";
        EXPECT_NE(access(migratingMarker.c_str(), F_OK), 0);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "OnAppMigrationFailedTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "OnAppMigrationFailedTest001 End";
}

/*
 * @tc.name: CreateAppMigratingMarkersTest001
 * @tc.desc: Verify that CreateAppMigratingMarkers creates markers and app states.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, CreateAppMigratingMarkersTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAppMigratingMarkersTest001 Start";
    try {
        int32_t userId = 100;
        vector<string> bundleNames = {"com.test.app1", "com.test.app2"};

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager");

        migrationManager_->CreateAppMigratingMarkers(userId, bundleNames);

        string marker1 = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app1/.migrating";
        string marker2 = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app2/.migrating";

        EXPECT_EQ(access(marker1.c_str(), F_OK), 0);
        EXPECT_EQ(access(marker2.c_str(), F_OK), 0);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CreateAppMigratingMarkersTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CreateAppMigratingMarkersTest001 End";
}

/*
 * @tc.name: CopyDatabaseTest001
 * @tc.desc: Verify that CopyDatabase returns false when new directory cannot be created.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, CopyDatabaseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDatabaseTest001 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app";

        bool result = migrationManager_->CopyDatabase(userId, bundleName);
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CopyDatabaseTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "CopyDatabaseTest001 End";
}

/*
 * @tc.name: ParallelCopyDatabaseTest001
 * @tc.desc: Verify that ParallelCopyDatabase handles empty bundle names.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, ParallelCopyDatabaseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParallelCopyDatabaseTest001 Start";
    try {
        int32_t userId = 100;
        vector<string> bundleNames;

        bool result = migrationManager_->ParallelCopyDatabase(userId, bundleNames);
        EXPECT_TRUE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "ParallelCopyDatabaseTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "ParallelCopyDatabaseTest001 End";
}

/*
 * @tc.name: DoMigrationTest001
 * @tc.desc: Verify that DoMigration handles empty bundles.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, DoMigrationTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoMigrationTest001 Start";
    try {
        int32_t userId = 100;

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(""));

        migrationManager_->DoMigration(userId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "DoMigrationTest001 FAILED";
    }
    GTEST_LOG_(INFO) << "DoMigrationTest001 End";
}

/*
 * @tc.name: DoMigrationTest002
 * @tc.desc: Verify that DoMigration skips bundle when bundle equals filemanager.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, DoMigrationTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoMigrationTest002 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager002";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager002/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager002/rdb/clouddisk.db");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));
        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager002/rdb/clouddisk.db"));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(E_OK)).WillRepeatedly(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>(filemanager), Return(E_OK)));

        migrationManager_->DoMigration(userId);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "DoMigrationTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "DoMigrationTest002 End";
}

/*
 * @tc.name: DoMigrationTest003
 * @tc.desc: Verify that DoMigration processes bundles with migrating marker.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, DoMigrationTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoMigrationTest003 Start";
    try {
        int32_t userId = 100;
        string filemanager = "com.test.filemanager003";
        string bundle = "com.test.app003";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager003/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager003/rdb/clouddisk.db");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/.migrating");

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));
        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(
                Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager003/rdb/clouddisk.db"));
        EXPECT_CALL(*assistantMock_, GetRdbStore(_, _, _, _)).WillRepeatedly(Return(rdbStoreMock_));
        EXPECT_CALL(*rdbStoreMock_, QuerySql(An<const string &>(), An<const Values &>()))
            .WillRepeatedly(Return(resultSetMock_));
        EXPECT_CALL(*resultSetMock_, GoToNextRow()).WillOnce(Return(E_OK)).WillRepeatedly(Return(-1));
        EXPECT_CALL(*resultSetMock_, GetString(_, _)).WillOnce(DoAll(SetArgReferee<1>(bundle), Return(E_OK)));

        migrationManager_->DoMigration(userId);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "DoMigrationTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "DoMigrationTest003 End";
}

/*
 * @tc.name: CreateAppMigratingMarkersTest002
 * @tc.desc: Verify that CreateAppMigratingMarkers handles existing directory.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, CreateAppMigratingMarkersTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateAppMigratingMarkersTest002 Start";
    try {
        int32_t userId = 100;
        vector<string> bundleNames = {"com.test.app002"};

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app002");

        migrationManager_->CreateAppMigratingMarkers(userId, bundleNames);

        string marker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app002/.migrating";
        EXPECT_EQ(access(marker.c_str(), F_OK), 0);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CreateAppMigratingMarkersTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "CreateAppMigratingMarkersTest002 End";
}

/*
 * @tc.name: CopyDatabaseTest002
 * @tc.desc: Verify that CopyDatabase returns false when old db not exists.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, CopyDatabaseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDatabaseTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app002";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app002/rdb");

        bool result = migrationManager_->CopyDatabase(userId, bundleName);
        EXPECT_FALSE(result);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CopyDatabaseTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "CopyDatabaseTest002 End";
}

/*
 * @tc.name: CopyDatabaseTest003
 * @tc.desc: Verify that CopyDatabase copies database successfully.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, CopyDatabaseTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CopyDatabaseTest003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app003";
        string filemanager = "com.test.filemanager003";

        EXPECT_CALL(*parameterMock_, GetParameter(_, _)).WillRepeatedly(Return(filemanager));
        EXPECT_CALL(*assistantMock_, GetDefaultDatabasePath(_, _, _))
            .WillRepeatedly(Return("/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/rdb/clouddisk.db"));

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager003/rdb");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.filemanager003/rdb/clouddisk.db");
        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/rdb");

        bool result = migrationManager_->CopyDatabase(userId, bundleName);
        EXPECT_TRUE(result);

        string newDbPath = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/rdb/clouddisk.db";
        EXPECT_EQ(access(newDbPath.c_str(), F_OK), 0);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "CopyDatabaseTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "CopyDatabaseTest003 End";
}

/*
 * @tc.name: OnAppMigrationSuccessTest003
 * @tc.desc: Verify that OnAppMigrationSuccess handles when appStates has no appKey.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, OnAppMigrationSuccessTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAppMigrationSuccessTest003 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app003";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/.migrating");

        migrationManager_->OnAppMigrationSuccess(userId, bundleName);

        string migratedMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/.migrated";
        string migratingMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003/.migrating";

        EXPECT_EQ(access(migratedMarker.c_str(), F_OK), 0);
        EXPECT_NE(access(migratingMarker.c_str(), F_OK), 0);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app003");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "OnAppMigrationSuccessTest003 FAILED";
    }
    GTEST_LOG_(INFO) << "OnAppMigrationSuccessTest003 End";
}

/*
 * @tc.name: OnAppMigrationFailedTest002
 * @tc.desc: Verify that OnAppMigrationFailed handles when appStates has no appKey.
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, OnAppMigrationFailedTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnAppMigrationFailedTest002 Start";
    try {
        int32_t userId = 100;
        string bundleName = "com.test.app002";

        std::system("mkdir -p /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app002");
        std::system("touch /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app002/.migrating");

        migrationManager_->OnAppMigrationFailed(userId, bundleName);

        string migratingMarker = "/data/service/el2/100/hmdfs/cloudfile_manager/com.test.app002/.migrating";
        EXPECT_NE(access(migratingMarker.c_str(), F_OK), 0);

        std::system("rm -rf /data/service/el2/100/hmdfs/cloudfile_manager/com.test.app002");
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "OnAppMigrationFailedTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "OnAppMigrationFailedTest002 End";
}

/*
 * @tc.name: DeleteMarkerFileTest002
 * @tc.desc: Verify that DeleteMarkerFile handles file not exist (ENOENT).
 * @tc.type: FUNC
 */
HWTEST_F(MigrationManagerTest, DeleteMarkerFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DeleteMarkerFileTest002 Start";
    try {
        string path = "/data/service/el2/100/hmdfs/cloudfile_manager/nonexistent/.marker";

        migrationManager_->DeleteMarkerFile(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(ERROR) << "DeleteMarkerFileTest002 FAILED";
    }
    GTEST_LOG_(INFO) << "DeleteMarkerFileTest002 End";
}

} // namespace OHOS::FileManagement::CloudDisk::Test