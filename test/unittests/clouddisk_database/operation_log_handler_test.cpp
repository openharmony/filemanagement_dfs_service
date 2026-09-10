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

#include <chrono>
#include <mutex>
#include <thread>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "dfs_error.h"
#include "operation_log_const.h"
#include "operation_log_entry.h"
#include "operation_log_handler.h"
#include "operation_log_store.h"
#include "rdb_assistant.h"
#include "rdb_errno.h"
#include "result_set_mock.h"

namespace OHOS {
namespace FileManagement::CloudDisk {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

namespace {
OperationLogEntry CreateEntry(int64_t opTime)
{
    return OperationLogEntry{ opTime, OperationLogConst::OP_TYPE_DELETE, "/data/oplog/foo.txt",
        100, 200, "test_proc", 300, 400 };
}

void ClearQueue(OperationLogHandler &handler)
{
    std::lock_guard<std::mutex> lock(handler.queue_.mutex_);
    while (!handler.queue_.queue_.empty()) {
        handler.queue_.queue_.pop();
    }
}
} // namespace

class OperationLogHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp();
    void TearDown();
    shared_ptr<AssistantMock> insMock_ = nullptr;
    shared_ptr<RdbStoreMock> rdbStoreMock_ = nullptr;
    shared_ptr<ResultSetMock> resultSetMock_ = nullptr;
    shared_ptr<AbsSharedResultSetMock> absResultSetMock_ = nullptr;
};

void OperationLogHandlerTest::SetUp(void)
{
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
    insMock_->EnableMock();
    rdbStoreMock_ = make_shared<RdbStoreMock>();
    resultSetMock_ = make_shared<ResultSetMock>();
    absResultSetMock_ = make_shared<AbsSharedResultSetMock>();
    auto &store = OperationLogStore::GetInstance();
    store.rdbStore_ = nullptr;
    store.isInited_ = false;
    auto &handler = OperationLogHandler::GetInstance();
    handler.running_ = false;
    handler.isInited_ = false;
    ClearQueue(handler);
}

void OperationLogHandlerTest::TearDown(void)
{
    auto &store = OperationLogStore::GetInstance();
    store.rdbStore_ = nullptr;
    store.isInited_ = false;
    auto &handler = OperationLogHandler::GetInstance();
    handler.running_ = false;
    handler.isInited_ = false;
    ClearQueue(handler);
    insMock_->DisableMock();
    insMock_ = nullptr;
    rdbStoreMock_ = nullptr;
    resultSetMock_ = nullptr;
    absResultSetMock_ = nullptr;
}

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify GetInstance returns a consistent singleton instance.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetInstanceTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    auto &handler2 = OperationLogHandler::GetInstance();
    EXPECT_EQ(&handler, &handler2);
}

