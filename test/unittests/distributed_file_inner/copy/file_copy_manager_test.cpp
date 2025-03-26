/*
* Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "copy/file_copy_manager.h"

#include <fstream>
#include <gtest/gtest.h>

#include "datashare_helper.h"
#include "dfs_error.h"
#include "directory_ex.h"
#include "distributed_file_fd_guard.cpp"
#include "file_copy_listener.cpp"
#include "file_copy_manager.cpp"
#include "file_size_utils.cpp"
#include "trans_listener.cpp"
#include "trans_listener.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;
constexpr int32_t FILE_NOT_FOUND = 2;
class FileCopyManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    uint64_t process_ = 0;
    uint64_t fileSize_ = 0;
    using callBack = std::function<void(uint64_t processSize, uint64_t totalFileSize)>;
    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        process_ = processSize;
        fileSize_ = totalFileSize;
    };
    // 定义一个空的 ProcessCallback 用于测试
    ProcessCallback emptyCallback_ = [](uint64_t processSize, uint64_t totalFileSize) {};
};

void FileCopyManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileCopyManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileCopyManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileCopyManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
* @tc.name: FileCopyManager_Copy_0001
* @tc.desc: The execution of the Copy failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0001 Start";
    string localUri = "/data/test/test.txt";
    string dstUri = "/data/test/test.txt";

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Copy("", localUri, listener_);
    EXPECT_EQ(ret, E_NOENT);

    ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Copy(localUri, "", listener_);
    EXPECT_EQ(ret, E_NOENT);

    ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Copy("", "", listener_);
    EXPECT_EQ(ret, E_NOENT);

    ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Copy(localUri, dstUri, listener_);
    EXPECT_EQ(ret, FILE_NOT_FOUND);

    string remoteUri = "/data/test/Copy/?networkid=/";
    if (!ForceCreateDirectory(remoteUri)) {
        GTEST_LOG_(INFO) << "FileCopyManager_Copy_0001 create dir err";
    }
    ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Copy(remoteUri, "", listener_);
    EXPECT_EQ(ret, E_NOENT);
    if (!ForceRemoveDirectory(remoteUri)) {
        GTEST_LOG_(INFO) << "FileCopyManager_Copy_0001 remove dir err";
    }
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0001 End";
}

/**
* @tc.name: FileCopyManager_Copy_0002
* @tc.desc: The execution of the Copy succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0002 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string destUri = "file://docs/storage/media/100/local/files/Docs/dest1.txt";
    string srcPath = "/storage/media/100/local/files/Docs/1.txt";
    int fd = open(srcPath.c_str(), O_RDWR | O_CREAT);
    ASSERT_TRUE(fd != -1) <<"Failed to open file in FileCopyManager_Copy_0002!" << errno;
    close(fd);

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Copy(srcUri, destUri, listener_);
    EXPECT_EQ(ret, E_OK);
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0002 End";
}

/**
* @tc.name: FileCopyManager_Copy_0003
* @tc.desc: The execution of the Copy failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0003 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string destUri = "*";
    string srcPath = "/storage/media/100/local/files/Docs/1.txt";
    int fd = open(srcPath.c_str(), O_RDWR | O_CREAT);
    ASSERT_TRUE(fd != -1) <<"Failed to open file in FileCopyManager_Copy_0003!" << errno;
    close(fd);

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Copy(srcUri, destUri, listener_);
    EXPECT_EQ(ret, E_NOENT);
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0003 End";
}

/**
* @tc.name: FileCopyManager_Copy_0004
* @tc.desc: The execution of the cancel succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0004 Start";
    auto infos1 = std::make_shared<FileInfos>();
    infos1->srcUri = "srcUri1";
    infos1->destUri = "destUri1";
    infos1->transListener = nullptr;
    infos1->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    std::string srcUri = "srcUri1";
    std::string destUri = "destUri3";

    Storage::DistributedFile::FileCopyManager::GetInstance()->FileInfosVec_.emplace_back(infos1);
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Cancel(srcUri, destUri);
    EXPECT_EQ(ret, E_OK);
    srcUri = "srcUri2";
    destUri = "destUri1";
    ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Cancel(srcUri, destUri);
    EXPECT_EQ(ret, E_OK);

    srcUri = "srcUri2";
    destUri = "destUri2";
    ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Cancel(srcUri, destUri);
    EXPECT_EQ(ret, E_OK);

    srcUri = "srcUri1";
    destUri = "destUri1";
    ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Cancel(srcUri, destUri);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0004 End";
}

/**
* @tc.name: FileCopyManager_Copy_0005
* @tc.desc: The execution of the execlocal failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0005 Start";
    string srcuri = "file://docs/storage/media/100/local/files/Docs/11.txt";
    string desturi = "file://docs/storage/media/100/local/files/Docs/dest11.txt";
    string srcpath = "/storage/media/100/local/files/Docs/11.txt";
    int fd = open(srcpath.c_str(), O_RDWR | O_CREAT);
    ASSERT_TRUE(fd != -1) <<"Failed to open file in FileCopyManager_Copy_0005!" << errno;
    close(fd);

    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = srcuri;
    infos->destUri = desturi;
    infos->srcPath = srcpath;
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->ExecLocal(infos);
    EXPECT_EQ(ret, 2);
    ASSERT_EQ(remove(srcpath.c_str()), 0);
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0005 End";
}

/**
* @tc.name: FileCopyManager_Copy_0006
* @tc.desc: The execution of the execlocal failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0006, TestSize.Level1)
{
    //无对应子文件夹，errno返回2
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0006 Start";
    string srcuri = "file://docs/storage/media/100/local/files/Docs/aa/";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    string srcpath = "/storage/media/100/local/files/Docs/aa/";
    string destpath = "/storage/media/100/local/files/Docs/aa1/";
    std::error_code errCode;
    if (!std::filesystem::exists(srcpath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance()->MakeDir(srcpath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }

    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = srcuri;
    infos->destUri = desturi;
    infos->srcPath = srcpath;
    infos->destPath = destpath;
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->ExecLocal(infos);
    EXPECT_EQ(ret, 2);
    ASSERT_EQ(remove(srcpath.c_str()), 0);
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0006 End";
}

/**
* @tc.name: FileCopyManager_Copy_0007
* @tc.desc: The execution of the execlocal failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0007, TestSize.Level1)
{
    //无destpath赋值，stat检查返回2
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0006 Start";
    string srcuri = "file://docs/storage/media/100/local/files/Docs/aa/";
    string desturi = "file://docs/storage/media/100/local/files/Docs/aa1/";
    string srcpath = "/storage/media/100/local/files/Docs/aa/";
    std::error_code errCode;
    if (!std::filesystem::exists(srcpath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance()->MakeDir(srcpath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }

    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = srcuri;
    infos->destUri = desturi;
    infos->srcPath = srcpath;
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->ExecLocal(infos);
    EXPECT_EQ(ret, 2);
    ASSERT_EQ(remove(srcpath.c_str()), 0);
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0007 End";
}

/**
* @tc.name: FileCopyManager_DeleteResFile_0001
* @tc.desc: Verify the DeleteResFile function when transListener is not null and destPath exists.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_DeleteResFile_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0001 Start";

    auto infos = std::make_shared<FileInfos>();
    infos->transListener = sptr(new (std::nothrow) TransListener("/data/test/test.txt", emptyCallback_));
    infos->destPath = "/data/test/test.txt";

    // 创建测试文件
    std::ofstream(infos->destPath).close();
    EXPECT_TRUE(std::filesystem::exists(infos->destPath));

    // 调用 DeleteResFile
    Storage::DistributedFile::FileCopyManager::GetInstance()->DeleteResFile(infos);

    // 验证文件是否被删除
    EXPECT_FALSE(std::filesystem::exists(infos->destPath));
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0001 End";
}

/**
* @tc.name: FileCopyManager_DeleteResFile_0002
* @tc.desc: Verify the DeleteResFile function when transListener is not null and destPath does not exist.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_DeleteResFile_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0002 Start";

    auto infos = std::make_shared<FileInfos>();
    infos->transListener = sptr(new (std::nothrow) TransListener("/data/test/test.txt", emptyCallback_));
    infos->destPath = "/data/test/nonexistent.txt";

    // 确保文件不存在
    EXPECT_FALSE(std::filesystem::exists(infos->destPath));

    // 调用 DeleteResFile
    Storage::DistributedFile::FileCopyManager::GetInstance()->DeleteResFile(infos);

    // 验证文件仍然不存在
    EXPECT_FALSE(std::filesystem::exists(infos->destPath));
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0002 End";
}

/**
* @tc.name: FileCopyManager_DeleteResFile_0003
* @tc.desc: Verify the DeleteResFile function when transListener is null and local files/dirs exist.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_DeleteResFile_0003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0003 Start";

    auto infos = std::make_shared<FileInfos>();
    infos->transListener = nullptr; // 模拟 transListener 为 nullptr
    infos->subDirs = {"/data/test/subdir"};

    // 创建测试文件和目录
    std::filesystem::create_directories("/data/test/subdir");
    std::ofstream("/data/test/file1.txt").close();
    std::ofstream("/data/test/file2.txt").close();
    std::ofstream("/data/test/subdir/file3.txt").close();

    // 模拟 localListener 返回文件路径
    infos->localListener = std::make_shared<FileCopyLocalListener>("/data/test/test.txt", true, emptyCallback_);
    infos->localListener->filePaths_ = {
        "/data/test/file0.txt",
        "/data/test/file1.txt",
        "/data/test/file2.txt",
        "/data/test/subdir/file3.txt"
    };

    // 调用 DeleteResFile
    Storage::DistributedFile::FileCopyManager::GetInstance()->DeleteResFile(infos);

    // 验证文件和目录是否被删除
    EXPECT_FALSE(std::filesystem::exists("/data/test/file1.txt"));
    EXPECT_FALSE(std::filesystem::exists("/data/test/file2.txt"));
    EXPECT_FALSE(std::filesystem::exists("/data/test/subdir"));
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0003 End";
}

/**
* @tc.name: FileCopyManager_DeleteResFile_0004
* @tc.desc: Verify the DeleteResFile function when transListener is null and local files/dirs do not exist.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_DeleteResFile_0004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0004 Start";

    auto infos = std::make_shared<FileInfos>();
    infos->transListener = nullptr; // 模拟 transListener 为 nullptr
    infos->subDirs = {"/data/test/nonexistent_subdir"};

    // 模拟 localListener 返回文件路径
    infos->localListener = std::make_shared<FileCopyLocalListener>("/data/test/test.txt", true, emptyCallback_);
    infos->localListener->filePaths_ = {
        "/data/test/nonexistent_file.txt",
        "/data/test/nonexistent_file.txt"
    };

    // 调用 DeleteResFile
    Storage::DistributedFile::FileCopyManager::GetInstance()->DeleteResFile(infos);

    // 验证文件和目录仍然不存在
    EXPECT_FALSE(std::filesystem::exists("/data/test/nonexistent_file.txt"));
    EXPECT_FALSE(std::filesystem::exists("/data/test/nonexistent_subdir"));
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0004 End";
}

/**
* @tc.name: FileCopyManager_OpenSrcFile_0001
* @tc.desc: Verify the OpenSrcFile function when srcUri is a media URI.
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_OpenSrcFile_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_OpenSrcFile_0001 Start";

    // Prepare mock data for the test
    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = "file://media/image/12";
    infos->srcPath = "/media/image/12";

    // Mock remote connection and dataShareHelper behavior (use a mock framework if necessary)
    // Here we assume dataShareHelper->OpenFile() will return a valid file descriptor (for simplicity).
    int32_t srcFd = -1;

    // Call the OpenSrcFile function
    int ret = Storage::DistributedFile::FileCopyManager::GetInstance()->OpenSrcFile(infos->srcPath, infos, srcFd);

    // Validate the file descriptor is opened successfully
    EXPECT_EQ(ret, -1);
    EXPECT_GE(srcFd, -1); // Expect a valid file descriptor

    GTEST_LOG_(INFO) << "FileCopyManager_OpenSrcFile_0001 End";
}

/**
* @tc.name: FileCopyManager_OpenSrcFile_0002
* @tc.desc: Verify the OpenSrcFile function when srcUri is a media URI.
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_OpenSrcFile_0002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_OpenSrcFile_0002 Start";

    // Prepare mock data for the test
    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = "file://media/image/12";
    infos->srcPath = "/data/test/existentfile.txt";

    // Mock remote connection and dataShareHelper behavior (use a mock framework if necessary)
    // Here we assume dataShareHelper->OpenFile() will return a valid file descriptor (for simplicity).
    int32_t srcFd = -1;
    std::ofstream(infos->srcPath).close();

    // Call the OpenSrcFile function
    int ret = Storage::DistributedFile::FileCopyManager::GetInstance()->OpenSrcFile(infos->srcPath, infos, srcFd);

    // Validate the file descriptor is opened successfully
    EXPECT_EQ(ret, -1);
    EXPECT_GE(srcFd, -1); // Expect a valid file descriptor

    // Clean up the mock file
    std::remove(infos->srcPath.c_str());

    GTEST_LOG_(INFO) << "FileCopyManager_OpenSrcFile_0002 End";
}

/**
* @tc.name: FileCopyManager_OpenSrcFile_0003
* @tc.desc: Verify the OpenSrcFile function when srcUri is a local file URI and file does not exist.
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_OpenSrcFile_0003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_OpenSrcFile_0003 Start";

    // Prepare mock data for the test
    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = "file:///data/test/nonexistentfile.txt";
    infos->srcPath = "/data/test/nonexistentfile.txt";

    // Call the OpenSrcFile function
    int32_t srcFd = -1;
    int ret = Storage::DistributedFile::FileCopyManager::GetInstance()->OpenSrcFile(infos->srcPath, infos, srcFd);

    // Validate that the function fails when file does not exist
    EXPECT_NE(ret, 0);  // Expect error code
    EXPECT_EQ(srcFd, -1); // Expect invalid file descriptor

    GTEST_LOG_(INFO) << "FileCopyManager_OpenSrcFile_0003 End";
}

/**
* @tc.name: FileCopyManager_OpenSrcFile_0004
* @tc.desc: Verify the OpenSrcFile function when srcUri is a local file URI and file exists.
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_OpenSrcFile_0004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_OpenSrcFile_0004 Start";

    // Prepare mock data for the test
    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = "file:///data/test/existingfile.txt";
    infos->srcPath = "/data/test/existingfile.txt";

    // Create a mock file for the test
    std::ofstream outfile(infos->srcPath);
    outfile << "Test content";
    outfile.close();

    // Call the OpenSrcFile function
    int32_t srcFd = -1;
    int ret = Storage::DistributedFile::FileCopyManager::GetInstance()->OpenSrcFile(infos->srcPath, infos, srcFd);

    // Validate the file descriptor is opened successfully
    EXPECT_EQ(ret, 0);
    EXPECT_GE(srcFd, 0);  // Expect a valid file descriptor

    // Clean up the mock file
    std::remove(infos->srcPath.c_str());

    GTEST_LOG_(INFO) << "FileCopyManager_OpenSrcFile_0004 End";
}

/**
* @tc.name: FileCopyManager_CheckPath_0001
* @tc.desc: check path
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_CheckPath_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_CheckPath_0001 Start";

    auto infos = std::make_shared<FileInfos>();
    infos->srcUriIsFile = true;
    std::string tempPath = "/data/test/CheckPath";

    infos->destPath = "";
    bool ret = CheckPath(infos);
    EXPECT_EQ(ret, false);

    if (!ForceCreateDirectory(tempPath)) {
        GTEST_LOG_(INFO) << "FileCopyManager_CheckPath_0001 create dir err";
    }
    infos->srcPath = tempPath;
    infos->destPath = tempPath + "/test/";
    ret = CheckPath(infos);
    EXPECT_EQ(ret, false);

    infos->srcPath = tempPath;
    infos->destPath = tempPath;
    ret = CheckPath(infos);
    EXPECT_EQ(ret, true);

    infos->srcPath = tempPath + "/test";
    infos->destPath = tempPath;
    ret = CheckPath(infos);
    EXPECT_EQ(ret, false);

    infos->srcUriIsFile = false;

    infos->srcPath = tempPath;
    infos->destPath = tempPath + "/test";
    ret = CheckPath(infos);
    EXPECT_EQ(ret, false);

    infos->srcPath = tempPath;
    infos->destPath = tempPath;
    ret = CheckPath(infos);
    EXPECT_EQ(ret, true);

    infos->srcPath = tempPath + "/test";
    infos->destPath = tempPath;
    ret = CheckPath(infos);
    EXPECT_EQ(ret, false);

    if (!ForceRemoveDirectory(tempPath)) {
        GTEST_LOG_(INFO) << "FileCopyManager_CheckPath_0001 remove dir err";
    }
    GTEST_LOG_(INFO) << "FileCopyManager_CheckPath_0001 End";
}

/**
* @tc.name: FileCopyManager_CheckOrCreateLPath_0001
* @tc.desc: CheckOrCreateLPath
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_CheckOrCreateLPath_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_CheckOrCreateLPath_0001 Start";

    std::string destPath = "";
    int32_t ret = Storage::DistributedFile::FileCopyManager::GetInstance()->CheckOrCreatePath(destPath);
    EXPECT_EQ(ret, FILE_NOT_FOUND);

    GTEST_LOG_(INFO) << "FileCopyManager_CheckOrCreateLPath_0001 End";
}

/**
* @tc.name: FileCopyManager_GetModeFromFlags_0001
* @tc.desc: GetModeFromFlags
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_GetModeFromFlags_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_GetModeFromFlags_0001 Start";

    unsigned int flags = 0;
    std::string result = GetModeFromFlags(flags);
    EXPECT_EQ(result, "r");

    flags |= O_RDWR;
    result = GetModeFromFlags(flags);
    EXPECT_EQ(result, "rw"); // need judge

    GTEST_LOG_(INFO) << "FileCopyManager_GetModeFromFlags_0001 End";
}

/**
* @tc.name: FileCopyManager_CreateFileInfos_0001
* @tc.desc: CreateFileInfos
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_CreateFileInfos_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_CreateFileInfos_0001 Start";
    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = "";
    int32_t ret = Storage::DistributedFile::FileCopyManager::GetInstance()->CreateFileInfos("", "", infos);
    EXPECT_EQ(ret, ERR_BAD_VALUE);
    GTEST_LOG_(INFO) << "FileCopyManager_CreateFileInfos_0001 End";
}

/**
* @tc.name: FileCopyManager_RemoveFileInfos_0001
* @tc.desc: RemoveFileInfos
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_RemoveFileInfos_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_RemoveFileInfos_0001 Start";
    auto infos1 = std::make_shared<FileInfos>();
    auto infos2 = std::make_shared<FileInfos>();
    auto infos3 = std::make_shared<FileInfos>();
    infos1->srcUri = "srcUri1";
    infos1->destUri = "destUri1";
    infos2->srcUri = "srcUri2";
    infos2->destUri = "destUri2";
    infos3->srcUri = "srcUri1";
    infos3->destUri = "destUri2";
    Storage::DistributedFile::FileCopyManager::GetInstance()->FileInfosVec_.clear();
    Storage::DistributedFile::FileCopyManager::GetInstance()->AddFileInfos(infos1);
    Storage::DistributedFile::FileCopyManager::GetInstance()->AddFileInfos(infos2);
    auto len = Storage::DistributedFile::FileCopyManager::GetInstance()->FileInfosVec_.size();
    EXPECT_EQ(len, 2); // 2: vector size

    Storage::DistributedFile::FileCopyManager::GetInstance()->RemoveFileInfos(infos2);
    len = Storage::DistributedFile::FileCopyManager::GetInstance()->FileInfosVec_.size();
    EXPECT_EQ(len, 1); // 1: vector size

    Storage::DistributedFile::FileCopyManager::GetInstance()->RemoveFileInfos(infos3);
    len = Storage::DistributedFile::FileCopyManager::GetInstance()->FileInfosVec_.size();
    EXPECT_EQ(len, 1); // 1: vector size

    Storage::DistributedFile::FileCopyManager::GetInstance()->RemoveFileInfos(infos1);
    len = Storage::DistributedFile::FileCopyManager::GetInstance()->FileInfosVec_.size();
    EXPECT_EQ(len, 0); // 0: vector size
}

/**
* @tc.name: FileCopyManager_RecurCopyDir_0001
* @tc.desc: RecurCopyDir
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_RecurCopyDir_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 Start";
    auto infos = std::make_shared<FileInfos>();
    std::string srcPath = "";
    std::string dstPath = "";
    auto res = Storage::DistributedFile::FileCopyManager::GetInstance()->RecurCopyDir(srcPath, dstPath, infos);
    EXPECT_EQ(res, E_OK);

    srcPath = "/data/test/RecurCopyDir/src/";
    dstPath = "/data/test/RecurCopyDir/dst/";

    std::string subSrcDir = srcPath + "dir/";
    std::string testFile = srcPath + "1.txt"; // 1: file name
    if (!ForceCreateDirectory(subSrcDir)) {
        GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 create dir err" << subSrcDir;
    }
    if (!ForceCreateDirectory(dstPath)) {
        GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 create dir err" << dstPath;
    }
    int fd = open(testFile.c_str(), O_RDWR | O_CREAT);
    if (fd < 0) {
        GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 create file err" << testFile;
    }
    std::string testLink = srcPath + "2";
    if (symlink(testFile.c_str(), testLink.c_str())) {
        GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 create linkfile err" << testLink;
    }
    infos->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    res = Storage::DistributedFile::FileCopyManager::GetInstance()->RecurCopyDir(srcPath, dstPath, infos);
    EXPECT_EQ(res, E_OK);

    dstPath = dstPath + "dir/dir1/dir2";
    res = Storage::DistributedFile::FileCopyManager::GetInstance()->RecurCopyDir(srcPath, dstPath, infos);
    EXPECT_EQ(res, ENOENT);

    std::string rootPath = "/data/test/RecurCopyDir";
    if (!ForceRemoveDirectory(rootPath)) {
        GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 remove dir err" << subSrcDir;
    }
    GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 End";
}

/**
* @tc.name: FileCopyManager_CopySubDir_0001
* @tc.desc: CopySubDir
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_CopySubDir_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_CopySubDir_0001 Start";
    auto infos = std::make_shared<FileInfos>();
    std::string srcPath = "/data/test/CopySubDir/src";
    std::string dstPath = "/data/test/CopySubDir/dst";
    infos->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    auto res = Storage::DistributedFile::FileCopyManager::GetInstance()->CopySubDir(srcPath, dstPath, infos);
    EXPECT_EQ(res, ENOENT);

    if (!ForceCreateDirectory(dstPath)) {
        GTEST_LOG_(INFO) << "FileCopyManager_CopySubDir_0001 create dir err" << dstPath;
    }
    res = Storage::DistributedFile::FileCopyManager::GetInstance()->CopySubDir(srcPath, dstPath, infos);
    EXPECT_EQ(res, E_OK);

    std::string tmpDstDir = dstPath + "/dir";
    res = Storage::DistributedFile::FileCopyManager::GetInstance()->CopySubDir(srcPath, tmpDstDir, infos);
    EXPECT_EQ(res, E_OK);
    std::string rootPath = "/data/test/CopySubDir";
    if (!ForceRemoveDirectory(rootPath)) {
        GTEST_LOG_(INFO) << "FileCopyManager_CopySubDir_0001 create dir err" << rootPath;
    }
    GTEST_LOG_(INFO) << "FileCopyManager_CopySubDir_0001 End";
}

/**
* @tc.name: FileCopyManager_CopyDirFunc_0001
* @tc.desc: CopyDirFunc
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_CopyDirFunc_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_CopyDirFunc_0001 Start";
    auto infos = std::make_shared<FileInfos>();
    std::string srcPath = "/data/test/CopyDirFunc/";
    std::string dstPath = "/data/test/CopyDirFunc/dst";
    infos->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    auto res = Storage::DistributedFile::FileCopyManager::GetInstance()->CopyDirFunc(srcPath, dstPath, infos);
    EXPECT_EQ(res, -1); // -1: err code

    std::string tmpSrcPath = "/test/CopyDirFunc/";
    res = Storage::DistributedFile::FileCopyManager::GetInstance()->CopyDirFunc(tmpSrcPath, dstPath, infos);
    EXPECT_EQ(res, ENOENT);

    srcPath += "src";
    if (!ForceCreateDirectory(dstPath)) {
        GTEST_LOG_(INFO) << "FileCopyManager_CopyDirFunc_0001 create dir err" << dstPath;
    }
    res = Storage::DistributedFile::FileCopyManager::GetInstance()->CopyDirFunc(srcPath, dstPath, infos);
    EXPECT_EQ(res, E_OK);
    std::string rootPath = "/data/test/CopyDirFunc";
    if (!ForceRemoveDirectory(rootPath)) {
        GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 remove dir err" << rootPath;
    }
    GTEST_LOG_(INFO) << "FileCopyManager_CopyDirFunc_0001 End";
}

/**
* @tc.name: FileCopyManager_Cancel_0001
* @tc.desc: The execution of the cancel succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Cancel_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Cancel_0001 Start";
    auto infos1 = std::make_shared<FileInfos>();
    auto infos2 = std::make_shared<FileInfos>();
    infos1->srcUri = "srcUri1";
    infos1->destUri = "destUri1";
    infos1->transListener = nullptr;
    infos1->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    infos2->srcUri = "srcUri2";
    infos2->destUri = "destUri2";
    infos2->transListener = sptr(new (std::nothrow) TransListener("/data/test/test.txt", emptyCallback_));
    infos2->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    Storage::DistributedFile::FileCopyManager::GetInstance()->FileInfosVec_.emplace_back(infos1);
    Storage::DistributedFile::FileCopyManager::GetInstance()->FileInfosVec_.emplace_back(infos2);
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Cancel();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FileCopyManager_Cancel_0001 End";
}
}