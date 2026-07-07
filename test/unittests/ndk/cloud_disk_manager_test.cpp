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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cerrno>
#include <climits>
#include <securec.h>
#include <string>
#include <sys/stat.h>
#include <sys/xattr.h>

#include "cloud_disk_xattr_mock.h"
#include "oh_cloud_disk_manager.h"
#include "oh_cloud_disk_utils.h"
#include "cloud_disk_service_manager_mock.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

namespace {
constexpr int32_t TEST_USER_ID = 100;
constexpr mode_t TEST_DIRECTORY_MODE = S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP |
    S_IROTH | S_IXOTH;
const string SANDBOX_SYNC_FOLDER = "/storage/Users/currentUser/sync";
const string SANDBOX_FILE_PATH = "/storage/Users/currentUser/sync/a.txt";
const string MNT_SYNC_FOLDER = "/mnt/hmdfs/100/account/device_view/local/files/Docs/sync";
const string MNT_FILE_PATH = "/mnt/hmdfs/100/account/device_view/local/files/Docs/sync/a.txt";
const string PLACEHOLDER_XATTR_KEY = "user.clouddisk.placeholder";
const string PERM_CLOUD_DISK_SERVICE = "ohos.permission.ACCESS_CLOUD_DISK_INFO";

CloudDisk_PathInfo ToPathInfo(const string &path)
{
    return {const_cast<char *>(path.c_str()), path.length()};
}
} // namespace

class CloudDiskManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void CloudDiskManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskManagerTest::SetUp(void)
{
    ResetXattrMock();
}

void CloudDiskManagerTest::TearDown(void)
{
    Mock::VerifyAndClearExpectations(&CloudDiskServiceManagerMock::GetInstance());
}

/**
 * @tc.name: AllocFieldTest001
 * @tc.desc: Verify the AllocField function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, AllocFieldTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AllocFieldTest001 start";
    try {
        std::string value = "123456";
        int32_t length = -2;
        char * fileId = AllocField(value.c_str(), length);
        EXPECT_EQ(fileId, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AllocFieldTest001 failed";
    }
    GTEST_LOG_(INFO) << "AllocFieldTest001 end";
}

/**
 * @tc.name: AllocFieldTest002
 * @tc.desc: Verify the AllocField function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, AllocFieldTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AllocFieldTest002 start";
    try {
        std::string value = "123456";
        int32_t length = 10;
        char * fileId = AllocField(nullptr, length);
        EXPECT_EQ(fileId, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AllocFieldTest002 failed";
    }
    GTEST_LOG_(INFO) << "AllocFieldTest002 end";
}

/**
 * @tc.name: AllocFieldTest003
 * @tc.desc: Verify the AllocField function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, AllocFieldTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AllocFieldTest003 start";
    try {
        std::string value = "123456";
        int32_t length = value.size();
        char * fileId = AllocField(value.c_str(), length);
        EXPECT_NE(fileId, nullptr);
        delete []fileId;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AllocFieldTest003 failed";
    }
    GTEST_LOG_(INFO) << "AllocFieldTest003 end";
}

/**
 * @tc.name: AllocFieldTest004
 * @tc.desc: Verify the AllocField function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, AllocFieldTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AllocFieldTest004 start";
    try {
        std::string value = std::string("123") + '\0' + std::string("456");
        constexpr int32_t length = 7;

        char expectResult[length + 1] = {'1', '2', '3', '\0', '4', '5', '6', '\0'};
        char * fileId = AllocField(value.c_str(), length);
        EXPECT_EQ(strncmp(expectResult, fileId, length + 1), 0);
        delete []fileId;

        char *fileId2 = new(std::nothrow) char[length + 1];
        if (fileId2 == nullptr) {
            GTEST_LOG_(ERROR) << "new failed" << std::endl;
            return;
        }
        if (strncpy_s(fileId2, length + 1, value.c_str(), length) != 0) {
            GTEST_LOG_(ERROR) << "strncpy_s failed" << std::endl;
            delete[] fileId2;
            return;
        }
        fileId2[length] = '\0';
        char expectResult2[length + 1] = {'1', '2', '3', '\0', '\0', '\0', '\0', '\0'};
        EXPECT_EQ(strncmp(expectResult2, fileId2, length + 1), 0);
        delete []fileId2;
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AllocFieldTest004 failed";
    }
    GTEST_LOG_(INFO) << "AllocFieldTest004 end";
}

/**
 * @tc.name: IsValidPathInfoTest001
 * @tc.desc: Verify the IsValidPathInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsValidPathInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidPathInfoTest001 start";
    try {
        std::string value = "123456";
        int32_t length = value.size();
        bool ret = IsValidPathInfo(nullptr, length);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidPathInfoTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsValidPathInfoTest001 end";
}

/**
 * @tc.name: IsValidPathInfoTest002
 * @tc.desc: Verify the IsValidPathInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsValidPathInfoTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidPathInfoTest002 start";
    try {
        std::string value = "123456";
        bool ret = IsValidPathInfo(value.c_str(), 0);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidPathInfoTest002 failed";
    }
    GTEST_LOG_(INFO) << "IsValidPathInfoTest002 end";
}

/**
 * @tc.name: IsValidPathInfoTest003
 * @tc.desc: Verify the IsValidPathInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsValidPathInfoTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidPathInfoTest003 start";
    try {
        std::string value = "123456";
        int32_t length = value.size();
        bool ret = IsValidPathInfo(value.c_str(), length + 10);
        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidPathInfoTest003 failed";
    }
    GTEST_LOG_(INFO) << "IsValidPathInfoTest003 end";
}

/**
 * @tc.name: IsValidPathInfoTest004
 * @tc.desc: Verify the IsValidPathInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsValidPathInfoTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidPathInfoTest004 start";
    try {
        std::string value = "123456";
        int32_t length = value.size();
        bool ret = IsValidPathInfo(value.c_str(), length);
        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidPathInfoTest004 failed";
    }
    GTEST_LOG_(INFO) << "IsValidPathInfoTest004 end";
}

/**
 * @tc.name: IsPathInSyncFolderTest001
 * @tc.desc: Verify the sync folder path boundary helper
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPathInSyncFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPathInSyncFolderTest001 start";
    try {
        EXPECT_TRUE(IsPathInSyncFolder("/mock/sync", "/mock/sync"));
        EXPECT_TRUE(IsPathInSyncFolder("/mock/sync", "/mock/sync/a.txt"));
        EXPECT_TRUE(IsPathInSyncFolder("/mock/sync/", "/mock/sync/sub/a.txt"));
        EXPECT_FALSE(IsPathInSyncFolder("", "/mock/sync/a.txt"));
        EXPECT_FALSE(IsPathInSyncFolder("/mock/sync", ""));
        EXPECT_FALSE(IsPathInSyncFolder("/mock/sync", "/mock/other/a.txt"));
        EXPECT_FALSE(IsPathInSyncFolder("/mock/sync", "/mock/sync-other/a.txt"));
        EXPECT_FALSE(IsPathInSyncFolder("/mock/sync", "/mock/sync2/a.txt"));
        EXPECT_FALSE(IsPathInSyncFolder("/mock/sync", "/mock"));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPathInSyncFolderTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsPathInSyncFolderTest001 end";
}

/**
 * @tc.name: ConvertToErrorCodeTest001
 * @tc.desc: Verify the ConvertToErrorCode function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, ConvertToErrorCodeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest001 start";
    try {
        int32_t innerErrorCode = -1;
        CloudDisk_ErrorCode ret = ConvertToErrorCode(innerErrorCode);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertToErrorCodeTest001 failed";
    }
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest001 end";
}

/**
 * @tc.name: ConvertToErrorCodeTest002
 * @tc.desc: Verify the ConvertToErrorCode function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, ConvertToErrorCodeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest002 start";
    try {
        int32_t innerErrorCode = 0;
        CloudDisk_ErrorCode ret = ConvertToErrorCode(innerErrorCode);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertToErrorCodeTest002 failed";
    }
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest002 end";
}

/**
 * @tc.name: ConvertToErrorCodeTest003
 * @tc.desc: Verify the ConvertToErrorCode function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, ConvertToErrorCodeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest003 start";
    try {
        int32_t innerErrorCode = 34400001;
        CloudDisk_ErrorCode ret = ConvertToErrorCode(innerErrorCode);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_EQ(ConvertToErrorCode(E_FILE_ALREADY_EXISTS), CloudDisk_ErrorCode::CLOUD_DISK_FILE_ALREADY_EXISTS);
        EXPECT_EQ(ConvertToErrorCode(E_NO_SPACE_LEFT), CloudDisk_ErrorCode::CLOUD_DISK_NO_SPACE_LEFT);
        EXPECT_EQ(ConvertToErrorCode(E_NOT_A_DIRECTORY), CloudDisk_ErrorCode::CLOUD_DISK_NOT_A_DIRECTORY);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertToErrorCodeTest003 failed";
    }
    GTEST_LOG_(INFO) << "ConvertToErrorCodeTest003 end";
}

/**
 * @tc.name: CreatePlaceholderFileTest001
 * @tc.desc: Verify create placeholder rejects invalid relative path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, CreatePlaceholderFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileTest001 start";
    GTEST_LOG_(INFO) << "[BRANCH] OH_CloudDisk_CreatePlaceholder invalid relative path";
    std::string syncFolder = "/storage/Users/currentUser/Docs";
    CloudDisk_SyncFolderPath syncFolderPath = {
        .value = const_cast<char *>(syncFolder.c_str()),
        .length = syncFolder.length(),
    };
    CloudDisk_PathInfo relativePathInfo = {
        .value = nullptr,
        .length = 0,
    };
    CloudDisk_PlaceholderInfo info = {
        .logicalSize = 1024,
        .atimeMs = 1,
        .mtimeMs = 2,
    };
    auto ret = OH_CloudDisk_CreatePlaceholder(syncFolderPath, relativePathInfo, info);

    EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileTest001 end";
}

/**
 * @tc.name: CreatePlaceholderFileTest002
 * @tc.desc: Verify create placeholder file rejects invalid sync folder path
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, CreatePlaceholderFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileTest002 start";
    GTEST_LOG_(INFO) << "[BRANCH] OH_CloudDisk_CreatePlaceholder invalid sync folder path";
    std::string relativePath = "test.txt";
    CloudDisk_SyncFolderPath syncFolderPath = {
        .value = nullptr,
        .length = 0,
    };
    CloudDisk_PathInfo relativePathInfo = {
        .value = const_cast<char *>(relativePath.c_str()),
        .length = relativePath.length(),
    };
    CloudDisk_PlaceholderInfo info = {
        .logicalSize = 1024,
        .atimeMs = 1,
        .mtimeMs = 2,
    };
    auto ret = OH_CloudDisk_CreatePlaceholder(syncFolderPath, relativePathInfo, info);

    EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileTest002 end";
}

/**
 * @tc.name: CreatePlaceholderFileTest003
 * @tc.desc: Verify create placeholder rejects invalid relative path length
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, CreatePlaceholderFileTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileTest003 start";
    GTEST_LOG_(INFO) << "[BRANCH] OH_CloudDisk_CreatePlaceholder invalid relative path length";
    std::string syncFolder = "/storage/Users/currentUser/Docs";
    std::string relativePath = "test.txt";
    CloudDisk_SyncFolderPath syncFolderPath = {
        .value = const_cast<char *>(syncFolder.c_str()),
        .length = syncFolder.length(),
    };
    CloudDisk_PathInfo relativePathInfo = {
        .value = const_cast<char *>(relativePath.c_str()),
        .length = relativePath.length() + 1,
    };
    CloudDisk_PlaceholderInfo info = {
        .logicalSize = 1024,
        .atimeMs = 1,
        .mtimeMs = 2,
    };
    auto ret = OH_CloudDisk_CreatePlaceholder(syncFolderPath, relativePathInfo, info);

    EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileTest003 end";
}

/**
 * @tc.name: CreatePlaceholderFileTest004
 * @tc.desc: Verify create placeholder file returns success when manager succeeds
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, CreatePlaceholderFileTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileTest004 start";
    GTEST_LOG_(INFO) << "[BRANCH] OH_CloudDisk_CreatePlaceholder manager success";
    std::string syncFolder = "/storage/Users/currentUser/Docs";
    std::string relativePath = "test.txt";
    CloudDisk_SyncFolderPath syncFolderPath = {
        .value = const_cast<char *>(syncFolder.c_str()),
        .length = syncFolder.length(),
    };
    CloudDisk_PathInfo relativePathInfo = {
        .value = const_cast<char *>(relativePath.c_str()),
        .length = relativePath.length(),
    };
    CloudDisk_PlaceholderInfo info = {
        .logicalSize = 1024,
        .atimeMs = 1,
        .mtimeMs = 2,
    };
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    EXPECT_CALL(mock, CreatePlaceholderFile(syncFolder, relativePath, _))
        .WillOnce(Invoke([](const std::string &, const std::string &, const PlaceholderInfo &innerInfo) {
            EXPECT_EQ(innerInfo.logicalSize, 1024);
            EXPECT_EQ(innerInfo.atimeMs, 1);
            EXPECT_EQ(innerInfo.mtimeMs, 2);
            return E_OK;
        }));

    auto ret = OH_CloudDisk_CreatePlaceholder(syncFolderPath, relativePathInfo, info);

    EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileTest004 end";
}

/**
 * @tc.name: CreatePlaceholderFileTest005
 * @tc.desc: Verify create placeholder file maps manager errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, CreatePlaceholderFileTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreatePlaceholderFileTest005 start";
    GTEST_LOG_(INFO) << "[BRANCH] OH_CloudDisk_CreatePlaceholder manager failure";
    std::string syncFolder = "/storage/Users/currentUser/Docs";
    std::string relativePath = "test.txt";
    CloudDisk_SyncFolderPath syncFolderPath = {
        .value = const_cast<char *>(syncFolder.c_str()),
        .length = syncFolder.length(),
    };
    CloudDisk_PathInfo relativePathInfo = {
        .value = const_cast<char *>(relativePath.c_str()),
        .length = relativePath.length(),
    };
    CloudDisk_PlaceholderInfo info = {
        .logicalSize = 1024,
        .atimeMs = 1,
        .mtimeMs = 2,
    };
    auto &mock = CloudDiskServiceManagerMock::GetInstance();
    EXPECT_CALL(mock, CreatePlaceholderFile(syncFolder, relativePath, _))
        .WillOnce(Return(E_FILE_ALREADY_EXISTS));

    auto ret = OH_CloudDisk_CreatePlaceholder(syncFolderPath, relativePathInfo, info);

    EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_FILE_ALREADY_EXISTS);
    GTEST_LOG_(INFO) << "CreatePlaceholderFileTest005 end";
}

/**
 * @tc.name: ConvertXattrErrnoTest001
 * @tc.desc: Verify the ConvertXattrErrno function maps supported errno values
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, ConvertXattrErrnoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ConvertXattrErrnoTest001 start";
    try {
        EXPECT_EQ(ConvertXattrErrno(ENOENT), CloudDisk_ErrorCode::CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST);
        EXPECT_EQ(ConvertXattrErrno(EACCES), CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED);
        EXPECT_EQ(ConvertXattrErrno(EPERM), CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED);
        EXPECT_EQ(ConvertXattrErrno(EOPNOTSUPP), CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
        EXPECT_EQ(ConvertXattrErrno(EIO), CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ConvertXattrErrnoTest001 failed";
    }
    GTEST_LOG_(INFO) << "ConvertXattrErrnoTest001 end";
}

/**
 * @tc.name: ParsePlaceholderXattrValueTest001
 * @tc.desc: Verify placeholder xattr value parsing
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, ParsePlaceholderXattrValueTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ParsePlaceholderXattrValueTest001 start";
    try {
        const char zero[] = {'0'};
        const char falseValue[] = {'f', 'a', 'l', 's', 'e'};
        const char upperFalseValue[] = {'F', 'A', 'L', 'S', 'E'};
        const char binaryFalse[] = {'\0'};
        const char one[] = {'1'};
        const char trueValue[] = {'t', 'r', 'u', 'e'};
        const char upperTrueValue[] = {'T', 'R', 'U', 'E'};
        const char binaryTrue[] = {'\1'};
        EXPECT_FALSE(ParsePlaceholderXattrValue(zero, sizeof(zero)));
        EXPECT_FALSE(ParsePlaceholderXattrValue(falseValue, sizeof(falseValue)));
        EXPECT_FALSE(ParsePlaceholderXattrValue(upperFalseValue, sizeof(upperFalseValue)));
        EXPECT_FALSE(ParsePlaceholderXattrValue(binaryFalse, sizeof(binaryFalse)));
        EXPECT_TRUE(ParsePlaceholderXattrValue(nullptr, 0));
        EXPECT_TRUE(ParsePlaceholderXattrValue(one, sizeof(one)));
        EXPECT_TRUE(ParsePlaceholderXattrValue(trueValue, sizeof(trueValue)));
        EXPECT_TRUE(ParsePlaceholderXattrValue(upperTrueValue, sizeof(upperTrueValue)));
        EXPECT_TRUE(ParsePlaceholderXattrValue(binaryTrue, sizeof(binaryTrue)));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ParsePlaceholderXattrValueTest001 failed";
    }
    GTEST_LOG_(INFO) << "ParsePlaceholderXattrValueTest001 end";
}

/**
 * @tc.name: CheckPermissionsTest001
 * @tc.desc: Verify CheckPermissions short-circuits optional permission and system app checks
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, CheckPermissionsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CheckPermissionsTest001 start";
    try {
        auto &optionalMockState = GetXattrMockState();
        EXPECT_EQ(CheckPermissions("", false), CloudDisk_ErrorCode::CLOUD_DISK_OK);
        EXPECT_FALSE(optionalMockState.permissionChecked);
        EXPECT_FALSE(optionalMockState.systemAppChecked);

        ResetXattrMock();
        auto &permissionMockState = GetXattrMockState();
        permissionMockState.permissionGranted = false;
        EXPECT_EQ(CheckPermissions(PERM_CLOUD_DISK_SERVICE, false),
            CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED);
        EXPECT_TRUE(permissionMockState.permissionChecked);
        EXPECT_FALSE(permissionMockState.systemAppChecked);

        ResetXattrMock();
        auto &systemAppMockState = GetXattrMockState();
        systemAppMockState.systemApp = false;
        EXPECT_EQ(CheckPermissions("", true), CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED);
        EXPECT_FALSE(systemAppMockState.permissionChecked);
        EXPECT_TRUE(systemAppMockState.systemAppChecked);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CheckPermissionsTest001 failed";
    }
    GTEST_LOG_(INFO) << "CheckPermissionsTest001 end";
}

/**
 * @tc.name: PathToMntPathBySandboxPathTest001
 * @tc.desc: Verify PathToMntPathBySandboxPath converts and validates sandbox paths
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, PathToMntPathBySandboxPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest001 start";
    try {
        auto &successMockState = GetXattrMockState();
        successMockState.userId = TEST_USER_ID;
        string realPath;
        CloudDisk_ErrorCode ret = PathToMntPathBySandboxPath(SANDBOX_FILE_PATH, realPath);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
        EXPECT_EQ(realPath, MNT_FILE_PATH);
        EXPECT_TRUE(successMockState.realpathCalled);
        EXPECT_THAT(successMockState.realpathPaths, ElementsAre(MNT_FILE_PATH));

        ResetXattrMock();
        auto &invalidPrefixMockState = GetXattrMockState();
        realPath.clear();
        ret = PathToMntPathBySandboxPath("/invalid/currentUser/sync/a.txt", realPath);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_TRUE(realPath.empty());
        EXPECT_FALSE(invalidPrefixMockState.realpathCalled);

        ResetXattrMock();
        auto &missingPathMockState = GetXattrMockState();
        missingPathMockState.userId = TEST_USER_ID;
        missingPathMockState.realpathFail = true;
        missingPathMockState.realpathFailPath = MNT_FILE_PATH;
        realPath.clear();
        ret = PathToMntPathBySandboxPath(SANDBOX_FILE_PATH, realPath);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST);
        EXPECT_TRUE(realPath.empty());
        EXPECT_TRUE(missingPathMockState.realpathCalled);
        EXPECT_THAT(missingPathMockState.realpathPaths, ElementsAre(MNT_FILE_PATH));

        ResetXattrMock();
        auto &permissionDeniedMockState = GetXattrMockState();
        permissionDeniedMockState.userId = TEST_USER_ID;
        permissionDeniedMockState.realpathFail = true;
        permissionDeniedMockState.realpathFailPath = MNT_FILE_PATH;
        permissionDeniedMockState.realpathErrno = EACCES;
        realPath.clear();
        ret = PathToMntPathBySandboxPath(SANDBOX_FILE_PATH, realPath);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED);
        EXPECT_TRUE(realPath.empty());
        EXPECT_TRUE(permissionDeniedMockState.realpathCalled);
        EXPECT_THAT(permissionDeniedMockState.realpathPaths, ElementsAre(MNT_FILE_PATH));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest001 failed";
    }
    GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest001 end";
}

/**
 * @tc.name: PathToMntPathBySandboxPathTest002
 * @tc.desc: Verify PathToMntPathBySandboxPath rejects unsafe or invalid realpath results
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, PathToMntPathBySandboxPathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest002 start";
    try {
        auto &traversalMockState = GetXattrMockState();
        traversalMockState.userId = TEST_USER_ID;
        traversalMockState.realpathResults[MNT_FILE_PATH] = "/tmp/outside.txt";
        string realPath;
        CloudDisk_ErrorCode ret = PathToMntPathBySandboxPath(SANDBOX_FILE_PATH, realPath);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_TRUE(realPath.empty());
        EXPECT_TRUE(traversalMockState.realpathCalled);
        EXPECT_THAT(traversalMockState.realpathPaths, ElementsAre(MNT_FILE_PATH));

        ResetXattrMock();
        auto &tooLongMockState = GetXattrMockState();
        tooLongMockState.userId = TEST_USER_ID;
        tooLongMockState.realpathResults[MNT_FILE_PATH] = string(PATH_MAX + 1, 'a');
        realPath.clear();
        ret = PathToMntPathBySandboxPath(SANDBOX_FILE_PATH, realPath);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_TRUE(realPath.empty());
        EXPECT_TRUE(tooLongMockState.realpathCalled);
        EXPECT_THAT(tooLongMockState.realpathPaths, ElementsAre(MNT_FILE_PATH));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest002 failed";
    }
    GTEST_LOG_(INFO) << "PathToMntPathBySandboxPathTest002 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest001
 * @tc.desc: Verify QueryPlaceholderByXattr maps file and xattr probe outcomes
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, QueryPlaceholderByXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest001 start";
    try {
        auto &placeholderMockState = GetXattrMockState();
        placeholderMockState.getxattrValue = {'1'};
        placeholderMockState.getxattrSizeRet = static_cast<ssize_t>(placeholderMockState.getxattrValue.size());
        placeholderMockState.getxattrValueRet = static_cast<ssize_t>(placeholderMockState.getxattrValue.size());
        bool isPlaceholder = false;
        CloudDisk_ErrorCode ret = QueryPlaceholderByXattr(MNT_FILE_PATH, isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
        EXPECT_TRUE(isPlaceholder);
        EXPECT_TRUE(placeholderMockState.lstatCalled);
        EXPECT_TRUE(placeholderMockState.getxattrCalled);
        EXPECT_EQ(placeholderMockState.lstatPath, MNT_FILE_PATH);
        EXPECT_EQ(placeholderMockState.getxattrPath, MNT_FILE_PATH);
        EXPECT_THAT(placeholderMockState.getxattrSizes, ElementsAre(0, 1));

        ResetXattrMock();
        auto &normalFileMockState = GetXattrMockState();
        normalFileMockState.getxattrValue = {'1', '1'};
        normalFileMockState.getxattrSizeRet = static_cast<ssize_t>(normalFileMockState.getxattrValue.size());
        normalFileMockState.getxattrValueRet = static_cast<ssize_t>(normalFileMockState.getxattrValue.size());
        isPlaceholder = true;
        ret = QueryPlaceholderByXattr(MNT_FILE_PATH, isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(normalFileMockState.lstatCalled);
        EXPECT_TRUE(normalFileMockState.getxattrCalled);
        EXPECT_THAT(normalFileMockState.getxattrSizes, ElementsAre(0, 2));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest001 failed";
    }
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest001 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest002
 * @tc.desc: Verify QueryPlaceholderByXattr stops before xattr on invalid file states
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, QueryPlaceholderByXattrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest002 start";
    try {
        auto &missingFileMockState = GetXattrMockState();
        missingFileMockState.lstatRet = -1;
        missingFileMockState.lstatErrno = ENOENT;
        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = QueryPlaceholderByXattr(MNT_FILE_PATH, isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST);
        EXPECT_TRUE(isPlaceholder);
        EXPECT_TRUE(missingFileMockState.lstatCalled);
        EXPECT_FALSE(missingFileMockState.getxattrCalled);

        ResetXattrMock();
        auto &directoryMockState = GetXattrMockState();
        directoryMockState.lstatMode = TEST_DIRECTORY_MODE;
        ret = QueryPlaceholderByXattr(MNT_SYNC_FOLDER, isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_TRUE(directoryMockState.lstatCalled);
        EXPECT_FALSE(directoryMockState.getxattrCalled);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest002 failed";
    }
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest002 end";
}

/**
 * @tc.name: QueryPlaceholderByXattrTest003
 * @tc.desc: Verify QueryPlaceholderByXattr maps getxattr size and read failures
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, QueryPlaceholderByXattrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest003 start";
    try {
        auto &sizeErrorMockState = GetXattrMockState();
        sizeErrorMockState.getxattrSizeRet = MOCK_XATTR_RET_FAIL;
        sizeErrorMockState.getxattrSizeErrno = ENOTSUP;
        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = QueryPlaceholderByXattr(MNT_FILE_PATH, isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
        EXPECT_TRUE(isPlaceholder);
        EXPECT_TRUE(sizeErrorMockState.lstatCalled);
        EXPECT_TRUE(sizeErrorMockState.getxattrCalled);
        EXPECT_THAT(sizeErrorMockState.getxattrSizes, ElementsAre(0));

        ResetXattrMock();
        auto &readErrorMockState = GetXattrMockState();
        readErrorMockState.getxattrSizeRet = 1;
        readErrorMockState.getxattrValueRet = MOCK_XATTR_RET_FAIL;
        readErrorMockState.getxattrValueErrno = EIO;
        ret = QueryPlaceholderByXattr(MNT_FILE_PATH, isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN);
        EXPECT_TRUE(readErrorMockState.lstatCalled);
        EXPECT_TRUE(readErrorMockState.getxattrCalled);
        EXPECT_THAT(readErrorMockState.getxattrSizes, ElementsAre(0, 1));

        ResetXattrMock();
        errno = 0;
        ssize_t xattrRet = getxattr(MNT_FILE_PATH.c_str(), PLACEHOLDER_XATTR_KEY.c_str(), nullptr, 1);
        auto &invalidBufferMockState = GetXattrMockState();
        EXPECT_EQ(xattrRet, MOCK_XATTR_RET_FAIL);
        EXPECT_EQ(errno, EFAULT);
        EXPECT_TRUE(invalidBufferMockState.getxattrCalled);
        EXPECT_THAT(invalidBufferMockState.getxattrSizes, ElementsAre(1));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest003 failed";
    }
    GTEST_LOG_(INFO) << "QueryPlaceholderByXattrTest003 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest001
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile returns true when placeholder xattr exists
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest001 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.userId = TEST_USER_ID;
        xattrMockState.getxattrValue = {'1'};
        xattrMockState.getxattrSizeRet = static_cast<ssize_t>(xattrMockState.getxattrValue.size());
        xattrMockState.getxattrValueRet = static_cast<ssize_t>(xattrMockState.getxattrValue.size());

        bool isPlaceholder = false;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
        EXPECT_TRUE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.lstatCalled);
        EXPECT_TRUE(xattrMockState.getxattrCalled);
        EXPECT_EQ(xattrMockState.lstatPath, MNT_FILE_PATH);
        EXPECT_EQ(xattrMockState.getxattrPath, MNT_FILE_PATH);
        EXPECT_EQ(xattrMockState.getxattrName, PLACEHOLDER_XATTR_KEY);
        EXPECT_THAT(xattrMockState.getxattrSizes, ElementsAre(0, 1));
        EXPECT_TRUE(xattrMockState.permissionChecked);
        EXPECT_TRUE(xattrMockState.systemAppChecked);
        EXPECT_EQ(xattrMockState.permissionName, PERM_CLOUD_DISK_SERVICE);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest001 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest002
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile maps missing xattr as an error
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest002 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.userId = TEST_USER_ID;
        xattrMockState.getxattrSizeRet = MOCK_XATTR_RET_FAIL;
        xattrMockState.getxattrSizeErrno = ENODATA;

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.lstatCalled);
        EXPECT_TRUE(xattrMockState.getxattrCalled);
        EXPECT_EQ(xattrMockState.lstatPath, MNT_FILE_PATH);
        EXPECT_EQ(xattrMockState.getxattrPath, MNT_FILE_PATH);
        EXPECT_THAT(xattrMockState.getxattrSizes, ElementsAre(0));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest002 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest002 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest003
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile maps lstat errors before xattr query
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest003 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.userId = TEST_USER_ID;
        xattrMockState.lstatRet = -1;
        xattrMockState.lstatErrno = ENOENT;

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.lstatCalled);
        EXPECT_EQ(xattrMockState.lstatPath, MNT_FILE_PATH);
        EXPECT_FALSE(xattrMockState.getxattrCalled);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest003 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest003 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest004
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile rejects paths outside sync folder before xattr query
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest004 start";
    try {
        string filePath = "/storage/Users/currentUser/sync-other/a.txt";
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(filePath);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.userId = TEST_USER_ID;
        xattrMockState.getxattrValue = {'1'};
        xattrMockState.getxattrSizeRet = static_cast<ssize_t>(xattrMockState.getxattrValue.size());
        xattrMockState.getxattrValueRet = static_cast<ssize_t>(xattrMockState.getxattrValue.size());

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.realpathCalled);
        EXPECT_FALSE(xattrMockState.lstatCalled);
        EXPECT_FALSE(xattrMockState.getxattrCalled);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest004 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest004 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest005
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile rejects non-regular files before xattr query
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest005 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_SYNC_FOLDER);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.userId = TEST_USER_ID;
        xattrMockState.lstatMode = TEST_DIRECTORY_MODE;
        xattrMockState.getxattrValue = {'1'};
        xattrMockState.getxattrSizeRet = static_cast<ssize_t>(xattrMockState.getxattrValue.size());
        xattrMockState.getxattrValueRet = static_cast<ssize_t>(xattrMockState.getxattrValue.size());

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.lstatCalled);
        EXPECT_EQ(xattrMockState.lstatPath, MNT_SYNC_FOLDER);
        EXPECT_FALSE(xattrMockState.getxattrCalled);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest005 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest005 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest006
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile uses realpath before boundary and xattr checks
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest006 start";
    try {
        std::string syncFolder = "/storage/Users/currentUser/sync/../sync";
        std::string filePath = "/storage/Users/currentUser/sync/../sync/a.txt";
        std::string mntSyncFolder = "/mnt/hmdfs/100/account/device_view/local/files/Docs/sync/../sync";
        std::string mntFilePath = "/mnt/hmdfs/100/account/device_view/local/files/Docs/sync/../sync/a.txt";
        CloudDisk_SyncFolderPath syncFolderPath = {const_cast<char *>(syncFolder.c_str()), syncFolder.length()};
        CloudDisk_PathInfo path = {const_cast<char *>(filePath.c_str()), filePath.length()};
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.userId = TEST_USER_ID;
        xattrMockState.realpathResults[mntSyncFolder] = MNT_SYNC_FOLDER;
        xattrMockState.realpathResults[mntFilePath] = MNT_FILE_PATH;
        xattrMockState.getxattrValue = {'1'};
        xattrMockState.getxattrSizeRet = static_cast<ssize_t>(xattrMockState.getxattrValue.size());
        xattrMockState.getxattrValueRet = static_cast<ssize_t>(xattrMockState.getxattrValue.size());

        bool isPlaceholder = false;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
        EXPECT_TRUE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.realpathCalled);
        ASSERT_EQ(xattrMockState.realpathPaths.size(), 2);
        EXPECT_EQ(xattrMockState.realpathPaths[0], mntSyncFolder);
        EXPECT_EQ(xattrMockState.realpathPaths[1], mntFilePath);
        EXPECT_EQ(xattrMockState.lstatPath, MNT_FILE_PATH);
        EXPECT_EQ(xattrMockState.getxattrPath, MNT_FILE_PATH);
        EXPECT_THAT(xattrMockState.getxattrSizes, ElementsAre(0, 1));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest006 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest006 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest007
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile checks caller permission before path conversion
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest007 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.permissionGranted = false;

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.permissionChecked);
        EXPECT_EQ(xattrMockState.permissionName, PERM_CLOUD_DISK_SERVICE);
        EXPECT_FALSE(xattrMockState.systemAppChecked);
        EXPECT_FALSE(xattrMockState.realpathCalled);
        EXPECT_FALSE(xattrMockState.lstatCalled);
        EXPECT_FALSE(xattrMockState.getxattrCalled);

        ResetXattrMock();
        auto &systemAppMockState = GetXattrMockState();
        systemAppMockState.systemApp = false;

        isPlaceholder = true;
        ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(systemAppMockState.permissionChecked);
        EXPECT_TRUE(systemAppMockState.systemAppChecked);
        EXPECT_FALSE(systemAppMockState.realpathCalled);
        EXPECT_FALSE(systemAppMockState.lstatCalled);
        EXPECT_FALSE(systemAppMockState.getxattrCalled);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest007 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest007 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest008
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile rejects invalid arguments before permission checks
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest008 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();

        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, nullptr);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_FALSE(xattrMockState.permissionChecked);
        EXPECT_FALSE(xattrMockState.systemAppChecked);
        EXPECT_FALSE(xattrMockState.realpathCalled);
        EXPECT_FALSE(xattrMockState.lstatCalled);
        EXPECT_FALSE(xattrMockState.getxattrCalled);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest008 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest008 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest009
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile rejects invalid sandbox prefixes before file probing
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest009 start";
    try {
        std::string syncFolder = "/invalid/currentUser/sync";
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(syncFolder);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.permissionChecked);
        EXPECT_TRUE(xattrMockState.systemAppChecked);
        EXPECT_FALSE(xattrMockState.realpathCalled);
        EXPECT_FALSE(xattrMockState.lstatCalled);
        EXPECT_FALSE(xattrMockState.getxattrCalled);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest009 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest009 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest010
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile maps file path realpath failure before lstat
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest010 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.realpathFail = true;
        xattrMockState.realpathFailPath = MNT_FILE_PATH;

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.realpathCalled);
        ASSERT_EQ(xattrMockState.realpathPaths.size(), 2);
        EXPECT_EQ(xattrMockState.realpathPaths[0], MNT_SYNC_FOLDER);
        EXPECT_EQ(xattrMockState.realpathPaths[1], MNT_FILE_PATH);
        EXPECT_FALSE(xattrMockState.lstatCalled);
        EXPECT_FALSE(xattrMockState.getxattrCalled);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest010 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest010 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest011
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile returns false when placeholder xattr length is not one
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest011 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.getxattrValue = {'1', '1'};
        xattrMockState.getxattrSizeRet = static_cast<ssize_t>(xattrMockState.getxattrValue.size());
        xattrMockState.getxattrValueRet = static_cast<ssize_t>(xattrMockState.getxattrValue.size());

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.lstatCalled);
        EXPECT_TRUE(xattrMockState.getxattrCalled);
        EXPECT_EQ(xattrMockState.getxattrPath, MNT_FILE_PATH);
        EXPECT_THAT(xattrMockState.getxattrSizes, ElementsAre(0, 2));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest011 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest011 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest012
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile maps getxattr size query errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest012 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.getxattrSizeRet = MOCK_XATTR_RET_FAIL;
        xattrMockState.getxattrSizeErrno = EACCES;

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_PERMISSION_DENIED);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.lstatCalled);
        EXPECT_TRUE(xattrMockState.getxattrCalled);
        EXPECT_THAT(xattrMockState.getxattrSizes, ElementsAre(0));

        ResetXattrMock();
        auto &notSupportedMockState = GetXattrMockState();
        notSupportedMockState.getxattrSizeRet = MOCK_XATTR_RET_FAIL;
        notSupportedMockState.getxattrSizeErrno = ENOTSUP;

        isPlaceholder = true;
        ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(notSupportedMockState.lstatCalled);
        EXPECT_TRUE(notSupportedMockState.getxattrCalled);
        EXPECT_THAT(notSupportedMockState.getxattrSizes, ElementsAre(0));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest012 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest012 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest013
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile maps getxattr read transient errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest013 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.getxattrSizeRet = 1;
        xattrMockState.getxattrValueRet = MOCK_XATTR_RET_FAIL;
        xattrMockState.getxattrValueErrno = EIO;

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_TRY_AGAIN);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.lstatCalled);
        EXPECT_TRUE(xattrMockState.getxattrCalled);
        EXPECT_THAT(xattrMockState.getxattrSizes, ElementsAre(0, 1));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest013 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest013 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest014
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile rejects invalid path info before permission or file access
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest014 start";
    try {
        CloudDisk_SyncFolderPath invalidSyncFolderPath = {nullptr, 1};
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(invalidSyncFolderPath, path, &isPlaceholder);

        auto &invalidSyncMockState = GetXattrMockState();
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_FALSE(invalidSyncMockState.permissionChecked);
        EXPECT_FALSE(invalidSyncMockState.systemAppChecked);
        EXPECT_FALSE(invalidSyncMockState.realpathCalled);
        EXPECT_FALSE(invalidSyncMockState.lstatCalled);
        EXPECT_FALSE(invalidSyncMockState.getxattrCalled);

        ResetXattrMock();
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo invalidPath = {nullptr, 1};

        isPlaceholder = true;
        ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, invalidPath, &isPlaceholder);

        auto &invalidPathMockState = GetXattrMockState();
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_FALSE(invalidPathMockState.permissionChecked);
        EXPECT_FALSE(invalidPathMockState.systemAppChecked);
        EXPECT_FALSE(invalidPathMockState.realpathCalled);
        EXPECT_FALSE(invalidPathMockState.lstatCalled);
        EXPECT_FALSE(invalidPathMockState.getxattrCalled);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest014 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest014 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest015
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile rejects real paths outside the HMDFS mount prefix
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest015 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.userId = TEST_USER_ID;
        xattrMockState.realpathResults[MNT_FILE_PATH] = "/tmp/outside.txt";

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.permissionChecked);
        EXPECT_TRUE(xattrMockState.systemAppChecked);
        EXPECT_TRUE(xattrMockState.realpathCalled);
        EXPECT_FALSE(xattrMockState.lstatCalled);
        EXPECT_FALSE(xattrMockState.getxattrCalled);
        EXPECT_THAT(xattrMockState.realpathPaths, ElementsAre(MNT_SYNC_FOLDER, MNT_FILE_PATH));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest015 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest015 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest016
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile rejects an empty realpath result
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest016, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest016 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(SANDBOX_FILE_PATH);
        auto &xattrMockState = GetXattrMockState();
        xattrMockState.userId = TEST_USER_ID;
        xattrMockState.realpathResults[MNT_FILE_PATH] = "";

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(xattrMockState.permissionChecked);
        EXPECT_TRUE(xattrMockState.systemAppChecked);
        EXPECT_TRUE(xattrMockState.realpathCalled);
        EXPECT_FALSE(xattrMockState.lstatCalled);
        EXPECT_FALSE(xattrMockState.getxattrCalled);
        EXPECT_THAT(xattrMockState.realpathPaths, ElementsAre(MNT_SYNC_FOLDER, MNT_FILE_PATH));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest016 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest016 end";
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
