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
#include "copy/file_size_utils.h"

#include <fstream>
#include <gtest/gtest.h>

#include "accesstoken_kit.h"
#include "datashare_helper.h"
#include "dfs_error.h"
#include "directory_ex.h"
#include "ipc_skeleton.h"

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
    ProcessCallback emptyCallback_;
};

void CreateFile(string fileName, std::size_t targetSize)
{
    const std::size_t bufferSize = 1024 * 1024;

    std::ofstream ofs(fileName.c_str(), std::ios::binary);
    if (!ofs) {
        GTEST_LOG_(INFO) << "ofs is null, fileName = " << fileName;
        return;
    }
    std::vector<char> buffer(bufferSize, 0);
    std::size_t written = 0;
    while (written < targetSize) {
        std::size_t toWrite = std::min(bufferSize, targetSize - written);
        ofs.write(buffer.data(), toWrite);
        if (!ofs) {
            GTEST_LOG_(INFO) << "ofs is null, break";
            break;
        }
        written += toWrite;
    }
    ofs.close();
    GTEST_LOG_(INFO) << "create file success, fileName = " << fileName;
}

void PrepareData(string srcPath, string dstPath)
{
    std::error_code errCode;
    string tmpPath = srcPath;
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    tmpPath = srcPath + "test1/";
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    tmpPath = srcPath + "test1/test2/";
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    const std::size_t fileSize = 200 * 1024 * 1024;
    CreateFile(srcPath + "1.txt", fileSize);
    CreateFile(srcPath + "2.txt", fileSize);

    std::ofstream outfile3(srcPath + "3.txt");
    outfile3 << "Test 3.txt";
    outfile3.close();

    std::ofstream outfile4(srcPath + "test1/4.txt");
    outfile4 << "Test 4.txt";
    outfile4.close();

    std::ofstream outfile5(srcPath + "test1/5.txt");
    outfile5 << "Test 5.txt";
    outfile5.close();

    CreateFile(srcPath + "test1/test2/6.txt", fileSize);

    std::ofstream outfile7(srcPath + "test1/test2/7.txt");
    outfile7 << "Test 7.txt";
    outfile7.close();
    GTEST_LOG_(INFO) << "PrepareData";
}

void PrepareData2(string srcPath, string dstPath)
{
    std::error_code errCode;
    const std::size_t fileSize = 200 * 1024 * 1024;
    string tmpPath = srcPath;
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    }
    tmpPath = srcPath + "test1/";
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    }
    CreateFile(tmpPath + "1.txt", fileSize);
    CreateFile(tmpPath + "2.txt", fileSize);
    tmpPath = srcPath + "test2/";
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    }
    CreateFile(tmpPath + "3.txt", fileSize);
    CreateFile(tmpPath + "4.txt", fileSize);

    tmpPath = srcPath + "test3/";
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    }
    CreateFile(tmpPath + "5.txt", fileSize);
    CreateFile(tmpPath + "6.txt", fileSize);
    std::ofstream outfile1(srcPath + "7.txt");
    outfile1 << "Test 7.txt";
    outfile1.close();

    std::ofstream outfile2(srcPath + "8.txt");
    outfile2 << "Test 8.txt";
    outfile2.close();
    GTEST_LOG_(INFO) << "PrepareData2";
}

void DeleteData(string srcPath, string dstPath)
{
    ForceRemoveDirectory(srcPath.c_str());
    ForceRemoveDirectory(string(dstPath + "dsttest1").c_str());
    ForceRemoveDirectory(string(dstPath + "dsttest2").c_str());
    ForceRemoveDirectory(string(dstPath + "dsttest3").c_str());
    ForceRemoveDirectory(string(dstPath + "dsttest4").c_str());
    ForceRemoveDirectory(string(dstPath + "dsttest5").c_str());
    ForceRemoveDirectory(string(dstPath + "dsttest6").c_str());
    GTEST_LOG_(INFO) << "DeleteData";
}

