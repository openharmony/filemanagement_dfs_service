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

#include "dk_container.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace DriveKit;

class DKContainerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DKContainerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DKContainerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void DKContainerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DKContainerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetPrivateDatabase
 * @tc.desc: Verify the DKContainer::GetPrivateDatabase function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKContainerTest, GetPrivateDatabase, TestSize.Level1)
{
    DKAppBundleName bundleName = "DKContainerTest";
    DKContainerName containerName = "GetPrivateDatabase";
    shared_ptr<DriveKitNative> driveKit = nullptr;
    DKContainer mDkContainer(bundleName, containerName, driveKit);
    shared_ptr<DKDatabase> databasePtr = mDkContainer.GetPrivateDatabase();
    EXPECT_EQ(databasePtr, nullptr);
}

/**
 * @tc.name: GetDatabaseWithdatabaseScope001
 * @tc.desc: Verify the DKContainer::GetDatabaseWithdatabaseScope function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKContainerTest, GetDatabaseWithdatabaseScope001, TestSize.Level1)
{
    DKAppBundleName bundleName = "DKContainerTest";
    DKContainerName containerName = "GetDatabaseWithdatabaseScope001";
    shared_ptr<DriveKitNative> driveKit = nullptr;
    DKDatabaseScope databaseScope = DKDatabaseScope::DK_PUBLIC_DATABASE;
    DKContainer mDkContainer(bundleName, containerName, driveKit);
    shared_ptr<DKDatabase> databasePtr = mDkContainer.GetDatabaseWithdatabaseScope(databaseScope);
    EXPECT_EQ(databasePtr, nullptr);
}

/**
 * @tc.name: GetDatabaseWithdatabaseScope002
 * @tc.desc: Verify the DKContainer::GetDatabaseWithdatabaseScope function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKContainerTest, GetDatabaseWithdatabaseScope002, TestSize.Level1)
{
    DKAppBundleName bundleName = "DKContainerTest";
    DKContainerName containerName = "GetDatabaseWithdatabaseScope002";
    shared_ptr<DriveKitNative> driveKit = nullptr;
    DKDatabaseScope databaseScope = DKDatabaseScope::DK_PRIVATE_DATABASE;
    DKContainer mDkContainer(bundleName, containerName, driveKit);
    shared_ptr<DKDatabase> databasePtr = mDkContainer.GetDatabaseWithdatabaseScope(databaseScope);
    EXPECT_EQ(databasePtr, nullptr);
}

/**
 * @tc.name: GetDatabaseWithdatabaseScope003
 * @tc.desc: Verify the DKContainer::GetDatabaseWithdatabaseScope function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKContainerTest, GetDatabaseWithdatabaseScope003, TestSize.Level1)
{
    DKAppBundleName bundleName = "DKContainerTest";
    DKContainerName containerName = "GetDatabaseWithdatabaseScope003";
    shared_ptr<DriveKitNative> driveKit = nullptr;
    DKDatabaseScope databaseScope = DKDatabaseScope::DK_SHARED_DATABASE;
    DKContainer mDkContainer(bundleName, containerName, driveKit);
    shared_ptr<DKDatabase> databasePtr = mDkContainer.GetDatabaseWithdatabaseScope(databaseScope);
    EXPECT_EQ(databasePtr, nullptr);
}

/**
 * @tc.name: SaveSubscription
 * @tc.desc: Verify the DKContainer::SaveSubscription function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKContainerTest, SaveSubscription, TestSize.Level1)
{
    DKAppBundleName bundleName = "DKContainerTest";
    DKContainerName containerName = "SaveSubscription";
    shared_ptr<DriveKitNative> driveKit = nullptr;
    shared_ptr<DKContext> contex = nullptr;
    DKSubscription subscription;
    SaveSubscriptionCallback callback = nullptr;
    DKContainer mDkContainer(bundleName, containerName, driveKit);
    DKLocalErrorCode ret = mDkContainer.SaveSubscription(contex, subscription, callback);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}

/**
 * @tc.name: DeleteSubscription
 * @tc.desc: Verify the DKContainer::DeleteSubscription function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DKContainerTest, DeleteSubscription, TestSize.Level1)
{
    DKAppBundleName bundleName = "DKContainerTest";
    DKContainerName containerName = "DeleteSubscription";
    shared_ptr<DriveKitNative> driveKit = nullptr;
    shared_ptr<DKContext> contex = nullptr;
    DKSubscriptionId id = "";
    DelSubscriptionCallback callback = nullptr;;
    DKContainer mDkContainer(bundleName, containerName, driveKit);
    DKLocalErrorCode ret = mDkContainer.DeleteSubscription(contex, id, callback);
    EXPECT_EQ(ret, DKLocalErrorCode::NO_ERROR);
}
} // namespace OHOS::FileManagement::CloudSync::Test
