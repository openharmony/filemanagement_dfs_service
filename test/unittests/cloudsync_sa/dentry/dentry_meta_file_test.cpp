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

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
constexpr uint32_t TEST_USER_ID = 201;
constexpr uint64_t TEST_ISIZE = 1024;

class DentryMetaFileTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};
void DentryMetaFileTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void DentryMetaFileTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
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
 * @tc.name: MetaFileCreate
 * @tc.desc: Verify the MetaFile::DoCreate function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DentryMetaFileTest, MetaFileCreate, TestSize.Level1)
{
    std::string cacheDir =
        "/data/service/el2/" + std::to_string(TEST_USER_ID) + "/hmdfs/cache/cloud_cache/dentry_cache/cloud/";
    ForceRemoveDirectory(cacheDir);

    MetaFile mFile(TEST_USER_ID, "/");
    MetaBase mBase1("file1", "id1");
    mBase1.size = TEST_ISIZE;
    int ret = mFile.DoCreate(mBase1);
    EXPECT_EQ(ret, 0);
    MetaBase mBase2("file2", "id2");
    mBase2.size = TEST_ISIZE;
    ret = mFile.DoCreate(mBase2);
    EXPECT_EQ(ret, 0);
    MetaFile mFile2(TEST_USER_ID, "/a/b");
    MetaBase mBase3("file3", "id3");
    ret = mFile2.DoCreate(mBase3);
}

/**
 * @tc.name: MetaFileLookup
 * @tc.desc: Verify the MetaFile::DoLookup function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKA
 */
HWTEST_F(DentryMetaFileTest, MetaFileLookup, TestSize.Level1)
{
    MetaFile mFile(TEST_USER_ID, "/");
    MetaBase mBase1("file1");
    int ret = mFile.DoLookup(mBase1);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(mBase1.size, TEST_ISIZE);
}

/**
 * @tc.name: MetaFileUpdate
 * @tc.desc: Verify the MetaFile::DoUpdate function
 * @tc.type: FUNC
 * @tc.require: SR000HRKKC
 */
HWTEST_F(DentryMetaFileTest, MetaFileUpdate, TestSize.Level1)
{
    MetaFile mFile(TEST_USER_ID, "/");
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
    MetaFile mFile(TEST_USER_ID, "/");
    MetaBase mBase1("file2");
    int ret = mFile.DoRename(mBase1, "file4");
    EXPECT_EQ(ret, 0);
    MetaBase mBase2("file4");
    ret = mFile.DoLookup(mBase2);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(mBase2.size, TEST_ISIZE);
}

/**
 * @tc.name: MetaFileRemove
 * @tc.desc: Verify the MetaFile::DoRemove function
 * @tc.type: FUNC
 * @tc.require: SR000HRKJB
 */
HWTEST_F(DentryMetaFileTest, MetaFileRemove, TestSize.Level1)
{
    MetaFile mFile(TEST_USER_ID, "/");
    MetaBase mBase1("file1");
    int ret = mFile.DoRemove(mBase1);
    EXPECT_EQ(ret, 0);
    MetaBase mBase2("file1");
    ret = mFile.DoLookup(mBase2);
    EXPECT_EQ(ret, ENOENT);
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
        auto m = MetaFileMgr::GetInstance().GetMetaFile(TEST_USER_ID, "/o/p/q/r/s/t");
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
        auto m = MetaFileMgr::GetInstance().GetMetaFile(TEST_USER_ID, "/o/p/q/r/s/t");
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
} // namespace OHOS::FileManagement::CloudSync::Test
