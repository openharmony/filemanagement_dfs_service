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

#include "utils_directory.h"

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

#include "directory_ex.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Utils {
using namespace testing;
using namespace testing::ext;
using namespace std;

class UtilsDirectoryTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void UtilsDirectoryTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void UtilsDirectoryTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void UtilsDirectoryTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void UtilsDirectoryTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: ForceCreateDirectoryTest001
 * @tc.desc: Verify the ForceCreateDirectory function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, ForceCreateDirectoryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest001 Start";
    try {
        string path = "/data/tdd";
        ForceCreateDirectory(path);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " ForceCreateDirectoryTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest001 End";
}

/**
 * @tc.name: ForceCreateDirectoryTest002
 * @tc.desc: Verify the ForceCreateDirectoryTest002 function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, ForceCreateDirectoryTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest002 Start";
    try {
        string path = "/data/tdd";
        mode_t mode = (S_IRWXU | S_IRWXG | S_IRWXO); // 00777
        ForceCreateDirectory(path, mode);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ForceCreateDirectoryTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest002 End";
}

/**
 * @tc.name: ForceCreateDirectoryTest003
 * @tc.desc: Verify the ForceCreateDirectoryTest003 function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, ForceCreateDirectoryTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest003 Start";
    try {
        string path = "/data/tdd";
        mode_t mode = (S_IRWXU | S_IRWXG | S_IRWXO);
        uid_t uid = UID_ROOT;
        gid_t gid = 0; // root
        ForceCreateDirectory(path, mode, uid, gid);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ForceCreateDirectoryTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "ForceCreateDirectoryTest003 End";
}

/**
 * @tc.name: ForceRemoveDirectoryTest001
 * @tc.desc: Verify the ForceRemoveDirectory function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, ForceRemoveDirectoryTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForceRemoveDirectory001 Start";
    try {
        string path = "/data/tdd";
        ForceRemoveDirectory(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ForceRemoveDirectoryTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "ForceRemoveDirectoryTest001 End";
}

/**
 * @tc.name: ForceRemoveDirectoryDeepFirstTest001Test001
 * @tc.desc: Verify the ForceRemoveDirectoryDeepFirst function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, ForceRemoveDirectoryDeepFirstTest001Test001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForceRemoveDirectoryDeepFirstTest001Test001 Start";
    try {
        string path = "/data/tdd";
        ForceRemoveDirectory(path);
        EXPECT_FALSE(ForceRemoveDirectoryDeepFirst(path));

        mode_t mode = (S_IRWXU | S_IRWXG | S_IRWXO);
        uid_t uid = UID_ROOT;
        gid_t gid = 0; // root
        ForceCreateDirectory(path, mode, uid, gid);
        
        string subPath = path + "/sub";
        ForceCreateDirectory(path, mode, uid, gid);

        string fileStr = path + "/test.txt";
        int32_t fd = open(fileStr.c_str(), O_RDWR | O_CREAT);
        ASSERT_TRUE(fd != -1) << "ForceIsFolderTest001 Create File Failed!";
        close(fd);

        EXPECT_TRUE(ForceRemoveDirectoryDeepFirst(path));
        EXPECT_NE(access(fileStr.c_str(), F_OK), 0);
        EXPECT_NE(access(subPath.c_str(), F_OK), 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ForceRemoveDirectoryDeepFirstTest001Test001 ERROR";
    }
    GTEST_LOG_(INFO) << "ForceRemoveDirectoryDeepFirstTest001Test001 End";
}

/**
 * @tc.name: IsFolderTest001
 * @tc.desc: Verify the IsFolder & IsFile function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, IsFolderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsFolderTest001 Start";
    try {
        string path;
        EXPECT_FALSE(IsFolder(path));

        path = "/data/tdd";
        ForceRemoveDirectory(path);
        EXPECT_FALSE(IsFolder(path));

        mode_t mode = (S_IRWXU | S_IRWXG | S_IRWXO);
        uid_t uid = UID_ROOT;
        gid_t gid = 0; // root
        ForceCreateDirectory(path, mode, uid, gid);
        EXPECT_TRUE(IsFolder(path));
        
        string fileStr = path + "/test.txt";
        int32_t fd = open(fileStr.c_str(), O_RDWR | O_CREAT);
        ASSERT_TRUE(fd != -1) << "IsFolderTest001 Create File Failed!";
        close(fd);
        EXPECT_TRUE(IsFile(fileStr));
        
        auto ret = unlink(fileStr.c_str());
        ASSERT_TRUE(ret != -1) << "Failed to delete file in IsFolderTest001! " << errno;
        ForceRemoveDirectory(path);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsFolderTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "IsFolderTest001 End";
}

/**
 * @tc.name: GetFilePathTest001
 * @tc.desc: Verify the GetFilePath function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, GetFilePathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFilePathTest001 Start";
    try {
        string path = "/data/tdd";
        ForceRemoveDirectory(path);

        mode_t mode = (S_IRWXU | S_IRWXG | S_IRWXO);
        uid_t uid = UID_ROOT;
        gid_t gid = 0; // root
        ForceCreateDirectory(path, mode, uid, gid);
        
        string subPath = path + "/sub";
        ForceCreateDirectory(subPath, mode, uid, gid);

        string fileStr1 = path + "/test.txt";
        string fileStr2 = subPath + "/1.txt";
        string fileStr3 = subPath + "/2.txt";
        int32_t fd = open(fileStr1.c_str(), O_RDWR | O_CREAT);
        ASSERT_TRUE(fd != -1) << "GetFilePathTest001 Create fileStr1 Failed!";
        close(fd);
        fd = open(fileStr2.c_str(), O_RDWR | O_CREAT);
        ASSERT_TRUE(fd != -1) << "GetFilePathTest001 Create fileStr2 Failed!";
        close(fd);
        fd = open(fileStr3.c_str(), O_RDWR | O_CREAT);
        ASSERT_TRUE(fd != -1) << "GetFilePathTest001 Create fileStr3 Failed!";
        close(fd);
        
        auto rlt = GetFilePath(fileStr1);
        std::vector<std::string> ept;
        ept.emplace_back(fileStr1);
        EXPECT_EQ(rlt, ept);
        
        rlt.clear();
        rlt = GetFilePath(path);
        EXPECT_EQ(rlt.size(), 3);

        EXPECT_NE(find(rlt.begin(), rlt.end(), fileStr1), rlt.end());
        EXPECT_NE(find(rlt.begin(), rlt.end(), fileStr2), rlt.end());
        EXPECT_NE(find(rlt.begin(), rlt.end(), fileStr3), rlt.end());
        EXPECT_TRUE(ForceRemoveDirectoryDeepFirst(path));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " GetFilePathTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetFilePathTest001 End";
}

/**
 * @tc.name: ChangeOwnerRecursiveTest001
 * @tc.desc: Verify the ChangeOwnerRecursive function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, ChangeOwnerRecursiveTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ChangeOwnerRecursiveTest001 Start";
    try {
        string path = "/data/tdd";
        ForceRemoveDirectory(path);

        mode_t mode = (S_IRWXU | S_IRWXG | S_IRWXO);
        uid_t uid = UID_ROOT;
        gid_t gid = 0; // root
        ForceCreateDirectory(path, mode, uid, gid);
        
        string subPath = path + "/sub";
        ForceCreateDirectory(subPath, mode, uid, gid);

        string fileStr = path + "/test.txt";
        int32_t fd = open(fileStr.c_str(), O_RDWR | O_CREAT);
        ASSERT_TRUE(fd != -1) << "ChangeOwnerRecursiveTest001 Create fileStr Failed!";
        close(fd);

        EXPECT_EQ(ChangeOwnerRecursive(fileStr, UID_SYSTEM, 1), -1);
        EXPECT_EQ(ChangeOwnerRecursive(path, UID_SYSTEM, 1), 0);
        EXPECT_TRUE(ForceRemoveDirectoryDeepFirst(path));
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " ChangeOwnerRecursiveTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "ChangeOwnerRecursiveTest001 End";
}

/**
 * @tc.name: IsInt32Test001
 * @tc.desc: Verify the IsInt32 function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, IsInt32Test001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsInt32Test001 Start";
    try {
        nlohmann::json jsonObj;
        string key = "test";
        EXPECT_EQ(IsInt32(jsonObj, key), false);

        jsonObj["test"] = 20;
        EXPECT_EQ(IsInt32(jsonObj, key), true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " IsInt32Test001 ERROR";
    }
    GTEST_LOG_(INFO) << "IsInt32Test001 End";
}

/**
 * @tc.name: SysEventWriteTest001
 * @tc.desc: Verify the SysEventWrite function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, SysEventWriteTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SysEventWriteTest001 Start";
    try {
        string uid;
        SysEventWrite(uid);
        uid = "test";
        SysEventWrite(uid);
        SysEventFileParse(1000);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << " SysEventWriteTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "SysEventWriteTest001 End";
}

/**
 * @tc.name: IsFilePathInvalid001
 * @tc.desc: Verify the IsFilePathInvalid function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(UtilsDirectoryTest, IsFilePathInvalid001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsFilePathInvalid001 Start";
    try {
        std::string testPath1 = "../test../test1";
        std::string testPath2 = "/../test../test1";
        std::string testPath3 = "test../../test";
        std::string testPath4 = "test../../";
        std::string testPath5 = "test../test../..";
        std::string testPath6 = "/test/..test/..";

        std::string testPath7 = "test";
        std::string testPath8 = "/test/test../test";
        std::string testPath9 = "/test../test../test";
        std::string testPath10 = "/test../test../test../";
        std::string testPath11 = "/test../test../test../..test";

        bool isForbid1 = IsFilePathInvalid(testPath1);
        EXPECT_TRUE(isForbid1);
        bool isForbid2 = IsFilePathInvalid(testPath2);
        EXPECT_TRUE(isForbid2);
        bool isForbid3 = IsFilePathInvalid(testPath3);
        EXPECT_TRUE(isForbid3);
        bool isForbid4 = IsFilePathInvalid(testPath4);
        EXPECT_TRUE(isForbid4);
        bool isForbid5 = IsFilePathInvalid(testPath5);
        EXPECT_TRUE(isForbid5);
        bool isForbid6 = IsFilePathInvalid(testPath6);
        EXPECT_TRUE(isForbid6);

        bool isForbid7 = IsFilePathInvalid(testPath7);
        EXPECT_FALSE(isForbid7);
        bool isForbid8 = IsFilePathInvalid(testPath8);
        EXPECT_FALSE(isForbid8);
        bool isForbid9 = IsFilePathInvalid(testPath9);
        EXPECT_FALSE(isForbid9);
        bool isForbid10 = IsFilePathInvalid(testPath10);
        EXPECT_FALSE(isForbid10);
        bool isForbid11 = IsFilePathInvalid(testPath11);
        EXPECT_FALSE(isForbid11);
        
    } catch (...) {
        GTEST_LOG_(INFO) << " IsFilePathInvalid001 ERROR";
    }
    GTEST_LOG_(INFO) << "IsFilePathInvalid001 End";
}
} // namespace Utils
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS