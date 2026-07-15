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

#include "cloud_disk_service.cpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "assistant.h"
#include "cloud_disk_service_access_token_mock.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class CloudDiskServiceStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<CloudDiskServiceAccessTokenMock> dfsuAccessToken_;
    static inline shared_ptr<AssistantMock> insMock_;
};

void CloudDiskServiceStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    dfsuAccessToken_ = make_shared<CloudDiskServiceAccessTokenMock>();
    CloudDiskServiceAccessTokenMock::dfsuAccessToken = dfsuAccessToken_;
    insMock_ = make_shared<AssistantMock>();
    Assistant::ins = insMock_;
    insMock_->EnableMock();
}

void CloudDiskServiceStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    insMock_->DisableMock();
    CloudDiskServiceAccessTokenMock::dfsuAccessToken = nullptr;
    dfsuAccessToken_ = nullptr;
    Assistant::ins = nullptr;
    insMock_ = nullptr;
}

void CloudDiskServiceStaticTest::SetUp()
{
}

void CloudDiskServiceStaticTest::TearDown()
{
}

/**
 * @tc.name: CheckPermissionsTest001
 * @tc.desc: Verify the CheckPermissions function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CheckPermissionsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionsTest001 start";
    try {
        string permission = "";
        bool isSystemApp = true;
        auto res = CheckPermissions(permission, isSystemApp);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckPermissionsTest001 failed";
    }
    GTEST_LOG_(INFO) << "CheckPermissionsTest001 end";
}

/**
 * @tc.name: CheckPermissionsTest002
 * @tc.desc: Verify the CheckPermissions function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CheckPermissionsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionsTest002 start";
    try {
        string permission = "permission";
        bool isSystemApp = true;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(true));
        auto res = CheckPermissions(permission, isSystemApp);
        EXPECT_EQ(res, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckPermissionsTest002 failed";
    }
    GTEST_LOG_(INFO) << "CheckPermissionsTest002 end";
}

/**
 * @tc.name: CheckPermissionsTest003
 * @tc.desc: Verify the CheckPermissions function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CheckPermissionsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionsTest003 start";
    try {
        string permission = "permission";
        bool isSystemApp = true;
        EXPECT_CALL(*dfsuAccessToken_, CheckCallerPermission(_)).WillOnce(Return(false));
        auto res = CheckPermissions(permission, isSystemApp);
        EXPECT_EQ(res, E_PERMISSION_DENIED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckPermissionsTest003 failed";
    }
    GTEST_LOG_(INFO) << "CheckPermissionsTest003 end";
}

/**
 * @tc.name: GetErrorNumTest001
 * @tc.desc: Verify the GetErrorNum function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetErrorNumTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetErrorNumTest001 start";
    try {
        int32_t error = EDQUOT;
        auto res = GetErrorNum(error);
        EXPECT_EQ(res, static_cast<int32_t>(ErrorReason::NO_SPACE_LEFT));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetErrorNumTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetErrorNumTest001 end";
}

/**
 * @tc.name: GetErrorNumTest002
 * @tc.desc: Verify the GetErrorNum function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetErrorNumTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetErrorNumTest002 start";
    try {
        int32_t error = ERANGE;
        auto res = GetErrorNum(error);
        EXPECT_EQ(res, static_cast<int32_t>(ErrorReason::OUT_OF_RANGE));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetErrorNumTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetErrorNumTest002 end";
}

/**
 * @tc.name: GetErrorNumTest003
 * @tc.desc: Verify the GetErrorNum function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetErrorNumTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetErrorNumTest003 start";
    try {
        int32_t error = ENOENT;
        auto res = GetErrorNum(error);
        EXPECT_EQ(res, static_cast<int32_t>(ErrorReason::NO_SUCH_FILE));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetErrorNumTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetErrorNumTest003 end";
}

/**
 * @tc.name: GetErrorNumTest004
 * @tc.desc: Verify the GetErrorNum function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetErrorNumTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetErrorNumTest004 start";
    try {
        int32_t error = -1;
        auto res = GetErrorNum(error);
        EXPECT_EQ(res, static_cast<int32_t>(ErrorReason::NO_SYNC_STATE));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetErrorNumTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetErrorNumTest004 end";
}

/**
 * @tc.name: SetFileSyncStatesTest001
 * @tc.desc: Verify the SetFileSyncStates function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetFileSyncStatesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest001 start";
    try {
        FileSyncState fileSyncState;
        fileSyncState.path = "/a/b/c";
        string syncFolderPath = "/a/b/";
        fileSyncState.state = SyncState::SYNCING;
        int32_t testUserId = 1;
        FailedList failedList;
        EXPECT_CALL(*insMock_, setxattr(_, _, _, _, _)).WillOnce(Return(0));
        auto res = SetFileSyncStates(fileSyncState, testUserId, failedList, syncFolderPath);
        EXPECT_TRUE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest001 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest001 end";
}

/**
 * @tc.name: SetFileSyncStatesTest002
 * @tc.desc: Verify the SetFileSyncStates function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetFileSyncStatesTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest002 start";
    try {
        FileSyncState fileSyncState;
        fileSyncState.path = "/a/b/c";
        string syncFolderPath = "/a/d";
        int32_t testUserId = 1;
        FailedList failedList;
        auto res = SetFileSyncStates(fileSyncState, testUserId, failedList, syncFolderPath);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest002 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest002 end";
}

/**
 * @tc.name: SetFileSyncStatesTest003
 * @tc.desc: Verify the SetFileSyncStates function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetFileSyncStatesTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest003 start";
    try {
        FileSyncState fileSyncState;
        fileSyncState.path = "/a/b/c";
        string syncFolderPath = "/a/b";
        fileSyncState.state = static_cast<SyncState>(6);
        int32_t testUserId = 1;
        FailedList failedList;
        auto res = SetFileSyncStates(fileSyncState, testUserId, failedList, syncFolderPath);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest003 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest003 end";
}

/**
 * @tc.name: SetFileSyncStatesTest004
 * @tc.desc: Verify the SetFileSyncStates function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetFileSyncStatesTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest004 start";
    try {
        FileSyncState fileSyncState;
        fileSyncState.path = "/a/b/c";
        string syncFolderPath = "/a/b";
        fileSyncState.state = SyncState::SYNCING;
        int32_t testUserId = 1;
        FailedList failedList;
        EXPECT_CALL(*insMock_, setxattr(_, _, _, _, _)).WillOnce(Return(1));
        auto res = SetFileSyncStates(fileSyncState, testUserId, failedList, syncFolderPath);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest004 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest004 end";
}

/**
 * @tc.name: SetFileSyncStatesTest005
 * @tc.desc: Verify the SetFileSyncStates function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, SetFileSyncStatesTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest005 start";
    try {
        FileSyncState fileSyncState;
        fileSyncState.path = "/a/b/c";
        string syncFolderPath = "";
        fileSyncState.state = SyncState::SYNCING;
        int32_t testUserId = 1;
        FailedList failedList;
        EXPECT_CALL(*insMock_, setxattr(_, _, _, _, _)).WillOnce(Return(1));
        auto res = SetFileSyncStates(fileSyncState, testUserId, failedList, syncFolderPath);
        EXPECT_FALSE(res);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileSyncStatesTest005 failed";
    }
    GTEST_LOG_(INFO) << "SetFileSyncStatesTest005 end";
}

/**
 * @tc.name: GetFileSyncStateTest001
 * @tc.desc: Verify the GetFileSyncState function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetFileSyncStateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStateTest001 start";
    try {
        string filePath = "/a/b/c";
        string syncFolderPath = "/a/d/";
        int32_t testUserId = 1;
        auto res = GetFileSyncState(filePath, testUserId, syncFolderPath);
        EXPECT_EQ(res.error, ErrorReason::INVALID_ARGUMENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileSyncStateTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStateTest001 end";
}

/**
 * @tc.name: GetFileSyncStateTest002
 * @tc.desc: Verify the GetFileSyncState function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetFileSyncStateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStateTest002 start";
    try {
        string filePath = "/a/b/c";
        string syncFolderPath = "/a/c";
        int32_t testUserId = 1;
        auto res = GetFileSyncState(filePath, testUserId, syncFolderPath);
        EXPECT_EQ(res.isSuccess, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileSyncStateTest002 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStateTest002 end";
}

/**
 * @tc.name: GetFileSyncStateTest003
 * @tc.desc: Verify the GetFileSyncState function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetFileSyncStateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStateTest003 start";
    try {
        string filePath = "/a/b/c";
        string syncFolderPath = "";
        int32_t testUserId = 1;
        auto res = GetFileSyncState(filePath, testUserId, syncFolderPath);
        EXPECT_EQ(res.isSuccess, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileSyncStateTest003 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStateTest003 end";
}

/**
 * @tc.name: GetFileSyncStateTest004
 * @tc.desc: Verify the GetFileSyncState function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetFileSyncStateTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileSyncStateTest004 start";
    try {
        string filePath = "/invalid/path/file";
        string syncFolderPath = "/invalid/path";
        int32_t testUserId = 1;
        auto res = GetFileSyncState(filePath, testUserId, syncFolderPath);
        EXPECT_EQ(res.isSuccess, false);
        EXPECT_EQ(res.error, ErrorReason::NO_SUCH_FILE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFileSyncStateTest004 failed";
    }
    GTEST_LOG_(INFO) << "GetFileSyncStateTest004 end";
}

/**
 * @tc.name: CheckSyncFolderBundleName_PathConvertFail_001
 * @tc.desc: Verify CheckSyncFolderBundleName when PathToPhysicalPath fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CheckSyncFolderBundleName_PathConvertFail_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_PathConvertFail_001 start";
    try {
        std::string syncFolder = "/test/mockFailed/PathToPhysicalPath";
        int32_t userId = 100;
        std::string bundleName = "com.ohos.test";

        int32_t ret = CheckSyncFolderBundleName(syncFolder, userId, bundleName);
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_PathConvertFail_001 failed";
    }
    GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_PathConvertFail_001 end";
}

/**
 * @tc.name: CheckSyncFolderBundleName_NotRegistered_002
 * @tc.desc: Verify CheckSyncFolderBundleName when sync folder not registered
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CheckSyncFolderBundleName_NotRegistered_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_NotRegistered_002 start";
    try {
        std::string syncFolder = "/test/mockFailed/GetSyncFolderValueByIndex";
        int32_t userId = 100;
        std::string bundleName = "com.ohos.test";

        int32_t ret = CheckSyncFolderBundleName(syncFolder, userId, bundleName);
        EXPECT_EQ(ret, E_SYNC_FOLDER_NOT_REGISTERED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_NotRegistered_002 failed";
    }
    GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_NotRegistered_002 end";
}

/**
 * @tc.name: CheckSyncFolderBundleName_BundleNameMismatch_003
 * @tc.desc: Verify CheckSyncFolderBundleName when bundleName mismatch
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CheckSyncFolderBundleName_BundleNameMismatch_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_BundleNameMismatch_003 start";
    try {
        std::string syncFolder = "/test/";
        int32_t userId = 100;
        std::string bundleName = "com.ohos.test";

        int32_t ret = CheckSyncFolderBundleName(syncFolder, userId, bundleName);
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_UNAUTHORIZED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_BundleNameMismatch_003 failed";
    }
    GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_BundleNameMismatch_003 end";
}

/**
 * @tc.name: CheckSyncFolderBundleName_Success_004
 * @tc.desc: Verify CheckSyncFolderBundleName with successful check
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, CheckSyncFolderBundleName_Success_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_Success_004 start";
    try {
        std::string syncFolder = "/test/";
        int32_t userId = 100;
        std::string bundleName = "";

        int32_t ret = CheckSyncFolderBundleName(syncFolder, userId, bundleName);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_Success_004 failed";
    }
    GTEST_LOG_(INFO) << "CheckSyncFolderBundleName_Success_004 end";
}

/**
 * @tc.name: GetHmdfsPath_PathConvertFail_001
 * @tc.desc: Verify GetHmdfsPath when PathToMntPathBySandboxPath fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetHmdfsPath_PathConvertFail_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHmdfsPath_PathConvertFail_001 start";
    try {
        std::string syncFolder = "/test/mockFailed";
        std::string path = "/test/mockFailed/PathToMntPathBySandboxPath";
        int32_t userId = 100;
        std::string hmdfsPath;

        int32_t ret = GetHmdfsPath(syncFolder, path, userId, hmdfsPath);
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetHmdfsPath_PathConvertFail_001 failed";
    }
    GTEST_LOG_(INFO) << "GetHmdfsPath_PathConvertFail_001 end";
}

/**
 * @tc.name: GetHmdfsPath_AccessFailed_002
 * @tc.desc: Verify GetHmdfsPath with successful path conversion
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetHmdfsPath_AccessFailed_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHmdfsPath_AccessFailed_002 start";
    try {
        std::string syncFolder = "/test/mockSuccess";
        std::string path = "PathToMntPathBySandboxPath";
        int32_t userId = 100;
        std::string hmdfsPath;

        EXPECT_CALL(*insMock_, access(_, _)).WillOnce(Return(-1));

        int32_t ret = GetHmdfsPath(syncFolder, path, userId, hmdfsPath);
        EXPECT_EQ(ret, E_SYNC_FOLDER_PATH_NOT_EXIST);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetHmdfsPath_AccessFailed_002 failed";
    }
    GTEST_LOG_(INFO) << "GetHmdfsPath_AccessFailed_002 end";
}

/**
 * @tc.name: GetHmdfsPath_Success_003
 * @tc.desc: Verify GetHmdfsPath with successful path conversion
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetHmdfsPath_Success_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHmdfsPath_Success_003 start";
    try {
        std::string syncFolder = "/test/mockSuccess";
        std::string path = "PathToMntPathBySandboxPath";
        int32_t userId = 100;
        std::string hmdfsPath;

        EXPECT_CALL(*insMock_, access(_, _)).WillOnce(Return(0));

        int32_t ret = GetHmdfsPath(syncFolder, path, userId, hmdfsPath);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetHmdfsPath_Success_003 failed";
    }
    GTEST_LOG_(INFO) << "GetHmdfsPath_Success_003 end";
}

/**
 * @tc.name: GetHmdfsPath_RelativePathStartWithSlash_004
 * @tc.desc: Verify GetHmdfsPath when relative path starts with '/'
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetHmdfsPath_RelativePathStartWithSlash_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHmdfsPath_RelativePathStartWithSlash_004 start";
    try {
        std::string syncFolder = "/test/mockSuccess";
        std::string path = "/file.txt";
        int32_t userId = 100;
        std::string hmdfsPath;

        int32_t ret = GetHmdfsPath(syncFolder, path, userId, hmdfsPath);
        EXPECT_EQ(ret, E_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetHmdfsPath_RelativePathStartWithSlash_004 failed";
    }
    GTEST_LOG_(INFO) << "GetHmdfsPath_RelativePathStartWithSlash_004 end";
}

/**
 * @tc.name: GetHmdfsPath_RelativePathStartWithSlash_005
 * @tc.desc: Verify GetHmdfsPath when relative path ends with '/'
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, GetHmdfsPath_RelativePathStartWithSlash_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetHmdfsPath_RelativePathStartWithSlash_005 start";
    try {
        std::string syncFolder = "/test/mockSuccess";
        std::string path = "file.txt/";
        int32_t userId = 100;
        std::string hmdfsPath;

        int32_t ret = GetHmdfsPath(syncFolder, path, userId, hmdfsPath);
        EXPECT_EQ(ret, E_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetHmdfsPath_RelativePathStartWithSlash_005 failed";
    }
    GTEST_LOG_(INFO) << "GetHmdfsPath_RelativePathStartWithSlash_005 end";
}

/**
 * @tc.name: ConvertPlaceholderToEmptyFile_OpenFail_001
 * @tc.desc: Verify ConvertPlaceholderToEmptyFile with nonexistent file
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ConvertPlaceholderToEmptyFile_OpenFail_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_OpenFail_001 start";
    try {
        string invalidPath = "/tmp/nonexistent_placeholder_file_12345";
        int32_t ret = ConvertPlaceholderToEmptyFile(invalidPath);
        EXPECT_NE(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_OpenFail_001 failed";
    }
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_OpenFail_001 end";
}

/**
 * @tc.name: ConvertPlaceholderToEmptyFile_NotPlaceholder_003
 * @tc.desc: Verify ConvertPlaceholderToEmptyFile with non-placeholder file
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ConvertPlaceholderToEmptyFile_NotPlaceholder_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_NotPlaceholder_003 start";
    try {
        string testFilePath = "/data/test_non_placeholder_" + to_string(time(nullptr));
        int fd = open(testFilePath.c_str(), O_CREAT | O_RDWR, 0644);
        if (fd >= 0) {
            close(fd);
            int32_t ret = ConvertPlaceholderToEmptyFile(testFilePath);
            EXPECT_EQ(ret, ENODATA);
            unlink(testFilePath.c_str());
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_NotPlaceholder_003 failed";
    }
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_NotPlaceholder_003 end";
}

/**
 * @tc.name: ConvertPlaceholderToEmptyFile_Success_006
 * @tc.desc: Verify ConvertPlaceholderToEmptyFile with valid placeholder file
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ConvertPlaceholderToEmptyFile_Success_006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_Success_006 start";
    insMock_->DisableMock();
    try {
        string testFilePath = "/data/test_placeholder_success_" + to_string(time(nullptr));
        int fd = open(testFilePath.c_str(), O_CREAT | O_RDWR, 0644);
        if (fd < 0) {
            GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_Success_006 failed";
            return;
        }
        char placeholderValue = '1';
        if (fsetxattr(fd, CLOUD_DISK_PLACEHOLDER_XATTR, &placeholderValue, sizeof(placeholderValue), 0) != 0) {
            close(fd);
            unlink(testFilePath.c_str());
            GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_Success_006 failed";
            return;
        }
        close(fd);

        int32_t ret = ConvertPlaceholderToEmptyFile(testFilePath);
        EXPECT_EQ(ret, 0);

        struct stat st;
        if (stat(testFilePath.c_str(), &st) == 0) {
            EXPECT_EQ(st.st_size, 0);
        }

        char newValue = '0';
        fd = open(testFilePath.c_str(), O_RDONLY);
        if (fd >= 0) {
            fgetxattr(fd, CLOUD_DISK_PLACEHOLDER_XATTR, &newValue, sizeof(newValue));
            close(fd);
        }

        unlink(testFilePath.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_Success_006 failed";
    }
    Assistant::ins = insMock_;
    insMock_->EnableMock();
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_Success_006 end";
}

/**
 * @tc.name: ConvertPlaceholderToEmptyFile_GetXattrFail_002
 * @tc.desc: Verify ConvertPlaceholderToEmptyFile when fgetxattr fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ConvertPlaceholderToEmptyFile_GetXattrFail_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_GetXattrFail_002 start";
    try {
        string testFilePath = "/data/test_getxattr_fail_" + to_string(time(nullptr));
        int fd = open(testFilePath.c_str(), O_CREAT | O_RDWR | O_NOFOLLOW, 0644);
        if (fd < 0) {
            GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_GetXattrFail_002 failed";
            return;
        }
        close(fd);

        fd = open(testFilePath.c_str(), O_RDWR | O_NOFOLLOW);
        if (fd < 0) {
            unlink(testFilePath.c_str());
            GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_GetXattrFail_002 failed";
            return;
        }
        ssize_t xattrRet = fgetxattr(fd, CLOUD_DISK_PLACEHOLDER_XATTR, nullptr, 0);
        close(fd);
        if (xattrRet < 0) {
            int32_t ret = ConvertPlaceholderToEmptyFile(testFilePath);
            EXPECT_NE(ret, 0);
        }
        unlink(testFilePath.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_GetXattrFail_002 failed";
    }
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_GetXattrFail_002 end";
}

/**
 * @tc.name: ConvertPlaceholderToEmptyFile_TruncateFail_004
 * @tc.desc: Verify ConvertPlaceholderToEmptyFile when ftruncate fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ConvertPlaceholderToEmptyFile_TruncateFail_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_TruncateFail_004 start";
    try {
        string testFilePath = "/data/test_truncate_fail_" + to_string(time(nullptr));
        int fd = open(testFilePath.c_str(), O_CREAT | O_RDWR, 0644);
        if (fd >= 0) {
            char placeholderValue = '1';
            insMock_->DisableMock();
            fsetxattr(fd, CLOUD_DISK_PLACEHOLDER_XATTR, &placeholderValue,
                      sizeof(placeholderValue), 0);
            Assistant::ins = insMock_;
            insMock_->EnableMock();
            close(fd);

            EXPECT_CALL(*insMock_, ftruncate(_, _)).WillOnce(Return(-1));
            int32_t ret = ConvertPlaceholderToEmptyFile(testFilePath);
            EXPECT_NE(ret, 0);
            unlink(testFilePath.c_str());
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_TruncateFail_004 failed";
    }
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_TruncateFail_004 end";
}

/**
 * @tc.name: ConvertPlaceholderToEmptyFile_SetXattrFail_005
 * @tc.desc: Verify ConvertPlaceholderToEmptyFile when fsetxattr fails
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ConvertPlaceholderToEmptyFile_SetXattrFail_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_SetXattrFail_005 start";
    try {
        string testFilePath = "/data/test_setxattr_fail_" + to_string(time(nullptr));
        int fd = open(testFilePath.c_str(), O_CREAT | O_RDWR, 0644);
        if (fd >= 0) {
            char placeholderValue = '1';
            insMock_->DisableMock();
            fsetxattr(fd, CLOUD_DISK_PLACEHOLDER_XATTR, &placeholderValue,
                      sizeof(placeholderValue), 0);
            Assistant::ins = insMock_;
            insMock_->EnableMock();
            close(fd);

            EXPECT_CALL(*insMock_, ftruncate(_, _)).WillOnce(Return(0));
            EXPECT_CALL(*insMock_, fsetxattr(_, _, _, _, _)).WillOnce(Return(-1));
            int32_t ret = ConvertPlaceholderToEmptyFile(testFilePath);
            EXPECT_EQ(ret, 0);
            unlink(testFilePath.c_str());
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_SetXattrFail_005 failed";
    }
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_SetXattrFail_005 end";
}

/**
 * @tc.name: ConvertPlaceholderToEmptyFile_XattrValueZero_008
 * @tc.desc: Verify ConvertPlaceholderToEmptyFile when xattr value is already 0
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ConvertPlaceholderToEmptyFile_XattrValueZero_008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_XattrValueZero_008 start";
    try {
        string testFilePath = "/data/test_xattr_zero_" + to_string(time(nullptr));
        int fd = open(testFilePath.c_str(), O_CREAT | O_RDWR, 0644);
        if (fd >= 0) {
            char xattrValue = '0';
            insMock_->DisableMock();
            fsetxattr(fd, CLOUD_DISK_PLACEHOLDER_XATTR, &xattrValue,
                      sizeof(xattrValue), 0);
            Assistant::ins = insMock_;
            insMock_->EnableMock();
            close(fd);

            int32_t ret = ConvertPlaceholderToEmptyFile(testFilePath);
            EXPECT_EQ(ret, EINVAL);

            unlink(testFilePath.c_str());
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_XattrValueZero_008 failed";
    }
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_XattrValueZero_008 end";
}

/**
 * @tc.name: ConvertPlaceholderToEmptyFile_SetXattrFail_009
 * @tc.desc: Verify ConvertPlaceholderToEmptyFile when xattr value is already 0
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceStaticTest, ConvertPlaceholderToEmptyFile_SetXattrFail_009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_SetXattrFail_009 start";
    try {
        string testFilePath = "/data/test_fsetxattr_fail" + to_string(time(nullptr));
        int fd = open(testFilePath.c_str(), O_CREAT | O_RDWR, 0644);
        if (fd >= 0) {
            char placeholderValue = '1';
            insMock_->DisableMock();
            fsetxattr(fd, CLOUD_DISK_PLACEHOLDER_XATTR, &placeholderValue,
                      sizeof(placeholderValue), 0);
            Assistant::ins = insMock_;
            insMock_->EnableMock();
            close(fd);

            chmod(testFilePath.c_str(), 0444);

            EXPECT_CALL(*insMock_, ftruncate(_, _)).WillRepeatedly(Return(0));
            EXPECT_CALL(*insMock_, fsetxattr(_, _, _, _, _)).WillOnce(Return(-1));

            int32_t ret = ConvertPlaceholderToEmptyFile(testFilePath);
            EXPECT_NE(ret, 0);

            chmod(testFilePath.c_str(), 0644);
            unlink(testFilePath.c_str());
        }
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_SetXattrFail_009 failed";
    }
    GTEST_LOG_(INFO) << "ConvertPlaceholderToEmptyFile_SetXattrFail_009 end";
}

} // namespace OHOS::FileManagement::CloudDiskService::Test