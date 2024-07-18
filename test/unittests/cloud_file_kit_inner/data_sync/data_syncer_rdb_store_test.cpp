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

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class MockDataSyncerRdbStore : public DataSyncerRdbStore {
public:
    MOCK_METHOD2(Insert, int32_t(int32_t userId, const string &bundleName));
    MOCK_METHOD3(UpdateSyncState, int32_t(int32_t userId, const string &bundleName, SyncState syncState));
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
        EXPECT_CALL(mockDataSyncerRdbStore, UpdateSyncState(userId, bundleName, SyncState::INIT))
            .WillOnce(Return(E_OK));
        EXPECT_CALL(mockDataSyncerRdbStore, UpdateSyncState(userId, bundleName, SyncState::SYNC_SUCCEED))
            .WillOnce(Return(E_OK));

        EXPECT_EQ(mockDataSyncerRdbStore.UpdateSyncState(userId, bundleName, SyncState::INIT), E_OK);
        EXPECT_EQ(mockDataSyncerRdbStore.UpdateSyncState(userId, bundleName, SyncState::SYNC_SUCCEED), E_OK);
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
}