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

#include "fuse_operations.h"
#include "cloud_disk_inode.h"
#include "cloud_file_utils.h"
#include "file_operations_helper.h"
#include "file_operations_local.h"
#include "file_operations_base.h"
#include "parameters.h"
#include "utils_log.h"
#include "assistant.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FileOperationsLocalTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline FileOperationsLocal* fileoperationslocal_ = new FileOperationsLocal();
    static inline shared_ptr<AssistantMock> insMock = nullptr;
};

void FileOperationsLocalTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
}

void FileOperationsLocalTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    fileoperationslocal_ = nullptr;
    Assistant::ins = nullptr;
    insMock = nullptr;
}

void FileOperationsLocalTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileOperationsLocalTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: LookUpTest001
 * @tc.desc: Verify the LookUp function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, LookUpTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookUpTest001 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 0;
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 1;
        const char *name = "mock";
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(0));
        
        fileoperationslocal_->Lookup(req, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookUpTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "LookUpTest001 End";
}

/**
 * @tc.name: LookUpTest002
 * @tc.desc: Verify the LookUp function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, LookUpTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookUpTest001 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 100;
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 0;
        const char *name = "mock";
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_entry(_, _)).WillOnce(Return(0));
        
        fileoperationslocal_->Lookup(req, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookUpTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "LookUpTest001 End";
}

/**
 * @tc.name: LookUpTest003
 * @tc.desc: Verify the LookUp function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, LookUpTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookUpTest003 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 100;
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 1;
        const char *name = "mock";
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_entry(_, _)).WillOnce(Return(0));
        
        fileoperationslocal_->Lookup(req, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookUpTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "LookUpTest003 End";
}

/**
 * @tc.name: LookUpTest004
 * @tc.desc: Verify the LookUp function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, LookUpTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookUpTest004 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 100;
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 100;
        const char *name = "mock";
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_entry(_, _)).WillOnce(Return(0));
        
        fileoperationslocal_->Lookup(req, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookUpTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "LookUpTest004 End";
}

/**
 * @tc.name: GetAttrTest001
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, GetAttrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest001 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 0;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        struct fuse_file_info fi;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(0));
        
        fileoperationslocal_->GetAttr(req, ino, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAttrTest001 End";
}

/**
 * @tc.name: GetAttrTest002
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, GetAttrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest002 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 100;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        struct fuse_file_info fi;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_attr(_, _, _)).WillOnce(Return(0));
        
        fileoperationslocal_->GetAttr(req, ino, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAttrTest002 End";
}

/**
 * @tc.name: GetAttrTest003
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, GetAttrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest003 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 100;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = -1;
        struct fuse_file_info fi;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(0));
        
        fileoperationslocal_->GetAttr(req, ino, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAttrTest003 End";
}

/**
 * @tc.name: GetAttrTest004
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, GetAttrTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest004 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 100;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0 ;
        struct fuse_file_info fi;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_attr(_, _, _)).WillOnce(Return(0));
        
        fileoperationslocal_->GetAttr(req, ino, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "GetAttrTest004 End";
}

/**
 * @tc.name: ReadDirTest001
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, ReadDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest001 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 100;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_buf(_, _, _)).WillOnce(Return(0));
        
        fileoperationslocal_->ReadDir(req, ino, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest001 End";
}

/**
 * @tc.name: ReadDirTest002
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, ReadDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest002 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 0;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        
        fileoperationslocal_->ReadDir(req, ino, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest002 End";
}

/**
 * @tc.name: ReadDirTest003
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, ReadDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest003 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 100;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = -1;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(0));
        
        fileoperationslocal_->ReadDir(req, ino, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest003 End";
}

/**
 * @tc.name: ReadDirTest004
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, ReadDirTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest004 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 100;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        
        fileoperationslocal_->ReadDir(req, ino, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest004 End";
}

/**
 * @tc.name: ReadDirTest005
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, ReadDirTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest005 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 1;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_buf(_, _, _)).WillOnce(Return(0));
        
        fileoperationslocal_->ReadDir(req, ino, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest005 End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test