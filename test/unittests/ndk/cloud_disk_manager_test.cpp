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

#include <securec.h>
#include <string>

#include "oh_cloud_disk_manager.h"
#include "oh_cloud_disk_utils.h"
#include "cloud_disk_service_manager_mock.h"

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

namespace {
const string SANDBOX_SYNC_FOLDER = "/storage/Users/currentUser/sync";
const string RELATIVE_FILE_PATH = "a.txt";

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
 * @tc.name: IsPlaceholderFileIpcTest001
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile delegates to cloud disk service
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileIpcTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest001 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(RELATIVE_FILE_PATH);
        auto &mock = CloudDiskServiceManagerMock::GetInstance();
        EXPECT_CALL(mock, IsPlaceholderFile(SANDBOX_SYNC_FOLDER, RELATIVE_FILE_PATH, _))
            .WillOnce(DoAll(SetArgReferee<2>(true), Return(E_OK)));

        bool isPlaceholder = false;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
        EXPECT_TRUE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest001 end";
}

/**
 * @tc.name: IsPlaceholderFileIpcTest002
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile returns non-placeholder result
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileIpcTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest002 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(RELATIVE_FILE_PATH);
        auto &mock = CloudDiskServiceManagerMock::GetInstance();
        EXPECT_CALL(mock, IsPlaceholderFile(SANDBOX_SYNC_FOLDER, RELATIVE_FILE_PATH, _))
            .WillOnce(DoAll(SetArgReferee<2>(false), Return(E_OK)));

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest002 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest002 end";
}

/**
 * @tc.name: IsPlaceholderFileIpcTest003
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile maps service errors
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileIpcTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest003 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(RELATIVE_FILE_PATH);
        auto &mock = CloudDiskServiceManagerMock::GetInstance();
        EXPECT_CALL(mock, IsPlaceholderFile(SANDBOX_SYNC_FOLDER, RELATIVE_FILE_PATH, _))
            .WillOnce(Return(34400002));

        bool isPlaceholder = false;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_SYNC_FOLDER_PATH_UNAUTHORIZED);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest003 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest003 end";
}

/**
 * @tc.name: IsPlaceholderFileIpcTest004
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile rejects null result pointer before IPC
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileIpcTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest004 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo path = ToPathInfo(RELATIVE_FILE_PATH);

        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, nullptr);
        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest004 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest004 end";
}

/**
 * @tc.name: IsPlaceholderFileIpcTest005
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile rejects invalid path before IPC
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileIpcTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest005 start";
    try {
        CloudDisk_PathInfo path = ToPathInfo(RELATIVE_FILE_PATH);
        CloudDisk_SyncFolderPath invalidSyncFolderPath = {nullptr, 1};
        bool isPlaceholder = true;

        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(invalidSyncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest005 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest005 end";
}

/**
 * @tc.name: IsPlaceholderFileIpcTest006
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile rejects invalid file path before IPC
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileIpcTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest006 start";
    try {
        CloudDisk_SyncFolderPath syncFolderPath = ToPathInfo(SANDBOX_SYNC_FOLDER);
        CloudDisk_PathInfo invalidPath = {nullptr, 1};
        bool isPlaceholder = true;

        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, invalidPath, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_INVALID_ARG);
        EXPECT_FALSE(isPlaceholder);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest006 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileIpcTest006 end";
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
