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

#include "../../../utils/dentry/src/meta_file_clouddisk.cpp"
#include "assistant.h"

namespace OHOS::FileManagement {
using namespace testing;
using namespace testing::ext;

constexpr uint32_t TEST_USER_ID = 201;
constexpr uint64_t TEST_ISIZE = 1024;

class MetaFileCloudDiskTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline std::shared_ptr<CloudDisk::AssistantMock> insMock = nullptr;
};

void MetaFileCloudDiskTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void MetaFileCloudDiskTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void MetaFileCloudDiskTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock = std::make_shared<CloudDisk::AssistantMock>();
    CloudDisk::AssistantMock::EnableMock();
    CloudDisk::Assistant::ins = insMock;
}

void MetaFileCloudDiskTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    CloudDisk::AssistantMock::DisableMock();
    CloudDisk::Assistant::ins = nullptr;
    insMock = nullptr;
}

/**
 * @tc.name: SetFileType001
 * @tc.desc: Verify the SetFileType function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, SetFileType001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetFileType001 Start";
    try {
        HmdfsDentry de;
        MetaHelper::SetFileType(&de, FILE_TYPE_CONTENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetFileType001 ERROR";
    }
    GTEST_LOG_(INFO) << "SetFileType001 End";
}

/**
 * @tc.name: FindDentryPage001
 * @tc.desc: Verify the FindDentryPage function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, FindDentryPage001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindDentryPage001 Start";
    try {
        // init InitDcacheLookupCtx
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        MetaBase mBase("file1", "id1");
        mBase.size = TEST_ISIZE;
        DcacheLookupCtx ctx;
        InitDcacheLookupCtx(&ctx, mBase, -1);

        // FindDentryPage -> nullptr || FilePosLock -> 1
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillOnce(Return(1));
        auto dentryBlk = FindDentryPage(0, &ctx);
        EXPECT_EQ(dentryBlk, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindDentryPage001 ERROR";
    }
    GTEST_LOG_(INFO) << "FindDentryPage001 End";
}

/**
 * @tc.name: FindDentryPage002
 * @tc.desc: Verify the FindDentryPage function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, FindDentryPage002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindDentryPage002 Start";
    try {
        // init InitDcacheLookupCtx
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        MetaBase mBase("file1", "id1");
        mBase.size = TEST_ISIZE;
        DcacheLookupCtx ctx;
        InitDcacheLookupCtx(&ctx, mBase, -1);

        // FindDentryPage -> nullptr || FilePosLock -> 0&& ReadFile ->0
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(0));
        auto dentryBlk = FindDentryPage(0, &ctx);
        EXPECT_EQ(dentryBlk, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindDentryPage002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindDentryPage002 End";
}

/**
 * @tc.name: FindDentryPage003
 * @tc.desc: Verify the FindDentryPage function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, FindDentryPage003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindDentryPage003 Start";
    try {
        // init InitDcacheLookupCtx
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        MetaBase mBase("file1", "id1");
        mBase.size = TEST_ISIZE;
        DcacheLookupCtx ctx;
        InitDcacheLookupCtx(&ctx, mBase, -1);

        // FindDentryPage -> not nullptr || FilePosLock -> 0&& ReadFile -> DENTRYGROUP_SIZE
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(DENTRYGROUP_SIZE));
        auto dentryBlk = FindDentryPage(0, &ctx);
        EXPECT_NE(dentryBlk, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindDentryPage003 ERROR";
    }
    GTEST_LOG_(INFO) << "FindDentryPage003 End";
}

/**
 * @tc.name: InLevel001
 * @tc.desc: Verify the InLeve function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, InLevel001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "InLevel001 Start";
    try {
        // init InitDcacheLookupCtx
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        MetaBase mBase("file1", "id1");
        mBase.size = TEST_ISIZE;
        DcacheLookupCtx ctx;
        InitDcacheLookupCtx(&ctx, mBase, -1);

        // FindDentryPage -> not nullptr || FilePosLock -> 0&& ReadFile -> DENTRYGROUP_SIZE
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));

        HmdfsDentry *de = InLevel(2, &ctx);
        EXPECT_EQ(de, nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "InLevel001 ERROR";
    }
    GTEST_LOG_(INFO) << "InLevel001 End";
}

/**
 * @tc.name: GetCreateInfo001
 * @tc.desc: Verify the GetCreateInfo function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, GetCreateInfo001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreateInfo001 Start";
    try {
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        MetaBase mBase("file1", "id1");
        uint32_t bitPos = 0;
        unsigned long bidx = 0;
        HmdfsDentryGroup dentryBlk = {0};
        uint32_t namehash = 0;

        // HAndleFileFd -> success
        EXPECT_CALL(*insMock, fstat(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ftruncate(_, _)).WillRepeatedly(Return(0));

        // FilePosLock -> 2
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(2));

        int ret = mFile.GetCreateInfo(mBase, bitPos, namehash, bidx, dentryBlk);
        EXPECT_EQ(ret, 2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCreateInfo001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCreateInfo001 End";
}

/**
 * @tc.name: GetCreateInfo002
 * @tc.desc: Verify the GetCreateInfo function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, GetCreateInfo002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreateInfo002 Start";
    try {
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        MetaBase mBase("file1", "id1");
        uint32_t bitPos = 0;
        unsigned long bidx = 0;
        HmdfsDentryGroup dentryBlk = {0};
        uint32_t namehash = 0;

        // HAndleFileFd -> success
        EXPECT_CALL(*insMock, fstat(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ftruncate(_, _)).WillRepeatedly(Return(0));

        // FilePosLock -> 0 && ReadFile -> 0 ENOENT
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(0));

        int ret = mFile.GetCreateInfo(mBase, bitPos, namehash, bidx, dentryBlk);
        EXPECT_EQ(ret, 2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCreateInfo002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCreateInfo002 End";
}

/**
 * @tc.name: GetCreateInfo003
 * @tc.desc: Verify the GetCreateInfo function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, GetCreateInfo003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreateInfo003 Start";
    try {
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        MetaBase mBase("file1", "id1");
        uint32_t bitPos = 0;
        unsigned long bidx = 0;
        HmdfsDentryGroup dentryBlk = {0};
        uint32_t namehash = 0;

        // HAndleFileFd -> success
        EXPECT_CALL(*insMock, fstat(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ftruncate(_, _)).WillRepeatedly(Return(0));

        // FilePosLock -> 0 && ReadFile -> DENTRYGROUP_SIZE
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));

        int ret = mFile.GetCreateInfo(mBase, bitPos, namehash, bidx, dentryBlk);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCreateInfo003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCreateInfo003 End";
}

/**
 * @tc.name: DoCreate001
 * @tc.desc: Verify the DoCreate function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, DoCreate001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCreate001 Start";
    try {
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        mFile.fd_ = UniqueFd(1);
        MetaBase mBase("file1", "id1");

        // InLevel -> nullPtr
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));

        // GetCreateInfo -> 2
        EXPECT_CALL(*insMock, fstat(_, _)).WillRepeatedly(Return(-1));

        int ret = mFile.DoCreate(mBase);
        EXPECT_NE(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCreate001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoCreate001 End";
}

/**
 * @tc.name: DoCreate002
 * @tc.desc: Verify the DoCreate function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, DoCreate002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCreate002 Start";
    try {
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        mFile.fd_ = UniqueFd(1);
        MetaBase mBase("file1", "id1");

        // GetCreateInfo -> success
        EXPECT_CALL(*insMock, fstat(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ftruncate(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        // WriteFile->0
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillRepeatedly(Return(0));

        int ret = mFile.DoCreate(mBase);
        EXPECT_NE(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCreate002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoCreate002 End";
}

/**
 * @tc.name: DoCreate003
 * @tc.desc: Verify the DoCreate function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, DoCreate003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCreate003 Start";
    try {
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        mFile.fd_ = UniqueFd(1);
        MetaBase mBase("file1", "id1");

        // GetCreateInfo -> success
        EXPECT_CALL(*insMock, fstat(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ftruncate(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        // WriteFile->DENTRYGROUP_SIZE
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));

        int ret = mFile.DoCreate(mBase);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCreate003 ERROR";
    }
    GTEST_LOG_(INFO) << "DoCreate003 End";
}

/**
 * @tc.name: LoadChildren001
 * @tc.desc: Verify the LoadChildren function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, LoadChildren001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadChildren001 Start";
    try {
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        mFile.fd_ = UniqueFd(1);
        std::vector<MetaBase> bases;
        struct stat mockStat = {.st_size = 4 * 4096};

        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillOnce(Return(2));

        int ret = mFile.LoadChildren(bases);
        EXPECT_EQ(ret, 2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadChildren001 ERROR";
    }
    GTEST_LOG_(INFO) << "LoadChildren001 End";
}

/**
 * @tc.name: LoadChildren002
 * @tc.desc: Verify the LoadChildren function
 * @tc.type: FUNC
 */
HWTEST_F(MetaFileCloudDiskTest, LoadChildren002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LoadChildren002 Start";
    try {
        CloudDiskMetaFile mFile(TEST_USER_ID, "/", "id1");
        mFile.fd_ = UniqueFd(1);
        std::vector<MetaBase> bases;
        struct stat mockStat = {.st_size = 4 * 4096};

        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));

        int ret = mFile.LoadChildren(bases);
        EXPECT_EQ(ret, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LoadChildren002 ERROR";
    }
    GTEST_LOG_(INFO) << "LoadChildren002 End";
}
} // namespace OHOS::FileManagement