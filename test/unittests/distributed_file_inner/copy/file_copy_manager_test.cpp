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
#include <gtest/gtest.h>
#include "dfs_error.h"

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;

class FileCopyManagerTest : public testing::Test {
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
    string srcUri = "";
    string destUri = "/data/test/test.txt";

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Copy(srcUri, destUri, listener);
    EXPECT_EQ(ret, E_NOENT);
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

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Copy(srcUri, destUri, listener);
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

    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Copy(srcUri, destUri, listener);
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
    string srcUri = "file://docs/storage/media/100/local/files/Docs/11.txt";
    string destUri = "file://docs/storage/media/100/local/files/Docs/dest11.txt";
    string srcpath = "/storage/media/100/local/files/Docs/11.txt";
    int fd = open(srcpath.c_str(), O_RDWR | O_CREAT);
    ASSERT_TRUE(fd != -1) <<"Failed to open file in FileCopyManager_Copy_0004!" << errno;
    close(fd);
    auto ret = Storage::DistributedFile::FileCopyManager::GetInstance()->Cancel(srcUri, destUri);
    EXPECT_EQ(ret, E_OK);
    ASSERT_EQ(remove(srcpath.c_str()), 0);
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
    EXPECT_EQ(ret, E_NOENT);
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
}