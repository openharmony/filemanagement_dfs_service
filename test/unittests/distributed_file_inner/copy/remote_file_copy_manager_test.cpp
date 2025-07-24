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

#include "copy/remote_file_copy_manager.h"

#include <fstream>
#include <gtest/gtest.h>

#include "datashare_helper.h"
#include "dfs_error.h"
#include "directory_ex.h"
#include "distributed_file_fd_guard.h"
#include "file_copy_listener.h"
#include "file_copy_manager.h"
#include "file_size_utils.h"
#include "sandbox_helper.h"
#include "trans_listener.h"

std::string g_physicalPath = "/test/test";
int32_t g_getPhysicalPath = 0;

namespace OHOS::AppFileService {
int32_t SandboxHelper::GetPhysicalPath(const std::string &fileUri, const std::string &userId, std::string &physicalPath)
{
    physicalPath = g_physicalPath;
    return g_getPhysicalPath;
}
} // namespace OHOS::AppFileService

namespace OHOS::Storage::DistributedFile::Test {
using namespace OHOS::FileManagement;
using namespace testing;
using namespace testing::ext;
using namespace std;
class RemoteFileCopyManagerTest : public testing::Test {
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

void RemoteFileCopyManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void RemoteFileCopyManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void RemoteFileCopyManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void RemoteFileCopyManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
* @tc.name: RemoteFileCopyManager_Copy_0001
* @tc.desc: The execution of the Copy failed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(RemoteFileCopyManagerTest, RemoteFileCopyManager_Copy_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "FileCopyManager_Copy_0001 Start";
    string localUri = "/data/test/test.txt";
    string dstUri = "/data/test/test.txt";
    int32_t userId = 100;
    string copyPath = "/data/storage/el2/distributedfiles/123412345/test.txt";

    sptr<IFileTransListener> listenerCallback;

    auto ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoteCopy("", localUri, listenerCallback, userId, copyPath);
    EXPECT_EQ(ret, EINVAL);

    ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoteCopy(localUri, "", listenerCallback, userId, copyPath);
    EXPECT_EQ(ret, EINVAL);

    ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoteCopy("", "", listenerCallback, userId, copyPath);
    EXPECT_EQ(ret, EINVAL);

    ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoteCopy(localUri, dstUri, listenerCallback, userId, copyPath);
    EXPECT_EQ(ret, ENOENT);

    string remoteUri = "/data/test/Copy/?networkid=/";
    if (!ForceCreateDirectory(remoteUri)) {
        GTEST_LOG_(INFO) << "RemoteFileCopyManager_Copy_0001 create dir err";
    }
    ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoteCopy(remoteUri, "", listenerCallback, userId, copyPath);
    EXPECT_EQ(ret, EINVAL);
    if (!ForceRemoveDirectory(remoteUri)) {
        GTEST_LOG_(INFO) << "RemoteFileCopyManager_Copy_0001 remove dir err";
    }
    GTEST_LOG_(INFO) << "RemoteFileCopyManager_Copy_0001 End";
}

/**
* @tc.name: RemoteFileCopyManager_Copy_0002
* @tc.desc: The execution of the Copy succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(RemoteFileCopyManagerTest, RemoteFileCopyManager_Copy_0002, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RemoteFileCopyManager_Copy_0002 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/1.txt";
    string destUri = "file://docs/storage/media/100/local/files/Docs/dest1.txt";
    string srcPath = "/storage/media/100/local/files/Docs/1.txt";
    int32_t userId = 100;
    string copyPath = "/data/storage/el2/distributedfiles/123412345/dest1.txt";

    sptr<IFileTransListener> listenerCallback;
    int fd = open(srcPath.c_str(), O_RDWR | O_CREAT);
    ASSERT_TRUE(fd != -1) <<"Failed to open file in RemoteFileCopyManager_Copy_0002!" << errno;
    close(fd);

    auto ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoteCopy(srcUri, destUri, listenerCallback, userId, copyPath);
    EXPECT_EQ(ret, ENOENT);
    ASSERT_EQ(remove(srcPath.c_str()), 0);
    GTEST_LOG_(INFO) << "RemoteFileCopyManager_Copy_0002 End";
}

/**
* @tc.name: RemoteFileCopyManager_RemoteCancel_0001
* @tc.desc: The execution of the cancel succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(RemoteFileCopyManagerTest, RemoteFileCopyManager_RemoteCancel_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RemoteFileCopyManager_Cancel_0001 Start";
    auto infos1 = std::make_shared<FileInfos>();
    infos1->srcUri = "srcUri1";
    infos1->destUri = "destUri1";
    infos1->transListener = nullptr;
    infos1->localListener = std::make_shared<FileCopyLocalListener>("",
        true, [](uint64_t processSize, uint64_t totalSize) -> void {});
    std::string srcUri = "srcUri1";
    std::string destUri = "destUri3";

    Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->FileInfosVec_.emplace_back(infos1);
    auto ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoteCancel(srcUri, destUri);
    EXPECT_EQ(ret, E_OK);
    srcUri = "srcUri2";
    destUri = "destUri1";
    ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoteCancel(srcUri, destUri);
    EXPECT_EQ(ret, E_OK);

    srcUri = "srcUri2";
    destUri = "destUri2";
    ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoteCancel(srcUri, destUri);
    EXPECT_EQ(ret, E_OK);

    srcUri = "srcUri1";
    destUri = "destUri1";
    ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoteCancel(srcUri, destUri);
    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "RemoteFileCopyManager_RemoteCancel_0001 End";
}

/**
* @tc.name: RemoteFileCopyManager_RemoveFileInfos_0001
* @tc.desc: RemoveFileInfos
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(RemoteFileCopyManagerTest, RemoteFileCopyManager_RemoveFileInfos_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RemoteFileCopyManager_RemoveFileInfos_0001 Start";
    auto infos1 = std::make_shared<FileInfos>();
    auto infos2 = std::make_shared<FileInfos>();
    auto infos3 = std::make_shared<FileInfos>();
    infos1->srcUri = "srcUri1";
    infos1->destUri = "destUri1";
    infos2->srcUri = "srcUri2";
    infos2->destUri = "destUri2";
    infos3->srcUri = "srcUri1";
    infos3->destUri = "destUri2";
    Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->FileInfosVec_.clear();
    Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->AddFileInfos(infos1);
    Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->AddFileInfos(infos2);
    auto len = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->FileInfosVec_.size();
    EXPECT_EQ(len, 2); // 2: vector size

    Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoveFileInfos(infos2);
    len = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->FileInfosVec_.size();
    EXPECT_EQ(len, 1); // 1: vector size

    Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoveFileInfos(infos3);
    len = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->FileInfosVec_.size();
    EXPECT_EQ(len, 1); // 1: vector size

    Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->RemoveFileInfos(infos1);
    len = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->FileInfosVec_.size();
    EXPECT_EQ(len, 0); // 0: vector size
}

/**
* @tc.name: RemoteFileCopyManager_CreateFileInfos_0001
* @tc.desc: CreateFileInfos
* @tc.type: FUNC
* @tc.require: I7TDJK
*/
HWTEST_F(RemoteFileCopyManagerTest, RemoteFileCopyManager_CreateFileInfos_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RemoteFileCopyManager_CreateFileInfos_0001 Start";
    auto infos = std::make_shared<FileInfos>();
    infos->srcUri = "";
    int32_t userId = 100;
    string copyPath = "/data/storage/el2/distributedfiles/123412345/test.txt";
    int32_t ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->CreateFileInfos("", "", infos, userId, copyPath);
    EXPECT_EQ(ret, ENOENT);
    GTEST_LOG_(INFO) << "RemoteFileCopyManager_CreateFileInfos_0001 End";
}

/**
* @tc.name: FileCopyManager_Cancel_0001
* @tc.desc: The execution of the cancel succeed.
* @tc.type: FUNC
* @tc.require: I7TDJK
 */
HWTEST_F(RemoteFileCopyManagerTest, RemoteFileCopyManager_IsMediaUri_0001, TestSize.Level0)
{
    GTEST_LOG_(INFO) << "RemoteFileCopyManager_IsMediaUri_0001 Start";
    string srcUri = "file://docs/storage/media/100/local/files/Docs/test.txt";
    int32_t ret = Storage::DistributedFile::RemoteFileCopyManager::GetInstance()->IsMediaUri(srcUri);
    EXPECT_EQ(ret, false);
    GTEST_LOG_(INFO) << "RemoteFileCopyManager_IsMediaUri_0001 End";
}
}