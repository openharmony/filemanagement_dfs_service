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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

#include "dk_database.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace DriveKit;

class DKDatabaseTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DKDatabaseTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DKDatabaseTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DKDatabaseTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DKDatabaseTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: SaveRecord
 * @tc.desc: Verify the DKDatabase::SaveRecord function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKDatabaseTest, SaveRecord, TestSize.Level1)
{
    std::shared_ptr<DKContainer> container = nullptr;
    std::shared_ptr<DKContext> context = nullptr;
    DKRecord record;
    DKDatabase::SaveRecordCallback callback = nullptr;
    DKSavePolicy policy = DKSavePolicy::DK_SAVE_IF_UNCHANGED;
    DKDatabaseScope scope = DKDatabaseScope::DK_PUBLIC_DATABASE;

    DKDatabase mdkDatabase(container, scope);
    DKLocalErrorCode ret = mdkDatabase.SaveRecord(context, move(record), policy, callback);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: FetchRecordWithId
 * @tc.desc: Verify the DKDatabase::FetchRecordWithId function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKDatabaseTest, FetchRecordWithId, TestSize.Level1)
{
    std::shared_ptr<DKContainer> container = nullptr;
    std::shared_ptr<DKContext> context =  nullptr;
    DKDatabase::FetchRecordCallback callback = nullptr;
    DKRecordType recordType = "DKDatabaseTest";
    std::vector<DKFieldKey> desiredKeys;
    DKRecordId recordId = "FetchRecordWithId";
    DKDatabaseScope scope = DKDatabaseScope::DK_PUBLIC_DATABASE;

    DKDatabase mdkDatabase(container, scope);
    DKLocalErrorCode ret = mdkDatabase.FetchRecordWithId(context, recordType,
                                                        recordId, desiredKeys, callback);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: FetchRecordsWithQuery
 * @tc.desc: Verify the DKDatabase::FetchRecordsWithQuery function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKDatabaseTest, FetchRecordsWithQuery, TestSize.Level1)
{
    std::shared_ptr<DKContainer> container = nullptr;
    std::shared_ptr<DKContext> context =  nullptr;
    DKDatabase::FetchRecordsCallback callback = nullptr;
    std::vector<DKFieldKey> desiredKeys;
    DKQuery query;
    int resultLimit = 1;
    DKQueryCursor cursor = "FetchRecordsWithQuery";
    DKDatabaseScope scope = DKDatabaseScope::DK_PUBLIC_DATABASE;

    DKDatabase mdkDatabase(container, scope);
    DKLocalErrorCode ret = mdkDatabase.FetchRecordsWithQuery(context, desiredKeys,
                                                            query, resultLimit, cursor, callback);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetStartCursor
 * @tc.desc: Verify the DKDatabase::GetStartCursor function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKDatabaseTest, GetStartCursor, TestSize.Level1)
{
    std::shared_ptr<DKContainer> container = nullptr;
    DKRecordType recordType = "DKDatabaseTest";
    DKQueryCursor cursor = "FetchDatabaseChanges";
    DKDatabaseScope scope = DKDatabaseScope::DK_PUBLIC_DATABASE;

    DKDatabase mdkDatabase(container, scope);
    DKError ret = mdkDatabase.GetStartCursor(recordType, cursor);
    EXPECT_EQ(ret.dkErrorCode, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GenerateIds
 * @tc.desc: Verify the DKDatabase::GenerateIds function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKDatabaseTest, GenerateIds, TestSize.Level1)
{
    std::shared_ptr<DKContainer> container = nullptr;
    DKDatabaseScope scope = DKDatabaseScope::DK_PUBLIC_DATABASE;
    int count = 0;
    std::vector<DKRecordId> ids;

    DKDatabase mdkDatabase(container, scope);
    DKError ret = mdkDatabase.GenerateIds(count, ids);
    EXPECT_EQ(ret.dkErrorCode, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: GetLock
 * @tc.desc: Verify the DKDatabase::GetLock function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKDatabaseTest, GetLock, TestSize.Level1)
{
    std::shared_ptr<DKContainer> container = nullptr;
    DKDatabaseScope scope = DKDatabaseScope::DK_PUBLIC_DATABASE;
    DKLock lock;

    DKDatabase mdkDatabase(container, scope);
    DKError ret = mdkDatabase.GetLock(lock);
    EXPECT_EQ(ret.dkErrorCode, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: NewAssetReadSession
 * @tc.desc: Verify the DKDatabase::NewAssetReadSession function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKDatabaseTest, NewAssetReadSession, TestSize.Level1)
{
    std::shared_ptr<DKContainer> container = nullptr;
    DKDatabaseScope scope = DKDatabaseScope::DK_PUBLIC_DATABASE;
    DKRecordType recordType = "";
    DKRecordId recordId = "";
    DKFieldKey assetKey = "";
    DKAssetPath assetPath = "";

    DKDatabase mdkDatabase(container, scope);
    EXPECT_NE(nullptr, mdkDatabase.NewAssetReadSession(recordType, recordId, assetKey, assetPath));
}
} // namespace OHOS::FileManagement::CloudSync::Test
