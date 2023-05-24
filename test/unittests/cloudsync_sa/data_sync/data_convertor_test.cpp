/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "data_convertor.h"
#include "dfs_error.h"
#include "rdb_helper.h"
#include "rdb_open_callback.h"
#include "rdb_store_config.h"
#include "sync_rule/cloud_status.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace NativeRdb;
class DataConvertorMock final : public DataConvertor {
public:
    MOCK_METHOD2(Convert, int32_t(DriveKit::DKRecord &, NativeRdb::ResultSet &));
};

class ResultSetMock : public ResultSet {
public:
    ~ResultSetMock() {}
    MOCK_METHOD1(GetAllColumnNames, int(std::vector<std::string> &));
    MOCK_METHOD1(GetColumnCount, int(int &));
    MOCK_METHOD2(GetColumnType, int(int, ColumnType &));
    MOCK_METHOD2(GetColumnIndex, int(const std::string &, int &));
    MOCK_METHOD2(GetColumnName, int(int, std::string &));
    MOCK_METHOD1(GetRowCount, int(int &));
    MOCK_METHOD1(GoTo, int(int));
    MOCK_METHOD1(GoToRow, int(int));
    MOCK_METHOD0(GoToFirstRow, int());
    MOCK_METHOD0(GoToLastRow, int());
    MOCK_METHOD0(GoToNextRow, int());
    MOCK_METHOD0(GoToPreviousRow, int());
    MOCK_METHOD1(IsEnded, int(bool &));
    MOCK_METHOD1(IsAtLastRow, int(bool &));
    MOCK_METHOD2(GetBlob, int(int, std::vector<uint8_t> &));
    MOCK_METHOD2(GetString, int(int, std::string &));
    MOCK_METHOD2(GetInt, int(int, int &));
    MOCK_METHOD2(GetLong, int(int, int64_t &));
    MOCK_METHOD2(GetDouble, int(int, double &));
    MOCK_METHOD2(IsColumnNull, int(int, bool &));
    MOCK_METHOD0(Close, int());
    MOCK_METHOD1(GetRow, int(RowEntity &));

    MOCK_CONST_METHOD0(IsClosed, bool());
    MOCK_CONST_METHOD1(GetRowIndex, int(int &));
    MOCK_CONST_METHOD1(IsStarted, int(bool &));
    MOCK_CONST_METHOD1(IsAtFirstRow, int(bool &));

    MOCK_METHOD2(GetAsset, int(int32_t, ValueObject::Asset &));
    MOCK_METHOD2(GetAssets, int(int32_t, ValueObject::Assets &));
    MOCK_METHOD2(Get, int(int32_t, ValueObject &));
    MOCK_METHOD1(GetModifyTime, int(std::string&));
    MOCK_METHOD2(GetSize, int(int32_t, size_t &));
};

class DataConvertorTest : public testing::Test {
public:
    static void GenerateDefaultTable();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DataConvertorMock> dataConvertor_;

    static const std::string DATABASE_NAME;
    static std::shared_ptr<RdbStore> rdbStore_;
};

class SqliteSharedOpenCallback : public RdbOpenCallback {
public:
    int OnCreate(RdbStore &rdbStore) override;
    int OnUpgrade(RdbStore &rdbStore, int oldVersion, int newVersion) override;
    static const std::string CREATE_TABLE_TEST;
};

std::string const SqliteSharedOpenCallback::CREATE_TABLE_TEST =
    "CREATE TABLE test (id INTEGER PRIMARY KEY AUTOINCREMENT, data1 TEXT,data2 INTEGER, data3 FLOAT, data4 BLOB);";

int SqliteSharedOpenCallback::OnCreate(RdbStore &rdbStore)
{
    GTEST_LOG_(INFO) << "OnCreate";
    return E_OK;
}

int SqliteSharedOpenCallback::OnUpgrade(RdbStore &rdbStore, int oldVersion, int newVersion)
{
    GTEST_LOG_(INFO) << "OnUpgrade";
    return E_OK;
}
const int ID1 = 1;
const int ID2 = 2;
const int ID3 = 3;
static const std::string RDB_TEST_PATH = "/data/test/";
const std::string DataConvertorTest::DATABASE_NAME = RDB_TEST_PATH + "dataConvertortest.db";
std::shared_ptr<RdbStore> DataConvertorTest::rdbStore_ = nullptr;

void DataConvertorTest::GenerateDefaultTable()
{
    std::shared_ptr<RdbStore> &store = DataConvertorTest::rdbStore_;

    int64_t id;
    ValuesBucket values;

    values.PutInt("id", ID1);
    values.PutString("data1", std::string("hello"));
    values.PutInt("data2", 1);
    values.PutDouble("data3", 1.0);
    values.PutBlob("data4", std::vector<uint8_t>{1});
    store->Insert(id, "test", values);

    values.Clear();
    values.PutInt("id", ID2);
    values.PutString("data1", std::string("1"));
    values.PutInt("data2", 1);
    values.PutDouble("data3", 1.0);
    values.PutBlob("data4", std::vector<uint8_t>{});
    store->Insert(id, "test", values);

    values.Clear();
    values.PutInt("id", ID3);
    values.PutString("data1", std::string("hello world"));
    values.PutInt("data2", 1);
    values.PutDouble("data3", 1.0);
    values.PutBlob("data4", std::vector<uint8_t>{});
    store->Insert(id, "test", values);
}

void DataConvertorTest::SetUpTestCase(void)
{
    RdbStoreConfig sqliteSharedRstConfig(DataConvertorTest::DATABASE_NAME);
    SqliteSharedOpenCallback sqliteSharedRstHelper;
    int errCode = E_OK;
    DataConvertorTest::rdbStore_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(sqliteSharedRstConfig, 1, sqliteSharedRstHelper, errCode);
    EXPECT_NE(DataConvertorTest::rdbStore_, nullptr);
    DataConvertorTest::rdbStore_->ExecuteSql(SqliteSharedOpenCallback::CREATE_TABLE_TEST);
    GenerateDefaultTable();
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DataConvertorTest::TearDownTestCase(void)
{
    NativeRdb::RdbHelper::DeleteRdbStore(DataConvertorTest::DATABASE_NAME);
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DataConvertorTest::SetUp(void)
{
    dataConvertor_ = make_shared<DataConvertorMock>();
    GTEST_LOG_(INFO) << "SetUp";
}

void DataConvertorTest::TearDown(void)
{
    dataConvertor_ = nullptr;
}

/**
 * @tc.name: ResultSetToRecords001
 * @tc.desc: Verify the ResultSetToRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, DataConvertorTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToRecords001 Begin";
    try {
        std::unique_ptr<ResultSetMock> resultSet = std::make_unique<ResultSetMock>();
        std::vector<DriveKit::DKRecord> records;

        EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(Return(1));

        int32_t ret = dataConvertor_->ResultSetToRecords(std::move(resultSet), records);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ResultSetToRecords001 ERROR";
    }

    GTEST_LOG_(INFO) << "ResultSetToRecords001 End";
}

/**
 * @tc.name: ResultSetToRecords002
 * @tc.desc: Verify the ResultSetToRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, DataConvertorTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToRecords002 Begin";
    try {
        std::unique_ptr<ResultSetMock> resultSet = std::make_unique<ResultSetMock>();
        std::vector<DriveKit::DKRecord> records;

        EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(Return(0));
        EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(1));

        int32_t ret = dataConvertor_->ResultSetToRecords(std::move(resultSet), records);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ResultSetToRecords002 ERROR";
    }

    GTEST_LOG_(INFO) << "ResultSetToRecords002 End";
}

/**
 * @tc.name: ResultSetToRecords003
 * @tc.desc: Verify the ResultSetToRecords function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, ResultSetToRecords003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ResultSetToRecords003 Begin";
    try {
        std::unique_ptr<ResultSetMock> resultSet = std::make_unique<ResultSetMock>();
        std::vector<DriveKit::DKRecord> records;
        const int32_t rowCount = 3;
        EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
        EXPECT_CALL(*resultSet, GoToNextRow())
            .Times(rowCount)
            .WillOnce(Return(0))
            .WillOnce(Return(0))
            .WillRepeatedly(Return(1));
        EXPECT_CALL(*dataConvertor_, Convert(_, _)).WillRepeatedly(Return(0));

        int32_t ret = dataConvertor_->ResultSetToRecords(std::move(resultSet), records);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ResultSetToRecords003 ERROR";
    }

    GTEST_LOG_(INFO) << "ResultSetToRecords003 End";
}

/**
 * @tc.name: RecordToValueBucket001
 * @tc.desc: Verify the RecordToValueBucket function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, RecordToValueBucket001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecordToValueBucket001 Begin";
    try {
        DriveKit::DKRecord record;
        NativeRdb::ValuesBucket valueBucket;
        int32_t ret = dataConvertor_->RecordToValueBucket(record, valueBucket);
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RecordToValueBucket001 ERROR";
    }

    GTEST_LOG_(INFO) << "RecordToValueBucket001 End";
}

/**
 * @tc.name: RecordToValueBucket002
 * @tc.desc: Verify the RecordToValueBucket function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, RecordToValueBucket002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecordToValueBucket002 Begin";
    try {
        std::vector<DriveKit::DKRecord> records;
        NativeRdb::ValuesBucket valueBucket;
        std::vector<std::string> selectionArgs;
        std::unique_ptr<ResultSet> rstSet = DataConvertorTest::rdbStore_->QuerySql("SELECT * FROM test", selectionArgs);
        int32_t ret = dataConvertor_->ResultSetToRecords(std::move(rstSet), records);
        EXPECT_EQ(E_OK, ret);
        EXPECT_NE(records.size(), 0);
        ret = dataConvertor_->RecordToValueBucket(records[0], valueBucket);
        EXPECT_EQ(E_INVAL_ARG, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RecordToValueBucket002 ERROR";
    }
    GTEST_LOG_(INFO) << "RecordToValueBucket002 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test