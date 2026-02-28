/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include "cloud_disk_common.h"
#include "cloud_disk_service_error.h"
#include "cloud_disk_service_metafile.h"
#include "convertor.h"
#include "securec.h"

namespace OHOS::FileManagement::CloudDiskService {

constexpr int32_t USER_ID = 111;
constexpr int32_t INDEX_ID = 10;
constexpr int32_t INODE = 222;
constexpr uint64_t BASE_SIZE = 1024;
constexpr uint16_t BASE_MODE = 0666;

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
    insMock->EnableMock();
}

void CloudDiskServiceMetafileTest::TearDown(void)
{
    insMock->DisableMock();
    Assistant::ins = nullptr;
    insMock = nullptr;
    GTEST_LOG_(INFO) << "TearDown";
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(sizeof(CloudDiskServiceDcacheHeader)));
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillOnce(Return(2));
        auto ret = mFile.DecodeDentryHeader();
        EXPECT_EQ(ret, 2);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.parentDentryFile_ = "parent_file";
        mFile.selfRecordId_ = "test_id_111";
        mFile.selfHash_ = 0x12345678;
        struct stat mockStat = {
            .st_size = 1,
        };
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
        EXPECT_CALL(*insMock, ftruncate(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillOnce(Return(INODE));
        auto ret = mFile.GenericDentryHeader();
        EXPECT_EQ(ret, INODE);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        string parentDentryFileStr = "parent_file";
        string selfRecordIdStr = "test_id_111";
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
        copy(selfRecordIdStr.begin(), min(selfRecordIdStr.end(), selfRecordIdStr.begin() + RECORD_ID_LEN),
            header.selfRecordId);
        header.selfHash = selfHashNum;

        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(DoAll(SetArgPointee<1>(mockStat), Return(0)));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _))
            .WillOnce(Return(static_cast<int64_t>(sizeof(CloudDiskServiceDcacheHeader))));
        auto ret = mFile.GenericDentryHeader();
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenericDentryHeaderTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "GenericDentryHeaderTest004 End";
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoCreate(base, bidx, bitPos);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_base_name";
        base.recordId = "test_id_111";
        base.atime = 0;
        base.mtime = 0;
        base.size = BASE_SIZE;
        base.mode = BASE_MODE;
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillOnce(Return(DENTRYGROUP_SIZE));
        auto ret = mFile.DoCreate(base, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_base_name";
        base.recordId = "test_id_111";
        base.atime = 0;
        base.mtime = 0;
        base.size = BASE_SIZE;
        base.mode = BASE_MODE;
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(0));
        auto ret = mFile.DoCreate(base, bidx, bitPos);
        EXPECT_EQ(ret, ENOENT);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_base_name";
        base.recordId = "test_id_111";
        base.atime = 0;
        base.mtime = 0;
        base.size = BASE_SIZE;
        base.mode = BASE_MODE;
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillOnce(Return(1));
        auto ret = mFile.DoCreate(base, bidx, bitPos);
        EXPECT_EQ(ret, EINVAL);
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
        string longName(1000, 'a');
        MetaBase base;
        base.name = longName;
        base.recordId = "record_123";
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoCreate(base, bidx, bitPos);
        EXPECT_EQ(ret, ENAMETOOLONG);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCreateTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "DoCreateTest005 End";
}

