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

#include <cstdint>
#include <string>
#include <vector>

#include "oh_cloud_disk_manager.h"
#include "oh_cloud_disk_utils.h"
#include "cloud_disk_service_manager_mock.h"

namespace OHOS {
namespace FileManagement::CloudDiskService {
namespace Test {

using namespace testing;
using namespace testing::ext;

namespace {
void PlaceholderCallback(const OH_CloudDisk_CallbackReqHead, OH_CloudDisk_CallbackContext) {}
} // namespace

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
 * @tc.name: RegisterCallbackTableTest001
 * @tc.desc: Verify callback table registration validates the callback and maps duplicate errors.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, RegisterCallbackTableTest001, TestSize.Level1)
{
    CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_EQ(OH_CloudDisk_RegisterCallbackTable(syncFolderPath, nullptr), CLOUD_DISK_INVALID_ARG);
    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), RegisterCallbackTable(testSyncFolderPath, _))
        .WillOnce(Return(E_CALLBACK_ALREADY_REGISTERED));
    EXPECT_EQ(OH_CloudDisk_RegisterCallbackTable(syncFolderPath, PlaceholderCallback),
              OH_CLOUD_DISK_CALLBACK_ALREADY_REGISTERED);
#else
    EXPECT_EQ(OH_CloudDisk_RegisterCallbackTable(syncFolderPath, PlaceholderCallback), CLOUD_DISK_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: UnregisterCallbackTableTest001
 * @tc.desc: Verify callback table unregistration maps the callback-specific not-registered error.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, UnregisterCallbackTableTest001, TestSize.Level1)
{
    CloudDisk_SyncFolderPath syncFolderPath = CreateValidSyncFolderPath();
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), UnregisterCallbackTable(testSyncFolderPath))
        .WillOnce(Return(E_CALLBACK_NOT_REGISTERED));
    EXPECT_EQ(OH_CloudDisk_UnregisterCallbackTable(syncFolderPath), OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED);
#else
    EXPECT_EQ(OH_CloudDisk_UnregisterCallbackTable(syncFolderPath), CLOUD_DISK_NOT_SUPPORTED);
#endif
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

