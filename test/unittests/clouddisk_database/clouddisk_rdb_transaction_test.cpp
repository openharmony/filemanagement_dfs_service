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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "clouddisk_rdb_transaction.h"
#include "clouddisk_rdbstore_mock.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement::CloudDisk {
namespace Test {
using namespace testing::ext;
using namespace std;

constexpr int32_t E_HAS_DB_ERROR = -222;
constexpr int32_t E_OK = 0;

class CloudDiskRdbTransactionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskRdbTransactionTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
}

void CloudDiskRdbTransactionTest::TearDownTestCase(void)
{
    std::cout << "TearDownTestCase" << std::endl;
}

void CloudDiskRdbTransactionTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void CloudDiskRdbTransactionTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: StartTest001
 * @tc.desc: Verify the Start function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbTransactionTest, StartTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Start Start";
    auto rdb = nullptr;
    auto transaction = make_shared<TransactionOperations>(rdb);
    auto ret = transaction->Start();
    EXPECT_EQ(ret.first, E_HAS_DB_ERROR);
    GTEST_LOG_(INFO) << "Start End";
}

/**
 * @tc.name: StartTest002
 * @tc.desc: Verify the Start function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbTransactionTest, StartTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Start Start";
    auto rdb = std::make_shared<RdbStoreMock>();
    auto transaction = make_shared<TransactionOperations>(rdb);
    auto ret = transaction->Start();
    EXPECT_EQ(ret.first, E_OK);
    GTEST_LOG_(INFO) << "Start End";
}

/**
 * @tc.name: FinishTest001
 * @tc.desc: Verify the Finish function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbTransactionTest, FinishTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Finish Start";
    auto rdb = nullptr;
    auto transaction = make_shared<TransactionOperations>(rdb);
    transaction->Finish();
    GTEST_LOG_(INFO) << "Finish End";
}

/**
 * @tc.name: FinishTest002
 * @tc.desc: Verify the Finish function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbTransactionTest, FinishTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "Finish Start";
    auto rdb = std::make_shared<RdbStoreMock>();
    auto transaction = make_shared<TransactionOperations>(rdb);
    transaction->Finish();
    GTEST_LOG_(INFO) << "Finish End";
}

/**
 * @tc.name: BeginTransactionTest001
 * @tc.desc: Verify the BeginTransaction function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbTransactionTest, BeginTransactionTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BeginTransaction Start";
    auto rdb = nullptr;
    auto transaction = make_shared<TransactionOperations>(rdb);
    int32_t ret = transaction->BeginTransaction();
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
    GTEST_LOG_(INFO) << "BeginTransaction End";
}

/**
 * @tc.name: BeginTransactionTest002
 * @tc.desc: Verify the BeginTransaction function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbTransactionTest, BeginTransactionTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BeginTransaction Start";
    auto rdb = std::make_shared<RdbStoreMock>();
    auto transaction = make_shared<TransactionOperations>(rdb);
    int32_t ret = transaction->BeginTransaction();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "BeginTransaction End";
}

/**
 * @tc.name: TransactionCommitTest001
 * @tc.desc: Verify the TransactionCommit function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbTransactionTest, TransactionCommitTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransactionCommit Start";
    auto rdb = nullptr;
    auto transaction = make_shared<TransactionOperations>(rdb);
    int32_t ret = transaction->TransactionCommit();
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
    GTEST_LOG_(INFO) << "TransactionCommit End";
}

/**
 * @tc.name: TransactionCommitTest002
 * @tc.desc: Verify the TransactionCommit function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbTransactionTest, TransactionCommitTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransactionCommit Start";
    auto rdb = std::make_shared<RdbStoreMock>();
    auto transaction = make_shared<TransactionOperations>(rdb);
    int32_t ret = transaction->TransactionCommit();
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
    GTEST_LOG_(INFO) << "TransactionCommit End";
}

/**
 * @tc.name: TransactionRollbackTest001
 * @tc.desc: Verify the TransactionRollback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbTransactionTest, TransactionRollbackTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransactionRollback Start";
    auto rdb = nullptr;
    auto transaction = make_shared<TransactionOperations>(rdb);
    int32_t ret = transaction->TransactionRollback();
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
    GTEST_LOG_(INFO) << "TransactionRollback End";
}

/**
 * @tc.name: TransactionRollbackTest002
 * @tc.desc: Verify the TransactionRollback function.
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(CloudDiskRdbTransactionTest, TransactionRollbackTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "TransactionRollback Start";
    auto rdb = std::make_shared<RdbStoreMock>();
    auto transaction = make_shared<TransactionOperations>(rdb);
    int32_t ret = transaction->TransactionRollback();
    EXPECT_EQ(ret, E_HAS_DB_ERROR);
    GTEST_LOG_(INFO) << "TransactionRollback End";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS