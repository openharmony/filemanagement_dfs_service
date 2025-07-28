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

#include "data_syncer_rdb_store.cpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "rdb_assistant.h"
#include "result_set_mock.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class DataSyncerRdbStoreStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DataSyncerRdbStoreStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DataSyncerRdbStoreStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DataSyncerRdbStoreStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DataSyncerRdbStoreStaticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetIntTest_001
 * @tc.desc: Verify the GetInt function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreStaticTest, GetIntTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetIntTest_001 Start";
    try {
        CloudDisk::Test::ResultSetMock resultSetMock;
        string key = "keyword";
        int32_t val = 1;
        EXPECT_CALL(resultSetMock, GetColumnIndex(_, _))
            .WillOnce(Return(E_RDB));

        auto ret = GetInt(key, val, resultSetMock);

        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetIntTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetIntTest_001 End";
}

/**
 * @tc.name: GetIntTest_002
 * @tc.desc: Verify the GetInt function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreStaticTest, GetIntTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetIntTest_002 Start";
    try {
        CloudDisk::Test::ResultSetMock resultSetMock;
        string key = "keyword";
        int32_t val = 1;
        EXPECT_CALL(resultSetMock, GetColumnIndex(_, _))
            .WillOnce(Return(E_OK));
        EXPECT_CALL(resultSetMock, GetInt(_, _))
            .WillOnce(Return(E_RDB));

        auto ret = GetInt(key, val, resultSetMock);

        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetIntTest_002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetIntTest_002 End";
}

/**
 * @tc.name: GetIntTest_003
 * @tc.desc: Verify the GetInt function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreStaticTest, GetIntTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetIntTest_003 Start";
    try {
        CloudDisk::Test::ResultSetMock resultSetMock;
        string key = "keyword";
        int32_t val = 1;
        EXPECT_CALL(resultSetMock, GetColumnIndex(_, _))
            .WillOnce(Return(E_OK));
        EXPECT_CALL(resultSetMock, GetInt(_, _))
            .WillOnce(Return(E_OK));

        auto ret = GetInt(key, val, resultSetMock);

        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetIntTest_003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetIntTest_003 End";
}

/**
 * @tc.name: GetLongTest_001
 * @tc.desc: Verify the GetLong function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreStaticTest, GetLongTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLongTest_001 Start";
    try {
        CloudDisk::Test::ResultSetMock resultSetMock;
        string key = "keyword";
        int64_t val = 1;
        EXPECT_CALL(resultSetMock, GetColumnIndex(_, _))
            .WillOnce(Return(E_RDB));

        auto ret = GetLong(key, val, resultSetMock);

        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLongTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLongTest_001 End";
}

/**
 * @tc.name: GetLongTest_002
 * @tc.desc: Verify the GetLong function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreStaticTest, GetLongTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLongTest_002 Start";
    try {
        CloudDisk::Test::ResultSetMock resultSetMock;
        string key = "keyword";
        int64_t val = 1;
        EXPECT_CALL(resultSetMock, GetColumnIndex(_, _))
            .WillOnce(Return(E_OK));
        EXPECT_CALL(resultSetMock, GetLong(_, _))
            .WillOnce(Return(E_RDB));

        auto ret = GetLong(key, val, resultSetMock);

        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLongTest_002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLongTest_002 End";
}

/**
 * @tc.name: GetLongTest_003
 * @tc.desc: Verify the GetLong function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreStaticTest, GetLongTest_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetLongTest_003 Start";
    try {
        CloudDisk::Test::ResultSetMock resultSetMock;
        string key = "keyword";
        int64_t val = 1;
        EXPECT_CALL(resultSetMock, GetColumnIndex(_, _))
            .WillOnce(Return(E_OK));
        EXPECT_CALL(resultSetMock, GetLong(_, _))
            .WillOnce(Return(E_OK));

        auto ret = GetLong(key, val, resultSetMock);

        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetLongTest_003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetLongTest_003 End";
}

/**
 * @tc.name: VersionAddDataSyncerErrorTypeTest_001
 * @tc.desc: Verify the VersionAddDataSyncerErrorType function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreStaticTest,
    VersionAddDataSyncerErrorTypeTest_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddDataSyncerErrorTypeTest_001 Start";
    try {
        CloudDisk::RdbStoreMock store;

        EXPECT_CALL(store, ExecuteSql(_, _))
            .WillOnce(Return(E_OK))
            .WillOnce(Return(E_RDB));

        VersionAddDataSyncerErrorType(store);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddDataSyncerErrorTypeTest_001 ERROR";
    }
    GTEST_LOG_(INFO) << "VersionAddDataSyncerErrorTypeTest_001 End";
}

/**
 * @tc.name: VersionAddDataSyncerErrorTypeTest_002
 * @tc.desc: Verify the VersionAddDataSyncerErrorType function
 * @tc.type: FUNC
 * @tc.require: issuesICE88S
 */
HWTEST_F(DataSyncerRdbStoreStaticTest,
    VersionAddDataSyncerErrorTypeTest_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "VersionAddDataSyncerErrorTypeTest_002 Start";
    try {
        CloudDisk::RdbStoreMock store;

        EXPECT_CALL(store, ExecuteSql(_, _))
            .WillOnce(Return(E_RDB))
            .WillOnce(Return(E_OK));

        VersionAddDataSyncerErrorType(store);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "VersionAddDataSyncerErrorTypeTest_002 ERROR";
    }
    GTEST_LOG_(INFO) << "VersionAddDataSyncerErrorTypeTest_002 End";
}
}
