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

#include <gtest/gtest.h>

#include "oh_cloud_disk_manager.h"

namespace OHOS {
namespace FileManagement::CloudDiskService {
namespace Test {

using namespace testing;
using namespace testing::ext;

class OhCloudDiskManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    static CloudDisk_SyncFolderPath CreateValidSyncFolderPath();
    
    static constexpr const char* testSyncFolderPath = "/data/test_sync";
    static constexpr size_t testSyncFolderLength = 15;
};

void OhCloudDiskManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void OhCloudDiskManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void OhCloudDiskManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void OhCloudDiskManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

CloudDisk_SyncFolderPath OhCloudDiskManagerTest::CreateValidSyncFolderPath()
{
    CloudDisk_SyncFolderPath path;
    path.value = const_cast<char*>(testSyncFolderPath);
    path.length = testSyncFolderLength;
    return path;
}

/**
 * @tc.name: GetSyncFolderChangesTest001
 * @tc.desc: Verify changesResult is nullptr returns CLOUD_DISK_INVALID_ARG
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, GetSyncFolderChangesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderChangesTest001 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
        uint64_t startUsn = 0;
        size_t count = 10;

        CloudDisk_ErrorCode ret = OH_CloudDisk_GetSyncFolderChanges(
            syncFolderPath, startUsn, count, nullptr);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "GetSyncFolderChangesTest001 end";
}

/**
 * @tc.name: GetSyncFolderChangesTest002
 * @tc.desc: Verify changesResult is not nullptr (parameter validation passed)
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, GetSyncFolderChangesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetSyncFolderChangesTest002 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
        uint64_t startUsn = 0;
        size_t count = 10;
        CloudDisk_ChangesResult* changesResult;

        CloudDisk_ErrorCode ret = OH_CloudDisk_GetSyncFolderChanges(
            syncFolderPath, startUsn, count, &changesResult);
        EXPECT_NE(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "GetSyncFolderChangesTest002 end";
}

/**
 * @tc.name: SetFileSyncStatesTest001
 * @tc.desc: Verify fileSyncStates is nullptr with bufferLength > 0 returns error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, SetFileSyncStatesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest001 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
        size_t bufferLength = 2;
        CloudDisk_FailedList* failedLists;
        size_t failedCount = 0;

        CloudDisk_ErrorCode ret = OH_CloudDisk_SetFileSyncStates(
            syncFolderPath, nullptr, bufferLength, &failedLists, &failedCount);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest001 end";
}

/**
 * @tc.name: SetFileSyncStatesTest002
 * @tc.desc: Verify failedLists is nullptr returns error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, SetFileSyncStatesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest002 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
        CloudDisk_FileSyncState states[1];
        states[0].filePathInfo.value = const_cast<char*>("/data/file1.txt");
        states[0].filePathInfo.length = 14;
        states[0].syncState = CloudDisk_SyncState::IDLE;
        size_t bufferLength = 1;
        size_t failedCount = 0;

        CloudDisk_ErrorCode ret = OH_CloudDisk_SetFileSyncStates(
            syncFolderPath, states, bufferLength, nullptr, &failedCount);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest002 end";
}

/**
 * @tc.name: SetFileSyncStatesTest003
 * @tc.desc: Verify failedCount is nullptr and failedLists is not nullptr returns error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, SetFileSyncStatesTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest003 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
        CloudDisk_FileSyncState states[1];
        states[0].filePathInfo.value = const_cast<char*>("/data/file1.txt");
        states[0].filePathInfo.length = 14;
        states[0].syncState = CloudDisk_SyncState::IDLE;
        size_t bufferLength = 1;
        CloudDisk_FailedList* failedLists;

        CloudDisk_ErrorCode ret = OH_CloudDisk_SetFileSyncStates(
            syncFolderPath, states, bufferLength, &failedLists, nullptr);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest003 end";
}

/**
 * @tc.name: SetFileSyncStatesTest004
 * @tc.desc: Verify fileSyncStates array element value is nullptr with valid failedLists and failedCount returns error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, SetFileSyncStatesTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest004 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
        CloudDisk_FileSyncState states[1];
        states[0].filePathInfo.value = nullptr;
        states[0].filePathInfo.length = 14;
        states[0].syncState = CloudDisk_SyncState::IDLE;
        size_t bufferLength = 1;
        CloudDisk_FailedList* failedLists;
        size_t failedCount;

        CloudDisk_ErrorCode ret = OH_CloudDisk_SetFileSyncStates(
            syncFolderPath, states, bufferLength, &failedLists, &failedCount);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest004 end";
}

/**
 * @tc.name: SetFileSyncStatesTest005
 * @tc.desc: Verify all parameters are valid and array element value is not nullptr
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, SetFileSyncStatesTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest005 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
        CloudDisk_FileSyncState states[1];
        states[0].filePathInfo.value = const_cast<char*>("/data/file1.txt");
        states[0].filePathInfo.length = 14;
        states[0].syncState = CloudDisk_SyncState::IDLE;
        size_t bufferLength = 1;
        CloudDisk_FailedList* failedLists;
        size_t failedCount;

        CloudDisk_ErrorCode ret = OH_CloudDisk_SetFileSyncStates(
            syncFolderPath, states, bufferLength, &failedLists, &failedCount);
        EXPECT_NE(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest005 end";
}

/**
 * @tc.name: GetFileSyncStatesTest001
 * @tc.desc: Verify paths is nullptr with bufferLength > 0 returns error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, GetFileSyncStatesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesTest001 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
        size_t bufferLength = 2;
        CloudDisk_ResultList* resultLists;
        size_t resultCount = 0;

        CloudDisk_ErrorCode ret = OH_CloudDisk_GetFileSyncStates(
            syncFolderPath, nullptr, bufferLength, &resultLists, &resultCount);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesTest001 end";
}

/**
 * @tc.name: GetFileSyncStatesTest002
 * @tc.desc: Verify resultLists is nullptr returns error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, GetFileSyncStatesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesTest002 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
        CloudDisk_PathInfo paths[1];
        paths[0].value = const_cast<char*>("/data/file1.txt");
        paths[0].length = 14;
        size_t bufferLength = 1;
        size_t resultCount = 0;

        CloudDisk_ErrorCode ret = OH_CloudDisk_GetFileSyncStates(
            syncFolderPath, paths, bufferLength, nullptr, &resultCount);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesTest002 end";
}

/**
 * @tc.name: GetFileSyncStatesTest003
 * @tc.desc: Verify resultCount is nullptr returns error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, GetFileSyncStatesTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesTest003 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
        CloudDisk_PathInfo paths[1];
        paths[0].value = const_cast<char*>("/data/file1.txt");
        paths[0].length = 14;
        size_t bufferLength = 1;
        CloudDisk_ResultList* resultLists;

        CloudDisk_ErrorCode ret = OH_CloudDisk_GetFileSyncStates(
            syncFolderPath, paths, bufferLength, &resultLists, nullptr);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesTest003 end";
}

/**
 * @tc.name: GetFileSyncStatesTest004
 * @tc.desc: Verify all parameters are valid (parameter validation passed)
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, GetFileSyncStatesTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStatesTest004 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
        CloudDisk_PathInfo paths[1];
        paths[0].value = const_cast<char*>("/data/file1.txt");
        paths[0].length = 14;
        size_t bufferLength = 1;
        CloudDisk_ResultList* resultLists;
        size_t resultCount;

        CloudDisk_ErrorCode ret = OH_CloudDisk_GetFileSyncStates(
            syncFolderPath, paths, bufferLength, &resultLists, &resultCount);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
    }
    GTEST_LOG_(INFO) << "GetFileSyncStatesTest004 end";
}

} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS