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

#include <algorithm>
#include <cerrno>
#include <securec.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string>
#include <vector>

#include "oh_cloud_disk_manager.h"
#include "oh_cloud_disk_utils.h"
#include "cloud_disk_service_manager_mock.h"

// Test-local syscall replacements for OH_CloudDisk_IsPlaceholderFile xattr branches.
namespace {
constexpr int MOCK_STAT_RET_SUCCESS = 0;
constexpr ssize_t MOCK_XATTR_RET_FAIL = -1;

struct XattrMockState {
    int lstatRet = MOCK_STAT_RET_SUCCESS;
    int lstatErrno = 0;
    ssize_t getxattrRet = MOCK_XATTR_RET_FAIL;
    int getxattrErrno = ENODATA;
    std::vector<char> getxattrValue;
    bool lstatCalled = false;
    bool getxattrCalled = false;
    std::string lstatPath;
    std::string getxattrPath;
    std::string getxattrName;
};

XattrMockState g_xattrMockState;

void ResetXattrMock()
{
    g_xattrMockState = XattrMockState();
}
} // namespace

extern "C" int lstat(const char *path, struct stat *buf)
{
    g_xattrMockState.lstatCalled = true;
    g_xattrMockState.lstatPath = path == nullptr ? "" : path;
    if (g_xattrMockState.lstatRet != 0) {
        errno = g_xattrMockState.lstatErrno;
        return g_xattrMockState.lstatRet;
    }
    if (buf != nullptr) {
        *buf = {};
        buf->st_mode = S_IFREG | 0644;
    }
    return 0;
}

extern "C" ssize_t getxattr(const char *path, const char *name, void *value, size_t size)
{
    g_xattrMockState.getxattrCalled = true;
    g_xattrMockState.getxattrPath = path == nullptr ? "" : path;
    g_xattrMockState.getxattrName = name == nullptr ? "" : name;
    if (g_xattrMockState.getxattrRet < 0) {
        errno = g_xattrMockState.getxattrErrno;
        return g_xattrMockState.getxattrRet;
    }
    if (value != nullptr && size > 0 && !g_xattrMockState.getxattrValue.empty()) {
        size_t copySize = std::min(size, g_xattrMockState.getxattrValue.size());
        errno_t ret = memcpy_s(value, size, g_xattrMockState.getxattrValue.data(), copySize);
        if (ret != EOK) {
            errno = EINVAL;
            return MOCK_XATTR_RET_FAIL;
        }
    }
    return g_xattrMockState.getxattrRet;
}

namespace OHOS::FileManagement::CloudDiskService::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

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
 * @tc.desc: Verify the ConvertXattrErrno function
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
        EXPECT_EQ(ConvertXattrErrno(ENOTSUP), CloudDisk_ErrorCode::CLOUD_DISK_NOT_SUPPORTED);
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
 * @tc.name: IsPlaceholderFileXattrTest001
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile returns true when placeholder xattr exists
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest001 start";
    try {
        std::string syncFolder = "/mock/sync";
        std::string filePath = "/mock/sync/a.txt";
        CloudDisk_SyncFolderPath syncFolderPath = {const_cast<char *>(syncFolder.c_str()), syncFolder.length()};
        CloudDisk_PathInfo path = {const_cast<char *>(filePath.c_str()), filePath.length()};
        g_xattrMockState.getxattrValue = {'1'};
        g_xattrMockState.getxattrRet = static_cast<ssize_t>(g_xattrMockState.getxattrValue.size());

        bool isPlaceholder = false;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
        EXPECT_TRUE(isPlaceholder);
        EXPECT_TRUE(g_xattrMockState.lstatCalled);
        EXPECT_TRUE(g_xattrMockState.getxattrCalled);
        EXPECT_EQ(g_xattrMockState.lstatPath, filePath);
        EXPECT_EQ(g_xattrMockState.getxattrPath, filePath);
        EXPECT_EQ(g_xattrMockState.getxattrName, GetPlaceholderXattrKey());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest001 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest001 end";
}

/**
 * @tc.name: IsPlaceholderFileXattrTest002
 * @tc.desc: Verify OH_CloudDisk_IsPlaceholderFile treats missing xattr as a normal file
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskManagerTest, IsPlaceholderFileXattrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest002 start";
    try {
        std::string syncFolder = "/mock/sync";
        std::string filePath = "/mock/sync/a.txt";
        CloudDisk_SyncFolderPath syncFolderPath = {const_cast<char *>(syncFolder.c_str()), syncFolder.length()};
        CloudDisk_PathInfo path = {const_cast<char *>(filePath.c_str()), filePath.length()};
        g_xattrMockState.getxattrRet = MOCK_XATTR_RET_FAIL;
        g_xattrMockState.getxattrErrno = ENODATA;

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_OK);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(g_xattrMockState.lstatCalled);
        EXPECT_TRUE(g_xattrMockState.getxattrCalled);
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
        std::string syncFolder = "/mock/sync";
        std::string filePath = "/mock/sync/a.txt";
        CloudDisk_SyncFolderPath syncFolderPath = {const_cast<char *>(syncFolder.c_str()), syncFolder.length()};
        CloudDisk_PathInfo path = {const_cast<char *>(filePath.c_str()), filePath.length()};
        g_xattrMockState.lstatRet = -1;
        g_xattrMockState.lstatErrno = ENOENT;

        bool isPlaceholder = true;
        CloudDisk_ErrorCode ret = OH_CloudDisk_IsPlaceholderFile(syncFolderPath, path, &isPlaceholder);

        EXPECT_EQ(ret, CloudDisk_ErrorCode::CLOUD_DISK_SYNC_FOLDER_PATH_NOT_EXIST);
        EXPECT_FALSE(isPlaceholder);
        EXPECT_TRUE(g_xattrMockState.lstatCalled);
        EXPECT_FALSE(g_xattrMockState.getxattrCalled);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest003 failed";
    }
    GTEST_LOG_(INFO) << "IsPlaceholderFileXattrTest003 end";
}
} // namespace OHOS::FileManagement::CloudDiskService::Test
