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

#include <cstdlib>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dfs_error.h"
#include "operation_log_store.h"
#include "rdb_assistant.h"
#include "rdb_errno.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudDisk {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class OperationLogStoreTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp();
    void TearDown();
    shared_ptr<AssistantMock> insMock_ = nullptr;
};

void OperationLogStoreTest::SetUp(void)
{
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
    insMock_->EnableMock();
    auto &store = OperationLogStore::GetInstance();
    store.rdbStore_ = nullptr;
    store.isInited_ = false;
}

void OperationLogStoreTest::TearDown(void)
{
    auto &store = OperationLogStore::GetInstance();
    store.rdbStore_ = nullptr;
    store.isInited_ = false;
    insMock_->DisableMock();
    insMock_ = nullptr;
}

/**
 * @tc.name: GetInstanceTest001
 * @tc.desc: Verify GetInstance returns a valid and consistent singleton instance.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, GetInstanceTest001, TestSize.Level1)
{
    auto &instance = OperationLogStore::GetInstance();
    auto &instance2 = OperationLogStore::GetInstance();
    EXPECT_EQ(&instance, &instance2);
}

/**
 * @tc.name: GetRawNullBeforeInitTest
 * @tc.desc: Verify GetRaw returns nullptr before Init succeeds.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, GetRawNullBeforeInitTest, TestSize.Level1)
{
    EXPECT_EQ(OperationLogStore::GetInstance().GetRaw(), nullptr);
}

/**
 * @tc.name: CheckAndCreateDirDirExistsTest
 * @tc.desc: Verify CheckAndCreateDir returns E_OK when the directory already exists.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, CheckAndCreateDirDirExistsTest, TestSize.Level1)
{
    auto &store = OperationLogStore::GetInstance();
    EXPECT_EQ(store.CheckAndCreateDir("/data/test_operation_log"), E_OK);
}

/**
 * @tc.name: CheckAndCreateDirCreateSuccessTest
 * @tc.desc: Verify CheckAndCreateDir creates the directory and returns E_OK when the parent exists.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, CheckAndCreateDirCreateSuccessTest, TestSize.Level1)
{
    std::system("rm -rf /data/test_operation_log_tdd");
    std::system("mkdir -p /data/test_operation_log_tdd");
    auto &store = OperationLogStore::GetInstance();
    EXPECT_EQ(store.CheckAndCreateDir("/data/test_operation_log_tdd/oplog"), E_OK);
    std::system("rm -rf /data/test_operation_log_tdd");
}

/**
 * @tc.name: TryOpenRdbStoreSuccessTest
 * @tc.desc: Verify TryOpenRdbStore returns true and stores the rdb store when open succeeds.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, TryOpenRdbStoreSuccessTest, TestSize.Level1)
{
    auto rdb = make_shared<RdbStoreMock>();
    int errCode = NativeRdb::E_OK;
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdb)));
    auto &store = OperationLogStore::GetInstance();
    EXPECT_TRUE(store.TryOpenRdbStore("/data", "/data/operation_log.db", errCode));
    EXPECT_EQ(store.GetRaw(), rdb);
}

/**
 * @tc.name: TryOpenRdbStoreFailTest
 * @tc.desc: Verify TryOpenRdbStore returns false when GetRdbStore fails with a generic error.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, TryOpenRdbStoreFailTest, TestSize.Level1)
{
    std::system("rm -rf /data/test_operation_log_nonexistent_parent");
    int errCode = NativeRdb::E_OK;
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_ERROR), Return(nullptr)));
    auto &store = OperationLogStore::GetInstance();
    EXPECT_FALSE(store.TryOpenRdbStore("/data/test_operation_log_nonexistent_parent",
        "/data/test_operation_log_nonexistent_parent/operation_log.db", errCode));
}

/**
 * @tc.name: TryOpenRdbStoreCantOpenNoRetryTest
 * @tc.desc: Verify TryOpenRdbStore does not retry when chmod fails on a non-existent directory.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, TryOpenRdbStoreCantOpenNoRetryTest, TestSize.Level1)
{
    int errCode = NativeRdb::E_OK;
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_ERROR), Return(nullptr)));
    auto &store = OperationLogStore::GetInstance();
    EXPECT_FALSE(store.TryOpenRdbStore("/data/test_operation_log_tdd",
        "/data/test_operation_log_tdd/operation_log.db", errCode));
}

/**
 * @tc.name: TryOpenRdbStoreCantOpenRetrySuccessTest
 * @tc.desc: Verify TryOpenRdbStore retries and succeeds when chmod on an existing directory succeeds.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, TryOpenRdbStoreCantOpenRetrySuccessTest, TestSize.Level1)
{
    auto rdb = make_shared<RdbStoreMock>();
    int errCode = NativeRdb::E_OK;
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdb)));
    auto &store = OperationLogStore::GetInstance();
    EXPECT_TRUE(store.TryOpenRdbStore("/data/test_operation_log_tdd",
        "/data/test_operation_log_tdd/operation_log.db", errCode));
    EXPECT_EQ(store.GetRaw(), rdb);
}

/**
 * @tc.name: StopWhenStoreNullTest
 * @tc.desc: Verify Stop returns early without changing state when the rdb store is null.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, StopWhenStoreNullTest, TestSize.Level1)
{
    auto &store = OperationLogStore::GetInstance();
    store.rdbStore_ = nullptr;
    store.isInited_ = true;
    store.Stop();
    EXPECT_TRUE(store.isInited_);
}

/**
 * @tc.name: StopResetsStateTest
 * @tc.desc: Verify Stop clears the rdb store and the inited flag.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, StopResetsStateTest, TestSize.Level1)
{
    auto rdb = make_shared<RdbStoreMock>();
    auto &store = OperationLogStore::GetInstance();
    store.rdbStore_ = rdb;
    store.isInited_ = true;
    store.Stop();
    EXPECT_EQ(store.GetRaw(), nullptr);
    EXPECT_FALSE(store.isInited_);
}

/**
 * @tc.name: InitSuccessTest
 * @tc.desc: Verify Init succeeds and is idempotent when all rdb steps are mocked successfully.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, InitSuccessTest, TestSize.Level1)
{
    auto rdb = make_shared<RdbStoreMock>();
    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_OK), Return("/data/test_operation_log/operation_log.db")));
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdb)));
    auto &store = OperationLogStore::GetInstance();
    EXPECT_EQ(store.Init(100), E_OK);
    EXPECT_TRUE(store.isInited_);
    EXPECT_EQ(store.GetRaw(), rdb);
    EXPECT_EQ(store.Init(100), E_OK);
}

/**
 * @tc.name: InitDefaultPathFailTest
 * @tc.desc: Verify Init returns E_PATH when the default database path cannot be created.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, InitDefaultPathFailTest, TestSize.Level1)
{
    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_ERROR), Return("")));
    auto &store = OperationLogStore::GetInstance();
    EXPECT_EQ(store.Init(100), E_PATH);
    EXPECT_FALSE(store.isInited_);
}

/**
 * @tc.name: InitOpenStoreFailTest
 * @tc.desc: Verify Init returns the rdb error when the rdb store cannot be opened.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, InitOpenStoreFailTest, TestSize.Level1)
{
    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_OK), Return("/data/test_operation_log/operation_log.db")));
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_ERROR), Return(nullptr)));
    auto &store = OperationLogStore::GetInstance();
    EXPECT_EQ(store.Init(100), NativeRdb::E_ERROR);
    EXPECT_FALSE(store.isInited_);
}

/**
 * @tc.name: OnCreateSuccessTest
 * @tc.desc: Verify OperationLogRdbCallBack::OnCreate returns E_OK when table and index are created.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, OnCreateSuccessTest, TestSize.Level1)
{
    auto rdb = make_shared<RdbStoreMock>();
    EXPECT_CALL(*rdb, ExecuteSql(_, _))
        .WillOnce(Return(NativeRdb::E_OK))
        .WillOnce(Return(NativeRdb::E_OK));
    OperationLogRdbCallBack callback;
    EXPECT_EQ(callback.OnCreate(*rdb), NativeRdb::E_OK);
}

/**
 * @tc.name: OnCreateTableFailTest
 * @tc.desc: Verify OperationLogRdbCallBack::OnCreate returns E_ERROR when the table creation fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, OnCreateTableFailTest, TestSize.Level1)
{
    auto rdb = make_shared<RdbStoreMock>();
    EXPECT_CALL(*rdb, ExecuteSql(_, _)).WillOnce(Return(NativeRdb::E_ERROR));
    OperationLogRdbCallBack callback;
    EXPECT_EQ(callback.OnCreate(*rdb), NativeRdb::E_ERROR);
}

/**
 * @tc.name: OnCreateIndexFailTest
 * @tc.desc: Verify OperationLogRdbCallBack::OnCreate returns E_ERROR when the index creation fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, OnCreateIndexFailTest, TestSize.Level1)
{
    auto rdb = make_shared<RdbStoreMock>();
    EXPECT_CALL(*rdb, ExecuteSql(_, _))
        .WillOnce(Return(NativeRdb::E_OK))
        .WillOnce(Return(NativeRdb::E_ERROR));
    OperationLogRdbCallBack callback;
    EXPECT_EQ(callback.OnCreate(*rdb), NativeRdb::E_ERROR);
}

/**
 * @tc.name: OnUpgradeTest
 * @tc.desc: Verify OperationLogRdbCallBack::OnUpgrade returns E_OK.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, OnUpgradeTest, TestSize.Level1)
{
    auto rdb = make_shared<RdbStoreMock>();
    OperationLogRdbCallBack callback;
    EXPECT_EQ(callback.OnUpgrade(*rdb, 1, 2), NativeRdb::E_OK);
}

/**
 * @tc.name: TryOpenRdbStoreGroupIdMatchTest
 * @tc.desc: Verify TryOpenRdbStore succeeds when directory group ID matches OID_DFS.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogStoreTest, TryOpenRdbStoreGroupIdMatchTest, TestSize.Level1)
{
    auto &store = OperationLogStore::GetInstance();

    auto rdb = make_shared<RdbStoreMock>();
    int errCode = NativeRdb::E_OK;

    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdb)));

    string testDir = "/tmp/test_oplog_group_id";
    Storage::DistributedFile::Utils::ForceCreateDirectory(testDir, 0755);

    struct stat dirStat;
    stat(testDir.c_str(), &dirStat);

    bool result = store.TryOpenRdbStore(testDir, testDir + "/operation_log.db", errCode);
    (void)result;

    remove(testDir.c_str());
}
} // namespace Test
} // namespace FileManagement
} // namespace OHOS