/**
 * @tc.name: RecordDeleteTest
 * @tc.desc: Verify RecordDelete enqueues an entry with the DELETE op type and all fields intact.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, RecordDeleteTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    auto &store = OperationLogStore::GetInstance();
    handler.isInited_ = false;
    store.isInited_ = false;
    store.rdbStore_ = rdbStoreMock_;
    Assistant::ins = insMock_;
    insMock_->EnableMock();

    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_OK), Return("/data/test_operation_log/operation_log.db")));
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdbStoreMock_)));
    ASSERT_EQ(handler.Init(0), E_OK);
    handler.Start();
    ASSERT_EQ(handler.RecordDelete(123, "/data/oplog/foo.txt", 100, 200, "test_proc", 300, 400, "aa"), E_OK);
    auto batch = handler.queue_.PopBatch();
    ASSERT_EQ(batch.size(), 1);
    EXPECT_EQ(batch[0].opTime, 123);
    EXPECT_EQ(batch[0].opType, OperationLogConst::OP_TYPE_DELETE);
    EXPECT_EQ(batch[0].filePath, "/data/oplog/foo.txt");
    EXPECT_EQ(batch[0].fileInode, 100);
    EXPECT_EQ(batch[0].fileUid, 200);
    EXPECT_EQ(batch[0].processName, "test_proc");
    EXPECT_EQ(batch[0].processPid, 300);
    EXPECT_EQ(batch[0].processUid, 400);
    handler.Stop();
}

/**
 * @tc.name: IsRecoverableErrorTest
 * @tc.desc: Verify IsRecoverableError only recognizes busy/locked/interrupt rdb errors.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, IsRecoverableErrorTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    EXPECT_TRUE(handler.IsRecoverableError(NativeRdb::E_SQLITE_BUSY));
    EXPECT_TRUE(handler.IsRecoverableError(NativeRdb::E_SQLITE_LOCKED));
    EXPECT_TRUE(handler.IsRecoverableError(NativeRdb::E_SQLITE_INTERRUPT));
    EXPECT_FALSE(handler.IsRecoverableError(NativeRdb::E_OK));
    EXPECT_FALSE(handler.IsRecoverableError(E_RDB));
}

/**
 * @tc.name: GetRecordCountNullStoreTest
 * @tc.desc: Verify GetRecordCount returns E_RDB when the rdb store is null.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetRecordCountNullStoreTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = nullptr;
    int32_t count = 0;
    EXPECT_EQ(handler.GetRecordCount(count), E_RDB);
}

/**
 * @tc.name: GetRecordCountQueryNullTest
 * @tc.desc: Verify GetRecordCount returns E_RDB when QueryByStep returns a null result set.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetRecordCountQueryNullTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>(), preCount))
        .WillOnce(Return(ByMove(nullptr)));
    int32_t count = 0;
    EXPECT_EQ(handler.GetRecordCount(count), E_RDB);
}

/**
 * @tc.name: GetRecordCountRowCountErrorTest
 * @tc.desc: Verify GetRecordCount returns E_RDB when GetRowCount fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetRecordCountRowCountErrorTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>(), preCount))
        .WillOnce(Return(ByMove(resultSetMock_)));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_))
        .WillOnce(DoAll(SetArgReferee<0>(0), Return(NativeRdb::E_ERROR)));
    int32_t count = 0;
    EXPECT_EQ(handler.GetRecordCount(count), E_RDB);
}

/**
 * @tc.name: GetRecordCountNegativeCountTest
 * @tc.desc: Verify GetRecordCount returns E_RDB when the reported count is negative.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetRecordCountNegativeCountTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>(), preCount))
        .WillOnce(Return(ByMove(resultSetMock_)));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_))
        .WillOnce(DoAll(SetArgReferee<0>(-1), Return(E_OK)));
    int32_t count = 0;
    EXPECT_EQ(handler.GetRecordCount(count), E_RDB);
}

/**
 * @tc.name: GetRecordCountSuccessTest
 * @tc.desc: Verify GetRecordCount returns E_OK and fills the count.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetRecordCountSuccessTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>(), preCount))
        .WillOnce(Return(ByMove(resultSetMock_)));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(5), Return(E_OK)));
    int32_t count = 0;
    EXPECT_EQ(handler.GetRecordCount(count), E_OK);
    EXPECT_EQ(count, 5);
}

/**
 * @tc.name: DoInsertNullStoreTest
 * @tc.desc: Verify DoInsert returns E_RDB when the rdb store is null.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, DoInsertNullStoreTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = nullptr;
    vector<OperationLogEntry> batch = { CreateEntry(1) };
    EXPECT_EQ(handler.DoInsert(batch), E_RDB);
}

/**
 * @tc.name: DoInsertQueryFailedTest
 * @tc.desc: Verify DoInsert returns E_RDB when the count query fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, DoInsertQueryFailedTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>(), preCount))
        .WillOnce(Return(ByMove(nullptr)));
    vector<OperationLogEntry> batch = { CreateEntry(1) };
    EXPECT_EQ(handler.DoInsert(batch), E_RDB);
}

/**
 * @tc.name: DoInsertSuccessTest
 * @tc.desc: Verify DoInsert inserts all entries when the record count is below the limit.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, DoInsertSuccessTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    auto &store = OperationLogStore::GetInstance();
    handler.isInited_ = false;
    store.isInited_ = false;
    store.rdbStore_ = rdbStoreMock_;
    Assistant::ins = insMock_;
    insMock_->EnableMock();

    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_OK), Return("/data/test_operation_log/operation_log.db")));
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdbStoreMock_)));
    ASSERT_EQ(handler.Init(0), E_OK);
    handler.Start();

    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), _, preCount))
        .WillOnce(Return(resultSetMock_));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(5), Return(E_OK)));
    EXPECT_CALL(*rdbStoreMock_, Insert(_, _, _)).WillRepeatedly(Return(NativeRdb::E_OK));
    vector<OperationLogEntry> batch = { CreateEntry(1), CreateEntry(2) };
    EXPECT_EQ(handler.DoInsert(batch), E_OK);

    handler.Stop();
}

/**
 * @tc.name: DoInsertTriggersCleanupTest
 * @tc.desc: Verify DoInsert cleans old records first when the record count reaches the limit.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, DoInsertTriggersCleanupTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>(), preCount))
        .WillOnce(Return(ByMove(resultSetMock_)));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_))
        .WillOnce(DoAll(SetArgReferee<0>(OperationLogConst::MAX_RECORD_COUNT), Return(E_OK)));
    EXPECT_CALL(*rdbStoreMock_, Query(_, _)).WillOnce(Return(absResultSetMock_));
    EXPECT_CALL(*absResultSetMock_, GoToNextRow()).WillOnce(Return(E_OK)).WillRepeatedly(Return(-1));
    EXPECT_CALL(*absResultSetMock_, GetLong(_, _)).WillOnce(DoAll(SetArgReferee<1>(1), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, Close()).WillOnce(Return(E_OK));
    EXPECT_CALL(*rdbStoreMock_, Delete(_, _)).WillOnce(Return(NativeRdb::E_OK));
    EXPECT_CALL(*rdbStoreMock_, Insert(_, _, _)).WillRepeatedly(Return(NativeRdb::E_OK));
    vector<OperationLogEntry> batch = { CreateEntry(1) };
    EXPECT_EQ(handler.DoInsert(batch), E_OK);
}

/**
 * @tc.name: DoInsertCleanupFailedTest
 * @tc.desc: Verify DoInsert returns the cleanup error when the record count reaches the limit
 *           and the cleanup of old records fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, DoInsertCleanupFailedTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>(), preCount))
        .WillOnce(Return(ByMove(resultSetMock_)));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_))
        .WillOnce(DoAll(SetArgReferee<0>(OperationLogConst::MAX_RECORD_COUNT), Return(E_OK)));
    EXPECT_CALL(*rdbStoreMock_, Query(_, _)).WillOnce(Return(nullptr));
    vector<OperationLogEntry> batch = { CreateEntry(1) };
    EXPECT_EQ(handler.DoInsert(batch), E_RDB);
}

/**
 * @tc.name: CheckAndCleanRecordsNullStoreTest
 * @tc.desc: Verify CheckAndCleanRecords returns E_RDB when the rdb store is null.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, CheckAndCleanRecordsNullStoreTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = nullptr;
    EXPECT_EQ(handler.CheckAndCleanRecords(), E_RDB);
}

/**
 * @tc.name: CheckAndCleanRecordsQueryNullTest
 * @tc.desc: Verify CheckAndCleanRecords returns E_RDB when the id query returns null.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, CheckAndCleanRecordsQueryNullTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_, Query(_, _)).WillOnce(Return(nullptr));
    EXPECT_EQ(handler.CheckAndCleanRecords(), E_RDB);
}

/**
 * @tc.name: CheckAndCleanRecordsEmptyIdsTest
 * @tc.desc: Verify CheckAndCleanRecords returns E_RDB when no ids are found.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, CheckAndCleanRecordsEmptyIdsTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_, Query(_, _)).WillOnce(Return(absResultSetMock_));
    EXPECT_CALL(*absResultSetMock_, GoToNextRow()).WillRepeatedly(Return(-1));
    EXPECT_CALL(*absResultSetMock_, Close()).WillOnce(Return(E_OK));
    EXPECT_EQ(handler.CheckAndCleanRecords(), E_RDB);
}

/**
 * @tc.name: CheckAndCleanRecordsSuccessTest
 * @tc.desc: Verify CheckAndCleanRecords deletes the oldest records and returns E_OK.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, CheckAndCleanRecordsSuccessTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_, Query(_, _)).WillOnce(Return(absResultSetMock_));
    EXPECT_CALL(*absResultSetMock_, GoToNextRow()).WillOnce(Return(E_OK)).WillRepeatedly(Return(-1));
    EXPECT_CALL(*absResultSetMock_, GetLong(_, _)).WillOnce(DoAll(SetArgReferee<1>(1), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, Close()).WillOnce(Return(E_OK));
    EXPECT_CALL(*rdbStoreMock_, Delete(_, _)).WillOnce(Return(NativeRdb::E_OK));
    EXPECT_EQ(handler.CheckAndCleanRecords(), E_OK);
}

/**
 * @tc.name: CheckAndCleanRecordsDeleteFailTest
 * @tc.desc: Verify CheckAndCleanRecords returns the error when deleting the old records fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, CheckAndCleanRecordsDeleteFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_, Query(_, _)).WillOnce(Return(absResultSetMock_));
    EXPECT_CALL(*absResultSetMock_, GoToNextRow()).WillOnce(Return(E_OK)).WillRepeatedly(Return(-1));
    EXPECT_CALL(*absResultSetMock_, GetLong(_, _)).WillOnce(DoAll(SetArgReferee<1>(1), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, Close()).WillOnce(Return(E_OK));
    EXPECT_CALL(*rdbStoreMock_, Delete(_, _)).WillOnce(Return(NativeRdb::E_ERROR));
    EXPECT_EQ(handler.CheckAndCleanRecords(), NativeRdb::E_ERROR);
}

/**
 * @tc.name: CleanOldRecordsTest
 * @tc.desc: Verify CleanOldRecords deletes the records older than the threshold and returns E_OK.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, CleanOldRecordsTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_, Delete(_, _)).WillOnce(Return(NativeRdb::E_OK));
    EXPECT_EQ(handler.CleanOldRecords(), E_OK);
}

/**
 * @tc.name: CleanOldRecordsNullStoreTest
 * @tc.desc: Verify CleanOldRecords returns E_RDB when the rdb store is not initialized.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, CleanOldRecordsNullStoreTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    EXPECT_EQ(handler.CleanOldRecords(), E_RDB);
}

/**
 * @tc.name: CleanOldRecordsDeleteFailTest
 * @tc.desc: Verify CleanOldRecords returns the error when deleting the old records fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, CleanOldRecordsDeleteFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_, Delete(_, _)).WillOnce(Return(NativeRdb::E_ERROR));
    EXPECT_EQ(handler.CleanOldRecords(), NativeRdb::E_ERROR);
}

/**
 * @tc.name: WriteBatchSuccessTest
 * @tc.desc: Verify WriteBatch returns E_OK when DoInsert succeeds on the first attempt.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, WriteBatchSuccessTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    auto &store = OperationLogStore::GetInstance();
    handler.isInited_ = false;
    store.isInited_ = false;
    store.rdbStore_ = rdbStoreMock_;
    Assistant::ins = insMock_;
    insMock_->EnableMock();

    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_OK), Return("/data/test_operation_log/operation_log.db")));
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdbStoreMock_)));
    ASSERT_EQ(handler.Init(0), E_OK);
    handler.Start();

    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), _, preCount))
        .WillOnce(Return(resultSetMock_));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(5), Return(E_OK)));
    EXPECT_CALL(*rdbStoreMock_, BeginTransaction()).WillOnce(Return(NativeRdb::E_OK));
    EXPECT_CALL(*rdbStoreMock_, Commit()).WillOnce(Return(NativeRdb::E_OK));
    EXPECT_CALL(*rdbStoreMock_, Insert(_, _, _)).WillOnce(Return(NativeRdb::E_OK));
    vector<OperationLogEntry> batch = { CreateEntry(1) };
    EXPECT_EQ(handler.WriteBatch(batch), E_OK);

    handler.Stop();
}

/**
 * @tc.name: WriteBatchNonRecoverableErrorTest
 * @tc.desc: Verify WriteBatch returns the non-recoverable error without retrying.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, WriteBatchNonRecoverableErrorTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    auto &store = OperationLogStore::GetInstance();
    handler.isInited_ = false;
    store.isInited_ = false;
    store.rdbStore_ = rdbStoreMock_;
    Assistant::ins = insMock_;
    insMock_->EnableMock();

    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_OK), Return("/data/test_operation_log/operation_log.db")));
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdbStoreMock_)));
    ASSERT_EQ(handler.Init(0), E_OK);
    handler.Start();

    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), _, preCount))
        .WillOnce(Return(resultSetMock_));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(5), Return(E_OK)));
    EXPECT_CALL(*rdbStoreMock_, BeginTransaction()).WillOnce(Return(NativeRdb::E_OK));
    EXPECT_CALL(*rdbStoreMock_, RollBack()).WillOnce(Return(NativeRdb::E_OK));
    EXPECT_CALL(*rdbStoreMock_, Insert(_, _, _)).WillOnce(Return(E_RDB));
    vector<OperationLogEntry> batch = { CreateEntry(1) };
    EXPECT_EQ(handler.WriteBatch(batch), E_RDB);

    handler.Stop();
}

/**
 * @tc.name: WriteBatchRecoverableThenSuccessTest
 * @tc.desc: Verify WriteBatch retries a recoverable error and succeeds on the next attempt.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, WriteBatchRecoverableThenSuccessTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    auto &store = OperationLogStore::GetInstance();
    handler.isInited_ = false;
    store.isInited_ = false;
    store.rdbStore_ = rdbStoreMock_;
    Assistant::ins = insMock_;
    insMock_->EnableMock();

    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_OK), Return("/data/test_operation_log/operation_log.db")));
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdbStoreMock_)));
    ASSERT_EQ(handler.Init(0), E_OK);
    handler.Start();

    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), _, preCount))
        .WillRepeatedly(Return(resultSetMock_));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(5), Return(E_OK)));
    EXPECT_CALL(*rdbStoreMock_, BeginTransaction()).WillRepeatedly(Return(NativeRdb::E_OK));
    EXPECT_CALL(*rdbStoreMock_, RollBack()).WillRepeatedly(Return(NativeRdb::E_OK));
    EXPECT_CALL(*rdbStoreMock_, Commit()).WillRepeatedly(Return(NativeRdb::E_OK));
    EXPECT_CALL(*rdbStoreMock_, Insert(_, _, _))
        .WillOnce(Return(NativeRdb::E_SQLITE_BUSY))
        .WillOnce(Return(NativeRdb::E_OK));
    vector<OperationLogEntry> batch = { CreateEntry(1) };
    EXPECT_EQ(handler.WriteBatch(batch), E_OK);

    handler.Stop();
}

/**
 * @tc.name: WriteBatchRetriesExhaustedTest
 * @tc.desc: Verify WriteBatch returns E_ERROR when all retries keep failing with a recoverable error.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, WriteBatchRetriesExhaustedTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    auto &store = OperationLogStore::GetInstance();
    handler.isInited_ = false;
    store.isInited_ = false;
    Assistant::ins = insMock_;
    insMock_->EnableMock();

    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_OK), Return("/data/test_operation_log/operation_log.db")));
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdbStoreMock_)));

    ASSERT_EQ(handler.Init(0), E_OK);
    handler.Start();

    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), _, preCount))
        .WillRepeatedly(Return(resultSetMock_));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(5), Return(E_OK)));
    EXPECT_CALL(*rdbStoreMock_, BeginTransaction()).WillRepeatedly(Return(NativeRdb::E_OK));
    EXPECT_CALL(*rdbStoreMock_, RollBack()).WillRepeatedly(Return(NativeRdb::E_OK));
    EXPECT_CALL(*rdbStoreMock_, Insert(_, _, _)).WillRepeatedly(Return(NativeRdb::E_SQLITE_BUSY));
    vector<OperationLogEntry> batch = { CreateEntry(1) };
    EXPECT_EQ(handler.WriteBatch(batch), NativeRdb::E_ERROR);

    handler.Stop();
}

/**
 * @tc.name: InitSuccessTest
 * @tc.desc: Verify Init succeeds and is idempotent when the store initialization succeeds.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, InitSuccessTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    auto &store = OperationLogStore::GetInstance();
    handler.isInited_ = false;
    store.isInited_ = false;
    Assistant::ins = insMock_;
    insMock_->EnableMock();
    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_OK), Return("/data/test_operation_log/operation_log.db")));
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdbStoreMock_)));
    EXPECT_EQ(handler.Init(100), E_OK);
    EXPECT_TRUE(handler.isInited_);
    EXPECT_EQ(handler.Init(100), E_OK);
}

/**
 * @tc.name: InitWithCustomUserIdTest
 * @tc.desc: Verify Init passes the given userId through to the store initialization.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, InitWithCustomUserIdTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    auto &store = OperationLogStore::GetInstance();
    handler.isInited_ = false;
    store.isInited_ = false;
    Assistant::ins = insMock_;
    insMock_->EnableMock();
    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_OK), Return("/data/test_operation_log/operation_log.db")));
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_OK), Return(rdbStoreMock_)));
    EXPECT_EQ(handler.Init(100), E_OK);
    EXPECT_TRUE(handler.isInited_);
}

/**
 * @tc.name: InitStoreFailTest
 * @tc.desc: Verify Init returns the rdb error when the store initialization fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, InitStoreFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    auto &store = OperationLogStore::GetInstance();
    handler.isInited_ = false;
    store.isInited_ = false;
    Assistant::ins = insMock_;
    insMock_->EnableMock();
    EXPECT_CALL(*insMock_, GetDefaultDatabasePath(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(NativeRdb::E_OK), Return("/data/test_operation_log/operation_log.db")));
    EXPECT_CALL(*insMock_, GetRdbStore(_, _, _, _))
        .WillOnce(DoAll(SetArgReferee<3>(NativeRdb::E_ERROR), Return(nullptr)));
    EXPECT_EQ(handler.Init(100), NativeRdb::E_ERROR);
    EXPECT_FALSE(handler.isInited_);
}

/**
 * @tc.name: StopWhenNotRunningTest
 * @tc.desc: Verify Stop returns immediately when the handler is not running.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, StopWhenNotRunningTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    handler.running_ = false;
    handler.Stop();
    EXPECT_FALSE(handler.running_);
}

/**
 * @tc.name: StartAndStopTest
 * @tc.desc: Verify Start spawns the write thread and Stop joins it and clears the running flag.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, StartAndStopTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    handler.Start();
    EXPECT_TRUE(handler.running_);
    handler.Stop();
    EXPECT_FALSE(handler.running_);
}

/**
 * @tc.name: StartAlreadyRunningTest
 * @tc.desc: Verify a second Start while the handler is already running is a no-op.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, StartAlreadyRunningTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    handler.Start();
    handler.Start();
    EXPECT_TRUE(handler.running_);
    handler.Stop();
    EXPECT_FALSE(handler.running_);
}

/**
 * @tc.name: WriteThreadLoopEmptyBatchTest
 * @tc.desc: Verify the write thread keeps looping when PopBatch returns an empty batch.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, WriteThreadLoopEmptyBatchTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    handler.Start();
    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    handler.Stop();
    EXPECT_FALSE(handler.running_);
}

/**
 * @tc.name: WriteThreadLoopProcessBatchSuccessTest
 * @tc.desc: Verify the write thread pops a queued batch and writes it successfully.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, WriteThreadLoopProcessBatchSuccessTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), _, preCount))
        .WillRepeatedly(Return(resultSetMock_));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(5), Return(E_OK)));
    EXPECT_CALL(*rdbStoreMock_, Insert(_, _, _)).WillRepeatedly(Return(NativeRdb::E_OK));
    handler.Start();
    handler.RecordDelete(123, "/data/oplog/foo.txt", 100, 200, "test_proc", 300, 400, "aa");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    handler.Stop();
    EXPECT_FALSE(handler.running_);
}

/**
 * @tc.name: WriteThreadLoopProcessBatchFailTest
 * @tc.desc: Verify the write thread logs the failure when writing a queued batch fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, WriteThreadLoopProcessBatchFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        QueryByStep(An<const AbsRdbPredicates &>(), _, preCount))
        .WillRepeatedly(Return(resultSetMock_));
    EXPECT_CALL(*resultSetMock_, GetRowCount(_))
        .WillRepeatedly(DoAll(SetArgReferee<0>(5), Return(E_OK)));
    EXPECT_CALL(*rdbStoreMock_, Insert(_, _, _)).WillRepeatedly(Return(E_RDB));
    handler.Start();
    handler.RecordDelete(123, "/data/oplog/foo.txt", 100, 200, "test_proc", 300, 400, "aa");
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    handler.Stop();
    EXPECT_FALSE(handler.running_);
}

/**
 * @tc.name: RecordDeleteNotInitedTest
 * @tc.desc: Verify RecordDelete returns error when handler is not initialized.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, RecordDeleteNotInitedTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();

    handler.isInited_.store(false);

    OperationLogEntry entry = {100, OperationLogConst::OP_TYPE_DELETE, "/data/oplog/foo.txt",
        100, 200, "test_proc", 300, 400, "aa"};

    int32_t ret = handler.RecordDelete(entry.opTime, entry.filePath, entry.fileInode,
        entry.fileUid, entry.processName, entry.processPid, entry.processUid, entry.cloudId);

    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: RecordDeleteNotRunningTest
 * @tc.desc: Verify RecordDelete returns error when handler is not running.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, RecordDeleteNotRunningTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();

    handler.isInited_.store(true);
    handler.running_.store(false);

    OperationLogEntry entry = {100, OperationLogConst::OP_TYPE_DELETE, "/data/oplog/foo.txt",
        100, 200, "test_proc", 300, 400, "aa"};

    int32_t ret = handler.RecordDelete(entry.opTime, entry.filePath, entry.fileInode,
        entry.fileUid, entry.processName, entry.processPid, entry.processUid, entry.cloudId);

    EXPECT_EQ(ret, E_RDB);
}

/**
 * @tc.name: CheckAndCleanRecordsGetLongFailTest
 * @tc.desc: Verify RecordDelete returns error when handler is not running.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, CheckAndCleanRecordsGetLongFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_, Query(_, _)).WillOnce(Return(absResultSetMock_));
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillRepeatedly(Return(-1));
    EXPECT_CALL(*absResultSetMock_, GetLong(_, _))
        .WillOnce(Return(E_ERROR))
        .WillOnce(DoAll(SetArgReferee<1>(1), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, Close()).WillOnce(Return(E_OK));
    EXPECT_CALL(*rdbStoreMock_, Delete(_, _)).WillOnce(Return(NativeRdb::E_OK));
    EXPECT_EQ(handler.CheckAndCleanRecords(), E_OK);
}

/**
 * @tc.name: QueryDirFileCountsSuccessTest
 * @tc.desc: Verify QueryDirFileCounts returns correct directory counts with GetParentDir filtering.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, QueryDirFileCountsSuccessTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;

    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));
    int goToNextRowCall = 0;
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly([&goToNextRowCall]() -> int {
            goToNextRowCall++;
            if (goToNextRowCall <= 6) return E_OK;
            return -1;
        });
    int getStringCall = 0;
    EXPECT_CALL(*absResultSetMock_, GetString(0, _))
        .WillRepeatedly([&getStringCall](int idx, string &path) -> int {
            const char* paths[] = {
                "/a/file1.txt", "/a/file2.txt", "/a/file3.txt",
                "/a/b/file1.txt", "/a/b/file2.txt", "/a/b/file3.txt"
            };
            path = paths[getStringCall % 6];
            getStringCall++;
            return E_OK;
        });

    EXPECT_CALL(*absResultSetMock_, Close());
    auto dirCounts = handler.QueryDirFileCounts(rdbStoreMock_);
    EXPECT_EQ(dirCounts.size(), 2u);
    EXPECT_EQ(dirCounts["/a"], 3);
    EXPECT_EQ(dirCounts["/a/b"], 3);
}
 
/**
 * @tc.name: QueryDirFileCountsBatchTest
 * @tc.desc: Verify QueryDirFileCounts uses batched queries with LIMIT/OFFSET.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, QueryDirFileCountsBatchTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    bool preCount = true;
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;

    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));

    int callCount = 0;
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly([&callCount]() -> int {
            callCount++;
            if (callCount <= 1500) return E_OK;
            return -1;
        });
    auto dirCounts = handler.QueryDirFileCounts(rdbStoreMock_);
    EXPECT_GT(dirCounts.size(), 0u);
}
 
/**
 * @tc.name: ReportFileStatsSuccessTest
 * @tc.desc: Verify ReportFileStats reports file statistics with GetParentDir filtering.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, ReportFileStatsSuccessTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_, Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));
    int callCount = 0;
    EXPECT_CALL(*absResultSetMock_, GetString(0, _)).WillRepeatedly(DoAll([&callCount](int idx, string &path) -> int {
                callCount++;
                if (callCount <= 3) {
                    path = "/a/file" + to_string(callCount) + ".txt";
                    return E_OK;
                }
                path = "/a/b/file" + to_string(callCount - 3) + ".txt";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetInt(1, _)).WillRepeatedly(DoAll([](int idx, int &val) -> int {
                val = 1;
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(2, _)).WillRepeatedly(DoAll(SetArgReferee<1>(1000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(3, _)).WillRepeatedly(DoAll(SetArgReferee<1>(2000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(4, _)).WillRepeatedly(DoAll(SetArgReferee<1>(3000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetString(5, _)).WillRepeatedly(DoAll(
            [](int idx, string &val) -> int {
                val = "test_process";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(6, _)).WillRepeatedly(DoAll(SetArgReferee<1>(4000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(7, _)).WillRepeatedly(DoAll(SetArgReferee<1>(5000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetString(8, _)).WillRepeatedly(DoAll(
            [](int idx, string &val) -> int {
            val = "test_cloudid";
            return E_OK;
        },
        Return(E_OK)));

    EXPECT_CALL(*absResultSetMock_, GoToNextRow()).WillRepeatedly([&callCount]() -> int {
            callCount++;
            if (callCount <= 6) return E_OK;
            return -1;
        });
    EXPECT_CALL(*absResultSetMock_, Close());
    handler.ReportFileStats(rdbStoreMock_, "/a");
}
 
/**
 * @tc.name: ReportDirStatsSuccessTest
 * @tc.desc: Verify ReportDirStats reports directory statistics with aggregate file_count.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, ReportDirStatsSuccessTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;

    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));
 
    int rowCall = 0;
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly([&rowCall]() -> int {
            rowCall++;
            if (rowCall == 1) return E_OK;
            return -1;
        });

    int pathCall = 0;
    EXPECT_CALL(*absResultSetMock_, GetString(0, _))
        .WillRepeatedly(DoAll(
            [&pathCall](int idx, string &path) -> int {
                pathCall++;
                path = "/a/record" + to_string(pathCall) + ".txt";
                return E_OK;
            },
            Return(E_OK)));

    EXPECT_CALL(*absResultSetMock_, GetInt(1, _))
        .WillRepeatedly(DoAll(
            [](int idx, int &val) -> int {
                val = 1;
                return E_OK;
            },
            Return(E_OK)));

    EXPECT_CALL(*absResultSetMock_, GetLong(2, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(1000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(3, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(2000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(4, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(3000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(5, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(4000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(6, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(5000), Return(E_OK)));

    EXPECT_CALL(*absResultSetMock_, Close());

    handler.ReportDirStats(rdbStoreMock_, "/a");
}

/**
 * @tc.name: ReportOperationLogStatDispatchTest
 * @tc.desc: Verify ReportOperationLogStat correctly dispatches to ReportFileStats or ReportDirStats
 *          based on directory file counts from QueryDirFileCounts.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, ReportOperationLogStatDispatchTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;

    bool preCount = true;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));

    int rowCall = 0;
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly([&rowCall]() -> int {
            rowCall++;
            if (rowCall <= 6) return E_OK;
            return -1;
        });

    int pathCall = 0;
    EXPECT_CALL(*absResultSetMock_, GetString(0, _))
        .WillRepeatedly(DoAll(
            [&pathCall](int idx, string &path) -> int {
                pathCall++;
                if (pathCall <= 3) {
                    path = "/a/record" + to_string(pathCall) + ".txt";
                } else {
                    path = "/a/b/record" + to_string(pathCall - 3) + ".txt";
                }
                return E_OK;
            },
            Return(E_OK)));

    handler.ReportOperationLogStat();
}
 
/**
 * @tc.name: ReportOperationLogStatNullStoreTest
 * @tc.desc: Verify ReportOperationLogStat returns E_RDB when rdb store is null.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, ReportOperationLogStatNullStoreTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = nullptr;

    EXPECT_EQ(handler.ReportOperationLogStat(), E_RDB);
}

/**
 * @tc.name: QueryDirFileCountsNullResultSetTest
 * @tc.desc: Verify QueryDirFileCounts returns empty map when Query returns nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, QueryDirFileCountsNullResultSetTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillOnce(Return(nullptr));
    auto dirCounts = handler.QueryDirFileCounts(rdbStoreMock_);
    EXPECT_TRUE(dirCounts.empty());
}

/**
 * @tc.name: ReportDirStatsEmptyResultSetTest
 * @tc.desc: Verify ReportDirStats returns early when no matching files.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, ReportDirStatsEmptyResultSetTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly(Return(-1));
    EXPECT_CALL(*absResultSetMock_, Close());
    handler.ReportDirStats(rdbStoreMock_, "/a");
}

/**
 * @tc.name: ReportDirStatsGetParentDirMismatchTest
 * @tc.desc: Verify ReportDirStats skip rows where GetParentDir does not match dir.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, ReportDirStatsGetParentDirMismatchTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));

    int rowCall = 0;
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly([&rowCall]() -> int {
            rowCall++;
            if (rowCall <= 2) return E_OK;
            return -1;
        });

    EXPECT_CALL(*absResultSetMock_, GetString(0, _))
        .WillRepeatedly(DoAll(
            [](int idx, string &path) -> int {
                path = "/other/path/file.txt";
                return E_OK;
            },
            Return(E_OK)));

    EXPECT_CALL(*absResultSetMock_, Close());
    handler.ReportDirStats(rdbStoreMock_, "/a");
}

/**
 * @tc.name: ReportOperationLogStatLargeCountTest
 * @tc.desc: Verify ReportOperationLogStat dispatches to ReportDirStats when count > MAX_FILE_COUNTPerDir.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, ReportOperationLogStatLargeCountTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));
    int rowCall = 0;
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly([&rowCall]() -> int {
            rowCall++;
            if (rowCall <= 100) return E_OK;
            return -1;
        });
    EXPECT_CALL(*absResultSetMock_, GetString(0, _))
        .WillRepeatedly(DoAll(
            [](int idx, string &path) -> int {
                path = "/a/file.txt";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetInt(1, _))
        .WillRepeatedly(DoAll(
            [](int idx, int &val) -> int {
                val = 1;
                return E_OK;
            },
            Return(E_OK)));

    EXPECT_CALL(*absResultSetMock_, GetLong(2, _)).WillRepeatedly(DoAll(SetArgReferee<1>(1000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(3, _)).WillRepeatedly(DoAll(SetArgReferee<1>(2000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(4, _)).WillRepeatedly(DoAll(SetArgReferee<1>(3000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(5, _)).WillRepeatedly(DoAll(SetArgReferee<1>(4000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, GetLong(6, _)).WillRepeatedly(DoAll(SetArgReferee<1>(5000), Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, Close()).WillRepeatedly(Return(E_OK));
    handler.ReportOperationLogStat();
}

/**
 * @tc.name: QueryDirFileCountsGetStringFailTest
 * @tc.desc: Verify QueryDirFileCounts continue When GetString returns non-E_OK.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, QueryDirFileCountsGetStringFailTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));
    int rowCall = 0;
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly([&rowCall]() -> int {
            rowCall++;
            if (rowCall <= 3) return E_OK;
            return -1;
        });
    int getStringCall = 0;
    EXPECT_CALL(*absResultSetMock_, GetString(0, _))
        .WillRepeatedly([&getStringCall](int idx, string &path) -> int {
            getStringCall++;
            if (getStringCall == 2) {
                return E_ERROR;
            }
            path = "/a/file" + to_string(getStringCall) + ".txt";
            return E_OK;
        });
    EXPECT_CALL(*absResultSetMock_, Close());
    auto dirCounts = handler.QueryDirFileCounts(rdbStoreMock_);
    EXPECT_EQ(dirCounts["/a"], 2);
}

/**
 * @tc.name: ReportFileStatsNullResultSetTest
 * @tc.desc: Verify ReportFileStats returns early when Query returns nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, ReportFileStatsNullResultSetTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillOnce(Return(nullptr));
    handler.ReportFileStats(rdbStoreMock_, "/a");
}

/**
 * @tc.name: GetCommonParentDirMultipleDirsTest
 * @tc.desc: Verify GetCommonParentDir is called When dirCounts has more than 10 directories.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetCommonParentDirMultipleDirsTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));
    int rowCall = 0;
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly([&rowCall]() -> int {
            rowCall++;
            if (rowCall <= 15) return E_OK;
            return -1;
        });
    int pathIdx = 0;
    EXPECT_CALL(*absResultSetMock_, GetString(0, _))
        .WillRepeatedly([&pathIdx](int idx, string &path) -> int {
            pathIdx++;
            path = "/data/" + string(1, ('a' + (pathIdx-1) % 15)) + "/file" + to_string(pathIdx) + ".txt";
            return E_OK;
        });
    EXPECT_CALL(*absResultSetMock_, Close()).WillRepeatedly(Return(E_OK));
    handler.ReportOperationLogStat();
}

/**
 * @tc.name: GetCommonParentDirSingleDirTest
 * @tc.desc: Verify GetCommonParentDir with single path returns parent.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetCommonParentDirSingleDirTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));
    int rowCall = 0;
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly([&rowCall]() -> int {
            rowCall++;
            if (rowCall <= 1) return E_OK;
            return -1;
        });
    EXPECT_CALL(*absResultSetMock_, GetString(0, _))
        .WillOnce(DoAll(
            [](int idx, string &path) -> int {
                path = "/a/b/file.txt";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*absResultSetMock_, Close()).WillRepeatedly(Return(E_OK));
    handler.ReportOperationLogStat();
}

/**
 * @tc.name: GetCommonParentDirSameDirTest
 * @tc.desc: Verify GetCommonParentDir with paths in same directory.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetCommonParentDirSameDirTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));
    int rowCall = 0;
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly([&rowCall]() -> int {
            rowCall++;
            if (rowCall <= 3) return E_OK;
            return -1;
        });
    int fileIdx = 0;
    EXPECT_CALL(*absResultSetMock_, GetString(0, _))
        .WillRepeatedly([&fileIdx](int idx, string &path) -> int {
            fileIdx++;
            path = "/a/b/file" + to_string(fileIdx) + ".txt";
            return E_OK;
        });
    EXPECT_CALL(*absResultSetMock_, Close()).WillRepeatedly(Return(E_OK));
    handler.ReportOperationLogStat();
}

/**
 * @tc.name: GetCommonParentDirEmptyPathsTest
 * @tc.desc: Verify GetCommonParentDir returns empty string when paths is empty.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetCommonParentDirEmptyPathsTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    std::vector<std::string> paths;
    std::string result = handler.GetCommonParentDir(paths);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetCommonParentDirNposDirectTest
 * @tc.desc: Verify GetCommonParentDir returns empty string when lastSlash == npos.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetCommonParentDirNposDirectTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    std::vector<std::string> paths = {"afile.txt", "bfile.txt"};
    std::string result = handler.GetCommonParentDir(paths);
    EXPECT_EQ(result, "");
}

/**
 * @tc.name: GetCommonParentDirLastSlashZeroTest
 * @tc.desc: Verify GetCommonParentDir returns "/" when lasstSlash == 0.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, GetCommonParentDirLastSlashZeroTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    std::vector<std::string> paths = {"/a/file.txt", "/b/file.txt"};
    std::string result = handler.GetCommonParentDir(paths);
    EXPECT_EQ(result, "/");
}

/**
 * @tc.name: ReadFileStatFieldsGetStringFailTest
 * @tc.desc: Verify ReadFileStatFields returns false when GetString fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, ReadFileStatFieldsGetStringFailTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    EXPECT_CALL(*resultSetMock_, GetString(0, _)).WillOnce(Return(E_ERROR));
    OperationLogEntry entry;
    bool result = handler.ReadFileStatFields(*resultSetMock_, "/a", entry);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ReportDirStatsGetStringFailTest
 * @tc.desc: Verify ReportDirStats continues When GetString fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, ReportDirStatsGetStringFailTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogStore::GetInstance().rdbStore_ = rdbStoreMock_;
    EXPECT_CALL(*rdbStoreMock_,
        Query(An<const AbsRdbPredicates &>(), An<const std::vector<std::string> &>()))
        .WillRepeatedly(Return(absResultSetMock_));
    int rowCall = 0;
    EXPECT_CALL(*absResultSetMock_, GoToNextRow())
        .WillRepeatedly([&rowCall]() -> int {
            rowCall++;
            if (rowCall <= 2) return E_OK;
            return -1;
        });
    int callIdx = 0;
    EXPECT_CALL(*absResultSetMock_, GetString(0, _))
        .WillRepeatedly([&callIdx](int idx, string &path) -> int {
            callIdx++;
            if (callIdx == 1) {
                return E_ERROR;
            }
            path = "/a/file.txt";
            return E_OK;
        });
    EXPECT_CALL(*absResultSetMock_, Close()).WillRepeatedly(Return(E_OK));
    int32_t result = handler.ReportDirStats(rdbStoreMock_, "/a");
    EXPECT_EQ(result, 1);
}

/**
 * @tc.name: WriteThreadDrainWriteBatchFailTest
 * @tc.desc: Verify WriteThreadLoop handlers WriteBatch failure during drain.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, WriteThreadDrainWriteBatchFailTest, TestSize.Level1) {
    auto &handler = OperationLogHandler::GetInstance();
    auto &store = OperationLogStore::GetInstance();
    store.rdbStore_ = rdbStoreMock_;
    handler.isInited_ = true;
    handler.running_ = true;

    EXPECT_CALL(*rdbStoreMock_, BeginTransaction()).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*rdbStoreMock_, Insert(_, _, _)).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*rdbStoreMock_, Commit()).WillRepeatedly(Return(E_ERROR));
    EXPECT_CALL(*rdbStoreMock_, RollBack()).WillRepeatedly(Return(E_OK));

    OperationLogEntry entry = {1000, OperationLogConst::OP_TYPE_DELETE, "/test/file.txt", 1, 1, "test", 1, 1, "aa"};
    handler.queue_.Push(entry);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    handler.Stop();
}

HWTEST_F(OperationLogHandlerTest, ReadFileStatsGetIntFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogEntry entry;
    EXPECT_CALL(*resultSetMock_, GetInt(1, entry.opType)).WillOnce(Return(E_ERROR));
    EXPECT_CALL(*resultSetMock_, GetString(0, entry.filePath))
        .WillRepeatedly(DoAll(
            [](int idx, std::string &val) -> int {
                val = "/a/file.txt";
                return E_OK;
            },
            Return(E_OK)));
    bool result = handler.ReadFileStatFields(*resultSetMock_, "/a", entry);
    EXPECT_FALSE(result);
}

HWTEST_F(OperationLogHandlerTest, ReadFileStatsGetLongOpTimeFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogEntry entry;
    EXPECT_CALL(*resultSetMock_, GetLong(2, entry.opTime)).WillOnce(Return(E_ERROR));
    EXPECT_CALL(*resultSetMock_, GetString(0, entry.filePath))
        .WillRepeatedly(DoAll(
            [](int idx, std::string &val) -> int {
                val = "/a/file.txt";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetInt(1, entry.opType))
        .WillRepeatedly(DoAll(
            [](int idx, int &val) -> int {
                val = 1;
                return E_OK;
            },
            Return(E_OK)));
    bool result = handler.ReadFileStatFields(*resultSetMock_, "/a", entry);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: ReadFileStatsGetLongFileInodeFailTest
 * @tc.desc: Verify ReadFileStatFields returns false when GetLong for fileInode fails.
 * @tc.type: FUNC
 */
