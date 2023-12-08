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
#include "result_set_mock.h"
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
    int32_t Convert(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valueBucket)
    {
        return E_OK;
    }
};

class DataConvertorTest : public testing::Test {
public:
    static void GenerateDefaultTable();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<DataConvertorMock> dataConvertor_;

    static const std::string databaseName;
    static std::shared_ptr<RdbStore> rdbStore_;
};

class SqliteSharedOpenCallback : public RdbOpenCallback {
public:
    int OnCreate(RdbStore &rdbStore) override;
    int OnUpgrade(RdbStore &rdbStore, int oldVersion, int newVersion) override;
    static const std::string createTableTest;
};

std::string const SqliteSharedOpenCallback::createTableTest =
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
const std::string DataConvertorTest::databaseName = RDB_TEST_PATH + "dataConvertortest.db";
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
    RdbStoreConfig sqliteSharedRstConfig(DataConvertorTest::databaseName);
    SqliteSharedOpenCallback sqliteSharedRstHelper;
    int errCode = E_OK;
    DataConvertorTest::rdbStore_ =
        OHOS::NativeRdb::RdbHelper::GetRdbStore(sqliteSharedRstConfig, 1, sqliteSharedRstHelper, errCode);
    EXPECT_NE(DataConvertorTest::rdbStore_, nullptr);
    DataConvertorTest::rdbStore_->ExecuteSql(SqliteSharedOpenCallback::createTableTest);
    GenerateDefaultTable();
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DataConvertorTest::TearDownTestCase(void)
{
    NativeRdb::RdbHelper::DeleteRdbStore(DataConvertorTest::databaseName);
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
        const int rowCount = 1;
        std::unique_ptr<ResultSetMock> resultSet = std::make_unique<ResultSetMock>();
        std::vector<DriveKit::DKRecord> records;

        EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(rowCount), Return(0)));
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
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " RecordToValueBucket001 ERROR";
    }

    GTEST_LOG_(INFO) << "RecordToValueBucket001 End";
}

/**
 * @tc.name: GetInt001
 * @tc.desc: Verify the GetInt function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetInt001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInt001 Begin";
    try {
        std::string key;
        int32_t val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
        int32_t ret = DataConvertor::GetInt(key, val, resultSet);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetInt001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetInt001 End";
}

/**
 * @tc.name: GetInt002
 * @tc.desc: Verify the GetInt function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetInt002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInt002 Begin";
    try {
        std::string key;
        int32_t val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(1));
        int32_t ret = DataConvertor::GetInt(key, val, resultSet);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetInt002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetInt002 End";
}

/**
 * @tc.name: GetInt003
 * @tc.desc: Verify the GetInt function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetInt003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInt003 Begin";
    try {
        std::string key;
        int32_t val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(0));
        int32_t ret = DataConvertor::GetInt(key, val, resultSet);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetInt003 ERROR";
    }

    GTEST_LOG_(INFO) << "GetInt003 End";
}

/**
 * @tc.name: GetLong001
 * @tc.desc: Verify the GetLong function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetLong001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLong001 Begin";
    try {
        std::string key;
        int64_t val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
        int32_t ret = DataConvertor::GetLong(key, val, resultSet);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLong001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetLong001 End";
}

/**
 * @tc.name: GetLong002
 * @tc.desc: Verify the GetLong function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetLong002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLong002 Begin";
    try {
        std::string key;
        int64_t val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(1));
        int32_t ret = DataConvertor::GetLong(key, val, resultSet);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLong002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetLong002 End";
}

/**
 * @tc.name: GetLong003
 * @tc.desc: Verify the GetLong function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetLong003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLong003 Begin";
    try {
        std::string key;
        int64_t val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(resultSet, GetLong(_, _)).WillOnce(Return(0));
        int32_t ret = DataConvertor::GetLong(key, val, resultSet);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLong003 ERROR";
    }

    GTEST_LOG_(INFO) << "GetLong003 End";
}

/**
 * @tc.name: GetDouble001
 * @tc.desc: Verify the GetDouble function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetDouble001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDouble001 Begin";
    try {
        std::string key;
        double val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
        int32_t ret = DataConvertor::GetDouble(key, val, resultSet);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetDouble001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetDouble001 End";
}

/**
 * @tc.name: GetDouble002
 * @tc.desc: Verify the GetDouble function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetDouble002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDouble002 Begin";
    try {
        std::string key;
        double val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(resultSet, GetDouble(_, _)).WillOnce(Return(1));
        int32_t ret = DataConvertor::GetDouble(key, val, resultSet);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetDouble002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetDouble002 End";
}

/**
 * @tc.name: GetDouble003
 * @tc.desc: Verify the GetDouble function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetDouble003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDouble003 Begin";
    try {
        std::string key;
        double val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(resultSet, GetDouble(_, _)).WillOnce(Return(0));
        int32_t ret = DataConvertor::GetDouble(key, val, resultSet);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetDouble003 ERROR";
    }

    GTEST_LOG_(INFO) << "GetDouble003 End";
}

/**
 * @tc.name: GetString001
 * @tc.desc: Verify the GetString function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetString001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetString001 Begin";
    try {
        std::string key;
        std::string val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
        int32_t ret = DataConvertor::GetString(key, val, resultSet);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetString001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetString001 End";
}

/**
 * @tc.name: GetString002
 * @tc.desc: Verify the GetString function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetString002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetString002 Begin";
    try {
        std::string key;
        std::string val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(1));
        int32_t ret = DataConvertor::GetString(key, val, resultSet);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetString002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetString002 End";
}

/**
 * @tc.name: GetString003
 * @tc.desc: Verify the GetString function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetString003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetString003 Begin";
    try {
        std::string key;
        std::string val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(0));
        int32_t ret = DataConvertor::GetString(key, val, resultSet);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetString003 ERROR";
    }

    GTEST_LOG_(INFO) << "GetString003 End";
}

/**
 * @tc.name: GetBool001
 * @tc.desc: Verify the GetBool function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetBool001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBool001 Begin";
    try {
        std::string key;
        bool val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(1));
        int32_t ret = DataConvertor::GetBool(key, val, resultSet);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetBool001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetBool001 End";
}

/**
 * @tc.name: GetBool002
 * @tc.desc: Verify the GetBool function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetBool002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBool002 Begin";
    try {
        std::string key;
        bool val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(1));
        int32_t ret = DataConvertor::GetBool(key, val, resultSet);
        EXPECT_EQ(E_RDB, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetBool002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetBool002 End";
}

/**
 * @tc.name: GetBool003
 * @tc.desc: Verify the GetBool function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetBool003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBool003 Begin";
    try {
        std::string key;
        bool val;
        ResultSetMock resultSet;
        EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(0));
        EXPECT_CALL(resultSet, GetInt(_, _)).WillOnce(Return(0));
        int32_t ret = DataConvertor::GetBool(key, val, resultSet);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetBool003 ERROR";
    }

    GTEST_LOG_(INFO) << "GetBool003 End";
}

/**
 * @tc.name: GetLongComp001
 * @tc.desc: Verify the GetLongComp function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetLongComp001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLongComp001 Begin";
    try {
        int64_t val;
        DriveKit::DKRecordField field(std::string("1"));
        int32_t ret = DataConvertor::GetLongComp(field, val);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLongComp001 ERROR";
    }

    GTEST_LOG_(INFO) << "GetLongComp001 End";
}

/**
 * @tc.name: GetLongComp002
 * @tc.desc: Verify the GetLongComp function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(DataConvertorTest, GetLongComp002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLongComp002 Begin";
    try {
        int64_t val;
        DriveKit::DKRecordField field(1);
        int32_t ret = DataConvertor::GetLongComp(field, val);
        EXPECT_EQ(E_OK, ret);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetLongComp002 ERROR";
    }

    GTEST_LOG_(INFO) << "GetLongComp002 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test