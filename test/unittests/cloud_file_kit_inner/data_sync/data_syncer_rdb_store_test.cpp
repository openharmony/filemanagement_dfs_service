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

#include "data_syncer_rdb_store.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "dfs_error.h"
#include "file_column.h"
#include "data_syncer_rdb_col.h"
#include "rdb_assistant.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MockDataSyncerRdbStore : public DataSyncerRdbStore {
public:
    MOCK_METHOD2(Insert, int32_t(int32_t userId, const string &bundleName));
    MOCK_METHOD4(UpdateSyncState, int32_t(int32_t userId, const string &bundleName,
        CloudSyncState cloudSyncState, ErrorType errorType));
    MOCK_METHOD3(GetLastSyncTime, int32_t(int32_t userId, const string &bundleName, int64_t &time));
    MOCK_METHOD2(QueryDataSyncer, int32_t(int32_t userId, shared_ptr<NativeRdb::ResultSet> &resultSet));
    MOCK_METHOD2(Query, int32_t(NativeRdb::AbsRdbPredicates predicates, shared_ptr<NativeRdb::ResultSet> &resultSet));
    MOCK_METHOD0(RdbInit, int32_t());
    MOCK_METHOD0(GetInstance, DataSyncerRdbStore&());
};

class DataSyncerRdbStoreTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataSyncerRdbStoreTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DataSyncerRdbStoreTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DataSyncerRdbStoreTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DataSyncerRdbStoreTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_001 Start";
    try {
        MockDataSyncerRdbStore mockDataSyncerRdbStore;
        int32_t userId = 1;
        EXPECT_CALL(mockDataSyncerRdbStore, Insert(userId, "testbundleName"))
            .WillOnce(Return(E_SEVICE_DIED))
            .WillOnce(Return(E_OK))
            .WillOnce(Return(E_RDB));
        
        EXPECT_EQ(mockDataSyncerRdbStore.Insert(userId, "testbundleName"), E_SEVICE_DIED);
        EXPECT_EQ(mockDataSyncerRdbStore.Insert(userId, "testbundleName"), E_OK);
        EXPECT_EQ(mockDataSyncerRdbStore.Insert(userId, "testbundleName"), E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_001 End";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_002 Start";
    try {
        MockDataSyncerRdbStore mockDataSyncerRdbStore;
        EXPECT_CALL(mockDataSyncerRdbStore, GetInstance())
            .WillOnce(ReturnRef(mockDataSyncerRdbStore));
        
        mockDataSyncerRdbStore.GetInstance();
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_002 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_002 End";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_003 Start";
    try {
        MockDataSyncerRdbStore mockDataSyncerRdbStore;
        EXPECT_CALL(mockDataSyncerRdbStore, RdbInit())
            .WillOnce(Return(E_RDB))
            .WillOnce(Return(E_OK));
        
        EXPECT_EQ(mockDataSyncerRdbStore.RdbInit(), E_RDB);
        EXPECT_EQ(mockDataSyncerRdbStore.RdbInit(), E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_003 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_003 End";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_004 Start";
    try {
        MockDataSyncerRdbStore mockDataSyncerRdbStore;
        int32_t userId = 1;
        string bundleName = "testbundleName";
        EXPECT_CALL(mockDataSyncerRdbStore,
            UpdateSyncState(userId, bundleName, CloudSyncState::DOWNLOADING, ErrorType::NO_ERROR))
            .WillOnce(Return(E_OK));
        EXPECT_CALL(mockDataSyncerRdbStore,
            UpdateSyncState(userId, bundleName, CloudSyncState::COMPLETED, ErrorType::NO_ERROR))
            .WillOnce(Return(E_OK));

        EXPECT_EQ(mockDataSyncerRdbStore
            .UpdateSyncState(userId, bundleName, CloudSyncState::DOWNLOADING, ErrorType::NO_ERROR), E_OK);
        EXPECT_EQ(mockDataSyncerRdbStore
            .UpdateSyncState(userId, bundleName, CloudSyncState::COMPLETED, ErrorType::NO_ERROR), E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_004 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_004 End";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_005 Start";
    try {
        MockDataSyncerRdbStore mockDataSyncerRdbStore;
        int32_t userId = 1;
        string bundleName = "testbundleName";
        int64_t time = 20240713;
        EXPECT_CALL(mockDataSyncerRdbStore, GetLastSyncTime(userId, bundleName, time))
            .WillOnce(Return(E_INVAL_ARG))
            .WillOnce(Return(E_OK));
        
        EXPECT_EQ(mockDataSyncerRdbStore.GetLastSyncTime(userId, bundleName, time), E_INVAL_ARG);
        EXPECT_EQ(mockDataSyncerRdbStore.GetLastSyncTime(userId, bundleName, time), E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_005 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_005 End";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_006 Start";
    try {
        MockDataSyncerRdbStore mockDataSyncerRdbStore;

        EXPECT_CALL(mockDataSyncerRdbStore, Query(_, _))
            .WillOnce(Return(E_RDB))
            .WillOnce(Return(E_OK));

        NativeRdb::AbsRdbPredicates predicates = NativeRdb::AbsRdbPredicates(DATA_SYNCER_TABLE);
        shared_ptr<NativeRdb::ResultSet> resultSet;
        auto queryRet1 = mockDataSyncerRdbStore.Query(predicates, resultSet);
        auto queryRet2 = mockDataSyncerRdbStore.Query(predicates, resultSet);
        EXPECT_EQ(queryRet1, E_RDB);
        EXPECT_EQ(queryRet2, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_006 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_006 End";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_007 Start";
    try {
        MockDataSyncerRdbStore mockDataSyncerRdbStore;
        EXPECT_CALL(mockDataSyncerRdbStore, RdbInit())
            .WillOnce(Return(E_RDB))
            .WillOnce(Return(E_OK));
        
        EXPECT_EQ(mockDataSyncerRdbStore.RdbInit(), E_RDB);
        EXPECT_EQ(mockDataSyncerRdbStore.RdbInit(), E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_007 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_007 End";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_008 Start";
    try {
        DataSyncerRdbStore dataSyncerRdbStore;
        int32_t userId = 1;
        string bundleName = "testBundleName";
        int32_t ret = dataSyncerRdbStore.Insert(userId, bundleName);
        EXPECT_EQ(ret, E_OK);

        userId = 0;
        ret = dataSyncerRdbStore.Insert(userId, bundleName);
        EXPECT_EQ(ret, E_OK);

        userId = -1;
        bundleName = "test01";
        ret = dataSyncerRdbStore.Insert(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_008 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_008 End";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_009 Start";
    try {
        DataSyncerRdbStore dataSyncerRdbStore;
        int32_t userId = 1;
        string bundleName = "testBundleName";
        int32_t ret = dataSyncerRdbStore.Insert(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
        ret = dataSyncerRdbStore.UpdateSyncState(userId, bundleName, CloudSyncState::COMPLETED, ErrorType::NO_ERROR);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_009 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_009 End";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_010 Start";
    try {
        DataSyncerRdbStore dataSyncerRdbStore;
        int32_t userId = 1;
        string bundleName = "testBundleName";
        int32_t ret = dataSyncerRdbStore.Insert(userId, bundleName);
        EXPECT_EQ(ret, E_OK);
        ret = dataSyncerRdbStore.UpdateSyncState(userId, bundleName, CloudSyncState::DOWNLOADING, ErrorType::NO_ERROR);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_010 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_010 End";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_011 Start";
    try {
        DataSyncerRdbStore dataSyncerRdbStore;
        int32_t userId = 0;
        string bundleName = "testBundleName";
        int64_t time = 20240722;

        int32_t ret = dataSyncerRdbStore.GetLastSyncTime(userId, bundleName, time);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_011 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_011 End";
}

HWTEST_F(DataSyncerRdbStoreTest, DataSyncerRdbStoreTest_012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_012 Start";
    try {
        DataSyncerRdbStore dataSyncerRdbStore;
        int32_t userId = 0;
        shared_ptr<NativeRdb::ResultSet> resultSet;

        int32_t ret = dataSyncerRdbStore.QueryDataSyncer(userId, resultSet);
        EXPECT_EQ(ret, E_OK);
        userId = 20;
        ret = dataSyncerRdbStore.QueryDataSyncer(userId, resultSet);
        EXPECT_EQ(ret, E_OK);

        userId = -1;
        ret = dataSyncerRdbStore.QueryDataSyncer(userId, resultSet);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_012 ERROR";
    }
    GTEST_LOG_(INFO) << "DataSyncerRdbStoreTest_012 End";
}

/**
 * @tc.name: GetSyncStateAndErrorTypeTest01
 * @tc.desc: Verify the GetSyncStateAndErrorType function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, GetSyncStateAndErrorTypeTest01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest01 Start";
    try {
        int32_t userId = 1;
        string bundleName = "test.bundle.name";
        CloudSyncState cloudSyncState = CloudSyncState::COMPLETED;
        ErrorType errorType = ErrorType::NO_ERROR;

        auto rdb = make_shared<CloudDisk::RdbStoreMock>();
        DataSyncerRdbStore dataSyncerRdbStore;
        dataSyncerRdbStore.rdb_ = rdb;
        vector<string> columns;

        EXPECT_CALL(*rdb, QueryByStep(_, columns, true)).WillOnce(Return(nullptr));

        auto ret = dataSyncerRdbStore
            .GetSyncStateAndErrorType(userId, bundleName, cloudSyncState, errorType);
        
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest01 ERROR";
    }
    GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest01 End";
}

/**
 * @tc.name: GetSyncStateAndErrorTypeTest02
 * @tc.desc: Verify the GetSyncStateAndErrorType function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, GetSyncStateAndErrorTypeTest02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest02 Start";
    try {
        int32_t userId = 1;
        string bundleName = "test.bundle.name";
        CloudSyncState cloudSyncState = CloudSyncState::COMPLETED;
        ErrorType errorType = ErrorType::NO_ERROR;

        auto rdb = make_shared<CloudDisk::RdbStoreMock>();
        DataSyncerRdbStore dataSyncerRdbStore;
        dataSyncerRdbStore.rdb_ = rdb;
        vector<string> columns;
        auto resultSet = make_shared<CloudDisk::AbsSharedResultSetMock>();

        EXPECT_CALL(*rdb, QueryByStep(_, columns, true)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_INVAL_ARG));

        auto ret = dataSyncerRdbStore
            .GetSyncStateAndErrorType(userId, bundleName, cloudSyncState, errorType);
        
        EXPECT_EQ(ret, E_INVAL_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest02 ERROR";
    }
    GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest02 End";
}

/**
 * @tc.name: GetSyncStateAndErrorTypeTest03
 * @tc.desc: Verify the GetSyncStateAndErrorType function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, GetSyncStateAndErrorTypeTest03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest03 Start";
    try {
        int32_t userId = 1;
        string bundleName = "test.bundle.name";
        CloudSyncState cloudSyncState = CloudSyncState::COMPLETED;
        ErrorType errorType = ErrorType::NO_ERROR;

        auto rdb = make_shared<CloudDisk::RdbStoreMock>();
        DataSyncerRdbStore dataSyncerRdbStore;
        dataSyncerRdbStore.rdb_ = rdb;
        vector<string> columns;
        auto resultSet = make_shared<CloudDisk::AbsSharedResultSetMock>();

        EXPECT_CALL(*rdb, QueryByStep(_, columns, true)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _)).WillOnce(Return(E_RDB));

        auto ret = dataSyncerRdbStore
            .GetSyncStateAndErrorType(userId, bundleName, cloudSyncState, errorType);
        
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest03 ERROR";
    }
    GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest03 End";
}

/**
 * @tc.name: GetSyncStateAndErrorTypeTest04
 * @tc.desc: Verify the GetSyncStateAndErrorType function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, GetSyncStateAndErrorTypeTest04, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest04 Start";
    try {
        int32_t userId = 1;
        string bundleName = "test.bundle.name";
        CloudSyncState cloudSyncState = CloudSyncState::COMPLETED;
        ErrorType errorType = ErrorType::NO_ERROR;

        auto rdb = make_shared<CloudDisk::RdbStoreMock>();
        DataSyncerRdbStore dataSyncerRdbStore;
        dataSyncerRdbStore.rdb_ = rdb;
        vector<string> columns;
        auto resultSet = make_shared<CloudDisk::AbsSharedResultSetMock>();

        EXPECT_CALL(*rdb, QueryByStep(_, columns, true)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
            .WillOnce(Return(E_OK))
            .WillOnce(Return(E_RDB));
        EXPECT_CALL(*resultSet, GetInt(_, _)).WillOnce(Return(E_OK));

        auto ret = dataSyncerRdbStore
            .GetSyncStateAndErrorType(userId, bundleName, cloudSyncState, errorType);
        
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest04 ERROR";
    }
    GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest04 End";
}

/**
 * @tc.name: GetSyncStateAndErrorTypeTest05
 * @tc.desc: Verify the GetSyncStateAndErrorType function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, GetSyncStateAndErrorTypeTest05, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest05 Start";
    try {
        int32_t userId = 1;
        string bundleName = "test.bundle.name";
        CloudSyncState cloudSyncState = CloudSyncState::COMPLETED;
        ErrorType errorType = ErrorType::NO_ERROR;

        auto rdb = make_shared<CloudDisk::RdbStoreMock>();
        DataSyncerRdbStore dataSyncerRdbStore;
        dataSyncerRdbStore.rdb_ = rdb;
        vector<string> columns;
        auto resultSet = make_shared<CloudDisk::AbsSharedResultSetMock>();

        EXPECT_CALL(*rdb, QueryByStep(_, columns, true)).WillOnce(Return(resultSet));
        EXPECT_CALL(*resultSet, GoToNextRow()).WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetColumnIndex(_, _))
            .WillOnce(Return(E_OK))
            .WillOnce(Return(E_OK));
        EXPECT_CALL(*resultSet, GetInt(_, _))
            .WillOnce(Return(E_OK))
            .WillOnce(Return(E_OK));

        auto ret = dataSyncerRdbStore
            .GetSyncStateAndErrorType(userId, bundleName, cloudSyncState, errorType);
        
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest05 ERROR";
    }
    GTEST_LOG_(INFO) << "GetSyncStateAndErrorTypeTest05 End";
}

/**
 * @tc.name: OnUpgradeTest01
 * @tc.desc: Verify the OnUpgrade function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, OnUpgradeTest01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnUpgradeTest01 Start";
    try {
        CloudDisk::RdbStoreMock store;
        DataSyncerRdbCallBack callback;
        int32_t oldVersion = 1;
        int32_t newVersion = 2;
        int32_t result = callback.OnUpgrade(store, oldVersion, newVersion);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnUpgradeTest01 ERROR";
    }
    GTEST_LOG_(INFO) << "OnUpgradeTest01 End";
}

/**
 * @tc.name: OnUpgradeTest02
 * @tc.desc: Verify the OnUpgrade function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, OnUpgradeTest02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnUpgradeTest02 Start";
    try {
        CloudDisk::RdbStoreMock store;
        DataSyncerRdbCallBack callback;
        int32_t oldVersion = 2;
        int32_t newVersion = 3;
        int32_t result = callback.OnUpgrade(store, oldVersion, newVersion);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnUpgradeTest02 ERROR";
    }
    GTEST_LOG_(INFO) << "OnUpgradeTest02 End";
}

/**
 * @tc.name: OnUpgradeTest03
 * @tc.desc: Verify the OnUpgrade function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, OnUpgradeTest03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnUpgradeTest03 Start";
    try {
        CloudDisk::RdbStoreMock store;
        DataSyncerRdbCallBack callback;
        int32_t oldVersion = 3;
        int32_t newVersion = 3;
        int32_t result = callback.OnUpgrade(store, oldVersion, newVersion);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnUpgradeTest03 ERROR";
    }
    GTEST_LOG_(INFO) << "OnUpgradeTest03 End";
}

/**
 * @tc.name: OnCreateTest01
 * @tc.desc: Verify the OnCreate function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, OnCreateTest01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateTest01 Start";
    try {
        CloudDisk::RdbStoreMock store;
        DataSyncerRdbCallBack callback;
        EXPECT_CALL(store, ExecuteSql(_, _)).WillOnce(Return(E_RDB));
        int32_t result = callback.OnCreate(store);
        EXPECT_EQ(result, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnCreateTest01 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateTest01 End";
}

/**
 * @tc.name: OnCreateTest02
 * @tc.desc: Verify the OnCreate function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, OnCreateTest02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnCreateTest02 Start";
    try {
        CloudDisk::RdbStoreMock store;
        DataSyncerRdbCallBack callback;
        EXPECT_CALL(store, ExecuteSql(_, _))
            .WillOnce(Return(E_OK))
            .WillOnce(Return(E_OK));
        int32_t result = callback.OnCreate(store);
        EXPECT_EQ(result, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OnCreateTest02 ERROR";
    }
    GTEST_LOG_(INFO) << "OnCreateTest02 End";
}

/**
 * @tc.name: UpdateSyncStateTest01
 * @tc.desc: Verify the UpdateSyncState function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, UpdateSyncStateTest01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateSyncStateTest01 Start";
    try {
        int32_t userId = 1;
        string bundleName = "bundleName";
        CloudSyncState cloudSyncState = CloudSyncState::COMPLETED;
        ErrorType errorType = ErrorType::NO_ERROR;

        auto rdb = make_shared<CloudDisk::RdbStoreMock>();
        DataSyncerRdbStore dataSyncerRdbStore;
        dataSyncerRdbStore.rdb_ = rdb;
        vector<string> whereArgs = {"1", "bundleName"};

        EXPECT_CALL(*rdb, Update(_, _, _, _, whereArgs)).WillOnce(Return(E_RDB));

        auto ret = dataSyncerRdbStore.UpdateSyncState(userId, bundleName,
            cloudSyncState, errorType);
        
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateSyncStateTest01 ERROR";
    }
    GTEST_LOG_(INFO) << "UpdateSyncStateTest01 End";
}

/**
 * @tc.name: UpdateSyncStateTest02
 * @tc.desc: Verify the UpdateSyncState function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, UpdateSyncStateTest02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateSyncStateTest02 Start";
    try {
        int32_t userId = 1;
        string bundleName = "bundleName";
        CloudSyncState cloudSyncState = CloudSyncState::COMPLETED;
        ErrorType errorType = ErrorType::NO_ERROR;

        auto rdb = make_shared<CloudDisk::RdbStoreMock>();
        DataSyncerRdbStore dataSyncerRdbStore;
        dataSyncerRdbStore.rdb_ = rdb;
        vector<string> whereArgs = {"1", "bundleName"};
        int rows = 0;

        EXPECT_CALL(*rdb, Update(rows, _, _, _, whereArgs))
            .WillOnce(DoAll(SetArgReferee<0>(1), Return(E_OK)));

        auto ret = dataSyncerRdbStore.UpdateSyncState(userId, bundleName,
            cloudSyncState, errorType);
        
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateSyncStateTest02 ERROR";
    }
    GTEST_LOG_(INFO) << "UpdateSyncStateTest02 End";
}

/**
 * @tc.name: UpdateSyncStateTest03
 * @tc.desc: Verify the UpdateSyncState function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreTest, UpdateSyncStateTest03, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateSyncStateTest03 Start";
    try {
        int32_t userId = 1;
        string bundleName = "bundleName";
        CloudSyncState cloudSyncState = CloudSyncState::COMPLETED;
        ErrorType errorType = ErrorType::NO_ERROR;

        auto rdb = make_shared<CloudDisk::RdbStoreMock>();
        DataSyncerRdbStore dataSyncerRdbStore;
        dataSyncerRdbStore.rdb_ = rdb;
        vector<string> whereArgs = {"1", "bundleName"};
        int rows = 0;

        EXPECT_CALL(*rdb, Update(rows, _, _, _, whereArgs))
            .WillOnce(DoAll(SetArgReferee<0>(0), Return(E_OK)));

        auto ret = dataSyncerRdbStore.UpdateSyncState(userId, bundleName,
            cloudSyncState, errorType);
        
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdateSyncStateTest03 ERROR";
    }
    GTEST_LOG_(INFO) << "UpdateSyncStateTest03 End";
}
}