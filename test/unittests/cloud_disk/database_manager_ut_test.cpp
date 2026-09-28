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

#include "database_manager.h"
#include "database_manager_mock.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace OHOS::FileManagement::CloudSync;
using namespace OHOS::NativeRdb;

class DatabaseManagerUtTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<DatabaseManager> databaseManager_ = nullptr;
};

void DatabaseManagerUtTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DatabaseManagerUtTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DatabaseManagerUtTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    databaseManager_ = make_shared<DatabaseManager>();
}

void DatabaseManagerUtTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    DataSyncerRdbStoreMock::proxy_ = nullptr;
    databaseManager_ = nullptr;
}

/**
 * @tc.name: IsBundleCloudSyncEnabledTest001
 * @tc.desc: Verify cloud-enabled bundle (rowCount >= 1) returns true
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerUtTest, IsBundleCloudSyncEnabledTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest001 start";
    try {
        DataSyncerRdbStoreMock::proxy_ = make_shared<DataSyncerRdbStoreMock>();
        auto rset = make_shared<ResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(1), Return(E_OK)));
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(rset), Return(E_OK)));
        bool result = databaseManager_->IsBundleCloudSyncEnabled(100, "com.test.cloud");
        EXPECT_TRUE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest001 end";
}

/**
 * @tc.name: IsBundleCloudSyncEnabledTest002
 * @tc.desc: Verify non-cloud bundle (rowCount == 0) returns false and is cached; second call hits negative cache
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerUtTest, IsBundleCloudSyncEnabledTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest002 start";
    try {
        DataSyncerRdbStoreMock::proxy_ = make_shared<DataSyncerRdbStoreMock>();
        auto rset = make_shared<ResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(0), Return(E_OK)));
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(rset), Return(E_OK)));
        bool result1 = databaseManager_->IsBundleCloudSyncEnabled(100, "com.test.noncloud");
        EXPECT_FALSE(result1);
        bool result2 = databaseManager_->IsBundleCloudSyncEnabled(100, "com.test.noncloud");
        EXPECT_FALSE(result2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest002 failed";
    }
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest002 end";
}

/**
 * @tc.name: IsBundleCloudSyncEnabledTest003
 * @tc.desc: Verify bundle already in rdbMap_ returns true without query
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerUtTest, IsBundleCloudSyncEnabledTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest003 start";
    try {
        databaseManager_->GetRdbStore("com.test.inrdbmap", 100);
        bool result = databaseManager_->IsBundleCloudSyncEnabled(100, "com.test.inrdbmap");
        EXPECT_TRUE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest003 failed";
    }
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest003 end";
}

/**
 * @tc.name: IsBundleCloudSyncEnabledTest004
 * @tc.desc: Verify QueryCloudSync failure (ret != E_OK) returns false
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerUtTest, IsBundleCloudSyncEnabledTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest004 start";
    try {
        DataSyncerRdbStoreMock::proxy_ = make_shared<DataSyncerRdbStoreMock>();
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _))
            .WillOnce(Return(E_INVAL_ARG));
        bool result = databaseManager_->IsBundleCloudSyncEnabled(100, "com.test.queryfail");
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest004 failed";
    }
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest004 end";
}

/**
 * @tc.name: IsBundleCloudSyncEnabledTest005
 * @tc.desc: Verify null resultSet (ret == E_OK) returns false
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerUtTest, IsBundleCloudSyncEnabledTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest005 start";
    try {
        DataSyncerRdbStoreMock::proxy_ = make_shared<DataSyncerRdbStoreMock>();
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(std::shared_ptr<ResultSet>()), Return(E_OK)));
        bool result = databaseManager_->IsBundleCloudSyncEnabled(100, "com.test.nullresult");
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest005 failed";
    }
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest005 end";
}

/**
 * @tc.name: IsBundleCloudSyncEnabledTest006
 * @tc.desc: Verify GetRowCount failure returns false without caching
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerUtTest, IsBundleCloudSyncEnabledTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest006 start";
    try {
        DataSyncerRdbStoreMock::proxy_ = make_shared<DataSyncerRdbStoreMock>();
        auto rset = make_shared<ResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(Return(E_INVAL_ARG));
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(rset), Return(E_OK)));
        bool result = databaseManager_->IsBundleCloudSyncEnabled(100, "com.test.rowcountfail");
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest006 failed";
    }
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest006 end";
}

/**
 * @tc.name: IsBundleCloudSyncEnabledTest007
 * @tc.desc: Verify negative rowCount returns false without caching
 * @tc.type: FUNC
 */
HWTEST_F(DatabaseManagerUtTest, IsBundleCloudSyncEnabledTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest007 start";
    try {
        DataSyncerRdbStoreMock::proxy_ = make_shared<DataSyncerRdbStoreMock>();
        auto rset = make_shared<ResultSetMock>();
        EXPECT_CALL(*rset, GetRowCount(_)).WillOnce(DoAll(SetArgReferee<0>(-1), Return(E_OK)));
        EXPECT_CALL(*DataSyncerRdbStoreMock::proxy_, QueryCloudSync(_, _, _))
            .WillOnce(DoAll(SetArgReferee<2>(rset), Return(E_OK)));
        bool result = databaseManager_->IsBundleCloudSyncEnabled(100, "com.test.negativerowcount");
        EXPECT_FALSE(result);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest007 failed";
    }
    GTEST_LOG_(INFO) << "IsBundleCloudSyncEnabledTest007 end";
}
} // namespace OHOS::FileManagement::CloudDisk::Test