void FileCopyManagerTest::SetUpTestCase(void)
{
    string srcPath = "/storage/media/100/local/files/Docs/srctest1/";
    string dstPath = "/storage/media/100/local/files/Docs/";
    PrepareData(srcPath, dstPath);
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileCopyManagerTest::TearDownTestCase(void)
{
    string srcPath = "/storage/media/100/local/files/Docs/srctest1/";
    string dstPath = "/storage/media/100/local/files/Docs/";
    DeleteData(srcPath, dstPath);
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void JudgeFileEqual(string srcPath, string dstPath)
{
    uint64_t srcSize = 0;
    uint64_t dstSize = 0;
    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath, srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath, dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);
}

void FileCopyManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileCopyManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

using callBack = std::function<void(uint64_t processSize, uint64_t totalFileSize)>;

/**
* @tc.name: FileCopyManager_Copy_0001
* @tc.desc: The execution of the Copy failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0001 Start";
    string localUri = "/data/test/test.txt";
    string dstUri = "/data/test/test.txt";

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy("", localUri, listener_);
    EXPECT_EQ(ret, EINVAL);

    ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(localUri, "", listener_);
    EXPECT_EQ(ret, EINVAL);

    ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy("", "", listener_);
    EXPECT_EQ(ret, EINVAL);

    ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(localUri, dstUri, listener_);
    EXPECT_EQ(ret, EINVAL);

    string remoteUri = "/data/test/Copy/?networkid=/";
    if (!ForceCreateDirectory(remoteUri)) {
        GTEST_LOG_(INFO) << "FileCopyManager_Copy_0001 create dir err";
    }
    ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(remoteUri, "", listener_);
    EXPECT_EQ(ret, EINVAL);
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
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0002 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dest1.txt";
    string srcPath = "/storage/media/100/local/files/Docs/1.txt";
    string dstPath = "/storage/media/100/local/files/Docs/dest1.txt";
    const std::size_t fileSize = 20 * 1024;
    CreateFile(srcPath, fileSize);
    EXPECT_TRUE(std::filesystem::exists(srcPath));

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_OK);
    JudgeFileEqual(srcPath, dstPath);
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    ASSERT_EQ(remove(dstPath.c_str()), 0);
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0002 End";
}

/**
* @tc.name: FileCopyManager_Copy_0003
* @tc.desc: The execution of the Copy succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0003, TestSize.Level0)
{
    //same uri
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0003 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string srcPath = "/storage/media/100/local/files/Docs/1.txt";
    int fd = open(srcPath.c_str(), O_RDWR | O_CREAT);
    ASSERT_TRUE(fd != -1) <<"Failed to open file in FileCopyManager_Copy_0003!" << errno;
    close(fd);

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, EINVAL);
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0003 End";
}

/**
* @tc.name: FileCopyManager_Copy_0004
* @tc.desc: The execution of the Copy succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0004 Start";
    std::error_code errCode;
    string srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/test1/a1.txt";
    string srcPath = "/storage/media/100/local/files/Docs/1.txt";
    string dstPath = "/storage/media/100/local/files/Docs/test1/a1.txt";
    const std::size_t fileSize = 20 * 1024;
    CreateFile(srcPath, fileSize);
    EXPECT_TRUE(std::filesystem::exists(srcPath));
    string tmpPath = "/storage/media/100/local/files/Docs/test1/";
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_OK);
    JudgeFileEqual(srcPath, dstPath);
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    ASSERT_EQ(remove(dstPath.c_str()), 0);
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0004 End";
}

/**
* @tc.name: FileCopyManager_Copy_0005
* @tc.desc: The execution of the Copy failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0005 Start";
    string localUri = "/data/test/test.txt";
    string dstUri = "/data/test/test.txt";

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy("", localUri, emptyCallback_);
    EXPECT_EQ(ret, EINVAL);

    ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(localUri, "", listener_);
    EXPECT_EQ(ret, EINVAL);

    ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(localUri, dstUri, listener_);
    EXPECT_EQ(ret, EINVAL);

    ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(localUri, dstUri, emptyCallback_);
    EXPECT_EQ(ret, EINVAL);

    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0005 End";
}

/**
 * @tc.name: FileCopyManager_Copy_0006
 * @tc.desc: The execution of the Copy failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0006 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/a1.txt";
    string srcPath = "/storage/media/100/local/files/Docs/1.txt";

    EXPECT_TRUE(OHOS::RemoveFile(srcPath));
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, emptyCallback_);
    EXPECT_EQ(ret, EINVAL);

    ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, ENOENT);

    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0006 End";
}

/**
 * @tc.name: FileCopyManager_Copy_0007
 * @tc.desc: The execution of the Copy failed.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Copy_0007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0007 Start";
    string srcUri = "file://docs/storage/External/mtp/1.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/a1.txt";
    string srcPath = "/storage/External/mtp/1.txt";

    EXPECT_TRUE(OHOS::RemoveFile(srcPath));
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, emptyCallback_);
    EXPECT_EQ(ret, EINVAL);

    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0007 End";
}

/**
* @tc.name: FileCopyManager_ExecLocal_0001
* @tc.desc: The execution of the execlocal failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_ExecLocal_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0001 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/11.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dest11.txt";
    string srcPath = "/storage/media/100/local/files/Docs/11.txt";
    string dstPath = "/storage/media/100/local/files/Docs/dest11.txt";
    // infos is nullptr
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(nullptr);
    EXPECT_EQ(ret, EINVAL);

    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = srcUri;
    infos->destUri = dstUri;
    // infos localListener is nullptr
    ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(infos);
    EXPECT_EQ(ret, EINVAL);

    infos->srcUriIsFile = true;
    infos->srcPath = srcPath;
    infos->destPath = srcPath;
    infos->localListener = FileCopyLocalListener::GetLocalListener(infos->srcPath,
        infos->srcUriIsFile, listener_);
    // srcPath and dstPath is same
    ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(infos);
    EXPECT_EQ(ret, EINVAL);

    infos->destPath = dstPath;
    // src file not exist
    ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(infos);
    EXPECT_EQ(ret, ENOENT);

    int fd = open(srcPath.c_str(), O_RDWR | O_CREAT);
    ASSERT_TRUE(fd != -1) <<"Failed to open file in FileCopyManager_ExecLocal_0001" << errno;
    close(fd);
    ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(infos);
    EXPECT_EQ(ret, E_OK);
    JudgeFileEqual(srcPath, dstPath);
    
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    ASSERT_EQ(remove(dstPath.c_str()), 0);

    infos->localListener = FileCopyLocalListener::GetLocalListener(infos->srcPath,
        infos->srcUriIsFile, emptyCallback_);
    // src file not exist, callback is nullptr
    ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(infos);
    EXPECT_EQ(ret, ENOENT);
    ASSERT_EQ(remove(dstPath.c_str()), 0);
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0001 End";
}

/**
* @tc.name: FileCopyManager_ExecLocal_0002
* @tc.desc: The execution of the execlocal success.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_ExecLocal_0002, TestSize.Level0)
{
    //无对应子文件夹，errno返回2
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0002 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srcaa11/";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dstaa11/";
    string srcPath = "/storage/media/100/local/files/Docs/srcaa11/";
    string dstPath = "/storage/media/100/local/files/Docs/dstaa11/";
    std::error_code errCode;
    if (!std::filesystem::exists(srcPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(srcPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }

    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = srcUri;
    infos->destUri = dstUri;
    infos->srcPath = srcPath;
    infos->destPath = dstPath;
    ProcessCallback processCallback = [](uint64_t processSize, uint64_t totalSize) -> void {
        GTEST_LOG_(INFO) << "processSize = " << processSize << "totalSize = " << totalSize;
    };
    infos->localListener = FileCopyLocalListener::GetLocalListener(infos->srcPath,
        infos->srcUriIsFile, processCallback);
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(infos);
    // dstPath not exist
    EXPECT_EQ(ret, ENOENT);

    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        EXPECT_EQ(res, E_OK);
    }
    // dir copy to dir
    std::ofstream outfile(srcPath + "1.txt");
    outfile << "FileCopyManager_ExecLocal_0002";
    outfile.close();
    ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(infos);
    EXPECT_EQ(ret, 0);
    JudgeFileEqual(srcPath + "1.txt", dstPath + "srcaa11/1.txt");
    infos->localListener->StopListener();

    ASSERT_EQ(ForceRemoveDirectory(srcPath.c_str()), true);
    ASSERT_EQ(ForceRemoveDirectory(dstPath.c_str()), true);
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0002 End";
}

/**
* @tc.name: FileCopyManager_ExecLocal_0003
* @tc.desc: The execution of the execlocal success.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_ExecLocal_0003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0003 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srcaa12/";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dstaa12/";
    string srcPath = "/storage/media/100/local/files/Docs/srcaa12/";
    string dstPath = "/storage/media/100/local/files/Docs/dstaa12/";
    std::error_code errCode;
    if (!std::filesystem::exists(srcPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(srcPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }

    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = srcUri;
    infos->destUri = dstUri;
    infos->srcPath = srcPath;
    infos->destPath = dstPath;
    infos->srcUriIsFile = true;
    ProcessCallback processCallback = [](uint64_t processSize, uint64_t totalSize) -> void {};
    infos->localListener = FileCopyLocalListener::GetLocalListener(infos->srcPath,
        infos->srcUriIsFile, processCallback);
    std::ofstream outfile(srcPath + "1.txt");
    outfile << "FileCopyManager_ExecLocal_0003";
    outfile.close();
    // srcUriIsFile is true, dstPath not exist
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(infos);
    EXPECT_NE(ret, E_OK);
    infos->localListener->StopListener();
    ASSERT_EQ(ForceRemoveDirectory(srcPath), true);
    if (!remove(dstPath.c_str())) {
        GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0003 remove dir err";
    }
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0003 End";
}

/**
* @tc.name: FileCopyManager_ExecLocal_0004
* @tc.desc: The execution of the execlocal failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_ExecLocal_0004, TestSize.Level0)
{
    //无对应子文件夹，errno返回2
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0004 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srcaa13/";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dstaa13/";
    string srcPath = "/storage/media/100/local/files/Docs/srcaa13/";
    string dstPath = "/storage/media/100/local/files/Docs/dstaa13/";
    std::error_code errCode;
    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = srcUri;
    infos->destUri = dstUri;
    infos->srcPath = srcPath;
    infos->destPath = dstPath;
    infos->srcUriIsFile = true;
    ProcessCallback processCallback = [](uint64_t processSize, uint64_t totalSize) -> void {};
    infos->localListener = FileCopyLocalListener::GetLocalListener(infos->srcPath,
        infos->srcUriIsFile, processCallback);

    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        EXPECT_EQ(res, E_OK);
    }
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(infos);
    // src path not exist
    EXPECT_EQ(ret, ENOENT);
    ASSERT_EQ(ForceRemoveDirectory(dstPath.c_str()), true);
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0004 End";
}

/**
* @tc.name: FileCopyManager_ExecLocal_0005
* @tc.desc: ExecLocal
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_ExecLocal_0005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0005 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/11.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/aa/";
    string srcPath = "/storage/media/100/local/files/Docs/11.txt";
    string dstPath = "/storage/media/100/local/files/Docs/aa/";
    int fd = open(srcPath.c_str(), O_RDWR | O_CREAT);
    ASSERT_TRUE(fd != -1) <<"Failed to open file in FileCopyManager_ExecLocal_0005!" << errno;
    close(fd);

    std::error_code errCode;
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        ASSERT_EQ(res, E_OK);
    }

    auto infos = std::make_shared<FileInfos>();
    infos->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    infos->srcUri = srcUri;
    infos->destUri = dstUri;
    infos->srcPath = srcPath;
    infos->destPath = dstPath;
    infos->srcUriIsFile = true;
    // file to dir
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(infos);
    EXPECT_EQ(ret, EINVAL);
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    EXPECT_TRUE(ForceRemoveDirectory(dstPath));
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0005 End";
}

/**
* @tc.name: FileCopyManager_ExecLocal_0006
* @tc.desc: ExecLocal
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_ExecLocal_0006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0006 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/11.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dest11.txt";
    string srcPath = "/storage/media/100/local/files/Docs/11.txt";
    string dstPath = "/aa/dest11.txt";

    auto infos = std::make_shared<FileInfos>();
    infos->srcUriIsFile = true;
    infos->srcPath = srcPath;
    infos->destPath = dstPath;
    infos->localListener = FileCopyLocalListener::GetLocalListener(infos->srcPath,
        infos->srcUriIsFile, listener_);

    // dstPath is invalid
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecLocal(infos);
    EXPECT_EQ(ret, ENOENT);
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0006 End";
}

/**
* @tc.name: FileCopyManager_DeleteResFile_0001
* @tc.desc: Verify the DeleteResFile function when transListener is not null and dstPath exists.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_DeleteResFile_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0001 Start";

    auto infos = std::make_shared<FileInfos>();
    infos->transListener = sptr(new (std::nothrow) TransListener("/data/test/test.txt", emptyCallback_));
    infos->destPath = "/data/test/test.txt";

    // 创建测试文件
    std::ofstream(infos->destPath).close();
    EXPECT_TRUE(std::filesystem::exists(infos->destPath));

    // 调用 DeleteResFile
    Storage::DistributedFile::FileCopyManager::GetInstance().DeleteResFile(infos);

    // 验证文件是否被删除
    EXPECT_FALSE(std::filesystem::exists(infos->destPath));
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0001 End";
}

/**
* @tc.name: FileCopyManager_DeleteResFile_0002
* @tc.desc: Verify the DeleteResFile function when transListener is not null and dstPath does not exist.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_DeleteResFile_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0002 Start";

    auto infos = std::make_shared<FileInfos>();
    infos->transListener = sptr(new (std::nothrow) TransListener("/data/test/test.txt", emptyCallback_));
    infos->destPath = "/data/test/nonexistent.txt";

    // 确保文件不存在
    EXPECT_FALSE(std::filesystem::exists(infos->destPath));

    // 调用 DeleteResFile
    Storage::DistributedFile::FileCopyManager::GetInstance().DeleteResFile(infos);

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
HWTEST_F(FileCopyManagerTest, FileCopyManager_DeleteResFile_0003, TestSize.Level0)
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
    Storage::DistributedFile::FileCopyManager::GetInstance().DeleteResFile(infos);

    // 验证文件和目录是否被删除
    EXPECT_FALSE(std::filesystem::exists("/data/test/file1.txt"));
    EXPECT_FALSE(std::filesystem::exists("/data/test/file2.txt"));
    EXPECT_FALSE(std::filesystem::exists("/data/test/subdir"));

    // 调用 DeleteResFile
    infos->localListener = nullptr;
    Storage::DistributedFile::FileCopyManager::GetInstance().DeleteResFile(infos);
    GTEST_LOG_(INFO) << "FileCopyManager_DeleteResFile_0003 End";
}

/**
* @tc.name: FileCopyManager_DeleteResFile_0004
* @tc.desc: Verify the DeleteResFile function when transListener is null and local files/dirs do not exist.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_DeleteResFile_0004, TestSize.Level0)
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
    Storage::DistributedFile::FileCopyManager::GetInstance().DeleteResFile(infos);

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
HWTEST_F(FileCopyManagerTest, FileCopyManager_OpenSrcFile_0001, TestSize.Level0)
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
    int ret = Storage::DistributedFile::FileCopyManager::GetInstance().OpenSrcFile(infos->srcPath, infos, srcFd);

    // Validate the file descriptor is opened successfully
    EXPECT_EQ(ret, 1);
    EXPECT_GE(srcFd, -1); // Expect a valid file descriptor

    GTEST_LOG_(INFO) << "FileCopyManager_OpenSrcFile_0001 End";
}

/**
* @tc.name: FileCopyManager_OpenSrcFile_0002
* @tc.desc: Verify the OpenSrcFile function when srcUri is a media URI.
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_OpenSrcFile_0002, TestSize.Level0)
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
    int ret = Storage::DistributedFile::FileCopyManager::GetInstance().OpenSrcFile(infos->srcPath, infos, srcFd);

    // Validate the file descriptor is opened successfully
    EXPECT_EQ(ret, 1);
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
HWTEST_F(FileCopyManagerTest, FileCopyManager_OpenSrcFile_0003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_OpenSrcFile_0003 Start";

    // Prepare mock data for the test
    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = "file:///data/test/nonexistentfile.txt";
    infos->srcPath = "/data/test/nonexistentfile.txt";

    // Call the OpenSrcFile function
    int32_t srcFd = -1;
    int ret = Storage::DistributedFile::FileCopyManager::GetInstance().OpenSrcFile(infos->srcPath, infos, srcFd);

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
HWTEST_F(FileCopyManagerTest, FileCopyManager_OpenSrcFile_0004, TestSize.Level0)
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
    int ret = Storage::DistributedFile::FileCopyManager::GetInstance().OpenSrcFile(infos->srcPath, infos, srcFd);

    // Validate the file descriptor is opened successfully
    EXPECT_EQ(ret, 0);
    EXPECT_GE(srcFd, 0);  // Expect a valid file descriptor

    // Clean up the mock file
    std::remove(infos->srcPath.c_str());

    GTEST_LOG_(INFO) << "FileCopyManager_OpenSrcFile_0004 End";
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

    std::string dstPath = "";
    int32_t ret = Storage::DistributedFile::FileCopyManager::GetInstance().CheckOrCreatePath(dstPath);
    EXPECT_EQ(ret, FILE_NOT_FOUND);

    GTEST_LOG_(INFO) << "FileCopyManager_CheckOrCreateLPath_0001 End";
}

/**
* @tc.name: FileCopyManager_RemoveFileInfos_0001
* @tc.desc: RemoveFileInfos
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_RemoveFileInfos_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_RemoveFileInfos_0001 Start";
    auto infos1 = std::make_shared<FileInfos>();
    auto infos2 = std::make_shared<FileInfos>();
    auto infos3 = std::make_shared<FileInfos>();
    infos1->srcUri = "srcUri1";
    infos1->destUri = "dstUri1";
    infos2->srcUri = "srcUri2";
    infos2->destUri = "dstUri2";
    infos3->srcUri = "srcUri1";
    infos3->destUri = "dstUri2";
    Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.clear();
    Storage::DistributedFile::FileCopyManager::GetInstance().AddFileInfos(infos1);
    Storage::DistributedFile::FileCopyManager::GetInstance().AddFileInfos(infos2);
    auto len = Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.size();
    EXPECT_EQ(len, 2); // 2: vector size

    Storage::DistributedFile::FileCopyManager::GetInstance().RemoveFileInfos(infos2);
    len = Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.size();
    EXPECT_EQ(len, 1); // 1: vector size

    Storage::DistributedFile::FileCopyManager::GetInstance().RemoveFileInfos(infos3);
    len = Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.size();
    EXPECT_EQ(len, 1); // 1: vector size

    Storage::DistributedFile::FileCopyManager::GetInstance().RemoveFileInfos(infos1);
    len = Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.size();
    EXPECT_EQ(len, 0); // 0: vector size
}

/**
* @tc.name: FileCopyManager_RecurCopyDir_0001
* @tc.desc: RecurCopyDir
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(FileCopyManagerTest, FileCopyManager_RecurCopyDir_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 Start";
    auto infos = std::make_shared<FileInfos>();
    std::string srcPath = "";
    std::string dstPath = "";
    auto res = Storage::DistributedFile::FileCopyManager::GetInstance().RecurCopyDir(srcPath, dstPath, infos);
    EXPECT_EQ(res, E_OK);

    srcPath = "/data/test/RecurCopyDir/src1/";
    dstPath = "/data/test/RecurCopyDir/dst1/";

    std::string subSrcDir = srcPath + "dir/";
    std::string testFile = subSrcDir + "1.txt"; // 1: file name
    if (!ForceCreateDirectory(subSrcDir)) {
        GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 create dir err" << subSrcDir;
    }
    if (!ForceCreateDirectory(dstPath)) {
        GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 create dir err" << dstPath;
    }
    int fd = open(testFile.c_str(), O_RDWR | O_CREAT);
    if (fd < 0) {
        GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 create file err" << testFile;
    } else {
        close(fd);
    }
    std::string testLink = srcPath + "2";
    if (symlink(testFile.c_str(), testLink.c_str())) {
        GTEST_LOG_(INFO) << "FileCopyManager_RecurCopyDir_0001 create linkfile err" << testLink;
    }
    infos->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    res = Storage::DistributedFile::FileCopyManager::GetInstance().RecurCopyDir(srcPath, dstPath, infos);
    EXPECT_EQ(res, E_OK);

    dstPath = dstPath + "dir/dir1/dir2";
    res = Storage::DistributedFile::FileCopyManager::GetInstance().RecurCopyDir(srcPath, dstPath, infos);
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
HWTEST_F(FileCopyManagerTest, FileCopyManager_CopySubDir_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_CopySubDir_0001 Start";
    auto infos = std::make_shared<FileInfos>();
    std::string srcPath = "/data/test/CopySubDir/src";
    std::string dstPath = "/data/test/CopySubDir/dst";
    infos->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    auto res = Storage::DistributedFile::FileCopyManager::GetInstance().CopySubDir(srcPath, dstPath, infos);
    EXPECT_EQ(res, ENOENT);

    if (!ForceCreateDirectory(dstPath)) {
        GTEST_LOG_(INFO) << "FileCopyManager_CopySubDir_0001 create dir err" << dstPath;
    }
    res = Storage::DistributedFile::FileCopyManager::GetInstance().CopySubDir(srcPath, dstPath, infos);
    EXPECT_EQ(res, E_OK);

    std::string tmpDstDir = dstPath + "/dir";
    res = Storage::DistributedFile::FileCopyManager::GetInstance().CopySubDir(srcPath, tmpDstDir, infos);
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
HWTEST_F(FileCopyManagerTest, FileCopyManager_CopyDirFunc_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_CopyDirFunc_0001 Start";
    auto infos = std::make_shared<FileInfos>();
    std::string srcPath = "/data/test/CopyDirFunc/";
    std::string dstPath = "/data/test/CopyDirFunc/dst";
    infos->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    auto res = Storage::DistributedFile::FileCopyManager::GetInstance().CopyDirFunc(srcPath, dstPath, infos);
    EXPECT_EQ(res, EINVAL);

    std::string tmpSrcPath = "/test/CopyDirFunc/";
    res = Storage::DistributedFile::FileCopyManager::GetInstance().CopyDirFunc(tmpSrcPath, dstPath, infos);
    EXPECT_EQ(res, ENOENT);

    srcPath += "src";
    if (!ForceCreateDirectory(dstPath)) {
        GTEST_LOG_(INFO) << "FileCopyManager_CopyDirFunc_0001 create dir err" << dstPath;
    }
    res = Storage::DistributedFile::FileCopyManager::GetInstance().CopyDirFunc(srcPath, dstPath, infos);
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
HWTEST_F(FileCopyManagerTest, FileCopyManager_Cancel_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Cancel_0001 Start";
    auto infos1 = std::make_shared<FileInfos>();
    auto infos2 = std::make_shared<FileInfos>();
    infos1->srcUri = "srcUri1";
    infos1->destUri = "dstUri1";
    infos1->transListener = nullptr;
    infos1->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    infos2->srcUri = "srcUri2";
    infos2->destUri = "dstUri2";
    infos2->transListener = sptr(new (std::nothrow) TransListener("/data/test/test.txt", emptyCallback_));
    infos2->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.emplace_back(infos1);
    Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.emplace_back(infos2);
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Cancel();
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FileCopyManager_Cancel_0001 End";
}

/**
* @tc.name: FileCopyManager_Cancel_0002
* @tc.desc: The execution of the cancel succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Cancel_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Cancel_0002 Start";
    auto infos1 = std::make_shared<FileInfos>();
    infos1->srcUri = "srcUri1";
    infos1->destUri = "dstUri1";
    infos1->transListener = nullptr;
    infos1->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    std::string srcUri = "srcUri1";
    std::string dstUri = "dstUri1";

    Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.emplace_back(infos1);
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(srcUri, dstUri, true);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FileCopyManager_Cancel_0002 End";
}

/**
* @tc.name: FileCopyManager_Cancel_0003
* @tc.desc: The execution of the cancel succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Cancel_0003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Cancel_0003 Start";
    auto infos1 = std::make_shared<FileInfos>();
    auto infos2 = std::make_shared<FileInfos>();
    infos1->srcUri = "srcUri1";
    infos1->destUri = "dstUri1";
    infos1->transListener = nullptr;
    infos1->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    infos2->srcUri = "srcUri2";
    infos2->destUri = "dstUri2";
    infos2->transListener = sptr(new (std::nothrow) TransListener("/data/test/test.txt", emptyCallback_));
    infos2->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.emplace_back(infos1);
    Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.emplace_back(infos2);
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(true);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "FileCopyManager_Cancel_0003 End";
}

/**
* @tc.name: FileCopyManager_Cancel_0004
* @tc.desc: The execution of the cancel succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_Cancel_0004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Cancel_0004 Start";
    auto infos1 = std::make_shared<FileInfos>();
    infos1->srcUri = "srcUri1";
    infos1->destUri = "dstUri1";
    infos1->transListener = nullptr;
    infos1->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    std::string srcUri = "srcUri1";
    std::string dstUri = "dstUri3";

    Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.emplace_back(infos1);
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(srcUri, dstUri);
    EXPECT_EQ(ret, E_OK);
    srcUri = "srcUri2";
    dstUri = "dstUri1";
    ret = Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(srcUri, dstUri);
    EXPECT_EQ(ret, E_OK);

    srcUri = "srcUri2";
    dstUri = "dstUri2";
    ret = Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(srcUri, dstUri);
    EXPECT_EQ(ret, E_OK);

    srcUri = "srcUri1";
    dstUri = "dstUri1";
    ret = Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(srcUri, dstUri);
    EXPECT_EQ(ret, E_OK);
    Storage::DistributedFile::FileCopyManager::GetInstance().FileInfosVec_.clear();
    GTEST_LOG_(INFO) << "FileCopyManager_Cancel_0004 End";
}

/**
 * @tc.name: FileCopyManager_MakeDir_0001
 * @tc.desc: test MakeDir function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_MakeDir_0001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FileCopyManager_MakeDir_0001";
    string srcPath = "/storage/media/100/local/files/Docs/bb/";
    std::error_code errCode;
    int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(srcPath);
    EXPECT_EQ(res, E_OK);
    ASSERT_EQ(ForceRemoveDirectory(srcPath.c_str()), true);
    GTEST_LOG_(INFO) << "FileCopyManager_MakeDir_0001 End";
}

/**
 * @tc.name: FileCopyManager_ExecCopy_0001
 * @tc.desc: ExecCopy with srcUriIsFile false and dstPath is a directory
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_ExecCopy_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_ExecCopy_0001 Start";
    string srcPath = "/storage/media/100/local/files/Docs/test_src_dir";
    string dstPath = "/storage/media/100/local/files/Docs/test_dest_dir";

    std::error_code errCode;
    if (!std::filesystem::exists(srcPath, errCode) && errCode.value() == E_OK) {
        Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(srcPath);
    }
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
    }

    auto infos = std::make_shared<FileInfos>();
    infos->srcUriIsFile = false;
    infos->srcPath = srcPath;
    infos->destPath = dstPath;
    infos->srcUri = "file://docs/storage/media/100/local/files/Docs/test_src_dir";
    infos->localListener = std::make_shared<FileCopyLocalListener>("", false, nullptr);
    std::ofstream outfile(srcPath + "/1.txt");
    outfile << "FileCopyManager_ExecCopy_0001";
    outfile.close();
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecCopy(infos);
    EXPECT_EQ(ret, E_OK);
    JudgeFileEqual(srcPath + "/1.txt", dstPath + "/test_src_dir/1.txt");

    ForceRemoveDirectory(srcPath.c_str());
    ForceRemoveDirectory(dstPath.c_str());
    GTEST_LOG_(INFO) << "FileCopyManager_ExecCopy_0001 End";
}

/**
 * @tc.name: FileCopyManager_ExecCopy_0002
 * @tc.desc: ExecCopy with srcUriIsFile false and dstPath is a directory, both paths end with '/'
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_ExecCopy_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_ExecCopy_0002 Start";
    string srcPath = "/storage/media/100/local/files/Docs/test_src_dir/";
    string dstPath = "/storage/media/100/local/files/Docs/test_dest_dir/";

    std::error_code errCode;
    if (!std::filesystem::exists(srcPath, errCode) && errCode.value() == E_OK) {
        Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(srcPath);
    }
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
    }

    auto infos = std::make_shared<FileInfos>();
    infos->srcUriIsFile = false;
    infos->srcPath = srcPath;
    infos->destPath = dstPath;
    infos->srcUri = "file://docs/storage/media/100/local/files/Docs/test_src_dir/";
    infos->localListener = std::make_shared<FileCopyLocalListener>("", false, nullptr);
    std::ofstream outfile(srcPath + "1.txt");
    outfile << "FileCopyManager_ExecLocal_0004";
    outfile.close();
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().ExecCopy(infos);
    EXPECT_EQ(ret, E_OK);
    JudgeFileEqual(srcPath + "1.txt", dstPath + "/test_src_dir/1.txt");

    ForceRemoveDirectory(srcPath.c_str());
    ForceRemoveDirectory(dstPath.c_str());
    GTEST_LOG_(INFO) << "FileCopyManager_ExecCopy_0002 End";
}

void JudgeCopyFile1(string srcPath, string dstPath)
{
    uint64_t srcSize = 0;
    uint64_t dstSize = 0;
    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "1.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest1/1.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "2.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest1/2.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "3.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest1/3.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test1/4.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest1/test1/4.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test1/5.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest1/test1/5.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test1/test2/6.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest1/test1/test2/6.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test1/test2/7.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest1/test1/test2/7.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);
}

void JudgeCopyFile2(string srcPath, string dstPath)
{
    uint64_t srcSize = 0;
    uint64_t dstSize = 0;
    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "4.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "test1/4.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "5.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "test1/5.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test2/6.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "test1/test2/6.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test2/7.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "test1/test2/7.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);
}

void JudgeCopyFile3(string srcPath, string dstPath)
{
    uint64_t srcSize = 0;
    uint64_t dstSize = 0;
    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "4.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "test1/4.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "5.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "test1/5.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test2/6.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "test1/test2/6.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test2/7.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "test1/test2/7.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);
}

void JudgeCopyFile4(string srcPath, string dstPath)
{
    uint64_t srcSize = 0;
    uint64_t dstSize = 0;
    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test1/1.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest2/test1/1.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test1/2.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest2/test1/2.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test2/3.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest2/test2/3.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test2/4.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest2/test2/4.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test3/5.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest2/test3/5.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "test3/6.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest2/test3/6.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "7.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest2/7.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);

    EXPECT_EQ(FileSizeUtils::GetFileSize(srcPath + "8.txt", srcSize), E_OK);
    EXPECT_EQ(FileSizeUtils::GetFileSize(dstPath + "srctest2/8.txt", dstSize), E_OK);
    EXPECT_EQ(srcSize, dstSize);
}

/**
 * @tc.name: FileCopyManager_TestCopy_0001
 * @tc.desc: test MakeDir function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0001 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srctest1";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dsttest1";
    string srcPath = "/storage/media/100/local/files/Docs/srctest1/";
    string dstPath = "/storage/media/100/local/files/Docs/dsttest1/";
    std::error_code errCode;
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }

    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        GTEST_LOG_(INFO) << "processSize = " << processSize << "totalFileSize = " << totalFileSize;
    };
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_OK);
    JudgeCopyFile1(srcPath, dstPath);
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0001 End";
}

/**
 * @tc.name: FileCopyManager_TestCopy_0002
 * @tc.desc: test MakeDir function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0002 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srctest1/";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dsttest2";
    string srcPath = "/storage/media/100/local/files/Docs/srctest1/";
    string dstPath = "/storage/media/100/local/files/Docs/dsttest2/";
    std::error_code errCode;
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        GTEST_LOG_(INFO) << "processSize = " << processSize << "totalFileSize = " << totalFileSize;
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_OK);
    JudgeCopyFile1(srcPath, dstPath);
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0002 End";
}

/**
 * @tc.name: FileCopyManager_TestCopy_0003
 * @tc.desc: test MakeDir function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0003, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0004 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srctest1";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dsttest3/";
    string srcPath = "/storage/media/100/local/files/Docs/srctest1/";
    string dstPath = "/storage/media/100/local/files/Docs/dsttest3/";
    std::error_code errCode;
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        GTEST_LOG_(INFO) << "processSize = " << processSize << "totalFileSize = " << totalFileSize;
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_OK);
    JudgeCopyFile1(srcPath, dstPath);
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0003 End";
}

/**
 * @tc.name: FileCopyManager_TestCopy_0004
 * @tc.desc: test MakeDir function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0004, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0005 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srctest1/";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dsttest4/";
    string srcPath = "/storage/media/100/local/files/Docs/srctest1/";
    string dstPath = "/storage/media/100/local/files/Docs/dsttest4/";
    std::error_code errCode;
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, emptyCallback_);
    EXPECT_EQ(ret, E_OK);
    JudgeCopyFile1(srcPath, dstPath);
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0004 End";
}

/**
 * @tc.name: FileCopyManager_TestCopy_0005
 * @tc.desc: test MakeDir function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0005, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0006 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srctest1/test1";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dsttest5";
    string srcPath = "/storage/media/100/local/files/Docs/srctest1/test1/";
    string dstPath = "/storage/media/100/local/files/Docs/dsttest5/";
    std::error_code errCode;
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        GTEST_LOG_(INFO) << "processSize = " << processSize << "totalFileSize = " << totalFileSize;
    };
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_OK);
    EXPECT_FALSE(std::filesystem::exists(dstPath + "srctest1/1.txt"));
    EXPECT_FALSE(std::filesystem::exists(dstPath + "srctest1/2.txt"));
    EXPECT_FALSE(std::filesystem::exists(dstPath + "srctest1/3.txt"));
    JudgeCopyFile2(srcPath, dstPath);
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0005 End";
}

/**
 * @tc.name: FileCopyManager_TestCopy_0006
 * @tc.desc: test MakeDir function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0006, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_ExecLocal_0007 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srctest1";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dsttest6";
    string srcPath = "/storage/media/100/local/files/Docs/srctest1/";
    string dstPath = "/storage/media/100/local/files/Docs/dsttest6/";
    std::error_code errCode;
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    std::ofstream outfile1(dstPath + "d1.txt");
    outfile1 << "Test d1.txt";
    outfile1.close();

    std::ofstream outfile2(dstPath + "d2.txt");
    outfile2 << "Test d2.txt";
    outfile2.close();

    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        GTEST_LOG_(INFO) << "processSize = " << processSize << "totalFileSize = " << totalFileSize;
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(std::filesystem::exists(dstPath + "d1.txt"));
    EXPECT_TRUE(std::filesystem::exists(dstPath + "d2.txt"));
    JudgeCopyFile1(srcPath, dstPath);
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0006 End";
}

/**
 * @tc.name: FileCopyManager_TestCopy_0007
 * @tc.desc: test MakeDir function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0007, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0007 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srctest2";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dsttest7";
    string srcPath = "/storage/media/100/local/files/Docs/srctest2/";
    string dstPath = "/storage/media/100/local/files/Docs/dsttest7/";
    std::error_code errCode;
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    PrepareData2(srcPath, dstPath);

    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        GTEST_LOG_(INFO) << "processSize = " << processSize << "totalFileSize = " << totalFileSize;
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_OK);
    JudgeCopyFile4(srcPath, dstPath);
    ForceRemoveDirectory(srcPath.c_str());
    ForceRemoveDirectory(dstPath.c_str());
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0007 End";
}

/**
 * @tc.name: FileCopyManager_TestCopy_0008
 * @tc.desc: test copy function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0008, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0008 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srctest2";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dsttest8";
    string srcPath = "/storage/media/100/local/files/Docs/srctest2/";
    string dstPath = "/storage/media/100/local/files/Docs/dsttest8/";
    std::error_code errCode;
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    PrepareData2(srcPath, dstPath);

    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        double pro = (double)processSize / (double)totalFileSize;
        GTEST_LOG_(INFO) << "processSize = " << processSize << " totalFileSize = " << totalFileSize << " pro = " << pro;
        if (pro > 0.6) {
            GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0008 Cancel";
            Storage::DistributedFile::FileCopyManager::GetInstance().Cancel();
        }
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_DFS_CANCEL_SUCCESS);
    EXPECT_TRUE(std::filesystem::exists(dstPath + "srctest2"));
    EXPECT_FALSE(std::filesystem::exists(dstPath + "srctest2/test1"));
    ForceRemoveDirectory(srcPath.c_str());
    ForceRemoveDirectory(dstPath.c_str());
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0008 End";
}

/**
 * @tc.name: FileCopyManager_TestCopy_0009
 * @tc.desc: test copy function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0009, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0009 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srctest2";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dsttest9";
    string srcPath = "/storage/media/100/local/files/Docs/srctest2/";
    string dstPath = "/storage/media/100/local/files/Docs/dsttest9/";
    std::error_code errCode;
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    PrepareData2(srcPath, dstPath);

    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        double pro = (double)processSize / (double)totalFileSize;
        GTEST_LOG_(INFO) << "processSize = " << processSize << " totalFileSize = " << totalFileSize << " pro = " << pro;
        if (pro > 0.6) {
            GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0009 Cancel";
            Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(srcUri, dstUri);
        }
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_DFS_CANCEL_SUCCESS);
    EXPECT_TRUE(std::filesystem::exists(dstPath + "srctest2"));
    EXPECT_FALSE(std::filesystem::exists(dstPath + "srctest2/test1"));
    ForceRemoveDirectory(srcPath.c_str());
    ForceRemoveDirectory(dstPath.c_str());
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0009 End";
}

/**
 * @tc.name: FileCopyManager_TestCopy_0010
 * @tc.desc: test copy function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0010, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0010 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srctest2";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dsttest10";
    string srcPath = "/storage/media/100/local/files/Docs/srctest2/";
    string dstPath = "/storage/media/100/local/files/Docs/dsttest10/";
    std::error_code errCode;
    if (!std::filesystem::exists(srcPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(srcPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    const std::size_t fileSize = 1000 * 1024 * 1024;
    CreateFile(srcPath + "1.txt", fileSize);
    CreateFile(srcPath + "2.txt", fileSize);

    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        double pro = (double)processSize / (double)totalFileSize;
        GTEST_LOG_(INFO) << "processSize = " << processSize << " totalFileSize = " << totalFileSize << " pro = " << pro;
        if (pro > 0.6) {
            GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0010 Cancel";
            Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(true);
        }
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_DFS_CANCEL_SUCCESS);
    JudgeFileEqual(srcPath + "1.txt", dstPath + "/srctest2/1.txt");
    EXPECT_TRUE(std::filesystem::exists(dstPath + "/srctest2/2.txt"));
    ForceRemoveDirectory(srcPath.c_str());
    ForceRemoveDirectory(dstPath.c_str());
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0010 End";
}

/**
 * @tc.name: FileCopyManager_TestCopy_0011
 * @tc.desc: test copy function.
 * @tc.type: FUNC
 * @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0011, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0011 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/srctest2";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/dsttest10";
    string srcPath = "/storage/media/100/local/files/Docs/srctest2/";
    string dstPath = "/storage/media/100/local/files/Docs/dsttest10/";
    std::error_code errCode;
    if (!std::filesystem::exists(srcPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(srcPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    if (!std::filesystem::exists(dstPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(dstPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    const std::size_t fileSize = 1000 * 1024 * 1024;
    CreateFile(srcPath + "1.txt", fileSize);
    CreateFile(srcPath + "2.txt", fileSize);

    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        double pro = (double)processSize / (double)totalFileSize;
        GTEST_LOG_(INFO) << "processSize = " << processSize << " totalFileSize = " << totalFileSize << " pro = " << pro;
        if (pro > 0.6) {
            GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0010 Cancel";
            Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(srcUri, dstUri, true);
        }
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_DFS_CANCEL_SUCCESS);
    JudgeFileEqual(srcPath + "1.txt", dstPath + "/srctest2/1.txt");
    EXPECT_TRUE(std::filesystem::exists(dstPath + "/srctest2/2.txt"));
    ForceRemoveDirectory(srcPath.c_str());
    ForceRemoveDirectory(dstPath.c_str());
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0011 End";
}

/**
* @tc.name: FileCopyManager_TestCopy_0012
* @tc.desc: The execution of the Copy succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0012, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0012 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/test1/a1.txt";
    string srcPath = "/storage/media/100/local/files/Docs/1.txt";
    string dstPath = "/storage/media/100/local/files/Docs/test1/a1.txt";
    const std::size_t fileSize = 500 * 1024 * 1024;
    CreateFile(srcPath, fileSize);
    EXPECT_TRUE(std::filesystem::exists(srcPath));
    std::error_code errCode;
    string tmpPath = "/storage/media/100/local/files/Docs/test1/";
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        GTEST_LOG_(INFO) << "processSize = " << processSize << "totalFileSize = " << totalFileSize;
        if ((double)processSize / (double)totalFileSize > 0.1) {
            Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(srcUri, dstUri, true);
        }
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_DFS_CANCEL_SUCCESS);
    EXPECT_TRUE(std::filesystem::exists(dstPath));
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    ForceRemoveDirectory(tmpPath.c_str());
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0012 End";
}

/**
* @tc.name: FileCopyManager_TestCopy_0013
* @tc.desc: The execution of the Copy succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0013, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0013 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/test1/a1.txt";
    string srcPath = "/storage/media/100/local/files/Docs/1.txt";
    string dstPath = "/storage/media/100/local/files/Docs/test1/a1.txt";
    const std::size_t fileSize = 500 * 1024 * 1024;
    CreateFile(srcPath, fileSize);
    EXPECT_TRUE(std::filesystem::exists(srcPath));
    std::error_code errCode;
    string tmpPath = "/storage/media/100/local/files/Docs/test1/";
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        GTEST_LOG_(INFO) << "processSize = " << processSize << "totalFileSize = " << totalFileSize;
        if ((double)processSize / (double)totalFileSize > 0.1) {
            Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(true);
        }
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_DFS_CANCEL_SUCCESS);
    EXPECT_TRUE(std::filesystem::exists(dstPath));
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    ForceRemoveDirectory(tmpPath.c_str());
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0013 End";
}

/**
* @tc.name: FileCopyManager_TestCopy_0014
* @tc.desc: The execution of the Copy succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0014, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0014 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/test1/a1.txt";
    string srcPath = "/storage/media/100/local/files/Docs/1.txt";
    string dstPath = "/storage/media/100/local/files/Docs/test1/a1.txt";
    const std::size_t fileSize = 500 * 1024 * 1024;
    CreateFile(srcPath, fileSize);
    EXPECT_TRUE(std::filesystem::exists(srcPath));
    std::error_code errCode;
    string tmpPath = "/storage/media/100/local/files/Docs/test1/";
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        GTEST_LOG_(INFO) << "processSize = " << processSize << "totalFileSize = " << totalFileSize;
        if ((double)processSize / (double)totalFileSize > 0.1) {
            Storage::DistributedFile::FileCopyManager::GetInstance().Cancel(srcUri, dstUri);
        }
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_DFS_CANCEL_SUCCESS);
    EXPECT_FALSE(std::filesystem::exists(dstPath));
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    ForceRemoveDirectory(tmpPath.c_str());
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0014 End";
}

/**
* @tc.name: FileCopyManager_TestCopy_0015
* @tc.desc: The execution of the Copy succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(FileCopyManagerTest, FileCopyManager_TestCopy_0015, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0015 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string dstUri = "file://docs/storage/media/100/local/files/Docs/test1/a1.txt";
    string srcPath = "/storage/media/100/local/files/Docs/1.txt";
    string dstPath = "/storage/media/100/local/files/Docs/test1/a1.txt";
    const std::size_t fileSize = 500 * 1024 * 1024;
    CreateFile(srcPath, fileSize);
    EXPECT_TRUE(std::filesystem::exists(srcPath));
    std::error_code errCode;
    string tmpPath = "/storage/media/100/local/files/Docs/test1/";
    if (!std::filesystem::exists(tmpPath, errCode) && errCode.value() == E_OK) {
        int res = Storage::DistributedFile::FileCopyManager::GetInstance().MakeDir(tmpPath);
        if (res != E_OK) {
            GTEST_LOG_(INFO) <<"Failed to mkdir";
        }
    } else if (errCode.value() != E_OK) {
        GTEST_LOG_(INFO) <<"fs exists failed";
    }
    callBack listener_ = [&](uint64_t processSize, uint64_t totalFileSize) {
        GTEST_LOG_(INFO) << "processSize = " << processSize << "totalFileSize = " << totalFileSize;
        if ((double)processSize / (double)totalFileSize > 0.1) {
            Storage::DistributedFile::FileCopyManager::GetInstance().Cancel();
        }
    };

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance().Copy(srcUri, dstUri, listener_);
    EXPECT_EQ(ret, E_DFS_CANCEL_SUCCESS);
    EXPECT_FALSE(std::filesystem::exists(dstPath));
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    ForceRemoveDirectory(tmpPath.c_str());
    GTEST_LOG_(INFO) << "FileCopyManager_TestCopy_0015 End";
}
}