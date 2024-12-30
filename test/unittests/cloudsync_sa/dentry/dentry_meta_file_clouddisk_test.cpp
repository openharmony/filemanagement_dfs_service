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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "directory_ex.h"
#include "meta_file.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
constexpr uint32_t TEST_USER_ID = 201;
constexpr uint64_t TEST_ISIZE = 1024;

class CloudDiskDentryMetaFileTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void CloudDiskDentryMetaFileTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskDentryMetaFileTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskDentryMetaFileTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void CloudDiskDentryMetaFileTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: CloudDiskMetaFileHandleFileByFd001
 * @tc.desc: Verify the MetaFile::HandleFileByFd function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskDentryMetaFileTest, CloudDiskMetaFileHandleFileByFd001, TestSize.Level1)
{
    MetaFile mFile(TEST_USER_ID, "/");
    unsigned long endBlock = 0;
    uint32_t level = 0;
    int ret = mFile.HandleFileByFd(endBlock, level);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: CloudDiskMetaFileHandleFileByFd002
 * @tc.desc: Verify the MetaFile::HandleFileByFd function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskDentryMetaFileTest, CloudDiskMetaFileHandleFileByFd002, TestSize.Level1)
{
    uint32_t userId = 100;
    CloudDiskMetaFile mFile(userId, "/", "id1");
    unsigned long endBlock = 0;
    uint32_t level = 64;
    int ret = mFile.HandleFileByFd(endBlock, level);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: CloudDiskMetaFileHandleFileByFd003
 * @tc.desc: Verify the MetaFile::HandleFileByFd function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskDentryMetaFileTest, CloudDiskMetaFileHandleFileByFd003, TestSize.Level1)
{
    uint32_t userId = 100;
    CloudDiskMetaFile mFile(userId, "/", "id1");
    unsigned long endBlock = 0;
    uint32_t level = 0;
    int ret = mFile.HandleFileByFd(endBlock, level);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: CloudDiskMetaFileDoChildUpdateTest
 * @tc.desc: Verify the CloudDiskMetaFile::DoChildUpdate function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKC
 */
HWTEST_F(CloudDiskDentryMetaFileTest, CloudDiskMetaFileDoChildUpdateTest, TestSize.Level1)
{
    CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
    UniqueFd fd(-1);
    mFile.fd_ = std::move(fd);
    string name = "";
    CloudDiskMetaFile::CloudDiskMetaFileCallBack callback;
    int ret = mFile.DoChildUpdate(name, callback);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: DoLookupAndRemoveTest
 * @tc.desc: Verify the CloudDiskMetaFile::DoLookupAndRemove function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskDentryMetaFileTest, DoLookupAndRemoveTest, TestSize.Level1)
{
    CloudDiskMetaFile mFile(100, "com.ohos.photos", "rootId");
    UniqueFd fd(-1);
    mFile.fd_ = std::move(fd);
    MetaBase mBase1(".trash", "rootId");
    int ret = mFile.DoLookupAndRemove(mBase1);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: CloudDiskDentryFileCreateTest
 * @tc.desc: Verify the CloudDiskMetaFile::DoCreate function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskDentryMetaFileTest, CloudDiskDentryFileCreateTest, TestSize.Level1)
{
    std::string cacheDir =
        "/data/service/el2/" + std::to_string(TEST_USER_ID) + "/hmdfs/cache/cloud_cache/dentry_cache/cloud/";
    ForceRemoveDirectory(cacheDir);

    string CloudId = "";
    CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
    MetaBase mBase1("file1", "id1");
    mBase1.size = TEST_ISIZE;
    int ret = mFile.DoCreate(mBase1);
    EXPECT_EQ(ret, 0);
    MetaBase mBase2("file2", "id2");
    mBase2.size = TEST_ISIZE;
    ret = mFile.DoCreate(mBase2);
    EXPECT_EQ(ret, 0);
    CloudDiskMetaFile mFile2(TEST_USER_ID, "/a/b", "id3");
    MetaBase mBase3("file3", "id3");
    ret = mFile2.DoCreate(mBase3);
}

/**
 * @tc.name: CloudDiskMetaFileLookupTest
 * @tc.desc: Verify the CloudDiskMetaFile::DoLookup function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(CloudDiskDentryMetaFileTest, CloudDiskMetaFileLookupTest, TestSize.Level1)
{
    CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
    MetaBase mBase1("file1");
    int ret = mFile.DoCreate(mBase1);
    ret = mFile.DoLookup(mBase1);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(mBase1.size, TEST_ISIZE);
}

/**
 * @tc.name: CloudDiskMetaFileUpdateTest
 * @tc.desc: Verify the CloudDiskMetaFile::DoUpdate function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKC
 */
HWTEST_F(CloudDiskDentryMetaFileTest, CloudDiskMetaFileUpdateTest, TestSize.Level1)
{
    CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
    MetaBase mBase1("file1", "id1");
    mBase1.size = 0;
    int ret = mFile.DoCreate(mBase1);
    ret = mFile.DoUpdate(mBase1);
    EXPECT_EQ(ret, 0);
    MetaBase mBase2("file1", "id1");
    ret = mFile.DoLookup(mBase2);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(mBase2.size, 0);
}

/**
 * @tc.name: CloudDiskMetaFileRenameTest
 * @tc.desc: Verify the CloudDiskMetaFile::DoRename function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKC
 */
HWTEST_F(CloudDiskDentryMetaFileTest, CloudDiskMetaFileRenameTest, TestSize.Level1)
{
    CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
    MetaBase mBase1("file1", "id1");
    std::shared_ptr<CloudDiskMetaFile> newMetaFile =
        make_shared<CloudDiskMetaFile>(TEST_USER_ID, "/", "id1");
    int ret = mFile.DoCreate(mBase1);
    ret = mFile.DoRename(mBase1, "id2", newMetaFile);
    EXPECT_EQ(ret, 0);
    MetaBase mBase2("id2");
    ret = mFile.DoLookup(mBase2);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: CloudDiskMetaFileRemoveTest
 * @tc.desc: Verify the CloudDiskMetaFile::DoRemove function
 * @tc.type: FUNC
 * @tc.require: SR000HRKJB
 */
HWTEST_F(CloudDiskDentryMetaFileTest, CloudDiskMetaFileRemoveTest, TestSize.Level1)
{
    CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
    MetaBase mBase1("file1", "id1");
    int ret = mFile.DoCreate(mBase1);
    ret = mFile.DoRemove(mBase1);
    EXPECT_EQ(ret, 0);
    MetaBase mBase2("file1", "id1");
    ret = mFile.DoLookup(mBase2);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name:LoadChildrenTest
 * @tc.desc: Verify the CloudDiskMetaFile::LoadChildren function
 * @tc.type: FUNC
 * @tc.require: SR000HRKJB
 */
HWTEST_F(CloudDiskDentryMetaFileTest, LoadChildrenTest, TestSize.Level1)
{
    CloudDiskMetaFile mFile(100, "/", "id1");
    MetaBase mBase1("file1", "id1");
    std::vector<MetaBase> bases;
    bases.push_back(mBase1);
    int32_t ret = mFile.LoadChildren(bases);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: CloudDiskMetaFileMgrTest001
 * @tc.desc: Verify the MetaFileMgr
 * @tc.type: FUNC
 * @tc.require: SR000HRKJB
 */
HWTEST_F(CloudDiskDentryMetaFileTest, CloudDiskMetaFileMgrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskMetaFileMgrTest001 Start";
    try {
        auto m = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(TEST_USER_ID, "/o/p/q/r/s/t", "id1");
        MetaBase mBase1("file1", "file1");
        EXPECT_EQ(m->DoCreate(mBase1), 0);
        m = nullptr;
        MetaFileMgr::GetInstance().CloudDiskClearAll();
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << "CloudDiskMetaFileMgrTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskMetaFileMgrTest001 End";
}

/**
 * @tc.name: CloudDiskMetaFileMgrTest002
 * @tc.desc: Verify the CloudDiskMetaFileMgr
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(CloudDiskDentryMetaFileTest, CloudDiskMetaFileMgrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CloudDiskMetaFileMgrTest002 Start";
    try {
        auto m = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(TEST_USER_ID, "/o/p/q/r/s/t", "id1");
        MetaBase mBase1("testLongLongfileName", "testLongLongfileName");
        EXPECT_EQ(m->DoCreate(mBase1), 0);
        m = nullptr;
        MetaFileMgr::GetInstance().CloudDiskClearAll();
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << "CloudDiskMetaFileMgrTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "CloudDiskMetaFileMgrTest002 End";
}

HWTEST_F(CloudDiskDentryMetaFileTest, DfsService_GetDentryFilePath_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_GetDentryFilePath_001 Start";
    try {
        CloudDiskMetaFile mFile(100, "/", "id1");
        string cacheFilePath = mFile.GetDentryFilePath();
        EXPECT_EQ((cacheFilePath != ""), true);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << "DfsService_GetDentryFilePath_001 ERROR";
    }
    GTEST_LOG_(INFO) << "DfsService_GetDentryFilePath_001 End";
}

HWTEST_F(CloudDiskDentryMetaFileTest, DfsService_DoLookupAndUpdate_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_DoLookupAndUpdate_001 Start";
    try {
        string fileName = "testLongLongfileName";
        string cloudId = "testLongLongfileName";
        unsigned long long size = 0x6b6b6b6b00000000;
        MetaBase metaBase(fileName, cloudId);
        metaBase.size = size;
        auto callback = [&metaBase] (MetaBase &m) {
            m.size = metaBase.size;
        };
        
        auto metaFile = MetaFileMgr::GetInstance().GetCloudDiskMetaFile(TEST_USER_ID, "/o/p/q/r/s/t", "id1");
        int32_t ret = metaFile->DoLookupAndUpdate(fileName, callback);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << "DfsService_DoLookupAndUpdate_001 ERROR";
    }
    GTEST_LOG_(INFO) << "DfsService_DoLookupAndUpdate_001 End";
}

HWTEST_F(CloudDiskDentryMetaFileTest, DfsService_DoChildUpdate_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_DoChildUpdate_001 Start";
    try {
        string name = "testName";
        CloudDiskMetaFile mFile(100, "/", "id1");
        CloudDiskMetaFile::CloudDiskMetaFileCallBack updateFunc;
        UniqueFd fd(-1);
        mFile.fd_ = std::move(fd);

        int32_t ret = mFile.DoChildUpdate(name, updateFunc);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << "DfsService_DoChildUpdate_001 ERROR";
    }
    GTEST_LOG_(INFO) << "DfsService_DoChildUpdate_001 End";
}

HWTEST_F(CloudDiskDentryMetaFileTest, DfsService_DoLookupAndRemove_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsService_DoLookupAndRemove_001 Start";
    try {
        CloudDiskMetaFile mFile(100, "com.ohos.photos", "rootId");
        MetaBase mBase1(".trash", "rootId");
        UniqueFd fd(-1);
        mFile.fd_ = std::move(fd);

        int32_t ret = mFile.DoLookupAndRemove(mBase1);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << "DfsService_DoLookupAndRemove_001 ERROR";
    }
    GTEST_LOG_(INFO) << "DfsService_DoLookupAndRemove_001 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test