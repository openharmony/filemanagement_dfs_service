/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <copy/file_copy_manager.h>
#include <copy/file_size_utils.h>
#include <gtest/gtest.h>
#include <securec.h>

#include "dfs_error.h"

#include "directory_ex.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class FileSizeUtilsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    uint64_t process = 0;
    uint64_t fileSize = 0;
    using callBack = std::function<void(uint64_t processSize, uint64_t totalFileSize)>;
    callBack listener = [&](uint64_t processSize, uint64_t totalFileSize) {
        process = processSize;
        fileSize = totalFileSize;
    };
};

void FileSizeUtilsTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileSizeUtilsTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileSizeUtilsTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileSizeUtilsTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
* @tc.name: FileSizeUtils_0001
* @tc.desc: The execution of the GetSize failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileSizeUtilsTest, FileSizeUtils_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileSizeUtils_0001 Start";
    string srcuri = "file://docs/storage/media/100/local/files/Docs/aa/";
    bool isSrcUri = true;
    auto ptr = std::make_shared<FileSizeUtils>();
    int32_t ret = ptr->GetSize(srcuri, isSrcUri, fileSize);
    EXPECT_EQ(ret, 2);

    ret = ptr->GetSize("", isSrcUri, fileSize);
    EXPECT_EQ(ret, ERR_BAD_VALUE);

    srcuri = "/data/test/FileSizeUtils_0001";
    if (!ForceCreateDirectory(srcuri)) {
        GTEST_LOG_(INFO) << "GetGetDirSize_0001 create dir err:" << srcuri;
    }
    ret = ptr->GetSize(srcuri, isSrcUri, fileSize);
    EXPECT_EQ(ret, E_OK);

    std::string testFile = srcuri + "/1.txt"; // 1: file name
    int fd = open(testFile.c_str(), O_RDWR | O_CREAT);
    if (fd < 0) {
        GTEST_LOG_(INFO) << "GetGetDirSize_0001 create file err" << testFile;
    }
    ret = ptr->GetSize(testFile, isSrcUri, fileSize);
    EXPECT_EQ(ret, E_OK);
    if (!ForceCreateDirectory(srcuri)) {
        GTEST_LOG_(INFO) << "GetGetDirSize_0001 create dir err:" << srcuri;
    }
    GTEST_LOG_(INFO) << "FileSizeUtils_0001 End";
}

/**
* @tc.name: IsDirectory_0001
* @tc.desc: Judge is directory
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileSizeUtilsTest, IsDirectory_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsDirectory_0001 Start";
    string srcuri = "/data/test/IsDirectory_0001";
    bool isSrcUri = true;
    bool isDirectory = false;
    auto ptr = std::make_shared<FileSizeUtils>();
    int32_t ret = ptr->IsDirectory("", isSrcUri, isDirectory);
    if (!ForceCreateDirectory(srcuri)) {
        GTEST_LOG_(INFO) << "GetGetDirSize_0001 create dir err:" << srcuri;
    }
    EXPECT_EQ(ret, ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "IsDirectory_0001 End";
}

/**
* @tc.name: FilterFunc_0001
* @tc.desc: filter file
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileSizeUtilsTest, FilterFunc_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FilterFunc_0001 Start";
    dirent dir;
    strcpy_s(dir.d_name, sizeof(dir.d_name), ".");
    auto ptr = std::make_shared<FileSizeUtils>();
    int ret = ptr->FilterFunc(&dir);
    EXPECT_EQ(ret, 0); // 0: dismatch

    strcpy_s(dir.d_name, sizeof(dir.d_name), "..");
    ret = ptr->FilterFunc(&dir);
    EXPECT_EQ(ret, 0); // 0: dismatch

    strcpy_s(dir.d_name, sizeof(dir.d_name), "test");
    ret = ptr->FilterFunc(&dir);
    EXPECT_EQ(ret, 1); // 1: match
    GTEST_LOG_(INFO) << "FilterFunc_0001 End";
}

/**
* @tc.name: FilterFunc_0001
* @tc.desc: get real path
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileSizeUtilsTest, GetRealPath_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRealPath_0001 Start";
    auto ptr = std::make_shared<FileSizeUtils>();
    std::string path = ".";
    std::string retStr = ptr->GetRealPath(path);
    EXPECT_EQ(retStr, "");

    path = "..";
    retStr = ptr->GetRealPath(path);
    EXPECT_EQ(retStr, "");
    GTEST_LOG_(INFO) << "GetRealPath_0001 End";
}

/**
* @tc.name: GetDirSize_0001
* @tc.desc: get dir size
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileSizeUtilsTest, GetGetDirSize_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetGetDirSize_0001 Start";
    std::string path = "/data/test/GetGetDirSize_0001";
    uint64_t size = 0;
    if (!ForceCreateDirectory(path)) {
        GTEST_LOG_(INFO) << "GetGetDirSize_0001 create dir err:" << path;
    }
    std::string tmpDir = path + "/dir";
    if (!ForceCreateDirectory(tmpDir)) {
        GTEST_LOG_(INFO) << "GetGetDirSize_0001 create dir err:" << tmpDir;
    }
    std::string testFile = path + "/1.txt"; // 1: file name
    int fd = open(testFile.c_str(), O_RDWR | O_CREAT);
    if (fd < 0) {
        GTEST_LOG_(INFO) << "GetGetDirSize_0001 create file err" << testFile;
    }
    std::string testLink = path + "/2"; // 2: link file name
    if (symlink(testFile.c_str(), testLink.c_str())) {
        GTEST_LOG_(INFO) << "GetGetDirSize_0001 create linkfile err" << testLink;
    }
    auto ptr = std::make_shared<FileSizeUtils>();
    int32_t ret = ptr->GetDirSize(path, size);
    EXPECT_EQ(ret, E_OK);
    if (!ForceRemoveDirectory(path)) {
        GTEST_LOG_(INFO) << "GetGetDirSize_0001 create dir err";
    }
    GTEST_LOG_(INFO) << "GetGetDirSize_0001 End";
}

/**
* @tc.name: IsFile_0001
* @tc.desc: Judge is file
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileSizeUtilsTest, IsFile_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsFile_0001 Start";
    auto ptr = std::make_shared<FileSizeUtils>();
    bool isFile = false;
    int32_t ret = ptr->IsFile("", isFile);
    EXPECT_EQ(ret, 2); // 2: file not found
    EXPECT_EQ(isFile, false);
    GTEST_LOG_(INFO) << "IsFile_0001 End";
}
}