/**
* @tc.name: DoCreateTest006
* @tc.desc: Verify the DoCreate function when dentry already exists
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoCreateTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoCreateTest006 Start";
    try {
        std::map<off_t, std::vector<char>> fileContent;
        auto readFileImpl = [&fileContent](int fd, off_t offset, size_t size, void *data) -> int64_t {
            auto it = fileContent.find(offset);
            if (it != fileContent.end() && it->second.size() >= size) {
                memcpy_s(data, size, it->second.data(), size);
            } else {
                memset_s(data, size, 0, size);
            }
            return DENTRYGROUP_SIZE;
        };
        auto writeFileImpl = [&fileContent](int fd, const void *data, off_t offset, size_t size) -> int64_t {
            std::vector<char> buf(size);
            memcpy_s(buf.data(), size, data, size);
            fileContent[offset] = std::move(buf);
            return DENTRYGROUP_SIZE;
        };
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        base.recordId = "record_123";
        base.atime = 0;
        base.mtime = 0;
        base.size = BASE_SIZE;
        base.mode = BASE_MODE;
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(testing::Invoke(readFileImpl));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillRepeatedly(testing::Invoke(writeFileImpl));
        auto ret = mFile.DoCreate(base, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
        ret = mFile.DoCreate(base, bidx, bitPos);
        EXPECT_EQ(ret, EEXIST);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoCreateTest006 ERROR";
    }
    GTEST_LOG_(INFO) << "DoCreateTest006 End";
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        string recordId = "test_id_111";
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoRemove(base, recordId, bidx, bitPos);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        string recordId = "test_id_111";
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoRemove(base, recordId, bidx, bitPos);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRemoveTest002Q ERROR";
    }
    GTEST_LOG_(INFO) << "DoRemoveTest002 End";
}

/**
* @tc.name: DoRemoveTest003
* @tc.desc: Verify the DoRemove function normal case
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoRemoveTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRemoveTest003 Start";
    try {
        std::map<off_t, std::vector<char>> fileContent;
        auto readFileImpl = [&fileContent](int fd, off_t offset, size_t size, void *data) -> int64_t {
            auto it = fileContent.find(offset);
            if (it != fileContent.end() && it->second.size() >= size) {
                memcpy_s(data, size, it->second.data(), size);
            } else {
                memset_s(data, size, 0, size);
            }
            return DENTRYGROUP_SIZE;
        };
        auto writeFileImpl = [&fileContent](int fd, const void *data, off_t offset, size_t size) -> int64_t {
            std::vector<char> buf(size);
            memcpy_s(buf.data(), size, data, size);
            fileContent[offset] = std::move(buf);
            return DENTRYGROUP_SIZE;
        };
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        base.recordId = "record_123";
        base.atime = 0;
        base.mtime = 0;
        base.size = BASE_SIZE;
        base.mode = BASE_MODE;
        string recordId;
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(testing::Invoke(readFileImpl));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillRepeatedly(testing::Invoke(writeFileImpl));
        auto ret = mFile.DoCreate(base, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
        ret = mFile.DoRemove(base, recordId, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRemoveTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRemoveTest003 End";
}

/**
* @tc.name: DoRemoveTest004
* @tc.desc: Verify the DoRemove function when WriteFile fails
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoRemoveTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoRemoveTest004 Start";
    try {
        std::map<off_t, std::vector<char>> fileContent;
        int writeCallCount = 0;
        auto readFileImpl = [&fileContent](int fd, off_t offset, size_t size, void *data) -> int64_t {
            auto it = fileContent.find(offset);
            if (it != fileContent.end() && it->second.size() >= size) {
                memcpy_s(data, size, it->second.data(), size);
            } else {
                memset_s(data, size, 0, size);
            }
            return DENTRYGROUP_SIZE;
        };
        auto writeFileImpl = [&fileContent, &writeCallCount](int fd, const void *data, off_t offset,
            size_t size) -> int64_t {
            std::vector<char> buf(size);
            memcpy_s(buf.data(), size, data, size);
            fileContent[offset] = std::move(buf);
            writeCallCount++;
            return (writeCallCount <= 1) ? DENTRYGROUP_SIZE : 1;
        };
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        base.recordId = "record_123";
        base.atime = 0;
        base.mtime = 0;
        base.size = BASE_SIZE;
        base.mode = BASE_MODE;
        string recordId;
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(testing::Invoke(readFileImpl));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillRepeatedly(testing::Invoke(writeFileImpl));
        auto ret = mFile.DoCreate(base, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
        ret = mFile.DoRemove(base, recordId, bidx, bitPos);
        EXPECT_EQ(ret, EIO);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRemoveTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRemoveTest004 End";
}

/**
* @tc.name: DoFlushTest001
* @tc.desc: Verify the DoFlush function when fd is invalid
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoFlushTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoFlushTest001 Start";
    try {
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        base.name = "test_name";
        string recordId = "record_123";
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoFlush(base, recordId, bidx, bitPos);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoFlushTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoFlushTest001 End";
}

/**
* @tc.name: DoFlushTest002
* @tc.desc: Verify the DoFlush function when dentry not found
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoFlushTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoFlushTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        string recordId = "record_123";
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoFlush(base, recordId, bidx, bitPos);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoFlushTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoFlushTest002 End";
}

/**
* @tc.name: DoFlushTest003
* @tc.desc: Verify the DoFlush function normal case
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoFlushTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoFlushTest003 Start";
    try {
        std::map<off_t, std::vector<char>> fileContent;
        auto readFileImpl = [&fileContent](int fd, off_t offset, size_t size, void *data) -> int64_t {
            auto it = fileContent.find(offset);
            if (it != fileContent.end() && it->second.size() >= size) {
                memcpy_s(data, size, it->second.data(), size);
            } else {
                memset_s(data, size, 0, size);
            }
            return DENTRYGROUP_SIZE;
        };
        auto writeFileImpl = [&fileContent](int fd, const void *data, off_t offset, size_t size) -> int64_t {
            std::vector<char> buf(size);
            memcpy_s(buf.data(), size, data, size);
            fileContent[offset] = std::move(buf);
            return DENTRYGROUP_SIZE;
        };
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        base.recordId = "record_123";
        base.atime = 0;
        base.mtime = 0;
        base.size = BASE_SIZE;
        base.mode = BASE_MODE;
        string recordId;
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(testing::Invoke(readFileImpl));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillRepeatedly(testing::Invoke(writeFileImpl));
        auto ret = mFile.DoCreate(base, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
        ret = mFile.DoRemove(base, recordId, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
        ret = mFile.DoFlush(base, recordId, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoFlushTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "DoFlushTest003 End";
}

/**
* @tc.name: DoFlushTest004
* @tc.desc: Verify the DoFlush function when WriteFile fails
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoFlushTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoFlushTest004 Start";
    try {
        std::map<off_t, std::vector<char>> fileContent;
        int writeCallCount = 0;
        auto readFileImpl = [&fileContent](int fd, off_t offset, size_t size, void *data) -> int64_t {
            auto it = fileContent.find(offset);
            if (it != fileContent.end() && it->second.size() >= size) {
                memcpy_s(data, size, it->second.data(), size);
            } else {
                memset_s(data, size, 0, size);
            }
            return DENTRYGROUP_SIZE;
        };
        auto writeFileImpl = [&fileContent, &writeCallCount](int fd, const void *data, off_t offset,
            size_t size) -> int64_t {
            std::vector<char> buf(size);
            memcpy_s(buf.data(), size, data, size);
            fileContent[offset] = std::move(buf);
            writeCallCount++;
            return (writeCallCount <= 2) ? DENTRYGROUP_SIZE : 1;
        };
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        base.recordId = "record_123";
        base.atime = 0;
        base.mtime = 0;
        base.size = BASE_SIZE;
        base.mode = BASE_MODE;
        string recordId;
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(testing::Invoke(readFileImpl));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillRepeatedly(testing::Invoke(writeFileImpl));
        auto ret = mFile.DoCreate(base, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
        ret = mFile.DoRemove(base, recordId, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
        ret = mFile.DoFlush(base, recordId, bidx, bitPos);
        EXPECT_EQ(ret, EIO);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoFlushTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "DoFlushTest004 End";
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        string recordId = "test_id_111";
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoUpdate(base, recordId, bidx, bitPos);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        string recordId = "test_id_111";
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoUpdate(base, recordId, bidx, bitPos);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoUpdateTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoUpdateTest002 End";
}

/**
* @tc.name: DoUpdateTest003
* @tc.desc: Verify the DoUpdate function normal case
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoUpdateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoUpdateTest003 Start";
    try {
        std::map<off_t, std::vector<char>> fileContent;
        auto readFileImpl = [&fileContent](int fd, off_t offset, size_t size, void *data) -> int64_t {
            auto it = fileContent.find(offset);
            if (it != fileContent.end() && it->second.size() >= size) {
                memcpy_s(data, size, it->second.data(), size);
            } else {
                memset_s(data, size, 0, size);
            }
            return DENTRYGROUP_SIZE;
        };
        auto writeFileImpl = [&fileContent](int fd, const void *data, off_t offset, size_t size) -> int64_t {
            std::vector<char> buf(size);
            memcpy_s(buf.data(), size, data, size);
            fileContent[offset] = std::move(buf);
            return DENTRYGROUP_SIZE;
        };
        CloudDiskServiceMetaFile mFile(100, 1, 123);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_name";
        base.recordId = "record_123";
        base.atime = 0;
        base.mtime = 0;
        base.size = BASE_SIZE;
        base.mode = BASE_MODE;
        string recordId;
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(testing::Invoke(readFileImpl));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillRepeatedly(testing::Invoke(writeFileImpl));
        auto ret = mFile.DoCreate(base, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
        base.atime = 1000;
        base.mtime = 2000;
        base.size = 2048;
        ret = mFile.DoUpdate(base, recordId, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoUpdateTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "DoUpdateTest003 End";
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        string recordId = "test_id_111";
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoRenameOld(base, recordId, bidx, bitPos);
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
    GTEST_LOG_(INFO) << "DoRenameOldTest002 Start";
    try {
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        string recordId = "test_id_111";
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoRenameOld(base, recordId, bidx, bitPos);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        string recordId = "test_id_111";
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoRenameNew(base, recordId, bidx, bitPos);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        string longName(1000, 'a');
        base.name = longName;
        string recordId = "test_id_111";
        unsigned long bidx;
        uint32_t bitPos;
        auto ret = mFile.DoRenameNew(base, recordId, bidx, bitPos);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_base_name";
        string recordId = "test_id_111";
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillOnce(Return(DENTRYGROUP_SIZE));
        auto ret = mFile.DoRenameNew(base, recordId, bidx, bitPos);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_base_name";
        string recordId = "test_id_111";
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        EXPECT_CALL(*insMock, WriteFile(_, _, _, _)).WillOnce(Return(1));
        auto ret = mFile.DoRenameNew(base, recordId, bidx, bitPos);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        base.name = "test_base_name";
        string recordId = "test_id_111";
        unsigned long bidx;
        uint32_t bitPos;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(1));
        auto ret = mFile.DoRenameNew(base, recordId, bidx, bitPos);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        string recordId = "test_id_111";
        auto newMetaFile = make_shared<CloudDiskServiceMetaFile>(USER_ID, 2, 456);
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        auto ret = mFile.DoRename(base, recordId, newMetaFile);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoRenameTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoRenameTest001 End";
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        string recordId = "test_id_111";
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        string recordId = "test_id_111";
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
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
* @tc.name: DoLookupByOffset001
* @tc.desc: Verify the DoLookupByOffset function when fd is invalid
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoLookupByOffset001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoLookupByOffset001 Start";
    try {
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        unsigned long bidx = 0;
        uint32_t bitPos = 0;
        auto ret = mFile.DoLookupByOffset(base, bidx, bitPos);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoLookupByOffset001 ERROR";
    }
    GTEST_LOG_(INFO) << "DoLookupByOffset001 End";
}

/**
* @tc.name: DoLookupByOffset002
* @tc.desc: Verify the DoLookupByOffset function when dentryPage not found
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoLookupByOffset002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoLookupByOffset002 Start";
    try {
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        unsigned long bidx = 0;
        uint32_t bitPos = 0;
        auto ret = mFile.DoLookupByOffset(base, bidx, bitPos);
        EXPECT_EQ(ret, ENOENT);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoLookupByOffset002 ERROR";
    }
    GTEST_LOG_(INFO) << "DoLookupByOffset002 End";
}

/**
* @tc.name: DoLookupByOffset003
* @tc.desc: Verify the DoLookupByOffset function
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoLookupByOffset003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoLookupByOffset003 Start";
    try {
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        unsigned long bidx = 0;
        uint32_t bitPos = 0;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        auto ret = mFile.DoLookupByOffset(base, bidx, bitPos);
        EXPECT_EQ(ret, E_OK);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoLookupByOffset003 ERROR";
    }
    GTEST_LOG_(INFO) << "DoLookupByOffset003 End";
}

/**
* @tc.name: DoLookupByOffset004
* @tc.desc: Verify the DoLookupByOffset function
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, DoLookupByOffset004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DoLookupByOffset004 Start";
    try {
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(1);
        MetaBase base;
        unsigned long bidx = 0;
        uint32_t bitPos = DENTRY_PER_GROUP;
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
        auto ret = mFile.DoLookupByOffset(base, bidx, bitPos);
        EXPECT_EQ(ret, EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "DoLookupByOffset004 ERROR";
    }
    GTEST_LOG_(INFO) << "DoLookupByOffset004 End";
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        base.name = "test_base_name";
        uint32_t bitPos = 0;
        uint32_t nameHash = 0;
        unsigned long bidx = 0;
        CloudDiskServiceDentryGroup dentryBlk;
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock, ReadFile(_, _, _, _)).WillRepeatedly(Return(DENTRYGROUP_SIZE));
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        base.name = "test_base_name";
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        base.name = "test_base_name";
        uint32_t bitPos = 0;
        uint32_t nameHash = 0;
        unsigned long bidx = 0;
        CloudDiskServiceDentryGroup dentryBlk;
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(Return(0));
        auto ret = mFile.GetCreateInfo(base, bitPos, nameHash, bidx, dentryBlk);
        EXPECT_EQ(ret, ENOENT);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
        mFile.fd_.Reset(-1);
        MetaBase base;
        base.name = "test_base_name";
        uint32_t bitPos = 0;
        uint32_t nameHash = 0;
        unsigned long bidx = 0;
        CloudDiskServiceDentryGroup dentryBlk;
        EXPECT_CALL(*insMock, fstat(_, _)).WillOnce(Return(0));
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
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
        CloudDiskServiceMetaFile mFile(USER_ID, INDEX_ID, INODE);
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
* @tc.name: GetCloudDiskServiceMetaFileTest001
* @tc.desc: Verify the GetCloudDiskServiceMetaFile function
* @tc.type: FUNC
* @tc.require: NA
*/
HWTEST_F(CloudDiskServiceMetafileTest, GetCloudDiskServiceMetaFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudDiskServiceMetaFileTest001 Start";
    try {
        MetaFileMgr &mgr = MetaFileMgr::GetInstance();
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
        MetaFileMgr &mgr = MetaFileMgr::GetInstance();
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
        MetaFileMgr &mgr = MetaFileMgr::GetInstance();
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
        MetaFileMgr &mgr = MetaFileMgr::GetInstance();
        mgr.metaFiles_.clear();
        mgr.metaFileList_.clear();
        uint32_t userId = 1001;
        uint32_t syncFolderIndex = 0;
        uint64_t inode = 123456789;
        auto metaFile = make_shared<CloudDiskServiceMetaFile>(userId, syncFolderIndex, inode);
        metaFile->parentDentryFile_ = "INODE";
        string path = "/test";
        auto ret = mgr.GetRelativePath(metaFile, path);
        EXPECT_EQ(ret, E_PATH_NOT_EXIST);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetRelativePathTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetRelativePathTest002 End";
}
} // namespace OHOS::FileManagement::CloudDiskService