/**
 * @tc.name: ConvertPlaceholderToFile_InvalidPath_001
 * @tc.desc: Verify OH_CloudDisk_ConvertPlaceholderToFile with invalid path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, ConvertPlaceholderToFile_InvalidPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderToFile_InvalidPath_001 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath;
        syncFolderPath.value = nullptr;
        syncFolderPath.length = 10;

        CloudDisk_PathInfo pathInfo;
        pathInfo.value = const_cast<char*>("/storage/Users/currentUser/testdir/file.txt");
        pathInfo.length = strlen(pathInfo.value);

        CloudDisk_ErrorCode ret = OH_CloudDisk_ConvertPlaceholderToFile(syncFolderPath, pathInfo);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
#else
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
#endif
        syncFolderPath.value = const_cast<char*>("/storage/Users/currentUser/testdir");
        syncFolderPath.length = 0;
        ret = OH_CloudDisk_ConvertPlaceholderToFile(syncFolderPath, pathInfo);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
#else
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertPlaceholderToFile_InvalidPath_001 failed";
    }
    GTEST_LOG_(INFO) << "ConvertPlaceholderToFile_InvalidPath_001 end";
}

/**
 * @tc.name: ConvertPlaceholderToFile_Success_002
 * @tc.desc: Verify OH_CloudDisk_ConvertPlaceholderToFile with valid parameters
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, ConvertPlaceholderToFile_Success_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderToFile_Success_002 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath;
        syncFolderPath.value = const_cast<char*>("/storage/Users/currentUser/testdir");
        syncFolderPath.length = strlen(syncFolderPath.value);

        CloudDisk_PathInfo pathInfo;
        pathInfo.value = const_cast<char*>("/storage/Users/currentUser/testdir/file.txt");
        pathInfo.length = strlen(pathInfo.value);

        EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(),
            ConvertPlaceholderToFile(_, _)).WillOnce(Return(
                OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_OK));
        CloudDisk_ErrorCode ret = OH_CloudDisk_ConvertPlaceholderToFile(syncFolderPath, pathInfo);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
#else
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertPlaceholderToFile_Success_002 failed";
    }
    GTEST_LOG_(INFO) << "ConvertPlaceholderToFile_Success_002 end";
}

/**
 * @tc.name: ConvertPlaceholderToFile_ErrorCode_003
 * @tc.desc: Verify OH_CloudDisk_ConvertPlaceholderToFile error code conversion
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, ConvertPlaceholderToFile_ErrorCode_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderToFile_ErrorCode_003 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath;
        syncFolderPath.value = const_cast<char*>("/storage/Users/currentUser/testdir");
        syncFolderPath.length = strlen(syncFolderPath.value);

        CloudDisk_PathInfo pathInfo;
        pathInfo.value = const_cast<char*>("/storage/Users/currentUser/testdir/file.txt");
        pathInfo.length = strlen(pathInfo.value);

        EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(),
            ConvertPlaceholderToFile(_, _)).WillOnce(Return(
                OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_NOT_A_PLACEHOLDER));
        CloudDisk_ErrorCode ret = OH_CloudDisk_ConvertPlaceholderToFile(syncFolderPath, pathInfo);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, CloudDisk_ErrorCode::OH_CLOUD_DISK_NOT_A_PLACEHOLDER);
#else
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertPlaceholderToFile_ErrorCode_003 failed";
    }
    GTEST_LOG_(INFO) << "ConvertPlaceholderToFile_ErrorCode_003 end";
}

/**
 * @tc.name: UpdatePlaceholder_InvalidPath_001
 * @tc.desc: Verify OH_CloudDisk_UpdatePlaceholder with invalid path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, UpdatePlaceholder_InvalidPath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdatePlaceholder_InvalidPath_001 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath;
        syncFolderPath.value = nullptr;
        syncFolderPath.length = 10;

        CloudDisk_PathInfo pathInfo;
        pathInfo.value = const_cast<char*>("/storage/Users/currentUser/testdir/file.txt");
        pathInfo.length = strlen(pathInfo.value);

        OH_CloudDisk_PlaceholderInfo metaData;
        metaData.logicalSize = 1024;
        metaData.mtimeMs = 1234567890;
        metaData.atimeMs = 1234567890;

        CloudDisk_ErrorCode ret = OH_CloudDisk_UpdatePlaceholder(syncFolderPath, pathInfo, metaData, nullptr);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
#else
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
#endif

        syncFolderPath.value = const_cast<char*>("/storage/Users/currentUser/testdir");
        syncFolderPath.length = 0;
        ret = OH_CloudDisk_UpdatePlaceholder(syncFolderPath, pathInfo, metaData, nullptr);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
#else
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdatePlaceholder_InvalidPath_001 failed";
    }
    GTEST_LOG_(INFO) << "UpdatePlaceholder_InvalidPath_001 end";
}

/**
 * @tc.name: UpdatePlaceholder_Test_002
 * @tc.desc: Verify OH_CloudDisk_UpdatePlaceholder success scenario
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, UpdatePlaceholder_Test_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdatePlaceholder_Test_002 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath;
        syncFolderPath.value = const_cast<char*>("/storage/Users/currentUser/testdir");
        syncFolderPath.length = strlen(syncFolderPath.value);

        CloudDisk_PathInfo pathInfo;
        pathInfo.value = const_cast<char*>("/storage/Users/currentUser/testdir/file.txt");
        pathInfo.length = strlen(pathInfo.value);

        OH_CloudDisk_PlaceholderInfo metaData;
        metaData.logicalSize = 1024;
        metaData.mtimeMs = 1234567890;
        metaData.atimeMs = 1234567890;

        EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), UpdatePlaceholder(_, _, _, _)).WillOnce(
            Return(OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_OK));
        CloudDisk_ErrorCode ret = OH_CloudDisk_UpdatePlaceholder(syncFolderPath, pathInfo, metaData, nullptr);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
#else
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdatePlaceholder_Test_002 failed";
    }
    GTEST_LOG_(INFO) << "UpdatePlaceholder_Test_002 end";
}

/**
 * @tc.name: UpdatePlaceholder_Test_003
 * @tc.desc: Verify OH_CloudDisk_UpdatePlaceholder permission denied scenario
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, UpdatePlaceholder_Test_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UpdatePlaceholder_Test_003 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath;
        syncFolderPath.value = const_cast<char*>("/storage/Users/currentUser/testdir");
        syncFolderPath.length = strlen(syncFolderPath.value);

        CloudDisk_PathInfo pathInfo;
        pathInfo.value = const_cast<char*>("/storage/Users/currentUser/testdir/file.txt");
        pathInfo.length = strlen(pathInfo.value);

        OH_CloudDisk_PlaceholderInfo metaData;
        metaData.logicalSize = 1024;
        metaData.mtimeMs = 1234567890;
        metaData.atimeMs = 1234567890;

        EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), UpdatePlaceholder(_, _, _, _)).WillOnce(
            Return(OHOS::FileManagement::CloudDiskService::CloudDiskServiceErrCode::E_PERMISSION_DENIED));
        CloudDisk_ErrorCode ret = OH_CloudDisk_UpdatePlaceholder(syncFolderPath, pathInfo, metaData, nullptr);
#ifdef SUPPORT_CLOUD_DISK_SERVICE
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED);
#else
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
#endif
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UpdatePlaceholder_Test_003 failed";
    }
    GTEST_LOG_(INFO) << "UpdatePlaceholder_Test_003 end";
}

/**
 * @tc.name: PlaceholderCustomInfo_Create_001
 * @tc.desc: Verify create forwards opaque custom information
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, PlaceholderCustomInfo_Create_001, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/testdir";
    std::string relativePath = "file.txt";
    CloudDisk_SyncFolderPath syncFolderPath = {const_cast<char *>(syncFolder.c_str()), syncFolder.size()};
    CloudDisk_PathInfo pathInfo = {const_cast<char *>(relativePath.c_str()), relativePath.size()};
    OH_CloudDisk_PlaceholderInfo metaData = {1024, 1, 2};
    std::vector<uint8_t> data = {0x00, 0x7F, 0xFF};
    OH_CloudDisk_PlaceholderCustomInfo customInfo = {data.size(), data.data()};

    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), CreatePlaceholderFile(_, _, _, _))
        .WillOnce(Invoke([&data](const std::string &, const std::string &, const PlaceholderInfo &,
                                const PlaceholderCustomInfo &innerCustomInfo) {
            EXPECT_EQ(innerCustomInfo.data, data);
            return E_OK;
        }));
    EXPECT_EQ(OH_CloudDisk_CreatePlaceholder(syncFolderPath, pathInfo, metaData, &customInfo),
              CloudDisk_ErrorCode::CLOUD_DISK_OK);
}

/**
 * @tc.name: PlaceholderCustomInfo_Create_002
 * @tc.desc: Verify create rejects oversized custom information before calling the manager
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, PlaceholderCustomInfo_Create_002, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/testdir";
    std::string relativePath = "file.txt";
    CloudDisk_SyncFolderPath syncFolderPath = {const_cast<char *>(syncFolder.c_str()), syncFolder.size()};
    CloudDisk_PathInfo pathInfo = {const_cast<char *>(relativePath.c_str()), relativePath.size()};
    OH_CloudDisk_PlaceholderInfo metaData = {1024, 1, 2};
    std::vector<uint8_t> data(4097, 1);
    OH_CloudDisk_PlaceholderCustomInfo customInfo = {data.size(), data.data()};

    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), CreatePlaceholderFile(_, _, _, _)).Times(0);
    EXPECT_EQ(OH_CloudDisk_CreatePlaceholder(syncFolderPath, pathInfo, metaData, &customInfo),
              CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
}

#ifdef SUPPORT_CLOUD_DISK_SERVICE
/**
 * @tc.name: PlaceholderCustomInfo_Update_001
 * @tc.desc: Verify update forwards non-empty and empty custom information
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, PlaceholderCustomInfo_Update_001, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/testdir";
    std::string relativePath = "file.txt";
    CloudDisk_SyncFolderPath syncFolderPath = {const_cast<char *>(syncFolder.c_str()), syncFolder.size()};
    CloudDisk_PathInfo pathInfo = {const_cast<char *>(relativePath.c_str()), relativePath.size()};
    OH_CloudDisk_PlaceholderInfo metaData = {1024, 1, 2};
    std::vector<uint8_t> data = {1, 2, 3};
    OH_CloudDisk_PlaceholderCustomInfo customInfo = {data.size(), data.data()};

    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), UpdatePlaceholder(_, _, _, _))
        .WillOnce(Invoke([&data](const std::string &, const std::string &, const PlaceholderInfo &,
                                const PlaceholderCustomInfo &innerCustomInfo) {
            EXPECT_EQ(innerCustomInfo.data, data);
            return E_OK;
        }));
    EXPECT_EQ(OH_CloudDisk_UpdatePlaceholder(syncFolderPath, pathInfo, metaData, &customInfo),
              CloudDisk_ErrorCode::CLOUD_DISK_OK);

    Mock::VerifyAndClearExpectations(&CloudDiskServiceManagerMock::GetInstance());
    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), UpdatePlaceholder(_, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::string &, const PlaceholderInfo &,
                            const PlaceholderCustomInfo &innerCustomInfo) {
            EXPECT_TRUE(innerCustomInfo.data.empty());
            return E_OK;
        }));
    EXPECT_EQ(OH_CloudDisk_UpdatePlaceholder(syncFolderPath, pathInfo, metaData, nullptr),
              CloudDisk_ErrorCode::CLOUD_DISK_OK);
}

/**
 * @tc.name: PlaceholderCustomInfo_Get_001
 * @tc.desc: Verify get copies data and reports required capacity without partial copy
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, PlaceholderCustomInfo_Get_001, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/testdir";
    std::string relativePath = "file.txt";
    CloudDisk_SyncFolderPath syncFolderPath = {const_cast<char *>(syncFolder.c_str()), syncFolder.size()};
    CloudDisk_PathInfo pathInfo = {const_cast<char *>(relativePath.c_str()), relativePath.size()};
    PlaceholderCustomInfo innerCustomInfo;
    innerCustomInfo.data = {1, 2, 3};
    uint8_t dataBuf[3] = {9, 9, 9};
    size_t dataLength = 2;

    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), GetPlaceholderCustomInfo(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(innerCustomInfo), Return(E_OK)));
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(syncFolderPath, pathInfo, dataBuf, &dataLength),
              CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(dataLength, innerCustomInfo.data.size());
    EXPECT_THAT(dataBuf, ElementsAre(9, 9, 9));

    Mock::VerifyAndClearExpectations(&CloudDiskServiceManagerMock::GetInstance());
    dataLength = sizeof(dataBuf);
    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), GetPlaceholderCustomInfo(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(innerCustomInfo), Return(E_OK)));
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(syncFolderPath, pathInfo, dataBuf, &dataLength),
              CloudDisk_ErrorCode::CLOUD_DISK_OK);
    EXPECT_EQ(dataLength, innerCustomInfo.data.size());
    EXPECT_THAT(dataBuf, ElementsAre(1, 2, 3));
}

/**
 * @tc.name: PlaceholderCustomInfo_Get_002
 * @tc.desc: Verify get maps placeholder custom information errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, PlaceholderCustomInfo_Get_002, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/testdir";
    std::string relativePath = "file.txt";
    CloudDisk_SyncFolderPath syncFolderPath = {const_cast<char *>(syncFolder.c_str()), syncFolder.size()};
    CloudDisk_PathInfo pathInfo = {const_cast<char *>(relativePath.c_str()), relativePath.size()};
    uint8_t dataBuf[1] = {};
    size_t dataLength = sizeof(dataBuf);

    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), GetPlaceholderCustomInfo(_, _, _))
        .WillOnce(Return(E_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND));
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(syncFolderPath, pathInfo, dataBuf, &dataLength),
              CloudDisk_ErrorCode::OH_CLOUD_DISK_PLACEHOLDER_CUSTOM_INFO_NOT_FOUND);
    EXPECT_EQ(dataLength, 0);

    Mock::VerifyAndClearExpectations(&CloudDiskServiceManagerMock::GetInstance());
    dataLength = sizeof(dataBuf);
    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), GetPlaceholderCustomInfo(_, _, _))
        .WillOnce(Return(E_NOT_A_PLACEHOLDER));
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(syncFolderPath, pathInfo, dataBuf, &dataLength),
              CloudDisk_ErrorCode::OH_CLOUD_DISK_NOT_A_PLACEHOLDER);

    Mock::VerifyAndClearExpectations(&CloudDiskServiceManagerMock::GetInstance());
    dataLength = sizeof(dataBuf);
    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), GetPlaceholderCustomInfo(_, _, _))
        .WillOnce(Return(E_FILE_NOT_EXIST));
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(syncFolderPath, pathInfo, dataBuf, &dataLength),
              CloudDisk_ErrorCode::OH_CLOUD_DISK_FILE_NOT_EXIST);

    Mock::VerifyAndClearExpectations(&CloudDiskServiceManagerMock::GetInstance());
    dataLength = sizeof(dataBuf);
    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), GetPlaceholderCustomInfo(_, _, _))
        .WillOnce(Return(E_SYNC_FOLDER_PATH_UNAUTHORIZED));
    EXPECT_EQ(OH_CloudDisk_GetPlaceholderCustomInfo(syncFolderPath, pathInfo, dataBuf, &dataLength),
              CloudDisk_ErrorCode::CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED);
}
#endif


/**
 * @tc.name: PlaceholderStateOnly_InvalidArgs_001
 * @tc.desc: Verify state-only conversion validates paths
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, PlaceholderStateOnly_InvalidArgs_001, TestSize.Level1)
{
    CloudDisk_SyncFolderPath invalidSyncFolder = {nullptr, 0};
    std::string relativePath = "file.txt";
    CloudDisk_PathInfo pathInfo = {const_cast<char *>(relativePath.c_str()), relativePath.size()};

#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_EQ(OH_CloudDisk_MarkFileAsPlaceholder(invalidSyncFolder, pathInfo),
              CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_UnmarkPlaceholderFile(invalidSyncFolder, pathInfo),
              CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
#else
    EXPECT_EQ(OH_CloudDisk_MarkFileAsPlaceholder(invalidSyncFolder, pathInfo),
              CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
    EXPECT_EQ(OH_CloudDisk_UnmarkPlaceholderFile(invalidSyncFolder, pathInfo),
              CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
#endif
}

/**
 * @tc.name: DehydrateFile_InvalidArgs_001
 * @tc.desc: Verify dehydration validates pointer and path arguments.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, DehydrateFile_InvalidArgs_001, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/testdir";
    std::string relativePath = "file.txt";
    CloudDisk_SyncFolderPath syncFolderPath{syncFolder.data(), syncFolder.length()};
    CloudDisk_PathInfo pathInfo{relativePath.data(), relativePath.length()};
#ifdef SUPPORT_CLOUD_DISK_SERVICE
    EXPECT_EQ(OH_CloudDisk_DehydrateFile(nullptr, &pathInfo), CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_DehydrateFile(&syncFolderPath, nullptr), CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
#else
    EXPECT_EQ(OH_CloudDisk_DehydrateFile(nullptr, &pathInfo), CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
    EXPECT_EQ(OH_CloudDisk_DehydrateFile(&syncFolderPath, nullptr), CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
#endif
}

#ifdef SUPPORT_CLOUD_DISK_SERVICE
/**
 * @tc.name: PlaceholderStateOnly_Mark_001
 * @tc.desc: Verify mark forwards to the manager and maps success and already-placeholder errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, PlaceholderStateOnly_Mark_001, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/testdir";
    std::string relativePath = "file.txt";
    CloudDisk_SyncFolderPath syncFolderPath = {const_cast<char *>(syncFolder.c_str()), syncFolder.size()};
    CloudDisk_PathInfo pathInfo = {const_cast<char *>(relativePath.c_str()), relativePath.size()};
    auto &mock = CloudDiskServiceManagerMock::GetInstance();

    EXPECT_CALL(mock, MarkFileAsPlaceholder(syncFolder, relativePath)).WillOnce(Return(E_OK));
    EXPECT_EQ(OH_CloudDisk_MarkFileAsPlaceholder(syncFolderPath, pathInfo), CloudDisk_ErrorCode::CLOUD_DISK_OK);

    Mock::VerifyAndClearExpectations(&mock);
    EXPECT_CALL(mock, MarkFileAsPlaceholder(syncFolder, relativePath)).WillOnce(Return(E_IS_A_PLACEHOLDER));
    EXPECT_EQ(OH_CloudDisk_MarkFileAsPlaceholder(syncFolderPath, pathInfo),
              CloudDisk_ErrorCode::OH_CLOUD_DISK_IS_A_PLACEHOLDER);
}

/**
 * @tc.name: PlaceholderStateOnly_Unmark_001
 * @tc.desc: Verify unmark maps placeholder-state precondition errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, PlaceholderStateOnly_Unmark_001, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/testdir";
    std::string relativePath = "file.txt";
    CloudDisk_SyncFolderPath syncFolderPath = {const_cast<char *>(syncFolder.c_str()), syncFolder.size()};
    CloudDisk_PathInfo pathInfo = {const_cast<char *>(relativePath.c_str()), relativePath.size()};
    auto &mock = CloudDiskServiceManagerMock::GetInstance();

    EXPECT_CALL(mock, UnmarkPlaceholderFile(syncFolder, relativePath)).WillOnce(Return(E_NOT_A_PLACEHOLDER));
    EXPECT_EQ(OH_CloudDisk_UnmarkPlaceholderFile(syncFolderPath, pathInfo),
              CloudDisk_ErrorCode::OH_CLOUD_DISK_NOT_A_PLACEHOLDER);

    Mock::VerifyAndClearExpectations(&mock);
    EXPECT_CALL(mock, UnmarkPlaceholderFile(syncFolder, relativePath))
        .WillOnce(Return(E_PLACEHOLDER_NOT_FULLY_HYDRATED));
    EXPECT_EQ(OH_CloudDisk_UnmarkPlaceholderFile(syncFolderPath, pathInfo),
              CloudDisk_ErrorCode::OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED);

    Mock::VerifyAndClearExpectations(&mock);
    EXPECT_CALL(mock, UnmarkPlaceholderFile(syncFolder, relativePath)).WillOnce(Return(E_OK));
    EXPECT_EQ(OH_CloudDisk_UnmarkPlaceholderFile(syncFolderPath, pathInfo), CloudDisk_ErrorCode::CLOUD_DISK_OK);
}

/**
 * @tc.name: DehydrateFile_ErrorMapping_001
 * @tc.desc: Verify dehydration forwards to the manager and maps state, callback, and authorization errors.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, DehydrateFile_ErrorMapping_001, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/testdir";
    std::string relativePath = "file.txt";
    CloudDisk_SyncFolderPath syncFolderPath{syncFolder.data(), syncFolder.length()};
    CloudDisk_PathInfo pathInfo{relativePath.data(), relativePath.length()};
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    const std::vector<std::pair<int32_t, CloudDisk_ErrorCode>> cases = {
        {E_OK, CLOUD_DISK_OK},
        {E_DEHYDRATE_DENIED, OH_CLOUD_DISK_DEHYDRATE_DENIED},
        {E_NOT_A_PLACEHOLDER, OH_CLOUD_DISK_NOT_A_PLACEHOLDER},
        {E_PLACEHOLDER_NOT_FULLY_HYDRATED, OH_CLOUD_DISK_PLACEHOLDER_NOT_FULLY_HYDRATED},
        {E_CALLBACK_NOT_REGISTERED, OH_CLOUD_DISK_CALLBACK_NOT_REGISTERED},
    };

    for (const auto &item : cases) {
        EXPECT_CALL(mock, DehydrateFile(syncFolder, relativePath)).WillOnce(Return(item.first));
        EXPECT_EQ(OH_CloudDisk_DehydrateFile(&syncFolderPath, &pathInfo), item.second);
        Mock::VerifyAndClearExpectations(&mock);
    }
}

/**
 * @tc.name: HydratePlaceholder_001
 * @tc.desc: Verify fetch and cancel dispatch and hydration error mapping.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, HydratePlaceholder_001, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/testdir";
    std::string relativePath = "file.txt";
    CloudDisk_SyncFolderPath syncFolderPath{syncFolder.data(), syncFolder.length()};
    CloudDisk_PathInfo pathInfo{relativePath.data(), relativePath.length()};
    auto &mock = CloudDiskServiceManagerMock::GetInstance();

    EXPECT_CALL(mock, StartHydration(syncFolder, relativePath, CLOUD_DISK_HYDRATE_PRIORITY_HIGH))
        .WillOnce(Return(E_HYDRATE_IN_PROGRESS));
    EXPECT_EQ(OH_CloudDisk_HydratePlaceholder(&syncFolderPath, &pathInfo, CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA,
                                              ::CLOUD_DISK_HYDRATE_PRIORITY_HIGH),
              OH_CLOUD_DISK_HYDRATE_IN_PROGRESS);
    Mock::VerifyAndClearExpectations(&mock);
    EXPECT_CALL(mock, CancelHydration(syncFolder, relativePath)).WillOnce(Return(E_NO_HYDRATION_IN_PROGRESS));
    EXPECT_EQ(OH_CloudDisk_HydratePlaceholder(&syncFolderPath, &pathInfo, CLOUD_DISK_CALLBACK_TYPE_CANCEL_FETCH_DATA,
                                              ::CLOUD_DISK_HYDRATE_PRIORITY_NORMAL),
              OH_CLOUD_DISK_NO_HYDRATION_IN_PROGRESS);
}

/**
 * @tc.name: HydratePlaceholder_002
 * @tc.desc: Verify pointer, callback type, and priority validation.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, HydratePlaceholder_002, TestSize.Level2)
{
    std::string syncFolder = "/storage/Users/currentUser/testdir";
    std::string relativePath = "file.txt";
    CloudDisk_SyncFolderPath syncFolderPath{syncFolder.data(), syncFolder.length()};
    CloudDisk_PathInfo pathInfo{relativePath.data(), relativePath.length()};
    EXPECT_EQ(OH_CloudDisk_HydratePlaceholder(nullptr, &pathInfo, CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA,
                                              ::CLOUD_DISK_HYDRATE_PRIORITY_NORMAL),
              CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_HydratePlaceholder(&syncFolderPath, nullptr, CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA,
                                              ::CLOUD_DISK_HYDRATE_PRIORITY_NORMAL),
              CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_HydratePlaceholder(&syncFolderPath, &pathInfo, CLOUD_DISK_CALLBACK_TYPE_FETCH_RANGE_DATA,
                                              ::CLOUD_DISK_HYDRATE_PRIORITY_NORMAL),
              CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(OH_CloudDisk_HydratePlaceholder(&syncFolderPath, &pathInfo, CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA,
                                              static_cast<OH_CloudDisk_HydratePriority>(3)),
              CLOUD_DISK_INVALID_ARG);
}

/**
 * @tc.name: Execute_001
 * @tc.desc: Verify Execute forwards the request key and data fields through independent IPC.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, Execute_001, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/sync";
    std::string path = "file.txt";
    std::vector<uint8_t> reqKey{1, 2, 3, 4};
    std::vector<uint8_t> data{5, 6, 7};
    OH_CloudDisk_CallbackReqHead reqHead{};
    reqHead.callbackType = CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA;
    reqHead.syncFolderPath = {syncFolder.data(), syncFolder.size()};
    reqHead.reqKey = {reqKey.data(), reqKey.size()};
    OH_CloudDisk_FetchData fetchData{4, data.size(), 7, {data.data(), data.size()}, true};
    OH_CloudDisk_CallbackResponse rsp{};
    rsp.fetchData = &fetchData;
    OH_CloudDisk_CallbackContext context{};
    OH_CloudDisk_FetchDataRequest fetchRequest{{path.data(), path.size()}, ::CLOUD_DISK_HYDRATE_PRIORITY_HIGH};
    context.fetchData = &fetchRequest;
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    EXPECT_CALL(mock, Execute(_)).WillOnce(Invoke([&](const CallbackExecuteRequest &request) {
        EXPECT_EQ(request.reqKey, reqKey);
        EXPECT_EQ(request.syncFolder, syncFolder);
        EXPECT_EQ(request.filePath, path);
        EXPECT_EQ(request.callbackType, static_cast<int32_t>(CloudDiskCallbackType::FETCH_DATA));
        EXPECT_EQ(request.offset, 4U);
        EXPECT_EQ(request.size, data.size());
        EXPECT_EQ(request.totalSize, 7U);
        EXPECT_EQ(request.data, data);
        EXPECT_TRUE(request.isComplete);
        return E_CANCELLED;
    }));
    EXPECT_EQ(OH_CloudDisk_Execute(reqHead, context, rsp), OH_CLOUD_DISK_CANCELLED);

    Mock::VerifyAndClearExpectations(&mock);
    fetchData.size = data.size() + 1;
    EXPECT_EQ(OH_CloudDisk_Execute(reqHead, context, rsp), CLOUD_DISK_INVALID_ARG);
    EXPECT_EQ(ConvertToErrorCode(E_ALREADY_HYDRATED), OH_CLOUD_DISK_ALREADY_HYDRATED);
}

/**
 * @tc.name: Execute_002
 * @tc.desc: Reject missing callback context, oversized data and unsigned offset overflow without IPC.
 * @tc.type: SECU
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, Execute_002, TestSize.Level2)
{
    std::string syncFolder = "/storage/Users/currentUser/sync";
    std::string path = "file.txt";
    uint8_t key = 1;
    uint8_t value = 1;
    OH_CloudDisk_CallbackReqHead head{{syncFolder.data(), syncFolder.size()},
        CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA, {&key, 1}};
    OH_CloudDisk_FetchDataRequest fetchRequest{{path.data(), path.size()}, ::CLOUD_DISK_HYDRATE_PRIORITY_NORMAL};
    OH_CloudDisk_CallbackContext context{};
    OH_CloudDisk_FetchData data{0, 1, 1, {&value, 1}, false};
    OH_CloudDisk_CallbackResponse response{};
    response.fetchData = &data;
    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), Execute(_)).Times(0);
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    context.fetchData = &fetchRequest;
    data.size = MAX_EXECUTE_DATA_SIZE + 1;
    data.data.dataSize = data.size;
    data.totalSize = data.size;
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    data = {UINT64_MAX, 1, UINT64_MAX, {&value, 1}, false};
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    data = {0, 1, 1, {nullptr, 1}, false};
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    head.callbackType = CLOUD_DISK_CALLBACK_TYPE_FETCH_RANGE_DATA;
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
    head.callbackType = CLOUD_DISK_CALLBACK_TYPE_FETCH_DATA;
    head.reqKey = {nullptr, 0};
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_INVALID_ARG);
}

/**
 * @tc.name: Execute_003
 * @tc.desc: Forward CANCEL callback context without dereferencing the unused response union.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(OhCloudDiskManagerTest, Execute_003, TestSize.Level1)
{
    std::string syncFolder = "/storage/Users/currentUser/sync";
    std::string path = "file.txt";
    uint8_t key = 1;
    OH_CloudDisk_CallbackReqHead head{{syncFolder.data(), syncFolder.size()},
        CLOUD_DISK_CALLBACK_TYPE_CANCEL_FETCH_DATA, {&key, 1}};
    CloudDisk_PathInfo pathInfo{path.data(), path.size()};
    OH_CloudDisk_CallbackContext context{};
    context.cancelFetchData = &pathInfo;
    OH_CloudDisk_CallbackResponse response{};
    EXPECT_CALL(CloudDiskServiceManagerMock::GetInstance(), Execute(_))
        .WillOnce(Invoke([&](const CallbackExecuteRequest &request) {
            EXPECT_EQ(request.syncFolder, syncFolder);
            EXPECT_EQ(request.filePath, path);
            EXPECT_EQ(request.callbackType, static_cast<int32_t>(CloudDiskCallbackType::CANCEL_FETCH_DATA));
            EXPECT_TRUE(request.data.empty());
            EXPECT_EQ(request.size, 0U);
            return E_OK;
        }));
    EXPECT_EQ(OH_CloudDisk_Execute(head, context, response), CLOUD_DISK_OK);
}
#endif

} // namespace Test
} // namespace FileManagement::CloudDiskService
} // namespace OHOS
