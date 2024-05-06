/*
 * Copyright (C) 2024 Huawei Device Co., Ltd. All rights reserved.
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
#include <memory>

#include "album_data_handler.h"
#include "cloud_disk_data_syncer.h"
#include "cloud_file_utils_mock.h"
#include "clouddisk_rdbstore.h"
#include "data_convertor_mock.h"
#include "dfs_error.h"
#include "dk_database.h"
#include "dk_record_mock.h"
#include "file_data_convertor.h"
#include "file_data_handler.h"
#include "rdb_store_mock.h"
#include "result_set_mock.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudSync;
using namespace OHOS::FileManagement::CloudDisk;

class CloudDiskDataConvertorMock final : public CloudDiskDataConvertor {
public:
    CloudDiskDataConvertorMock(int32_t userId,
                               std::string &bundleName,
                               OperationType type,
                               const std::function<void(NativeRdb::ResultSet &resultSet)> &func = nullptr)
        : CloudDiskDataConvertor(userId, bundleName, type)
    {
    }
    MOCK_METHOD2(Convert, int32_t(DriveKit::DKRecord &record, NativeRdb::ValuesBucket &valuesBucket));
};

class CloudDiskDataHandlerMock final : public CloudDiskDataHandler {
public:
    CloudDiskDataHandlerMock(int32_t userId, const string &bundleName, std::shared_ptr<RdbStore> rdb)
        : CloudDiskDataHandler(userId, bundleName, rdb, std::make_shared<bool>(false))
    {
    }

    MOCK_METHOD2(Query,
                 std::shared_ptr<NativeRdb::ResultSet>(const NativeRdb::AbsRdbPredicates &predicates,
                                                       const std::vector<std::string> &columns));
    MOCK_METHOD4(Update,
                 int32_t(int &changedRows,
                         const NativeRdb::ValuesBucket &values,
                         const std::string &whereClause,
                         const std::vector<std::string> &whereArgs));
    MOCK_METHOD4(Delete,
                 int32_t(int &deletedRows,
                         const std::string &tableName,
                         const string &whereClause,
                         const vector<string> &whereArgs));
    MOCK_METHOD3(Delete,
                 int32_t(int &deletedRows, const std::string &whereClause, const std::vector<std::string> &whereArgs));

    MOCK_METHOD1(OnRecordFailed, int32_t(const std::pair<DKRecordId, DKRecordOperResult> &entry));

    MOCK_METHOD3(BatchDetete,
                 int32_t(const string &whichTable, const string &whichColumn,
                        const std::vector<NativeRdb::ValueObject> &bindArgs));

    void SetIsChecking(bool isChecking)
    {
        isChecking_ = isChecking;
    }
};
class FileDataConvertorMock final : public FileDataConvertor {
public:
    FileDataConvertorMock(int32_t userId,
                          std::string &bundleName,
                          OperationType type,
                          const std::function<void(NativeRdb::ResultSet &resultSet)> &func = nullptr)
        : FileDataConvertor(userId, bundleName, type)
    {
    }
    MOCK_METHOD2(ResultSetToRecords,
                 int32_t(const std::shared_ptr<NativeRdb::ResultSet> resultSet,
                         std::vector<DriveKit::DKRecord> &records));
    MOCK_METHOD3(GetString, int32_t(const std::string &key, std::string &val, NativeRdb::ResultSet &resultSet));
};

class DKRecordOperResultMock final : public DKRecordOperResult {
public:
    DKRecordOperResultMock() {}
    MOCK_METHOD0(IsSuccess, bool());
};

class CloudDiskDataHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<CloudDiskDataHandlerMock> cloudDiskDataHandler_ = nullptr;
};

void CloudDiskDataHandlerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDiskDataHandlerTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDiskDataHandlerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
    const int32_t userId = 100;
    string bundleName = "com.ohos.test";
    auto rdb = make_shared<RdbStoreMock>();
    cloudDiskDataHandler_ = make_shared<CloudDiskDataHandlerMock>(userId, bundleName, rdb);
}

void CloudDiskDataHandlerTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
    cloudDiskDataHandler_ = nullptr;
}

/**
 * @tc.name: CloudDiskDataHandlerTest001
 * @tc.desc: Verify the CloudDiskDataHandler function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, CloudDiskDataHandlerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskDataHandler Start";
    int32_t userId = 100;
    string bundleName = "com.ohos.test";
    auto rdb = make_shared<RdbStoreMock>();
    auto stopFlag = make_shared<bool>(false);
    shared_ptr<CloudDiskDataHandler> dataHandler = make_shared<CloudDiskDataHandler>(userId, bundleName, rdb, stopFlag);
    EXPECT_NE(dataHandler, nullptr);
    GTEST_LOG_(INFO) << "CloudDiskDataHandler End";
}

/**
 * @tc.name: GetFetchConditionTest001
 * @tc.desc: Verify the GetFetchCondition function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetFetchConditionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFetchCondition Start";
    cloudDiskDataHandler_->SetIsChecking(true);
    FetchCondition cond;
    cloudDiskDataHandler_->GetFetchCondition(cond);
    EXPECT_EQ(cond.desiredKeys, cloudDiskDataHandler_->desiredKeys_);
    GTEST_LOG_(INFO) << "GetFetchCondition End";
}

/**
 * @tc.name: UTCTimeMilliSecondsTest001
 * @tc.desc: Verify the UTCTimeMilliSeconds function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, UTCTimeMilliSecondsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UTCTimeMilliSeconds Start";
    int64_t ret = cloudDiskDataHandler_->UTCTimeMilliSeconds();
    EXPECT_GE(ret, 0);
    GTEST_LOG_(INFO) << "UTCTimeMilliSeconds End";
}

/**
 * @tc.name: GetRetryRecordsTest001
 * @tc.desc: Verify the GetRetryRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetRetryRecordsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRetryRecords Start";
    std::vector<DriveKit::DKRecordId> records;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSet, GoToNextRow()).Times(2).WillOnce(Return(0)).WillOnce(Return(2));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillOnce(Return(E_OK));
    int32_t ret = cloudDiskDataHandler_->GetRetryRecords(records);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetRetryRecords End";
}

/**
 * @tc.name: GetRetryRecordsTest002
 * @tc.desc: Verify the GetRetryRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetRetryRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRetryRecords Start";
    std::vector<DriveKit::DKRecordId> records;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
    int32_t ret = cloudDiskDataHandler_->GetRetryRecords(records);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "GetRetryRecords End";
}

/**
 * @tc.name: HandleCreateConvertErrTest001
 * @tc.desc: Verify the HandleCreateConvertErr function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, HandleCreateConvertErrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCreateConvertErr Start";
    unsigned int length = cloudDiskDataHandler_->createFailSet_.size();
    ResultSetMock resultSet;
    const string cloudId = "sample_id";
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(resultSet, GetString(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(cloudId), Return(E_OK)));
    int32_t err = E_RDB;
    cloudDiskDataHandler_->HandleCreateConvertErr(err, resultSet);
    EXPECT_NE(cloudDiskDataHandler_->createFailSet_.size(), length);
    GTEST_LOG_(INFO) << "HandleCreateConvertErr End";
}

/**
 * @tc.name: HandleCreateConvertErrTest002
 * @tc.desc: Verify the HandleCreateConvertErr function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, HandleCreateConvertErrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleCreateConvertErr Start";
    unsigned int length = cloudDiskDataHandler_->createFailSet_.size();
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(E_RDB));
    int32_t err = E_RDB;
    cloudDiskDataHandler_->HandleCreateConvertErr(err, resultSet);
    EXPECT_EQ(cloudDiskDataHandler_->createFailSet_.size(), length);
    GTEST_LOG_(INFO) << "HandleCreateConvertErr End";
}

/**
 * @tc.name: HandleFdirtyConvertErrTest001
 * @tc.desc: Verify the HandleFdirtyConvertErr function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, HandleFdirtyConvertErrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleFdirtyConvertErr Start";

    int length = cloudDiskDataHandler_->modifyFailSet_.size();
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(E_OK));
    int32_t err = E_RDB;
    cloudDiskDataHandler_->HandleFdirtyConvertErr(err, resultSet);
    EXPECT_NE(cloudDiskDataHandler_->modifyFailSet_.size(), length);
    GTEST_LOG_(INFO) << "HandleFdirtyConvertErr End";
}

/**
 * @tc.name: HandleFdirtyConvertErrTest002
 * @tc.desc: Verify the HandleFdirtyConvertErr function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, HandleFdirtyConvertErrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleFdirtyConvertErr Start";

    int length = cloudDiskDataHandler_->modifyFailSet_.size();
    ResultSetMock resultSet;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(resultSet, GetString(_, _)).WillOnce(Return(E_RDB));
    int32_t err = E_RDB;
    cloudDiskDataHandler_->HandleFdirtyConvertErr(err, resultSet);
    EXPECT_EQ(cloudDiskDataHandler_->modifyFailSet_.size(), length);
    GTEST_LOG_(INFO) << "HandleFdirtyConvertErr End";
}

/**
 * @tc.name: QueryLocalByCloudIdTest001
 * @tc.desc: Verify the QueryLocalByCloudId function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, QueryLocalByCloudIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryLocalByCloudId Start";
    vector<string> recordIds;
    recordIds.emplace_back("sample_id1");
    recordIds.emplace_back("sample_id2");
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
    auto [resultSet, recordIdRowIdMap] = cloudDiskDataHandler_->QueryLocalByCloudId(recordIds);
    EXPECT_EQ(resultSet, nullptr);
    GTEST_LOG_(INFO) << "QueryLocalByCloudId End";
}

/**
 * @tc.name: QueryLocalByCloudIdTest002
 * @tc.desc: Verify the QueryLocalByCloudId function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, QueryLocalByCloudIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryLocalByCloudId Start";
    vector<string> recordIds;
    auto resultSetMock = make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSetMock));
    EXPECT_CALL(*resultSetMock, GetRowCount(_)).WillOnce(Return(-1));
    auto [resultSet, recordIdRowIdMap] = cloudDiskDataHandler_->QueryLocalByCloudId(recordIds);
    EXPECT_EQ(resultSet, nullptr);
    GTEST_LOG_(INFO) << "QueryLocalByCloudId End";
}

/**
 * @tc.name: OnFetchRecordsTest001
 * @tc.desc: Verify the OnFetchRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnFetchRecordsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchRecords Start";
    shared_ptr<vector<DKRecord>> records = std::make_shared<vector<DKRecord>>();
    DKRecord recordElem1;
    DKRecord recordElem2;
    records->emplace_back(recordElem1);
    records->emplace_back(recordElem2);
    OnFetchParams params;
    int32_t ret = cloudDiskDataHandler_->OnFetchRecords(records, params);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "OnFetchRecords End";
}

/**
 * @tc.name: OnFetchRecordsTest002
 * @tc.desc: Verify the OnFetchRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnFetchRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchRecords Start";
    shared_ptr<vector<DKRecord>> records = std::make_shared<vector<DKRecord>>();
    OnFetchParams params;
    int32_t ret = cloudDiskDataHandler_->OnFetchRecords(records, params);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "OnFetchRecords End";
}

/**
 * @tc.name: OnFetchRecordsTest003
 * @tc.desc: Verify the OnFetchRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnFetchRecordsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnFetchRecords Start";
    try {
        EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
        shared_ptr<vector<DKRecord>> records = std::make_shared<vector<DKRecord>>();
        OnFetchParams params;
        int32_t ret = cloudDiskDataHandler_->OnFetchRecords(records, params);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnFetchRecords ERROR";
    }
    GTEST_LOG_(INFO) << "OnFetchRecords End";
}

/**
 * @tc.name: PullRecordInsertTest001
 * @tc.desc: Verify the PullRecordInsert function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, PullRecordInsertTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordInsert Start";
    DKRecord record;
    OnFetchParams params;
    int32_t ret = cloudDiskDataHandler_->PullRecordInsert(record, params);
    EXPECT_EQ(ret, E_INVALID_ARGS);
    GTEST_LOG_(INFO) << "PullRecordInsert End";
}

/**
 * @tc.name: PullRecordInsertTest002
 * @tc.desc: Verify the PullRecordInsert function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, PullRecordInsertTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordInsert Start";
    DKRecord record;
    DKRecordData fields;
    DKRecordField dkRecordFieldString("test.txt");
    DKRecordField dkDirectlyRecycled(1);
    DKRecordField dkRecordFieldLong(1);
    fields.insert({DK_FILE_NAME, dkRecordFieldString});
    fields.insert({DK_DIRECTLY_RECYCLED, dkDirectlyRecycled});
    fields.insert({DK_FILE_TIME_RECYCLED, dkRecordFieldLong});
    record.SetRecordData(fields);
    record.SetRecordId("sample record Id");
    OnFetchParams params;
    int32_t ret = cloudDiskDataHandler_->PullRecordInsert(record, params);
    EXPECT_TRUE(ret == E_INVAL_ARG || ret == E_OK);
    GTEST_LOG_(INFO) << "PullRecordInsert End";
}

/**
 * @tc.name: PullRecordConflictTest001
 * @tc.desc: Verify the PullRecordConflict function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, PullRecordConflictTest001, TestSize.Level1)
{
    DKRecord record;
    int32_t ret = cloudDiskDataHandler_->PullRecordConflict(record);
    EXPECT_EQ(ret, E_INVALID_ARGS);
    GTEST_LOG_(INFO) << "PullRecordConflict End";
}

/**
 * @tc.name: PullRecordConflictTest002
 * @tc.desc: Verify the PullRecordConflict function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, PullRecordConflictTest002, TestSize.Level1)
{
    DKRecord record;
    DKRecordData fields;
    DKRecordField dkRecordFieldString("test.txt");
    DKRecordField dkDirectlyRecycled(1);
    DKRecordField dkRecordFieldLong(1);
    fields.insert({DK_FILE_NAME, dkRecordFieldString});
    fields.insert({DK_DIRECTLY_RECYCLED, dkDirectlyRecycled});
    fields.insert({DK_FILE_TIME_RECYCLED, dkRecordFieldLong});
    record.SetRecordData(fields);
    int32_t ret = cloudDiskDataHandler_->PullRecordConflict(record);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "PullRecordConflict End";
}

/**
 * @tc.name: PullRecordConflictTest003
 * @tc.desc: Verify the PullRecordConflict function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, PullRecordConflictTest003, TestSize.Level1)
{
    DKRecord record;
    DKRecordData fields;
    DKRecordField dkRecordFieldString("test.txt");
    DKRecordField dkDirectlyRecycled(1);
    DKRecordField dkRecordFieldLong(0);
    fields.insert({DK_FILE_NAME, dkRecordFieldString});
    fields.insert({DK_DIRECTLY_RECYCLED, dkDirectlyRecycled});
    fields.insert({DK_FILE_TIME_RECYCLED, dkRecordFieldLong});
    record.SetRecordData(fields);
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
    int32_t ret = cloudDiskDataHandler_->PullRecordConflict(record);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "PullRecordConflict End";
}

/**
 * @tc.name: PullRecordConflictTest004
 * @tc.desc: Verify the PullRecordConflict function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, PullRecordConflictTest004, TestSize.Level1)
{
    DKRecord record;
    DKRecordData fields;
    DKRecordField dkRecordFieldString("test");
    DKRecordField dkDirectlyRecycled(1);
    DKRecordField dkRecordFieldLong(0);
    fields.insert({DK_FILE_NAME, dkRecordFieldString});
    fields.insert({DK_DIRECTLY_RECYCLED, dkDirectlyRecycled});
    fields.insert({DK_FILE_TIME_RECYCLED, dkRecordFieldLong});
    record.SetRecordData(fields);
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
    int32_t ret = cloudDiskDataHandler_->PullRecordConflict(record);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "PullRecordConflict End";
}

/**
 * @tc.name: HandleConflictTest001
 * @tc.desc: Verify the HandleConflict function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, HandleConflictTest001, TestSize.Level1)
{
    string fullName = "fullName";
    const int lastDot = 0;
    DKRecord record;
    int32_t ret = cloudDiskDataHandler_->HandleConflict(nullptr, fullName, lastDot, record);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "HandleConflict End";
}

/**
 * @tc.name: HandleConflictTest002
 * @tc.desc: Verify the HandleConflict function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, HandleConflictTest002, TestSize.Level1)
{
    string fullName = "fullName";
    const int lastDot = 0;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(0), Return(E_RDB)));
    DKRecord record;
    int32_t ret = cloudDiskDataHandler_->HandleConflict(resultSet, fullName, lastDot, record);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "HandleConflict End";
}

/**
 * @tc.name: HandleConflictTest003
 * @tc.desc: Verify the HandleConflict function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, HandleConflictTest003, TestSize.Level1)
{
    string fullName = "fullName";
    const int lastDot = 0;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(0), Return(E_OK)));
    DKRecord record;
    int32_t ret = cloudDiskDataHandler_->HandleConflict(resultSet, fullName, lastDot, record);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "HandleConflict End";
}

/**
 * @tc.name: FindRenameFileTest001
 * @tc.desc: Verify the FindRenameFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, FindRenameFileTest001, TestSize.Level1)
{
    string renameFileCloudId = "renameFileCloudId";
    string fullName = "fullName.txt";
    size_t lastDot = fullName.rfind('.');
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GoToFirstRow()).Times(2).WillRepeatedly(Return(0));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).Times(3).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*resultSet, GetString(_, _)).Times(3)
    .WillOnce(DoAll(SetArgReferee<1>(fullName), Return(E_OK)))
    .WillOnce(DoAll(SetArgReferee<1>(fullName), Return(E_OK)))
    .WillOnce(DoAll(SetArgReferee<1>("qqq"), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(-1));
    int32_t ret = cloudDiskDataHandler_->FindRenameFile(resultSet, renameFileCloudId, fullName, lastDot);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FindRenameFile End";
}

/**
 * @tc.name: FindRenameFileTest002
 * @tc.desc: Verify the FindRenameFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, FindRenameFileTest002, TestSize.Level1)
{
    string renameFileCloudId = "renameFileCloudId";
    string fullName = "fullName.txt";
    size_t lastDot = fullName.rfind('.');
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*resultSet, GoToFirstRow()).WillOnce(Return(0));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(""), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(-1));
    int32_t ret = cloudDiskDataHandler_->FindRenameFile(resultSet, renameFileCloudId, fullName, lastDot);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FindRenameFile End";
}

/**
 * @tc.name: ConflictReNameTest001
 * @tc.desc: Verify the ConflictReName function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, ConflictReNameTest001, TestSize.Level1)
{
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Update(_, _, _, _)).WillOnce(Return(E_OK));
    DKRecord record;
    int32_t ret = cloudDiskDataHandler_->ConflictReName("cloudId", "newFileName", record);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "ConflictReName End";
}

/**
 * @tc.name: ConflictReNameTest002
 * @tc.desc: Verify the ConflictReName function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, ConflictReNameTest002, TestSize.Level1)
{
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Update(_, _, _, _)).WillOnce(Return(-1));
    DKRecord record;
    int32_t ret = cloudDiskDataHandler_->ConflictReName("cloudId", "newFileName", record);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "ConflictReName End";
}

/**
 * @tc.name: PullRecordUpdateTest001
 * @tc.desc: Verify the PullRecordUpdate function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, PullRecordUpdateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordUpdate Start";
    DKRecord record;
    ResultSetMock resultSet;
    OnFetchParams params;
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(resultSet, GetInt(_, _))
        .WillRepeatedly(DoAll(SetArgReferee<1>(static_cast<int32_t>(DirtyType::TYPE_MDIRTY)), Return(E_OK)));
    int32_t ret = cloudDiskDataHandler_->PullRecordUpdate(record, resultSet, params);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "PullRecordUpdate End";
}

/**
 * @tc.name: PullRecordUpdateTest002
 * @tc.desc: Verify the PullRecordUpdate function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, PullRecordUpdateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordUpdate Start";
    DKRecord record;
    ResultSetMock resultSet;
    OnFetchParams params;
    cloudDiskDataHandler_->localConvertor_.SetRootId("");
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
    int32_t ret = cloudDiskDataHandler_->PullRecordUpdate(record, resultSet, params);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "PullRecordUpdate End";
}

/**
 * @tc.name: PullRecordUpdateTest003
 * @tc.desc: Verify the PullRecordUpdate function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, PullRecordUpdateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordUpdate Start";
    DKRecord record;
    ResultSetMock resultSet;
    OnFetchParams params;
    cloudDiskDataHandler_->localConvertor_.SetRootId("sample_rootId");
    EXPECT_CALL(resultSet, GetColumnIndex(_, _)).Times(1).WillRepeatedly(Return(E_RDB));
    int32_t ret = cloudDiskDataHandler_->PullRecordUpdate(record, resultSet, params);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "PullRecordUpdate End";
}

/**
 * @tc.name: PullRecordDeleteTest001
 * @tc.desc: Verify the PullRecordDelete function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, PullRecordDeleteTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordDelete Start";
    DKRecord record;
    record.SetRecordId("sample_id");
    record.SetRecordType("sample_type");
    ResultSetMock resultSet;
    int32_t ret = cloudDiskDataHandler_->PullRecordDelete(record, resultSet);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "PullRecordDelete End";
}

/**
 * @tc.name: PullRecordDeleteTest002
 * @tc.desc: Verify the PullRecordDelete function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, PullRecordDeleteTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PullRecordDelete Start";
    DKRecord record;
    ResultSetMock resultSet;
    int32_t ret = cloudDiskDataHandler_->PullRecordDelete(record, resultSet);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "PullRecordDelete End";
}

/**
 * @tc.name: RecycleFileTest001
 * @tc.desc: Verify the RecycleFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, RecycleFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleFile Start";
    string recordId = "sample_id";
    string parentCloudId = "rootId";
    string name = "123";
    ResultSetMock resultSet;
    auto ret = cloudDiskDataHandler_->RecycleFile(recordId, parentCloudId, name, resultSet);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "RecycleFile End";
}

/**
 * @tc.name: RecycleFileTest002
 * @tc.desc: Verify the RecycleFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, RecycleFileTest002, TestSize.Level1)
{
    string recordId = "";
    ResultSetMock resultSet;
    string parentCloudId = "rootId";
    string name = "123";
    auto ret = cloudDiskDataHandler_->RecycleFile(recordId, parentCloudId, name, resultSet);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "RecycleFile End";
}

/**
 * @tc.name: RecycleFileTest003
 * @tc.desc: Verify the RecycleFile function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, RecycleFileTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecycleFile Start";
    string recordId = "sample_id";
    string parentCloudId = "rootId";
    string name = "123";
    ResultSetMock resultSet;
    EXPECT_CALL(*cloudDiskDataHandler_, Update(_, _, _, _)).WillOnce(Return(E_RDB));
    auto ret = cloudDiskDataHandler_->RecycleFile(recordId, parentCloudId, name, resultSet);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "RecycleFile End";
}

/**
 * @tc.name: SetRetryTest001
 * @tc.desc: Verify the SetRetry function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, SetRetryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRetry Start";
    string recordId = "sample_id";
    auto ret = cloudDiskDataHandler_->SetRetry(recordId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "SetRetry End";
}

/**
 * @tc.name: SetRetryTest002
 * @tc.desc: Verify the SetRetry function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, SetRetryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRetry Start";
    string recordId = "";
    auto ret = cloudDiskDataHandler_->SetRetry(recordId);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "SetRetry End";
}

/**
 * @tc.name: SetRetryTest003
 * @tc.desc: Verify the SetRetry function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, SetRetryTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetRetry Start";
    string recordId = "sample_id";
    EXPECT_CALL(*cloudDiskDataHandler_, Update(_, _, _, _)).WillOnce(Return(E_RDB));
    auto ret = cloudDiskDataHandler_->SetRetry(recordId);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "SetRetry End";
}

/**
 * @tc.name: GetCheckRecordsTest001
 * @tc.desc: Verify the GetCheckRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetCheckRecordsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCheckRecords Start";
    vector<DriveKit::DKRecordId> checkRecords;
    shared_ptr<vector<DKRecord>> records = std::make_shared<vector<DKRecord>>();
    DKRecordMock recordElem1;
    DKRecordMock recordElem2;
    records->emplace_back(recordElem1);
    records->emplace_back(recordElem2);
    int32_t ret = cloudDiskDataHandler_->GetCheckRecords(checkRecords, records);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "GetCheckRecords End";
}

/**
 * @tc.name: GetCheckRecordsTest002
 * @tc.desc: Verify the GetCheckRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetCheckRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCheckRecords Start";
    vector<DriveKit::DKRecordId> checkRecords;
    shared_ptr<vector<DKRecord>> records = std::make_shared<vector<DKRecord>>();
    int32_t ret = cloudDiskDataHandler_->GetCheckRecords(checkRecords, records);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "GetCheckRecords End";
}

/**
 * @tc.name: GetCheckRecordsTest003
 * @tc.desc: Verify the GetCheckRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetCheckRecordsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCheckRecords Start";
    try {
        EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
        vector<DriveKit::DKRecordId> checkRecords;
        shared_ptr<vector<DKRecord>> records = std::make_shared<vector<DKRecord>>();
        int32_t ret = cloudDiskDataHandler_->GetCheckRecords(checkRecords, records);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCheckRecords ERROR";
    }
    GTEST_LOG_(INFO) << "GetCheckRecords End";
}

/**
 * @tc.name: CleanCloudRecordTest001
 * @tc.desc: Verify the CleanCloudRecord function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, CleanCloudRecordTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCloudRecord Start";
    const int action = CleanAction::CLEAR_DATA;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(Return(-1));
    EXPECT_CALL(*cloudDiskDataHandler_, Delete(_, _, _)).WillOnce(Return(-1));
    int32_t ret = cloudDiskDataHandler_->CleanCloudRecord(action);
    EXPECT_EQ(ret, -1);
    GTEST_LOG_(INFO) << "CleanCloudRecord End";
}

/**
 * @tc.name: CleanCloudRecordTest002
 * @tc.desc: Verify the CleanCloudRecord function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, CleanCloudRecordTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCloudRecord Start";
    const int action = CleanAction::CLEAR_DATA;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(0), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(0));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));
    int32_t ret = cloudDiskDataHandler_->CleanCloudRecord(action);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "CleanCloudRecord End";
}

/**
 * @tc.name: CleanCloudRecordTest003
 * @tc.desc: Verify the CleanCloudRecord function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, CleanCloudRecordTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCloudRecord Start";
    const int action = CleanAction::CLEAR_DATA;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSet, GetRowCount(_)).WillRepeatedly(DoAll(SetArgReferee<0>(0), Return(E_OK)));
    EXPECT_CALL(*resultSet, GoToNextRow())
        .Times(2)
        .WillOnce(Return(0))
        .WillOnce(Return(1));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GetString(_, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*cloudDiskDataHandler_, BatchDetete(_, _, _)).WillOnce(Return(E_OK));
    EXPECT_CALL(*cloudDiskDataHandler_, Delete(_, _, _)).WillOnce(Return(E_OK));
    int32_t ret = cloudDiskDataHandler_->CleanCloudRecord(action);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "CleanCloudRecord End";
}

/**
 * @tc.name: OnDownloadAssetsParaMapTest001
 * @tc.desc: Verify the OnDownloadAssets function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnDownloadAssetsParaMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadAssets Start";
    const map<DriveKit::DKDownloadAsset, DriveKit::DKDownloadResult> resultMap;
    int32_t ret = cloudDiskDataHandler_->OnDownloadAssets(resultMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnDownloadAssets End";
}

/**
 * @tc.name: GetCreatedRecordsTest001
 * @tc.desc: Verify the GetCreatedRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetCreatedRecordsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreatedRecords Start";
    vector<DKRecord> records;
    int32_t ret = cloudDiskDataHandler_->GetCreatedRecords(records);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "GetCreatedRecords End";
}

/**
 * @tc.name: GetCreatedRecordsTest002
 * @tc.desc: Verify the GetCreatedRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetCreatedRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreatedRecords Start";
    vector<DKRecord> records;
    DKRecord recordElem1;
    records.emplace_back(recordElem1);
    int32_t ret = cloudDiskDataHandler_->GetCreatedRecords(records);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetCreatedRecords End";
}

/**
 * @tc.name: GetCreatedRecordsTest003
 * @tc.desc: Verify the GetCreatedRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetCreatedRecordsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreatedRecords Start";
    try {
        vector<DKRecord> records;
        EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
        int32_t ret = cloudDiskDataHandler_->GetCreatedRecords(records);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCreatedRecords ERROR";
    }
    GTEST_LOG_(INFO) << "GetCreatedRecords End";
}

/**
 * @tc.name: GetDeletedRecordsTest001
 * @tc.desc: Verify the GetDeletedRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetDeletedRecordsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeletedRecords Start";
    vector<DKRecord> records;
    int32_t ret = cloudDiskDataHandler_->GetDeletedRecords(records);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "GetDeletedRecords End";
}

/**
 * @tc.name: GetDeletedRecordsTest002
 * @tc.desc: Verify the GetDeletedRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetDeletedRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDeletedRecords Start";
    try {
        vector<DKRecord> records;
        EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
        int32_t ret = cloudDiskDataHandler_->GetDeletedRecords(records);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDeletedRecords ERROR";
    }
    GTEST_LOG_(INFO) << "GetDeletedRecords End";
}

/**
 * @tc.name: GetMetaModifiedRecordsTest001
 * @tc.desc: Verify the GetMetaModifiedRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetMetaModifiedRecordsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMetaModifiedRecords Start";
    vector<DKRecord> records;
    int32_t ret = cloudDiskDataHandler_->GetMetaModifiedRecords(records);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "GetMetaModifiedRecords End";
}

/**
 * @tc.name: GetMetaModifiedRecordsTest002
 * @tc.desc: Verify the GetMetaModifiedRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetMetaModifiedRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMetaModifiedRecords Start";
    try {
        vector<DKRecord> records;
        EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
        int32_t ret = cloudDiskDataHandler_->GetMetaModifiedRecords(records);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetMetaModifiedRecords ERROR";
    }
    GTEST_LOG_(INFO) << "GetMetaModifiedRecords End";
}

/**
 * @tc.name: GetFileModifiedRecordsTest001
 * @tc.desc: Verify the GetFileModifiedRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetFileModifiedRecordsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileModifiedRecords Start";
    vector<DKRecord> records;
    int32_t ret = cloudDiskDataHandler_->GetFileModifiedRecords(records);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "GetFileModifiedRecords End";
}

/**
 * @tc.name: GetFileModifiedRecordsTest002
 * @tc.desc: Verify the GetFileModifiedRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetFileModifiedRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileModifiedRecords Start";
    vector<DKRecord> records;
    DKRecord recordElem1;
    records.emplace_back(recordElem1);
    int32_t ret = cloudDiskDataHandler_->GetFileModifiedRecords(records);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "GetFileModifiedRecords End";
}

/**
 * @tc.name: GetFileModifiedRecordsTest003
 * @tc.desc: Verify the GetFileModifiedRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetFileModifiedRecordsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileModifiedRecords Start";
    try {
        vector<DKRecord> records;
        EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
        int32_t ret = cloudDiskDataHandler_->GetFileModifiedRecords(records);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileModifiedRecords ERROR";
    }
    GTEST_LOG_(INFO) << "GetFileModifiedRecords End";
}

/**
 * @tc.name: OnCreateRecordsTest001
 * @tc.desc: Verify the OnCreateRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnCreateRecordsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecords Start";
    const map<DKRecordId, DKRecordOperResult> testMap;
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
    int32_t ret = cloudDiskDataHandler_->OnCreateRecords(testMap);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "OnCreateRecords End";
}

/**
 * @tc.name: OnCreateRecordsTest002
 * @tc.desc: Verify the OnCreateRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnCreateRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecords Start";
    DKRecordId recordId;
    DKRecordOperResult result;
    map<DKRecordId, DKRecordOperResult> testMap;
    testMap.insert({recordId, result});
    auto resultSet = make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).Times(3).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(-1));
    int32_t ret = cloudDiskDataHandler_->OnCreateRecords(testMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnCreateRecords End";
}

/**
 * @tc.name: OnCreateRecordsTest003
 * @tc.desc: Verify the OnCreateRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnCreateRecordsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecords Start";
    DKRecordId recordId;
    DKRecordOperResult result;
    DKError error;
    error.SetLocalError(DriveKit::DKLocalErrorCode::EXISTS);
    result.SetDKError(error);
    map<DKRecordId, DKRecordOperResult> testMap;
    testMap.insert({recordId, result});
    auto resultSet = make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).Times(3).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(-1));
    int32_t ret = cloudDiskDataHandler_->OnCreateRecords(testMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnCreateRecords End";
}

/**
 * @tc.name: OnDeleteRecordsTest001
 * @tc.desc: Verify the OnDeleteRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnDeleteRecordsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDeleteRecords Start";
    DKRecordId recordId;
    DKRecordOperResult result;
    map<DKRecordId, DKRecordOperResult> testMap;
    testMap.insert({recordId, result});
    int32_t ret = cloudDiskDataHandler_->OnDeleteRecords(testMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnDeleteRecords End";
}

/**
 * @tc.name: OnDeleteRecordsTest002
 * @tc.desc: Verify the OnDeleteRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnDeleteRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDeleteRecords Start";
    DKRecordId recordId;
    DKRecordOperResult result;
    DKError error;
    error.SetLocalError(DriveKit::DKLocalErrorCode::EXISTS);
    result.SetDKError(error);
    map<DKRecordId, DKRecordOperResult> testMap;
    testMap.insert({recordId, result});
    int32_t ret = cloudDiskDataHandler_->OnDeleteRecords(testMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnDeleteRecords End";
}

/**
 * @tc.name:OnModifyMdirtyRecordsTest001
 * @tc.desc: Verify the OnModifyMdirtyRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnModifyMdirtyRecordsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords Start";
    const map<DKRecordId, DKRecordOperResult> testMap;
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
    int32_t ret = cloudDiskDataHandler_->OnModifyMdirtyRecords(testMap);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords End";
}

/**
 * @tc.name: OnModifyMdirtyRecordsTest002
 * @tc.desc: Verify the OnModifyMdirtyRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnModifyMdirtyRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords Start";
    DKRecordId recordId;
    DKRecordOperResult result;
    map<DKRecordId, DKRecordOperResult> testMap;
    testMap.insert({recordId, result});
    auto resultSet = make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).Times(3).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(-1));
    int32_t ret = cloudDiskDataHandler_->OnModifyMdirtyRecords(testMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords End";
}

/**
 * @tc.name: OnModifyMdirtyRecordsTest003
 * @tc.desc: Verify the OnModifyMdirtyRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnModifyMdirtyRecordsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords Start";
    DKRecordId recordId;
    DKRecordOperResult result;
    DKError error;
    error.SetLocalError(DriveKit::DKLocalErrorCode::EXISTS);
    result.SetDKError(error);
    map<DKRecordId, DKRecordOperResult> testMap;
    testMap.insert({recordId, result});
    auto resultSet = make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).Times(3).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(-1));
    int32_t ret = cloudDiskDataHandler_->OnModifyMdirtyRecords(testMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnModifyMdirtyRecords End";
}

/**
 * @tc.name:OnModifyFdirtyRecordsTest001
 * @tc.desc: Verify the OnModifyFdirtyRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnModifyFdirtyRecordsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords Start";
    map<DKRecordId, DKRecordOperResult> testMap;
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
    int32_t ret = cloudDiskDataHandler_->OnModifyFdirtyRecords(testMap);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords End";
}

/**
 * @tc.name: OnModifyFdirtyRecordsTest002
 * @tc.desc: Verify the OnModifyFdirtyRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnModifyFdirtyRecordsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords Start";
    DKRecordId recordId;
    DKRecordOperResult result;
    map<DKRecordId, DKRecordOperResult> testMap;
    testMap.insert({recordId, result});
    auto resultSet = make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).Times(3).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(-1));
    int32_t ret = cloudDiskDataHandler_->OnModifyFdirtyRecords(testMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords End";
}

/**
 * @tc.name: OnModifyFdirtyRecordsTest003
 * @tc.desc: Verify the OnModifyFdirtyRecords function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnModifyFdirtyRecordsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords Start";
    DKRecordId recordId;
    DKRecordOperResult result;
    DKError error;
    error.SetLocalError(DriveKit::DKLocalErrorCode::EXISTS);
    result.SetDKError(error);
    map<DKRecordId, DKRecordOperResult> testMap;
    testMap.insert({recordId, result});
    auto resultSet = make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).Times(3).WillRepeatedly(Return(E_OK));
    EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(-1));
    int32_t ret = cloudDiskDataHandler_->OnModifyFdirtyRecords(testMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnModifyFdirtyRecords End";
}

/**
 * @tc.name: OnDownloadAssetsTest001
 * @tc.desc: Verify the OnDownloadAssets function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnDownloadAssetsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadAssets Start";
    DriveKit::DKDownloadAsset asset;
    int32_t ret = cloudDiskDataHandler_->OnDownloadAssets(asset);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnDownloadAssets End";
}

/**
 * @tc.name: OnCreateRecordSuccessTest001
 * @tc.desc: Verify the OnCreateRecordSuccess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnCreateRecordSuccessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess Start";
    DKRecordId id = "";
    DKRecordOperResult result;
    pair<DKRecordId, DKRecordOperResult> entry = {id, result};
    string testString = "sample_string";
    LocalInfo localInfo = {0, 0};
    unordered_map<string, LocalInfo> localMap;
    localMap.insert({testString, localInfo});
    int32_t ret = cloudDiskDataHandler_->OnCreateRecordSuccess(entry, localMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess End";
}

/**
 * @tc.name: OnCreateRecordSuccessTest002
 * @tc.desc: Verify the OnCreateRecordSuccess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnCreateRecordSuccessTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess Start";
    DKRecordId id = "sample_id";
    DKRecordOperResult result;
    pair<DKRecordId, DKRecordOperResult> entry = {id, result};
    unordered_map<string, LocalInfo> localMap;
    int32_t ret = cloudDiskDataHandler_->OnCreateRecordSuccess(entry, localMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess End";
}

/**
 * @tc.name: OnCreateRecordSuccessTest003
 * @tc.desc: Verify the OnCreateRecordSuccess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnCreateRecordSuccessTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess Start";
    DKRecordId id = "sample_id";
    DKRecordOperResult result;
    pair<DKRecordId, DKRecordOperResult> entry = {id, result};
    string testString = id;
    LocalInfo localInfo = {0, 0};
    unordered_map<string, LocalInfo> localMap;
    localMap.insert({testString, localInfo});
    int32_t ret = cloudDiskDataHandler_->OnCreateRecordSuccess(entry, localMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnCreateRecordSuccess End";
}

/**
 * @tc.name: OnDeleteRecordSuccessTest001
 * @tc.desc: Verify the OnDeleteRecordSuccess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnDeleteRecordSuccessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDeleteRecordSuccess Start";
    DKRecordId recordId = "sample_id";
    DKRecordOperResult result;
    pair<DKRecordId, DKRecordOperResult> entry = {recordId, result};
    int32_t ret = cloudDiskDataHandler_->OnDeleteRecordSuccess(entry);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnDeleteRecordSuccess End";
}

/**
 * @tc.name: OnModifyRecordSuccessTest001
 * @tc.desc: Verify the OnModifyRecordSuccess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnModifyRecordSuccessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyRecordSuccess Start";
    DKRecordId recordId = "sample_id";
    DKRecordOperResult result;
    DKRecord record;
    DKRecordData data;
    record.SetRecordData(data);
    result.SetDKRecord(record);
    pair<DKRecordId, DKRecordOperResult> entry = {recordId, result};
    unordered_map<string, LocalInfo> localMap;
    int32_t ret = cloudDiskDataHandler_->OnModifyRecordSuccess(entry, localMap);
    EXPECT_EQ(ret, E_INVAL_ARG);
    GTEST_LOG_(INFO) << "OnModifyRecordSuccess End";
}

/**
 * @tc.name: OnModifyRecordSuccessTest002
 * @tc.desc: Verify the OnModifyRecordSuccess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnModifyRecordSuccessTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnModifyRecordSuccess Start";
    DKRecordId recordId = "sample_id";
    DKRecordOperResult result;
    DKRecord record;
    DKRecordData data;
    DKFieldKey key1 = DK_FILE_ATTRIBUTES;
    DKRecordFieldMap fieldMap;
    string string1InMap = DK_META_TIME_EDITED;
    int64_t meta_date_modified = 10;
    DKRecordField field1InMap(meta_date_modified);
    fieldMap.insert({string1InMap, field1InMap});
    string string2InMap = "sample_map";
    DKRecordField field2InMap;
    fieldMap.insert({string2InMap, field2InMap});
    DKRecordField field1(fieldMap);
    data.insert({key1, field1});
    DKFieldKey key2 = "sample_key";
    DKRecordField field2;
    data.insert({key2, field2});
    record.SetRecordData(data);
    result.SetDKRecord(record);
    pair<DKRecordId, DKRecordOperResult> entry = {recordId, result};
    string localString1 = recordId;
    LocalInfo localInfo1;
    unordered_map<string, LocalInfo> localMap;
    localMap.insert({localString1, localInfo1});
    int32_t ret = cloudDiskDataHandler_->OnModifyRecordSuccess(entry, localMap);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "OnModifyRecordSuccess End";
}

/**
 * @tc.name: IsTimeChangedTest001
 * @tc.desc: Verify the IsTimeChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, IsTimeChangedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged Start";
    DriveKit::DKRecord record;
    string type = CloudDisk::FileColumn::FILE_TIME_EDITED;
    string cloudId = "rootId";
    string testString1 = "testString1";
    LocalInfo localInfo1 = {0, 0};
    unordered_map<string, LocalInfo> localMap;
    localMap.insert({testString1, localInfo1});
    bool ret = cloudDiskDataHandler_->IsTimeChanged(record, localMap, cloudId, type);
    EXPECT_EQ(ret, true);
    GTEST_LOG_(INFO) << "IsTimeChanged End";
}

/**
 * @tc.name: IsTimeChangedTest002
 * @tc.desc: Verify the IsTimeChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, IsTimeChangedTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged Start";
    string type = CloudDisk::FileColumn::FILE_TIME_EDITED;
    string cloudId = "rootId";
    string testString1 = cloudId;
    LocalInfo localInfo1 = {0, 0};
    string testString2 = "sample_id";
    LocalInfo localInfo2 = {0, 0};
    unordered_map<string, LocalInfo> localMap;
    localMap.insert({testString1, localInfo1});
    localMap.insert({testString2, localInfo2});
    DKFieldKey key1 = "DK_FILE_ATTRIBUTES";
    DKRecordField field1;
    DKRecordData fields;
    fields.insert({key1, field1});
    DriveKit::DKRecord record;
    record.SetRecordData(fields);
    bool ret = cloudDiskDataHandler_->IsTimeChanged(record, localMap, cloudId, type);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsTimeChanged End";
}

/**
 * @tc.name: IsTimeChangedTest003
 * @tc.desc: Verify the IsTimeChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, IsTimeChangedTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged Start";
    string type = CloudDisk::FileColumn::FILE_TIME_EDITED;
    string cloudId = "rootId";
    string testString1 = cloudId;
    int64_t mdirtyTime = 0;
    int64_t fdirtyTime = 0;
    LocalInfo localInfo1 = {mdirtyTime, fdirtyTime};
    string testString2 = "sample_id";
    LocalInfo localInfo2 = {0, 0};
    unordered_map<string, LocalInfo> localMap;
    localMap.insert({testString1, localInfo1});
    localMap.insert({testString2, localInfo2});
    int64_t localTime = localInfo1.fdirtyTime;
    int64_t cloudTime = localTime;
    DKRecordFieldMap mapInField1;
    string mapString = DK_FILE_TIME_EDITED;
    DKRecordField mapField(cloudTime);
    mapInField1.insert({mapString, mapField});
    DKFieldKey key1 = "DK_FILE_ATTRIBUTES";
    DKRecordField field1(mapInField1);
    DKFieldKey key2 = "sample_key";
    DKRecordField field2;
    DKRecordData fields;
    fields.insert({key1, field1});
    fields.insert({key2, field2});
    DriveKit::DKRecord record;
    record.SetRecordData(fields);
    bool ret = cloudDiskDataHandler_->IsTimeChanged(record, localMap, cloudId, type);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsTimeChanged End";
}

/**
 * @tc.name: IsTimeChangedTest004
 * @tc.desc: Verify the IsTimeChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, IsTimeChangedTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged Start";
    string type = CloudDisk::FileColumn::FILE_TIME_EDITED;
    string cloudId = "rootId";

    string testString1 = cloudId;
    int64_t mdirtyTime = 0;
    int64_t fdirtyTime = 0;
    LocalInfo localInfo1 = {mdirtyTime, fdirtyTime};
    string testString2 = "sample_id";
    LocalInfo localInfo2 = {0, 0};
    unordered_map<string, LocalInfo> localMap;
    localMap.insert({testString1, localInfo1});
    localMap.insert({testString2, localInfo2});
    int64_t cloudTime = 20;
    DKRecordFieldMap mapInField1;
    string mapString = DK_FILE_TIME_EDITED;
    DKRecordField mapField(cloudTime);
    mapInField1.insert({mapString, mapField});
    DKFieldKey key1 = "DK_FILE_ATTRIBUTES";
    DKRecordField field1(mapInField1);
    DKFieldKey key2 = "sample_key";
    DKRecordField field2;
    DKRecordData fields;
    fields.insert({key1, field1});
    fields.insert({key2, field2});
    DriveKit::DKRecord record;
    record.SetRecordData(fields);
    bool ret = cloudDiskDataHandler_->IsTimeChanged(record, localMap, cloudId, type);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsTimeChanged End";
}

/**
 * @tc.name: IsTimeChangedTest005
 * @tc.desc: Verify the IsTimeChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, IsTimeChangedTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged Start";
    string type = CloudDisk::FileColumn::FILE_TIME_EDITED;
    string cloudId = "rootId";
    string testString1 = cloudId;
    int64_t mdirtyTime = 0;
    int64_t fdirtyTime = 0;
    LocalInfo localInfo1 = {mdirtyTime, fdirtyTime};
    string testString2 = "sample_id";
    LocalInfo localInfo2 = {0, 0};
    unordered_map<string, LocalInfo> localMap;
    localMap.insert({testString1, localInfo1});
    localMap.insert({testString2, localInfo2});
    string myString = "sample_string";
    DKRecordFieldMap mapInField1;
    string mapString = DK_FILE_TIME_EDITED;
    DKRecordField mapField(myString);
    mapInField1.insert({mapString, mapField});
    DKFieldKey key1 = "DK_FILE_ATTRIBUTES";
    DKRecordField field1(mapInField1);
    DKFieldKey key2 = "sample_key";
    DKRecordField field2;
    DKRecordData fields;
    fields.insert({key1, field1});
    fields.insert({key2, field2});
    DriveKit::DKRecord record;
    record.SetRecordData(fields);
    bool ret = cloudDiskDataHandler_->IsTimeChanged(record, localMap, cloudId, type);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsTimeChanged End";
}

/**
 * @tc.name: IsTimeChangedTest006
 * @tc.desc: Verify the IsTimeChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, IsTimeChangedTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged Start";
    string type = CloudDisk::FileColumn::FILE_TIME_RECYCLED;
    string cloudId = "rootId";
    string testString1 = cloudId;
    int64_t mdirtyTime = 0;
    int64_t fdirtyTime = 0;
    LocalInfo localInfo1 = {mdirtyTime, fdirtyTime};
    string testString2 = "sample_id";
    LocalInfo localInfo2 = {0, 0};
    unordered_map<string, LocalInfo> localMap;
    localMap.insert({testString1, localInfo1});
    localMap.insert({testString2, localInfo2});
    int64_t localTime = localInfo1.mdirtyTime;
    int64_t cloudTime = localTime;
    DKRecordFieldMap mapInField1;
    string mapString = DK_FILE_TIME_EDITED;
    DKRecordField mapField(cloudTime);
    mapInField1.insert({mapString, mapField});
    DKFieldKey key1 = "DK_FILE_ATTRIBUTES";
    DKRecordField field1(mapInField1);
    DKFieldKey key2 = "sample_key";
    DKRecordField field2;
    DKRecordData fields;
    fields.insert({key1, field1});
    fields.insert({key2, field2});
    DriveKit::DKRecord record;
    record.SetRecordData(fields);
    bool ret = cloudDiskDataHandler_->IsTimeChanged(record, localMap, cloudId, type);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsTimeChanged End";
}

/**
 * @tc.name: IsTimeChangedTest007
 * @tc.desc: Verify the IsTimeChanged function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, IsTimeChangedTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsTimeChanged Start";
    string type = CloudDisk::FileColumn::FILE_TIME_RECYCLED;
    string cloudId = "rootId";
    string testString1 = cloudId;
    int64_t mdirtyTime = 0;
    int64_t fdirtyTime = 0;
    LocalInfo localInfo1 = {mdirtyTime, fdirtyTime};
    string testString2 = "sample_id";
    LocalInfo localInfo2 = {0, 0};
    unordered_map<string, LocalInfo> localMap;
    localMap.insert({testString1, localInfo1});
    localMap.insert({testString2, localInfo2});
    string myString = "sample_string";
    DKRecordFieldMap mapInField1;
    string mapString = DK_FILE_TIME_EDITED;
    DKRecordField mapField(myString);
    mapInField1.insert({mapString, mapField});
    DKFieldKey key1 = "DK_FILE_ATTRIBUTES";
    DKRecordField field1(mapInField1);
    DKFieldKey key2 = "sample_key";
    DKRecordField field2;
    DKRecordData fields;
    fields.insert({key1, field1});
    fields.insert({key2, field2});
    DriveKit::DKRecord record;
    record.SetRecordData(fields);
    bool ret = cloudDiskDataHandler_->IsTimeChanged(record, localMap, cloudId, type);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "IsTimeChanged End";
}

/**
 * @tc.name: GetLocalInfoTest001
 * @tc.desc: Verify the GetLocalInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetLocalInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalInfo Start";
    map<DKRecordId, DKRecordOperResult> map;
    DKRecordId id = "sample_id";
    DKRecordOperResult result;
    map.insert({id, result});
    unordered_map<string, LocalInfo> infoMap;
    const string type = CloudDisk::FileColumn::CLOUD_ID;
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
    .Times(2)
    .WillOnce(Return(E_RDB))
    .WillOnce(Return(E_RDB));
    int32_t ret = cloudDiskDataHandler_->GetLocalInfo(map, infoMap, type);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "GetLocalInfo End";
}

/**
 * @tc.name: GetLocalInfoTest002
 * @tc.desc: Verify the GetLocalInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetLocalInfoTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalInfo Start";
    map<DKRecordId, DKRecordOperResult> map;
    unordered_map<string, LocalInfo> infoMap;
    const string type = CloudDisk::FileColumn::FILE_NAME;
    int32_t ret = cloudDiskDataHandler_->GetLocalInfo(map, infoMap, type);
    EXPECT_EQ(ret, E_UNKNOWN);
    GTEST_LOG_(INFO) << "GetLocalInfo End";
}

/**
 * @tc.name: GetLocalInfoTest003
 * @tc.desc: Verify the GetLocalInfo function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetLocalInfoTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLocalInfo Start";
    try {
        map<DKRecordId, DKRecordOperResult> map;
        DKRecordId id = "sample_id";
        DKRecordOperResult result;
        map.insert({id, result});
        unordered_map<string, LocalInfo> infoMap;
        const string type = CloudDisk::FileColumn::CLOUD_ID;
        EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
        int32_t ret = cloudDiskDataHandler_->GetLocalInfo(map, infoMap, type);
        EXPECT_EQ(ret, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLocalInfo ERROR";
    }
    GTEST_LOG_(INFO) << "GetLocalInfo End";
}

/**
 * @tc.name: BuildInfoMapTest001
 * @tc.desc: Verify the BuildInfoMap function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, BuildInfoMapTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildInfoMap Start";
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    unordered_map<string, LocalInfo> infoMap;
    const string type = CloudDisk::FileColumn::CLOUD_ID;
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
    .WillOnce(Return(E_RDB))
    .WillOnce(Return(E_RDB));
    int32_t ret = cloudDiskDataHandler_->BuildInfoMap(resultSet, infoMap, type);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "BuildInfoMap End";
}

/**
 * @tc.name: BuildInfoMapTest002
 * @tc.desc: Verify the BuildInfoMap function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, BuildInfoMapTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildInfoMap Start";
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    unordered_map<string, LocalInfo> infoMap;
    const string type = CloudDisk::FileColumn::CLOUD_ID;
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
    .Times(3)
    .WillOnce(Return(E_OK))
    .WillOnce(Return(E_RDB))
    .WillOnce(Return(E_RDB));
    int32_t ret = cloudDiskDataHandler_->BuildInfoMap(resultSet, infoMap, type);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "BuildInfoMap End";
}

/**
 * @tc.name: BuildInfoMapTest003
 * @tc.desc: Verify the BuildInfoMap function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, BuildInfoMapTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildInfoMap Start";
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    unordered_map<string, LocalInfo> infoMap;
    const string type = CloudDisk::FileColumn::CLOUD_ID;
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .Times(4)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_RDB))
        .WillOnce(Return(E_RDB));
    int32_t ret = cloudDiskDataHandler_->BuildInfoMap(resultSet, infoMap, type);
    EXPECT_EQ(ret, E_RDB);
    GTEST_LOG_(INFO) << "BuildInfoMap End";
}

/**
 * @tc.name: BuildInfoMapTest004
 * @tc.desc: Verify the BuildInfoMap function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, BuildInfoMapTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildInfoMap Start";
    std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
    unordered_map<string, LocalInfo> infoMap;
    const string type = CloudDisk::FileColumn::CLOUD_ID;
    EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
        .Times(3)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK));
    EXPECT_CALL(*resultSet, GoToNextRow())
        .Times(3)
        .WillOnce(Return(0))
        .WillOnce(Return(0))
        .WillOnce(Return(-1));
    EXPECT_CALL(*resultSet, GetString(_, _))
    .Times(2)
    .WillOnce(Return(E_OK))
    .WillOnce(Return(-1));
    EXPECT_CALL(*resultSet, GetLong(_, _))
        .Times(2)
        .WillOnce(Return(E_OK))
        .WillOnce(Return(E_OK));
    int32_t ret = cloudDiskDataHandler_->BuildInfoMap(resultSet, infoMap, type);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "BuildInfoMap End";
}

/**
 * @tc.name: ResetTest001
 * @tc.desc: Verify the Reset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, ResetTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Reset Start";
    cloudDiskDataHandler_->Reset();
    EXPECT_EQ(cloudDiskDataHandler_->modifyFailSet_.size(), 0);
    GTEST_LOG_(INFO) << "Reset End";
}

/**
 * @tc.name: GetDownloadAssetTest000
 * @tc.desc: Verify the GetDownloadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetDownloadAssetTest000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadAsset Start";
    try {
        std::string fileUri = "";
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        int result = cloudDiskDataHandler_->GetDownloadAsset(fileUri, outAssetsToDownload);
        EXPECT_EQ(result, E_INVAL_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDownloadAsset ERROR";
    }
    GTEST_LOG_(INFO) << "GetDownloadAsset End";
}

/**
 * @tc.name: GetDownloadAssetTest001
 * @tc.desc: Verify the GetDownloadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetDownloadAssetTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadAsset Start";
    try {
        std::string fileUri = "file://media/Photo/12/IMG_12345_0011/test.jpg";
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        g_getCloudIdFlag = 3;
        int result = cloudDiskDataHandler_->GetDownloadAsset(fileUri, outAssetsToDownload);
        EXPECT_EQ(result, E_INVAL_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDownloadAsset ERROR";
    }
    GTEST_LOG_(INFO) << "GetDownloadAsset End";
}

/**
 * @tc.name: GetDownloadAssetTest002
 * @tc.desc: Verify the GetDownloadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetDownloadAssetTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadAsset Start";
    try {
        std::string fileUri = "file://media/Photo/12/IMG_12345_0011/test.jpg";
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        g_getCloudIdFlag = 1;
        EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(nullptr));
        int result = cloudDiskDataHandler_->GetDownloadAsset(fileUri, outAssetsToDownload);
        EXPECT_EQ(result, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDownloadAsset ERROR";
    }
    GTEST_LOG_(INFO) << "GetDownloadAsset End";
}

/**
 * @tc.name: GetDownloadAssetTest003
 * @tc.desc: Verify the GetDownloadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetDownloadAssetTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadAsset Start";
    try {
        std::string fileUri = "file://media/Photo/12/IMG_12345_0011/test.jpg";
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        g_getCloudIdFlag = 1;
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(Return(E_RDB));
        int result = cloudDiskDataHandler_->GetDownloadAsset(fileUri, outAssetsToDownload);
        EXPECT_EQ(result, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDownloadAsset ERROR";
    }
    GTEST_LOG_(INFO) << "GetDownloadAsset End";
}

/**
 * @tc.name: GetDownloadAssetTest004
 * @tc.desc: Verify the GetDownloadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetDownloadAssetTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadAsset Start";
    try {
        std::string fileUri = "file://media/Photo/12/IMG_12345_0011/test.jpg";
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        g_getCloudIdFlag = 1;
        std::string fieldKey = "content";
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(FILE), Return(E_OK)));
        int result = cloudDiskDataHandler_->GetDownloadAsset(fileUri, outAssetsToDownload);
        for (auto &item : outAssetsToDownload)
        {
            EXPECT_EQ(item.fieldKey, fieldKey);
        }
        EXPECT_EQ(result, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDownloadAsset ERROR";
    }
    GTEST_LOG_(INFO) << "GetDownloadAsset End";
}

/**
 * @tc.name: GetDownloadAssetTest005
 * @tc.desc: Verify the GetDownloadAsset function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, GetDownloadAssetTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDownloadAsset Start";
    try {
        std::string fileUri = "file://media/Photo/12/IMG_12345_0011/test.jpg";
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        g_getCloudIdFlag = 1;
        std::string fieldKey = "content";
        std::shared_ptr<ResultSetMock> resultSet = std::make_shared<ResultSetMock>();
        EXPECT_CALL(*cloudDiskDataHandler_, Query(_, _)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSet, GetRowCount(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetInt(_, _)).WillRepeatedly(DoAll(SetArgReferee<1>(DIRECTORY), Return(E_OK)));
        int result = cloudDiskDataHandler_->GetDownloadAsset(fileUri, outAssetsToDownload);
        EXPECT_EQ(result, ERR_INVALID_VALUE);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetDownloadAsset ERROR";
    }
    GTEST_LOG_(INFO) << "GetDownloadAsset End";
}

/**
 * @tc.name: AppendFileToDownload000
 * @tc.desc: Verify the AppendFileToDownload function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, AppendFileToDownload000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AppendFileToDownload Start";
    try {
        std::string cloudId = "user.cloud.cloudid";
        std::string fieldKey = "content";
        vector<DriveKit::DKDownloadAsset> outAssetsToDownload;
        cloudDiskDataHandler_->AppendFileToDownload(cloudId, fieldKey, outAssetsToDownload);
        for (auto &item : outAssetsToDownload)
        {
            EXPECT_EQ(item.fieldKey, fieldKey);
        }
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AppendFileToDownload ERROR";
    }
    GTEST_LOG_(INFO) << "AppendFileToDownload End";
}

/**
 * @tc.name: OnDownloadSuccess001
 * @tc.desc: Verify the OnDownloadSuccess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnDownloadSuccess001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadSuccess Start";
    try {
        DriveKit::DKDownloadAsset asset;
        asset.asset.assetName = "user.cloud.cloudid.temp.download";
        EXPECT_CALL(*cloudDiskDataHandler_, Update(_, _, _, _)).WillOnce(Return(E_OK));
        int result = cloudDiskDataHandler_->OnDownloadSuccess(asset);
        EXPECT_EQ(result, E_OK);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnDownloadSuccess ERROR";
    }
    GTEST_LOG_(INFO) << "OnDownloadSuccess End";
}

/**
 * @tc.name: OnDownloadSuccess002
 * @tc.desc: Verify the OnDownloadSuccess function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, OnDownloadSuccess002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnDownloadSuccess Start";
    try {
        DriveKit::DKDownloadAsset asset;
        asset.asset.assetName = "user.cloud.cloudid.temp.download";
        EXPECT_CALL(*cloudDiskDataHandler_, Update(_, _, _, _)).WillOnce(Return(E_RDB));
        int result = cloudDiskDataHandler_->OnDownloadSuccess(asset);
        EXPECT_EQ(result, E_RDB);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnDownloadSuccess ERROR";
    }
    GTEST_LOG_(INFO) << "OnDownloadSuccess End";
}

/**
 * @tc.name: CleanCache000
 * @tc.desc: Verify the CleanCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, CleanCache000, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCache Start";
    try {
        const std::string fileUri = "";
        int result = cloudDiskDataHandler_->CleanCache(fileUri);
        EXPECT_EQ(result, E_INVAL_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanCache ERROR";
    }
    GTEST_LOG_(INFO) << "CleanCache End";
}

/**
 * @tc.name: CleanCache001
 * @tc.desc: Verify the CleanCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, CleanCache001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCache Start";
    try {
        const std::string fileUri = "file://media/Photo/12/IMG_12345_0011/test.jpg";
        g_getCloudIdFlag = 3;
        int result = cloudDiskDataHandler_->CleanCache(fileUri);
        EXPECT_EQ(result, E_INVAL_ARG);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanCache ERROR";
    }
    GTEST_LOG_(INFO) << "CleanCache End";
}

/**
 * @tc.name: CleanCache002
 * @tc.desc: Verify the CleanCache function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskDataHandlerTest, CleanCache002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCache Start";
    try {
        const std::string fileUri = "file://media/Photo/12/IMG_12345_0011/test.jpg";
        g_getCloudIdFlag = 0;
        const int32_t userId = 0;
        string bundleName = "com.ohos.test";
        auto rdb = make_shared<RdbStoreMock>();
        CloudDiskDataHandlerMock cloudDiskDataHandlerMock(userId, bundleName, rdb);
        int result = cloudDiskDataHandlerMock.CleanCache(fileUri);
        EXPECT_EQ(result, E_NO_SUCH_FILE);
    } catch(...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CleanCache ERROR";
    }
    GTEST_LOG_(INFO) << "CleanCache End";
}

} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS
