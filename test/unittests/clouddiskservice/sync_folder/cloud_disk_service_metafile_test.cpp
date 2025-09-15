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

#include "assistant.h"
#include "convertor.h"
#include "cloud_disk_service_error.h"

#include "cloud_disk_service_metafile.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;
using namespace testing;
using namespace testing::ext;

class CloudDiskServiceMetafileTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<AssistantMock> insMock;
};

void CloudDiskServiceMetafileTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void CloudDiskServiceMetafileTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void CloudDiskServiceMetafileTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
}

void CloudDiskServiceMetafileTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    Assistant::ins = nullptr;
    insMock = nullptr;
}

/**
 * @tc.name: DecodeDentryHeaderTest001
 * @tc.desc: Verify the DecodeDentryHeader function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DecodeDentryHeaderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecodeDentryHeaderTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        size_t headerSize = sizeof(CloudDiskServiceDcacheHeader);
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(headerSize));
        auto ret = mFile.DecodeDentryHeader();
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DecodeDentryHeaderTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DecodeDentryHeaderTest001 End";
}

/**
 * @tc.name: DecodeDentryHeaderTest002
 * @tc.desc: Verify the DecodeDentryHeader function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DecodeDentryHeaderTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DecodeDentryHeaderTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillOnce(Return(1));
        auto ret = mFile.DecodeDentryHeader();
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DecodeDentryHeaderTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DecodeDentryHeaderTest002 End";
}

/**
 * @tc.name: GenericDentryHeaderTest001
 * @tc.desc: Verify the GenericDentryHeader function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GenericDentryHeaderTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenericDentryHeaderTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(Return(-1));
        auto ret = mFile.GenericDentryHeader();
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenericDentryHeaderTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GenericDentryHeaderTest001 End";
}

/**
 * @tc.name: GenericDentryHeaderTest002
 * @tc.desc: Verify the GenericDentryHeader function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GenericDentryHeaderTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenericDentryHeaderTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        struct stat mockStat = {
            .st_size = 1,
        };
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
        EXPECT_CALL(*insMock, ftruncate(_, _)).WillOnce(Return(1));
        auto ret = mFile.GenericDentryHeader();
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenericDentryHeaderTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GenericDentryHeaderTest002 End";
}

/**
 * @tc.name: GenericDentryHeaderTest003
 * @tc.desc: Verify the GenericDentryHeader function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GenericDentryHeaderTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenericDentryHeaderTest003 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        struct stat mockStat = {
            .st_size = DENTRYGROUP_HEADER,
        };
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillOnce(Return(1));
        auto ret = mFile.GenericDentryHeader();
        EXPECT_EQ(ret, 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenericDentryHeaderTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "GenericDentryHeaderTest003 End";
}

/**
 * @tc.name: GenericDentryHeaderTest004
 * @tc.desc: Verify the GenericDentryHeader function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GenericDentryHeaderTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenericDentryHeaderTest004 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.parentDentryFile_ = "parent_file";
        mFile.selfRecordId_ = "record_123";
        mFile.selfHash_ = 0x12345678;
        struct stat mockStat = {
            .st_size = 1,
        };
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
        EXPECT_CALL(*insMock, ftruncate(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillOnce(Return(123));
        auto ret = mFile.GenericDentryHeader();
        EXPECT_EQ(ret, 123);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenericDentryHeaderTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "GenericDentryHeaderTest004 End";
}

/**
 * @tc.name: GenericDentryHeaderTest005
 * @tc.desc: Verify the GenericDentryHeader function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GenericDentryHeaderTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenericDentryHeaderTest005 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        string parentDentryFileStr = "parent_file";
        string selfRecordIdStr = "record_123";
        auto selfHashNum = 0x12345678;
        mFile.parentDentryFile_ = parentDentryFileStr;
        mFile.selfRecordId_ = selfRecordIdStr;
        mFile.selfHash_ = selfHashNum;
        struct stat mockStat = {
            .st_size = DENTRYGROUP_HEADER,
        };

        CloudDiskServiceDcacheHeader header = {};
        copy(parentDentryFileStr.begin(),
             min(parentDentryFileStr.end(), parentDentryFileStr.begin() + DENTRYFILE_NAME_LEN),
             header.parentDentryfile);
        copy(selfRecordIdStr.begin(),
             min(selfRecordIdStr.end(), selfRecordIdStr.begin() + RECORD_ID_LEN),
             header.selfRecordId);
        header.selfHash = selfHashNum;
        
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillOnce(Return(0)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _))
            .WillOnce(Return(static_cast<int64_t>(sizeof(CloudDiskServiceDcacheHeader))));
        auto ret = mFile.GenericDentryHeader();
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenericDentryHeaderTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "GenericDentryHeaderTest005 End";
}

/**
 * @tc.name: DoCreateTest001
 * @tc.desc: Verify the DoCreate function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoCreateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCreateTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        base.name = "test_name";
        base.recordId = "record_123";
        auto ret = mFile.DoCreate(base);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCreateTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoCreateTest001 End";
}

/**
 * @tc.name: DoCreateTest002
 * @tc.desc: Verify the DoCreate function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoCreateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCreateTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        string longName(1000, 'a');
        MetaBase base;
        base.name = longName;
        base.recordId = "record_123";
        auto ret = mFile.DoCreate(base);
        EXPECT_EQ(ret, ENAMETOOLONG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCreateTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoCreateTest002 End";
}

/**
 * @tc.name: DoCreateTest003
 * @tc.desc: Verify the DoCreate function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoCreateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCreateTest003 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        base.recordId = "record_123";
        base.atime = 0;
        base.mtime = 0;
        base.size = 1024;
        base.mode = 0644;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillOnce(Return(DENTRYGROUP_SIZE));
        auto ret = mFile.DoCreate(base);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCreateTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "DoCreateTest003 End";
}

/**
 * @tc.name: DoCreateTest004
 * @tc.desc: Verify the DoCreate function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoCreateTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCreateTest004 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        base.recordId = "record_123";
        base.atime = 0;
        base.mtime = 0;
        base.size = 1024;
        base.mode = 0644;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        auto ret = mFile.DoCreate(base);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCreateTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "DoCreateTest004 End";
}

/**
 * @tc.name: DoCreateTest005
 * @tc.desc: Verify the DoCreate function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoCreateTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCreateTest005 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        base.recordId = "record_123";
        base.atime = 0;
        base.mtime = 0;
        base.size = 1024;
        base.mode = 0644;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillOnce(Return(1));
        auto ret = mFile.DoCreate(base);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCreateTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "DoCreateTest005 End";
}

/**
 * @tc.name: DoRemoveTest001
 * @tc.desc: Verify the DoRemove function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoRemoveTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRemoveTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        string recordId = "record_123";
        auto ret = mFile.DoRemove(base, recordId);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRemoveTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRemoveTest001 End";
}

/**
 * @tc.name: DoRemoveTest002
 * @tc.desc: Verify the DoRemove function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoRemoveTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRemoveTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        string recordId = "record_123";
        auto ret = mFile.DoRemove(base, recordId);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRemoveTest002Q ERROR";
    }
    GTEST_LOG_(INFO) << "DoRemoveTest002 End";
}

/**
 * @tc.name: DoFlushTest001
 * @tc.desc: Verify the DoFlush function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoFlushTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoFlushTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        string recordId = "record_123";
        auto ret = mFile.DoRemove(base, recordId);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRemoveTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRemoveTest001 End";
}

/**
 * @tc.name: DoFlushTest002
 * @tc.desc: Verify the DoFlush function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoFlushTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoFlushTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        string recordId = "record_123";
        auto ret = mFile.DoRemove(base, recordId);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRemoveTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRemoveTest002 End";
}

/**
 * @tc.name: DoUpdateTest001
 * @tc.desc: Verify the DoUpdate function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoUpdateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoUpdateTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        string recordId = "record_123";
        auto ret = mFile.DoUpdate(base, recordId);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoUpdateTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoUpdateTest001 End";
}

/**
 * @tc.name: DoUpdateTest002
 * @tc.desc: Verify the DoUpdate function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoUpdateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoUpdateTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        string recordId = "record_123";
        auto ret = mFile.DoUpdate(base, recordId);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoUpdateTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoUpdateTest002 End";
}

/**
 * @tc.name: DoRenameOldTest001
 * @tc.desc: Verify the DoRenameOld function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoRenameOldTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRenameOldTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        string recordId = "record_123";
        auto ret = mFile.DoRenameOld(base, recordId);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRenameOldTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRenameOldTest001 End";
}

/**
 * @tc.name: DoRenameOldTest002
 * @tc.desc: Verify the DoRenameOld function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoRenameOldTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRenameOldTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        string recordId = "record_123";
        auto ret = mFile.DoRenameOld(base, recordId);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRenameOldTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRenameOldTest002 End";
}

/**
 * @tc.name: DoRenameNewTest001
 * @tc.desc: Verify the DoRenameNew function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoRenameNewTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRenameNewTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        string recordId = "record_123";
        auto ret = mFile.DoRenameNew(base, recordId);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRenameNewTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRenameNewTest001 End";
}

/**
 * @tc.name: DoRenameNewTest002
 * @tc.desc: Verify the DoRenameNew function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoRenameNewTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRenameNewTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        string longName(1000, 'a');
        base.name = longName;
        string recordId = "record_123";
        auto ret = mFile.DoRenameNew(base, recordId);
        EXPECT_EQ(ret, ENAMETOOLONG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRenameNewTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRenameNewTest002 End";
}

/**
 * @tc.name: DoRenameNewTest003
 * @tc.desc: Verify the DoRenameNew function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoRenameNewTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRenameNewTest003 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        string recordId = "record_123";
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillOnce(Return(DENTRYGROUP_SIZE));
        auto ret = mFile.DoRenameNew(base, recordId);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRenameNewTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRenameNewTest003 End";
}

/**
 * @tc.name: DoRenameNewTest004
 * @tc.desc: Verify the DoRenameNew function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoRenameNewTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRenameNewTest004 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        string recordId = "record_123";
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillOnce(Return(1));
        auto ret = mFile.DoRenameNew(base, recordId);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRenameNewTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRenameNewTest004 End";
}

/**
 * @tc.name: DoRenameNewTest005
 * @tc.desc: Verify the DoRenameNew function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoRenameNewTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRenameNewTest005 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        string recordId = "record_123";
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(1));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        auto ret = mFile.DoRenameNew(base, recordId);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRenameNewTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRenameNewTest005 End";
}

/**
 * @tc.name: DoRenameTest001
 * @tc.desc: Verify the DoRename function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoRenameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRenameTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        MetaBase base;
        string recordId = "record_123";
        auto newMetaFile = make_shared<CloudDiskServiceMetaFile>(100, 2, 456);
        auto ret = mFile.DoRename(base, recordId, newMetaFile);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRenameTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRenameTest001 End";
}

/**
 * @tc.name: DoRenameTest002
 * @tc.desc: Verify the DoRename function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoRenameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRenameTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        MetaBase base;
        string recordId = "record_123";
        auto newMetaFile = make_shared<CloudDiskServiceMetaFile>(100, 2, 456);
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        auto ret = mFile.DoRename(base, recordId, newMetaFile);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRenameTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRenameTest002 End";
}

/**
 * @tc.name: DoLookupByNameTest001
 * @tc.desc: Verify the DoLookupByName function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoLookupByNameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoLookupByNameTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        string recordId = "record_123";
        auto ret = mFile.DoLookupByName(base);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoLookupByNameTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoLookupByNameTest001 End";
}

/**
 * @tc.name: DoLookupByNameTest002
 * @tc.desc: Verify the DoLookupByName function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoLookupByNameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoLookupByNameTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        string recordId = "record_123";
        auto ret = mFile.DoLookupByName(base);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoLookupByNameTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoLookupByNameTest002 End";
}


/**
 * @tc.name: DoLookupByRecordIdTest001
 * @tc.desc: Verify the DoLookupByRecordId function when fd is invalid
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoLookupByRecordIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoLookupByRecordIdTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        uint8_t revalidate = 0;
        auto ret = mFile.DoLookupByRecordId(base, revalidate);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoLookupByRecordIdTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoLookupByRecordIdTest001 End";
}

/**
 * @tc.name: DoLookupByRecordIdTest002
 * @tc.desc: Verify the DoLookupByRecordId function when dentry not found
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, DoLookupByRecordIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoLookupByRecordIdTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        uint8_t revalidate = 0;
        auto ret = mFile.DoLookupByRecordId(base, revalidate);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoLookupByRecordIdTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoLookupByRecordIdTest002 End";
}

/**
 * @tc.name: GetCreateInfoTest001
 * @tc.desc: Verify the GetCreateInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetCreateInfoTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreateInfoTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        base.name = "test_name";
        uint32_t bitPos = 0;
        uint32_t nameHash = 0;
        unsigned long bidx = 0;
        CloudDiskServiceDentryGroup dentryBlk;
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        auto ret = mFile.GetCreateInfo(base, bitPos, nameHash, bidx, dentryBlk);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCreateInfoTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCreateInfoTest001 End";
}

/**
 * @tc.name: GetCreateInfoTest002
 * @tc.desc: Verify the GetCreateInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetCreateInfoTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreateInfoTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        base.name = "test_name";
        uint32_t bitPos = 0;
        uint32_t nameHash = 0;
        unsigned long bidx = 0;
        CloudDiskServiceDentryGroup dentryBlk;
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(Return(-1));
        auto ret = mFile.GetCreateInfo(base, bitPos, nameHash, bidx, dentryBlk);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCreateInfoTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCreateInfoTest002 End";
}

/**
 * @tc.name: GetCreateInfoTest003
 * @tc.desc: Verify the GetCreateInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetCreateInfoTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreateInfoTest003 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        base.name = "test_name";
        uint32_t bitPos = 0;
        uint32_t nameHash = 0;
        unsigned long bidx = 0;
        CloudDiskServiceDentryGroup dentryBlk;
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(-1));
        auto ret = mFile.GetCreateInfo(base, bitPos, nameHash, bidx, dentryBlk);
        EXPECT_EQ(ret, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCreateInfoTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCreateInfoTest003 End";
}

/**
 * @tc.name: GetCreateInfoTest004
 * @tc.desc: Verify the GetCreateInfo function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetCreateInfoTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCreateInfoTest003 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(-1);
        MetaBase base;
        base.name = "test_name";
        uint32_t bitPos = 0;
        uint32_t nameHash = 0;
        unsigned long bidx = 0;
        CloudDiskServiceDentryGroup dentryBlk;
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, FilePosLock(_, _, _, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(0));
        auto ret = mFile.GetCreateInfo(base, bitPos, nameHash, bidx, dentryBlk);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCreateInfoTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCreateInfoTest004 End";
}

/**
 * @tc.name: HandleFileByFdTest001
 * @tc.desc: Verify the HandleFileByFd function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, HandleFileByFdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleFileByFdTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(Return(-1));
        unsigned long endBlock = 10;
        uint32_t level = 0;
        auto ret = mFile.HandleFileByFd(endBlock, level);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleFileByFdTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleFileByFdTest001 End";
}

/**
 * @tc.name: HandleFileByFdTest002
 * @tc.desc: Verify the HandleFileByFd function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, HandleFileByFdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleFileByFdTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        struct stat mockStat = {
            .st_size = 1,
        };
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
        EXPECT_CALL(*insMock, ftruncate(_, _)).WillOnce(Return(1));
        unsigned long endBlock = 10;
        uint32_t level = 0;
        auto ret = mFile.HandleFileByFd(endBlock, level);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleFileByFdTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleFileByFdTest002 End";
}

/**
 * @tc.name: HandleFileByFdTest003
 * @tc.desc: Verify the HandleFileByFd function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, HandleFileByFdTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleFileByFdTest003 Start";
    try {
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        struct stat mockStat = {
            .st_size = 1,
        };
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
        EXPECT_CALL(*insMock, ftruncate(_, _)).WillOnce(Return(0));
        unsigned long endBlock = 10;
        uint32_t level = 0;
        auto ret = mFile.HandleFileByFd(endBlock, level);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "HandleFileByFdTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "HandleFileByFdTest003 End";
}

/**
 * @tc.name: GetMetaFileMgrInstanceTest001
 * @tc.desc: Verify the GetMetaFileMgrInstance function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetMetaFileMgrInstanceTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetMetaFileMgrInstanceTest001 Start";
    try {
        MetaFileMgr& instance1 = MetaFileMgr::GetInstance();
        MetaFileMgr& instance2 = MetaFileMgr::GetInstance();
        EXPECT_EQ(&instance1, &instance2);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetMetaFileMgrInstanceTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetMetaFileMgrInstanceTest001 End";
}

/**
 * @tc.name: GetCloudDiskServiceMetaFileTest001
 * @tc.desc: Verify the GetCloudDiskServiceMetaFile function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetCloudDiskServiceMetaFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudDiskServiceMetaFileTest001 Start";
    try {
        MetaFileMgr& mgr = MetaFileMgr::GetInstance();
        mgr.metaFiles_.clear();
        mgr.metaFileList_.clear();
        uint32_t userId = 1001;
        uint32_t syncFolderIndex = 0;
        uint64_t inode = 123456789;
        auto metaFile = mgr.GetCloudDiskServiceMetaFile(userId, syncFolderIndex, inode);
        EXPECT_TRUE(metaFile != nullptr);
        EXPECT_EQ(mgr.metaFiles_.size(), 1);
        EXPECT_EQ(mgr.metaFileList_.size(), 1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCloudDiskServiceMetaFileTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCloudDiskServiceMetaFileTest001 End";
}

/**
 * @tc.name: GetCloudDiskServiceMetaFileTest002
 * @tc.desc: Verify the GetCloudDiskServiceMetaFile function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetCloudDiskServiceMetaFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudDiskServiceMetaFileTest002 Start";
    try {
        MetaFileMgr& mgr = MetaFileMgr::GetInstance();
        mgr.metaFiles_.clear();
        mgr.metaFileList_.clear();
        uint32_t userId = 1001;
        uint32_t syncFolderIndex = 0;
        uint64_t inode = 123456789;
        constexpr uint32_t MAX_META_FILE_NUM = 150;
        for (int i = 0; i < MAX_META_FILE_NUM; i++) {
            uint32_t tempUserId = 1000 + i;
            uint32_t tempSyncFolderIndex = i;
            uint64_t tempInode = 1000000000 + i;
            shared_ptr<CloudDiskServiceMetaFile> mFile =
                make_shared<CloudDiskServiceMetaFile>(tempUserId, tempSyncFolderIndex, tempInode);
            MetaFileKey key(tempUserId,
                Convertor::ConvertToHex(tempSyncFolderIndex) + Convertor::ConvertToHex(tempInode));
            mgr.metaFileList_.emplace_back(key, mFile);
            auto it = prev(mgr.metaFileList_.end());
            mgr.metaFiles_[key] = it;
        }
        auto metaFile = mgr.GetCloudDiskServiceMetaFile(userId, syncFolderIndex, inode);
        EXPECT_TRUE(metaFile != nullptr);
        EXPECT_EQ(mgr.metaFiles_.size(), MAX_META_FILE_NUM);
        EXPECT_EQ(mgr.metaFileList_.size(), MAX_META_FILE_NUM);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCloudDiskServiceMetaFileTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCloudDiskServiceMetaFileTest002 End";
}

/**
 * @tc.name: GetRelativePathTest001
 * @tc.desc: Verify the GetRelativePath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetRelativePathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRelativePathTest001 Start";
    try {
        MetaFileMgr& mgr = MetaFileMgr::GetInstance();
        mgr.metaFiles_.clear();
        mgr.metaFileList_.clear();
        uint32_t userId = 1001;
        uint32_t syncFolderIndex = 0;
        uint64_t inode = 123456789;
        auto metaFile = make_shared<CloudDiskServiceMetaFile>(userId, syncFolderIndex, inode);
        metaFile->parentDentryFile_ = ROOT_PARENTDENTRYFILE;
        string path = "/test";
        auto ret = mgr.GetRelativePath(metaFile, path);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRelativePathTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetRelativePathTest001 End";
}

/**
 * @tc.name: GetRelativePathTest002
 * @tc.desc: Verify the GetRelativePath function
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(CloudDiskServiceMetafileTest, GetRelativePathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetRelativePathTest002 Start";
    try {
        MetaFileMgr& mgr = MetaFileMgr::GetInstance();
        mgr.metaFiles_.clear();
        mgr.metaFileList_.clear();
        uint32_t userId = 1001;
        uint32_t syncFolderIndex = 0;
        uint64_t inode = 123456789;
        auto metaFile = make_shared<CloudDiskServiceMetaFile>(userId, syncFolderIndex, inode);
        metaFile->parentDentryFile_ = "123";
        string path = "/test";
        auto ret = mgr.GetRelativePath(metaFile, path);
        EXPECT_EQ(ret, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRelativePathTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetRelativePathTest002 End";
}
} // namespace OHOS::FileManagement::CloudDiskService