HWTEST_F(OperationLogHandlerTest, ReadFileStatsGetLongFileInodeFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogEntry entry;
    EXPECT_CALL(*resultSetMock_, GetLong(3, entry.fileInode)).WillOnce(Return(E_ERROR));
    EXPECT_CALL(*resultSetMock_, GetString(0, entry.filePath))
        .WillRepeatedly(DoAll(
            [](int idx, std::string &val) -> int {
                val = "/a/file.txt";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetInt(1, entry.opType))
        .WillRepeatedly(DoAll(
            [](int idx, int &val) -> int {
                val = 1;
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(2, entry.opTime))
        .WillRepeatedly(DoAll(SetArgReferee<1>(1000), Return(E_OK)));
    bool result = handler.ReadFileStatFields(*resultSetMock_, "/a", entry);
    EXPECT_FALSE(result);
}

HWTEST_F(OperationLogHandlerTest, ReadFileStatsGetLongFileUidFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogEntry entry;
    EXPECT_CALL(*resultSetMock_, GetLong(4, entry.fileUid)).WillOnce(Return(E_ERROR));
    EXPECT_CALL(*resultSetMock_, GetString(0, entry.filePath))
        .WillRepeatedly(DoAll(
            [](int idx, std::string &val) -> int {
                val = "/a/file.txt";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetInt(1, entry.opType))
        .WillRepeatedly(DoAll(
            [](int idx, int &val) -> int {
                val = 1;
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(2, entry.opTime))
        .WillRepeatedly(DoAll(SetArgReferee<1>(1000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(3, entry.fileInode))
        .WillRepeatedly(DoAll(SetArgReferee<1>(2000), Return(E_OK)));
    bool result = handler.ReadFileStatFields(*resultSetMock_, "/a", entry);
    EXPECT_FALSE(result);
}

HWTEST_F(OperationLogHandlerTest, ReadFileStatsGetStringProcessNameFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogEntry entry;
    EXPECT_CALL(*resultSetMock_, GetString(5, entry.processName)).WillOnce(Return(E_ERROR));
    EXPECT_CALL(*resultSetMock_, GetString(0, entry.filePath))
        .WillRepeatedly(DoAll(
            [](int idx, std::string &val) -> int {
                val = "/a/file.txt";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetInt(1, entry.opType))
        .WillRepeatedly(DoAll(
            [](int idx, int &val) -> int {
                val = 1;
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(2, entry.opTime))
        .WillRepeatedly(DoAll(SetArgReferee<1>(1000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(3, entry.fileInode))
        .WillRepeatedly(DoAll(SetArgReferee<1>(2000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(4, entry.fileUid))
        .WillRepeatedly(DoAll(SetArgReferee<1>(3000), Return(E_OK)));
    bool result = handler.ReadFileStatFields(*resultSetMock_, "/a", entry);
    EXPECT_FALSE(result);
}

HWTEST_F(OperationLogHandlerTest, ReadFileStatsGetLongProcessPidFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogEntry entry;
    EXPECT_CALL(*resultSetMock_, GetLong(6, entry.processPid)).WillOnce(Return(E_ERROR));
    EXPECT_CALL(*resultSetMock_, GetString(0, entry.filePath))
        .WillRepeatedly(DoAll(
            [](int idx, std::string &val) -> int {
                val = "/a/file.txt";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetInt(1, entry.opType))
        .WillRepeatedly(DoAll(
            [](int idx, int &val) -> int {
                val = 1;
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(2, entry.opTime))
        .WillRepeatedly(DoAll(SetArgReferee<1>(1000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(3, entry.fileInode))
        .WillRepeatedly(DoAll(SetArgReferee<1>(2000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(4, entry.fileUid))
        .WillRepeatedly(DoAll(SetArgReferee<1>(3000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetString(5, entry.processName))
        .WillRepeatedly(DoAll(
            [](int idx, std::string &val) -> int {
                val = "test_process";
                return E_OK;
            },
            Return(E_OK)));
    bool result = handler.ReadFileStatFields(*resultSetMock_, "/a", entry);
    EXPECT_FALSE(result);
}

HWTEST_F(OperationLogHandlerTest, ReadFileStatsGetLongProcessUidFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogEntry entry;
    EXPECT_CALL(*resultSetMock_, GetLong(7, entry.processUid)).WillOnce(Return(E_ERROR));
    EXPECT_CALL(*resultSetMock_, GetString(0, entry.filePath))
        .WillRepeatedly(DoAll(
            [](int idx, std::string &val) -> int {
                val = "/a/file.txt";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetInt(1, entry.opType))
        .WillRepeatedly(DoAll(
            [](int idx, int &val) -> int {
                val = 1;
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(2, entry.opTime))
        .WillRepeatedly(DoAll(SetArgReferee<1>(1000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(3, entry.fileInode))
        .WillRepeatedly(DoAll(SetArgReferee<1>(2000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(4, entry.fileUid))
        .WillRepeatedly(DoAll(SetArgReferee<1>(3000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetString(5, entry.processName))
        .WillRepeatedly(DoAll(
            [](int idx, std::string &val) -> int {
                val = "test_process";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(6, entry.processPid))
        .WillRepeatedly(DoAll(SetArgReferee<1>(4000), Return(E_OK)));
    bool result = handler.ReadFileStatFields(*resultSetMock_, "/a", entry);
    EXPECT_FALSE(result);
}

HWTEST_F(OperationLogHandlerTest, ReadFileStatsGetStringCloudIdFailTest, TestSize.Level1)
{
    auto &handler = OperationLogHandler::GetInstance();
    OperationLogEntry entry;
    EXPECT_CALL(*resultSetMock_, GetString(8, entry.cloudId)).WillOnce(Return(E_ERROR));
    EXPECT_CALL(*resultSetMock_, GetString(0, entry.filePath))
        .WillRepeatedly(DoAll(
            [](int idx, std::string &val) -> int {
                val = "/a/file.txt";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetInt(1, entry.opType))
        .WillRepeatedly(DoAll(
            [](int idx, int &val) -> int {
                val = 1;
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(2, entry.opTime))
        .WillRepeatedly(DoAll(SetArgReferee<1>(1000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(3, entry.fileInode))
        .WillRepeatedly(DoAll(SetArgReferee<1>(2000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(4, entry.fileUid))
        .WillRepeatedly(DoAll(SetArgReferee<1>(3000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetString(5, entry.processName))
        .WillRepeatedly(DoAll(
            [](int idx, std::string &val) -> int {
                val = "test_process";
                return E_OK;
            },
            Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(6, entry.processPid))
        .WillRepeatedly(DoAll(SetArgReferee<1>(4000), Return(E_OK)));
    EXPECT_CALL(*resultSetMock_, GetLong(7, entry.processUid))
        .WillRepeatedly(DoAll(SetArgReferee<1>(5000), Return(E_OK)));
    bool result = handler.ReadFileStatFields(*resultSetMock_, "/a", entry);
    EXPECT_FALSE(result);
}
} // namespace Test
} // namespace FileManagement
} // namespace OHOS
