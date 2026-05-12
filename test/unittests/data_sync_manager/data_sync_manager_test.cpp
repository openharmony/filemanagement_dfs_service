/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "data_sync_manager.h"
#include "dfs_error.h"
#include "data_syncer_rdb_store.h"

namespace OHOS {
namespace FileManagement::CloudSync {
namespace Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
using namespace CloudFile;

class DataSyncManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<DataSyncManager> manager_ = nullptr;
    static inline std::shared_ptr<DataSyncerRdbStoreMock> rdbStoreMock_ = nullptr;
};

void DataSyncManagerTest::SetUpTestCase(void)
{
    std::cout << "SetUpTestCase" << std::endl;
    manager_ = make_shared<DataSyncManager>();
    rdbStoreMock_ = make_shared<DataSyncerRdbStoreMock>();
    DataSyncerRdbStoreMock::proxy_ = rdbStoreMock_;
}

void DataSyncManagerTest::TearDownTestCase(void)
{
    manager_ = nullptr;
    rdbStoreMock_ = nullptr;
    DataSyncerRdbStoreMock::proxy_ = nullptr;
    std::cout << "TearDownTestCase" << std::endl;
}

void DataSyncManagerTest::SetUp(void)
{
    std::cout << "SetUp" << std::endl;
}

void DataSyncManagerTest::TearDown(void)
{
    std::cout << "TearDown" << std::endl;
}

/**
 * @tc.name: StopDownloadAndUploadTaskTest001
 * @tc.desc: Verify the StopDownloadAndUploadTask function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, StopDownloadAndUploadTaskTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDownloadAndUploadTaskTest001 start";
    try {
        manager_->StopDownloadAndUploadTask();
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "StopDownloadAndUploadTaskTest001 failed";
    }
    GTEST_LOG_(INFO) << "StopDownloadAndUploadTaskTest001 end";
}

/**
 * @tc.name: IsFinishPullTest001
 * @tc.desc: Verify the ChangeAppSwitch function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, IsFinishPullTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsFinishPullTest001 start";
    try {
        bool finishFlag;

        int32_t ret = manager_->IsFinishPull(100, "", finishFlag);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "IsFinishPullTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsFinishPullTest001 end";
}

/**
 * @tc.name: GetDowngradeTaskStateTest001
 * @tc.desc: Verify the GetDowngradeDownloadTaskState function.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, GetDowngradeTaskStateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDowngradeTaskStateTest001 start";
    try {
        std::vector<std::string> bundleNames = {"com.ohos.photos", "com.ohos.gallery"};
        int32_t userId = 100;
        std::vector<CloudSync::DowngradeProgress> downgradeProgressList;
        int32_t ret = manager_->GetDowngradeDownloadTaskState(bundleNames, userId, downgradeProgressList);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "GetDowngradeTaskStateTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetDowngradeTaskStateTest001 end";
}

/**
 * @tc.name: StartTransferTest001
 * @tc.desc: Verify the StartTransfer function.
 * @tc.type: FUNC
 * @tc.require: issueTDD001
 */
HWTEST_F(DataSyncManagerTest, StartTransferTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartTransferTest001 start";
    try {
        std::string bundleName = "com.ohos.test";
        std::string targetUri = "file://docs/storage/Users/currentUser/test";
        sptr<CloudSync::IDowngradeDlCallback> callback = nullptr;
        int32_t ret = manager_->StartTransfer(bundleName, targetUri, callback);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "StartTransferTest001 failed";
    }
    GTEST_LOG_(INFO) << "StartTransferTest001 end";
}

/**
 * @tc.name: GetCachedTotalSizeTest001
 * @tc.desc: Verify the GetCachedTotalSize function returns E_OK with valid size.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, GetCachedTotalSizeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCachedTotalSizeTest001 start";
    try {
        std::string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        int64_t totalSize = 0;
        int64_t expectedSize = 1024;

        EXPECT_CALL(*rdbStoreMock_, GetTotalDownloadSize(userId, bundleName, _))
            .WillOnce(DoAll(SetArgReferee<2>(expectedSize), Return(E_OK)));

        int32_t ret = manager_->GetCachedTotalSize(bundleName, userId, totalSize);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(totalSize, expectedSize);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "GetCachedTotalSizeTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetCachedTotalSizeTest001 end";
}

/**
 * @tc.name: GetCachedTotalSizeTest002
 * @tc.desc: Verify the GetCachedTotalSize function handles E_RDB error correctly.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, GetCachedTotalSizeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCachedTotalSizeTest002 start";
    try {
        std::string bundleName = "com.ohos.photos";
        int32_t userId = 100;
        int64_t totalSize = 0;

        EXPECT_CALL(*rdbStoreMock_, GetTotalDownloadSize(userId, bundleName, _))
            .WillOnce(Return(E_RDB));

        int32_t ret = manager_->GetCachedTotalSize(bundleName, userId, totalSize);
        EXPECT_EQ(ret, E_RDB);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "GetCachedTotalSizeTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetCachedTotalSizeTest002 end";
}

/**
 * @tc.name: GetCachedTotalSizeTest003
 * @tc.desc: Verify the GetCachedTotalSize function passes parameters correctly.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, GetCachedTotalSizeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCachedTotalSizeTest003 start";
    try {
        std::string bundleName = "com.ohos.gallery";
        int32_t userId = 200;
        int64_t totalSize = 0;
        int64_t expectedSize = 2048;

        EXPECT_CALL(*rdbStoreMock_, GetTotalDownloadSize(userId, bundleName, _))
            .WillOnce(DoAll(SetArgReferee<2>(expectedSize), Return(E_OK)));

        int32_t ret = manager_->GetCachedTotalSize(bundleName, userId, totalSize);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(totalSize, expectedSize);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "GetCachedTotalSizeTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetCachedTotalSizeTest003 end";
}

/**
 * @tc.name: GetCachedTotalSizeTest004
 * @tc.desc: Verify the GetCachedTotalSize function with zero size.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, GetCachedTotalSizeTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCachedTotalSizeTest004 start";
    try {
        std::string bundleName = "com.ohos.docs";
        int32_t userId = 300;
        int64_t totalSize = 0;
        int64_t expectedSize = 0;

        EXPECT_CALL(*rdbStoreMock_, GetTotalDownloadSize(userId, bundleName, _))
            .WillOnce(DoAll(SetArgReferee<2>(expectedSize), Return(E_OK)));

        int32_t ret = manager_->GetCachedTotalSize(bundleName, userId, totalSize);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(totalSize, expectedSize);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "GetCachedTotalSizeTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetCachedTotalSizeTest004 end";
}

/**
 * @tc.name: GetCachedTotalSizeTest005
 * @tc.desc: Verify the GetCachedTotalSize function with large size.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, GetCachedTotalSizeTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCachedTotalSizeTest005 start";
    try {
        std::string bundleName = "com.ohos.videos";
        int32_t userId = 400;
        int64_t totalSize = 0;
        int64_t expectedSize = 10737418240; // 10GB

        EXPECT_CALL(*rdbStoreMock_, GetTotalDownloadSize(userId, bundleName, _))
            .WillOnce(DoAll(SetArgReferee<2>(expectedSize), Return(E_OK)));

        int32_t ret = manager_->GetCachedTotalSize(bundleName, userId, totalSize);
        EXPECT_EQ(ret, E_OK);
        EXPECT_EQ(totalSize, expectedSize);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "GetCachedTotalSizeTest005 failed";
    }
    GTEST_LOG_(INFO) << "GetCachedTotalSizeTest005 end";
}

/**
 * @tc.name: UpdateCachedFileSizeTest001
 * @tc.desc: Directory exists with files, calculate and update successfully
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, UpdateCachedFileSizeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdateCachedFileSizeTest001 start";
    try {
        std::string bundleName = "com.ohos.photos";
        int32_t userId = 100;

        int32_t ret = manager_->UpdateCachedFileSize(bundleName, userId);
        EXPECT_EQ(ret, E_OK);

    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "UpdateCachedFileSizeTest001 failed";
    }
    GTEST_LOG_(INFO) << "UpdateCachedFileSizeTest001 end";
}

/**
 * @tc.name: CleanCacheTest001
 * @tc.desc: DataSyncerRdbStore fails, return error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DataSyncManagerTest, CleanCacheTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CleanCacheTest001 start";
    try {
        std::string bundleName = "com.ohos.test006";
        int32_t userId = 100;
        int32_t ret = manager_->CleanCache(bundleName, userId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(true);
        GTEST_LOG_(INFO) << "CleanCacheTest001 failed";
    }
    GTEST_LOG_(INFO) << "CleanCacheTest001 end";
}
} // namespace Test
} // namespace FileManagement::CloudSync
} // namespace OHOS