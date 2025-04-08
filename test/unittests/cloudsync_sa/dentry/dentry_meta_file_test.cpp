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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "directory_ex.h"
#include "meta_file.h"
#include "base_interface_lib_mock.h"

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
constexpr uint32_t TEST_USER_ID = 201;
constexpr uint64_t TEST_ISIZE = 1024;
const string TEST_PATH = "/data/service/el2/100/hmdfs/cache/account_cache/dentry_cache/cloud/";

class DentryMetaFileTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    static inline shared_ptr<InterfaceLibMock> interfaceLibMock_ = nullptr;
};

void DentryMetaFileTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    interfaceLibMock_ = make_shared<InterfaceLibMock>();
    InterfaceLibMock::baseInterfaceLib_ = interfaceLibMock_;
}

void DentryMetaFileTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    InterfaceLibMock::baseInterfaceLib_ = nullptr;
    interfaceLibMock_ = nullptr;
}

void DentryMetaFileTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void DentryMetaFileTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: MetaFileHandleFileByFd001
 * @tc.desc: Verify the MetaFile::HandleFileByFd function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DentryMetaFileTest, MetaFileHandleFileByFd001, TestSize.Level1)
{
    MetaFile mFile(TEST_USER_ID, "/");
    unsigned long endBlock = 0;
    uint32_t level = 0;
    int ret = mFile.HandleFileByFd(endBlock, level);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: MetaFileHandleFileByFd002
 * @tc.desc: Verify the MetaFile::HandleFileByFd function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DentryMetaFileTest, MetaFileHandleFileByFd002, TestSize.Level1)
{
    uint32_t userId = 100;
    MetaFile mFile(userId, "/");
    unsigned long endBlock = 0;
    uint32_t level = 64;
    int ret = mFile.HandleFileByFd(endBlock, level);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: MetaFileHandleFileByFd003
 * @tc.desc: Verify the MetaFile::HandleFileByFd function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DentryMetaFileTest, MetaFileHandleFileByFd003, TestSize.Level1)
{
    uint32_t userId = 100;
    MetaFile mFile(userId, "/");
    unsigned long endBlock = 0;
    uint32_t level = 0;
    int ret = mFile.HandleFileByFd(endBlock, level);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: MetaFileCreate001
 * @tc.desc: Verify the MetaFile::DoCreate function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DentryMetaFileTest, MetaFileCreate001, TestSize.Level1)
{
    uint32_t userId = 100;
    std::string cacheDir =
        "/data/service/el2/" + std::to_string(userId) + "/hmdfs/cache/cloud_cache/dentry_cache/cloud/";
    ForceRemoveDirectory(cacheDir);

    MetaFile mFile(userId, "/");
    MetaBase mBase1("file1", "id1");
    mBase1.size = TEST_ISIZE;
    int ret = mFile.DoCreate(mBase1);
    EXPECT_EQ(ret, 0);
    MetaBase mBase2("file2", "id2");
    mBase2.size = TEST_ISIZE;
    ret = mFile.DoCreate(mBase2);
    EXPECT_EQ(ret, 0);
    MetaFile mFile2(userId, "/a/b");
    MetaBase mBase3("file3", "id3");
    ret = mFile2.DoCreate(mBase3);
}

/**
 * @tc.name: MetaFileCreate002
 * @tc.desc: Verify the MetaFile::DoCreate function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DentryMetaFileTest, MetaFileCreate002, TestSize.Level1)
{
    MetaFile mFile(TEST_USER_ID, "/");
    MetaBase mBase1("file1", "id1");
    int ret = mFile.DoCreate(mBase1);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: MetaFileLookup001
 * @tc.desc: Verify the MetaFile::DoLookup function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DentryMetaFileTest, MetaFileLookup001, TestSize.Level1)
{
    MetaFile mFile(TEST_USER_ID, "/");
    MetaBase mBase1("file1");
    int ret = mFile.DoLookup(mBase1);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: MetaFileLookup002
 * @tc.desc: Verify the MetaFile::DoLookup002 function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DentryMetaFileTest, MetaFileLookup, TestSize.Level1)
{
    uint32_t userId = 100;
    MetaFile mFile(userId, "/");
    MetaBase mBase1("file1");
    int ret = mFile.DoLookup(mBase1);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(mBase1.size, TEST_ISIZE);
}

/**
 * @tc.name: MetaFileUpdate001
 * @tc.desc: Verify the MetaFile::DoUpdate function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKC
 */
HWTEST_F(DentryMetaFileTest, MetaFileUpdate001, TestSize.Level1)
{
    MetaFile mFile(TEST_USER_ID, "/");
    MetaBase mBase1("file1", "id11");
    int ret = mFile.DoUpdate(mBase1);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: MetaFileUpdate
 * @tc.desc: Verify the MetaFile::DoUpdate function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKC
 */
HWTEST_F(DentryMetaFileTest, MetaFileUpdate, TestSize.Level1)
{
    uint32_t userId = 100;
    MetaFile mFile(userId, "/");
    MetaBase mBase1("file1", "id11");
    mBase1.size = 0;
    int ret = mFile.DoUpdate(mBase1);
    EXPECT_EQ(ret, 0);
    MetaBase mBase2("file1");
    ret = mFile.DoLookup(mBase2);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(mBase2.size, 0);
}

/**
 * @tc.name: MetaFileRename
 * @tc.desc: Verify the MetaFile::DoRename function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKC
 */
HWTEST_F(DentryMetaFileTest, MetaFileRename, TestSize.Level1)
{
    uint32_t userId = 100;
    MetaFile mFile(userId, "/");
    MetaBase mBase1("file2");
    int ret = mFile.DoRename(mBase1, "file4");
    EXPECT_EQ(ret, 0);
    MetaBase mBase2("file4");
    ret = mFile.DoLookup(mBase2);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(mBase2.size, TEST_ISIZE);
}

/**
 * @tc.name: MetaFileRemove001
 * @tc.desc: Verify the MetaFile::DoRemove function
 * @tc.type: FUNC
 * @tc.require: SR000HRKJB
 */
HWTEST_F(DentryMetaFileTest, MetaFileRemove001, TestSize.Level1)
{
    MetaFile mFile(TEST_USER_ID, "/");
    MetaBase mBase1("file1");
    int ret = mFile.DoRemove(mBase1);
    EXPECT_EQ(ret, EINVAL);
}

/**
 * @tc.name: LoadChildren001
 * @tc.desc: Verify the LoadChildren
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DentryMetaFileTest, LoadChildren001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadChildren001 Start";
    try {
        uint32_t userId = 100;
        MetaFile mFile(userId, "/");
        std::vector<MetaBase> bases;
        int ret = mFile.LoadChildren(bases);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << "LoadChildren001 ERROR";
    }
    GTEST_LOG_(INFO) << "LoadChildren001 End";
}

/**
 * @tc.name: LoadChildren002
 * @tc.desc: Verify the LoadChildren
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DentryMetaFileTest, LoadChildren002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadChildren002 Start";
    try {
        MetaFile mFile(TEST_USER_ID, "/");
        std::vector<MetaBase> bases;
        int ret = mFile.LoadChildren(bases);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << "LoadChildren002 ERROR";
    }
    GTEST_LOG_(INFO) << "LoadChildren002 End";
}

/**
 * @tc.name: MetaFileMgr001
 * @tc.desc: Verify the MetaFileMgr
 * @tc.type: FUNC
 * @tc.require: SR000HRKJB
 */
HWTEST_F(DentryMetaFileTest, MetaFileMgr001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MetaFileMgr001 Start";
    try {
        uint32_t userId = 100;
        auto m = MetaFileMgr::GetInstance().GetMetaFile(userId, "/o/p/q/r/s/t");
        MetaBase mBase1("file1", "file1");
        EXPECT_EQ(m->DoCreate(mBase1), 0);
        m = nullptr;
        MetaFileMgr::GetInstance().ClearAll();
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " MetaFileMgr001 ERROR";
    }
    GTEST_LOG_(INFO) << "MetaFileMgr001 End";
}

/**
 * @tc.name: MetaFileMgr002
 * @tc.desc: Verify the MetaFileMgr
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DentryMetaFileTest, MetaFileMgr002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MetaFileMgr002 Start";
    try {
        uint32_t userId = 100;
        auto m = MetaFileMgr::GetInstance().GetMetaFile(userId, "/o/p/q/r/s/t");
        MetaBase mBase1("testLongLongfileName", "testLongLongfileName");
        EXPECT_EQ(m->DoCreate(mBase1), 0);
        m = nullptr;
        MetaFileMgr::GetInstance().ClearAll();
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " MetaFileMgr002 ERROR";
    }
    GTEST_LOG_(INFO) << "MetaFileMgr002 End";
}

/**
 * @tc.name: MetaFileMgr003
 * @tc.desc: Verify the MetaFileMgr
 * @tc.type: FUNC
 * @tc.require: issueIBLAKM
 */
HWTEST_F(DentryMetaFileTest, MetaFileMgr003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MetaFileMgr003 Start";
    try {
        uint32_t userId = 100;
        for (int i = 0; i < MAX_META_FILE_NUM; i++) {
            auto m = MetaFileMgr::GetInstance().GetMetaFile(userId, to_string(i));
        }
        auto m = MetaFileMgr::GetInstance().GetMetaFile(userId, "/o/p/q/r/s/t");
        auto checkSize = MetaFileMgr::GetInstance().CheckMetaFileSize();
        if (checkSize == -1) {
            GTEST_LOG_(INFO) << "metaFiles_.size() and metaFileList_.size() not same";
            EXPECT_FALSE(false);
        } else if (checkSize > MAX_META_FILE_NUM) {
            GTEST_LOG_(INFO) << "metaFileList_.size() more than MAX_META_FILE_NUM";
            EXPECT_FALSE(false);
        }
        m = nullptr;
        MetaFileMgr::GetInstance().ClearAll();
        EXPECT_EQ(checkSize, MAX_META_FILE_NUM);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " MetaFileMgr003 ERROR";
    }
    GTEST_LOG_(INFO) << "MetaFileMgr003 End";
}

/**
 * @tc.name:RecordIdToCloudId001
 * @tc.desc: Verify the RecordIdToCloudId
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DentryMetaFileTest, RecordIdToCloudId001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecordIdToCloudId001 Start";
    try {
        std::string hexStr = "";
        string ret = MetaFileMgr::GetInstance().RecordIdToCloudId(hexStr);
        EXPECT_EQ(ret, "");
        MetaFileMgr::GetInstance().ClearAll();
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << "RecordIdToCloudId001 ERROR";
    }
    GTEST_LOG_(INFO) << "RecordIdToCloudId001 End";
}

/**
 * @tc.name:RecordIdToCloudId002
 * @tc.desc: Verify the RecordIdToCloudId
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DentryMetaFileTest, RecordIdToCloudId002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RecordIdToCloudId002 Start";
    try {
        std::string hexStr = "test";
        string ret = MetaFileMgr::GetInstance().RecordIdToCloudId(hexStr);
        EXPECT_EQ(ret, "");
        MetaFileMgr::GetInstance().ClearAll();
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << "RecordIdToCloudId002 ERROR";
    }
    GTEST_LOG_(INFO) << "RecordIdToCloudId002 End";
}

/**
 * @tc.name: GetParentDir001
 * @tc.desc: Verify the GetParentDir
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DentryMetaFileTest, GetParentDir001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetParentDir001 Start";
    try {
        string str = MetaFile::GetParentDir("");
        EXPECT_EQ(str, "");
        str = MetaFile::GetParentDir("/");
        EXPECT_EQ(str, "");
        str = MetaFile::GetParentDir("abc");
        EXPECT_EQ(str, "/");
        str = MetaFile::GetParentDir("/abc");
        EXPECT_EQ(str, "/");
        str = MetaFile::GetParentDir("/abc/def");
        EXPECT_EQ(str, "/abc");
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " GetParentDir001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetParentDir001 End";
}

/**
 * @tc.name: GetFileName001
 * @tc.desc: Verify the GetFileName
 * @tc.type: FUNC
 * @tc.require: issueI7SP3A
 */
HWTEST_F(DentryMetaFileTest, GetFileName001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFileName001 Start";
    try {
        string str = MetaFile::GetFileName("");
        EXPECT_EQ(str, "");
        str = MetaFile::GetFileName("/");
        EXPECT_EQ(str, "");
        str = MetaFile::GetFileName("abc");
        EXPECT_EQ(str, "");
        str = MetaFile::GetFileName("/abc/def");
        EXPECT_EQ(str, "def");
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " GetFileName001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetFileName001 End";
}

HWTEST_F(DentryMetaFileTest, HandleFileByFd_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleFileByFd_001 Start";
    try {
        uint32_t userId = 100;
        MetaFile mFile(userId, "/");
        unsigned long endBlock = 0;
        uint32_t level = MAX_BUCKET_LEVEL;
        int ret = mFile.HandleFileByFd(endBlock, level);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " HandleFileByFd_001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleFileByFd_001 End";
}

HWTEST_F(DentryMetaFileTest, HandleFileByFd_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleFileByFd_002 Start";
    try {
        uint32_t userId = 100;
        MetaFile mFile(userId, "/");
        unsigned long endBlock = DENTRY_PER_GROUP;
        uint32_t level = MAX_BUCKET_LEVEL;
        int ret = mFile.HandleFileByFd(endBlock, level);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " HandleFileByFd_002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleFileByFd_002 End";
}

HWTEST_F(DentryMetaFileTest, HandleFileByFd_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleFileByFd_003 Start";
    try {
        uint32_t userId = 100;
        MetaFile mFile(userId, "/");
        unsigned long endBlock = -100;
        uint32_t level = MAX_BUCKET_LEVEL;
        int ret = mFile.HandleFileByFd(endBlock, level);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " HandleFileByFd_003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleFileByFd_003 End";
}

HWTEST_F(DentryMetaFileTest, GetDentryfileName_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetDentryfileName_001 Start";
    try {
        uint32_t userId = 100;
        EXPECT_CALL(*interfaceLibMock_, GetDentryfileName(_, _)).WillOnce(Return("/"));
        MetaFile mFile(userId, "/");

        string path = "/test/";
        bool caseSense = true;
        EXPECT_TRUE(true);
        EXPECT_EQ(interfaceLibMock_->GetDentryfileName(path, caseSense), "/");
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " GetDentryfileName_001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetDentryfileName_001 End";
}

HWTEST_F(DentryMetaFileTest, GetOverallBucket_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetOverallBucket_001 Start";
    try {
        uint32_t userId = 100;
        MetaFile mFile(userId, "/");
        unsigned long endBlock = -100;
        uint32_t level = MAX_BUCKET_LEVEL;

        EXPECT_CALL(*interfaceLibMock_, GetOverallBucket(_)).WillOnce(Return(E_SUCCESS));
        int ret = mFile.HandleFileByFd(endBlock, level);
        EXPECT_EQ(ret, 0);
        EXPECT_EQ(interfaceLibMock_->GetOverallBucket(MAX_BUCKET_LEVEL), E_SUCCESS);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " GetOverallBucket_001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetOverallBucket_001 End";
}

HWTEST_F(DentryMetaFileTest, GetBucketaddr_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketaddr_001 Start";
    try {
        EXPECT_CALL(*interfaceLibMock_, GetBucketaddr(_, _)).WillOnce(Return(E_SUCCESS));
        uint32_t userId = 100;
        std::string cacheDir =
            "/data/service/el2/" + std::to_string(userId) + "/hmdfs/cache/cloud_cache/dentry_cache/cloud/";
        ForceRemoveDirectory(cacheDir);

        MetaFile mFile(userId, "/");
        MetaBase mBase1("file1", "id1");
        mBase1.size = TEST_ISIZE;
        int ret = mFile.DoCreate(mBase1);
        EXPECT_EQ(ret, E_SUCCESS);
        EXPECT_EQ(interfaceLibMock_->GetBucketaddr(MAX_BUCKET_LEVEL, MAX_BUCKET_LEVEL), E_SUCCESS);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " GetBucketaddr_001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBucketaddr_001 End";
}

HWTEST_F(DentryMetaFileTest, GetBucketaddr_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketaddr_002 Start";
    try {
        EXPECT_CALL(*interfaceLibMock_, GetBucketaddr(Eq(E_SUCCESS), _)).WillOnce(Return(E_FAIL));
        uint32_t userId = 100;
        std::string cacheDir =
            "/data/service/el2/" + std::to_string(userId) + "/hmdfs/cache/cloud_cache/dentry_cache/cloud/";
        ForceRemoveDirectory(cacheDir);

        MetaFile mFile(userId, "/");
        MetaBase mBase1("file1", "id1");
        mBase1.size = TEST_ISIZE;
        int ret = mFile.DoCreate(mBase1);
        EXPECT_EQ(ret, E_SUCCESS);
        EXPECT_EQ(interfaceLibMock_->GetBucketaddr(E_SUCCESS, MAX_BUCKET_LEVEL), E_FAIL);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " GetBucketaddr_002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBucketaddr_002 End";
}

HWTEST_F(DentryMetaFileTest, GetBucketByLevel_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetBucketByLevel_001 Start";
    try {
        EXPECT_CALL(*interfaceLibMock_, GetBucketByLevel(_)).WillOnce(Return(E_SUCCESS));
        uint32_t userId = 100;
        std::string cacheDir =
            "/data/service/el2/" + std::to_string(userId) + "/hmdfs/cache/cloud_cache/dentry_cache/cloud/";
        ForceRemoveDirectory(cacheDir);

        MetaFile mFile(userId, "/");
        MetaBase mBase1("file1", "id1");
        mBase1.size = TEST_ISIZE;
        int ret = mFile.DoCreate(mBase1);
        EXPECT_EQ(ret, E_SUCCESS);
        EXPECT_EQ(interfaceLibMock_->GetBucketByLevel(E_SUCCESS), E_SUCCESS);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " GetBucketByLevel_001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetBucketByLevel_001 End";
}

HWTEST_F(DentryMetaFileTest, RoomForFilename_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RoomForFilename_001 Start";
    try {
        EXPECT_CALL(*interfaceLibMock_, FindNextZeroBit(_, _, _)).WillOnce(Return(DENTRY_PER_GROUP));
        uint32_t userId = 100;
        std::string cacheDir =
            "/data/service/el2/" + std::to_string(userId) + "/hmdfs/cache/cloud_cache/dentry_cache/cloud/";
        ForceRemoveDirectory(cacheDir);

        MetaFile mFile(userId, "/");
        MetaBase mBase1("file1", "id1");
        mBase1.size = TEST_ISIZE;
        int ret = mFile.DoCreate(mBase1);
        uint8_t addr[] = {0};
        uint32_t maxSlots = DENTRY_PER_GROUP;
        uint32_t start = DENTRY_PER_GROUP;
        EXPECT_EQ(interfaceLibMock_->FindNextZeroBit(addr, maxSlots, start), DENTRY_PER_GROUP);
        EXPECT_EQ(ret, E_SUCCESS);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " RoomForFilename_001 ERROR";
    }
    GTEST_LOG_(INFO) << "RoomForFilename_001 End";
}

HWTEST_F(DentryMetaFileTest, RoomForFilename_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RoomForFilename_002 Start";
    try {
        EXPECT_CALL(*interfaceLibMock_, FindNextZeroBit(_, _, _)).WillOnce(Return(DENTRY_PER_GROUP));
        EXPECT_CALL(*interfaceLibMock_, FindNextBit(_, _, _)).WillOnce(Return(MAX_BUCKET_LEVEL));
        uint32_t userId = 100;
        std::string cacheDir =
            "/data/service/el2/" + std::to_string(userId) + "/hmdfs/cache/cloud_cache/dentry_cache/cloud/";
        ForceRemoveDirectory(cacheDir);

        MetaFile mFile(userId, "/");
        MetaBase mBase1("file1", "id1");
        mBase1.size = TEST_ISIZE;
        int ret = mFile.DoCreate(mBase1);
        uint8_t addr[] = {0};
        uint32_t maxSlots = DENTRY_PER_GROUP;
        uint32_t start = DENTRY_PER_GROUP;
        EXPECT_EQ(ret, E_SUCCESS);
        EXPECT_EQ(interfaceLibMock_->FindNextZeroBit(addr, maxSlots, start), DENTRY_PER_GROUP);
        EXPECT_EQ(interfaceLibMock_->FindNextBit(addr, maxSlots, start), MAX_BUCKET_LEVEL);
    } catch (...) {
        EXPECT_FALSE(false);
        GTEST_LOG_(INFO) << " RoomForFilename_002 ERROR";
    }
    GTEST_LOG_(INFO) << "RoomForFilename_002 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test