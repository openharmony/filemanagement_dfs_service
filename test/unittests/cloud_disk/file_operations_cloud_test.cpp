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

#include "file_operations_cloud.h"

#include "assistant.h"
#include "cloud_disk_inode.h"
#include "clouddisk_rdb_utils.h"
#include "dfs_error.h"

namespace OHOS::FileManagement::CloudDisk {
size_t CloudDiskRdbUtils::FuseDentryAlignSize(const char *name)
{
    return 0;
}
}

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FileOperationsCloudTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<FileOperationsCloud> fileOperationsCloud_ = nullptr;
    static inline shared_ptr<AssistantMock> insMock = nullptr;
};

void FileOperationsCloudTest::SetUpTestCase(void)
{
    fileOperationsCloud_ = make_shared<FileOperationsCloud>();
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileOperationsCloudTest::TearDownTestCase(void)
{
    Assistant::ins = nullptr;
    insMock = nullptr;
    fileOperationsCloud_ = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileOperationsCloudTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileOperationsCloudTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: LookupTest001
 * @tc.desc: Verify the Lookup function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, LookupTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest001 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        fuse_ino_t parent = FUSE_ROOT_ID;
        const char *name = "";
        fileOperationsCloud_->Lookup(nullptr, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookupTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "LookupTest001 End";
}

/**
 * @tc.name: LookupTest002
 * @tc.desc: Verify the Lookup function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, LookupTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        fuse_ino_t parent = -1;
        const char *name = "";
        fileOperationsCloud_->Lookup(nullptr, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookupTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "LookupTest002 End";
}

/**
 * @tc.name: LookupTest003
 * @tc.desc: Verify the Lookup function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, LookupTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest003 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_entry(_, _)).WillOnce(Return(E_OK));

        fuse_ino_t parent = 4;
        const char *name = ".trash";
        fileOperationsCloud_->Lookup(nullptr, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookupTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "LookupTest003 End";
}

/**
 * @tc.name: LookupTest004
 * @tc.desc: Verify the Lookup function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, LookupTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest004 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        fuse_ino_t parent = 4;
        const char *name = "mock";
        fileOperationsCloud_->Lookup(nullptr, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookupTest004  ERROR";
    }
    GTEST_LOG_(INFO) << "LookupTest004 End";
}

/**
 * @tc.name: LookupTest005
 * @tc.desc: Verify the Lookup function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, LookupTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest005 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_entry(_, _)).WillOnce(Return(E_OK));

        fuse_ino_t parent = 4;
        const char *name = "";
        fileOperationsCloud_->Lookup(nullptr, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookupTest005  ERROR";
    }
    GTEST_LOG_(INFO) << "LookupTest005 End";
}

/**
 * @tc.name: LookupTest006
 * @tc.desc: Verify the Lookup function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, LookupTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest006 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        fuse_ino_t parent = 2;
        const char *name = "mock";
        fileOperationsCloud_->Lookup(nullptr, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookupTest006  ERROR";
    }
    GTEST_LOG_(INFO) << "LookupTest006 End";
}

/**
 * @tc.name: LookupTest007
 * @tc.desc: Verify the Lookup function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, LookupTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest007 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_entry(_, _)).WillOnce(Return(E_OK));
        
        fuse_ino_t parent = 2;
        const char *name = "";
        fileOperationsCloud_->Lookup(nullptr, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookupTest007  ERROR";
    }
    GTEST_LOG_(INFO) << "LookupTest007 End";
}

/**
 * @tc.name: AccessTest001
 * @tc.desc: Verify the Access function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, AccessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessTest001 Start";
    try {
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));

        fuse_ino_t ino = FUSE_ROOT_ID;
        int mask = 0;
        fileOperationsCloud_->Access(nullptr, ino, mask);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AccessTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "AccessTest001 End";
}

/**
 * @tc.name: GetAttrTest001
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, GetAttrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_ino_t ino = FUSE_ROOT_ID;
        struct fuse_file_info *fi = nullptr;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_attr(_, _, _)).WillRepeatedly(Return(E_OK));
        fileOperationsCloud_->GetAttr(nullptr, ino, fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "GetAttrTest001 End";
}

/**
 * @tc.name: GetAttrTest002
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, GetAttrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest002 Start";
    try {
        CloudDiskFuseData data;
        fuse_ino_t ino = -1;
        struct fuse_file_info *fi = nullptr;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->GetAttr(nullptr, ino, fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "GetAttrTest002 End";
}

/**
 * @tc.name: OpenTest001
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, OpenTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = -1;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        fileOperationsCloud_->Open(req, ino, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "OpenTest001 End";
}

/**
 * @tc.name: OpenTest002
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, OpenTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenTest002 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 0;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_file_info fi;
        fi.flags |= (O_ACCMODE | O_WRONLY);

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        fileOperationsCloud_->Open(req, reinterpret_cast<fuse_ino_t>(&ino), &fi);
        EXPECT_TRUE(true);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "OpenTest002 End";
}

/**
 * @tc.name: OpenTest003
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, OpenTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenTest003 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 0;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_file_info fi;
        fi.flags |= O_APPEND;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        fileOperationsCloud_->Open(req, reinterpret_cast<fuse_ino_t>(&ino), &fi);
        EXPECT_TRUE(true);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "OpenTest003 End";
}

/**
 * @tc.name: OpenTest004
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, OpenTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenTest004 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 0;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_file_info fi;
        fi.flags |= O_DIRECT;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        fileOperationsCloud_->Open(req, reinterpret_cast<fuse_ino_t>(&ino), &fi);
        EXPECT_TRUE(true);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest004  ERROR";
    }
    GTEST_LOG_(INFO) << "OpenTest004 End";
}

/**
 * @tc.name: OpenTest005
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, OpenTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenTest005 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 0;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        fileOperationsCloud_->Open(req, reinterpret_cast<fuse_ino_t>(&ino), &fi);
        EXPECT_TRUE(true);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest005  ERROR";
    }
    GTEST_LOG_(INFO) << "OpenTest005 End";
}

/**
 * @tc.name: OpenTest006
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, OpenTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenTest006 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 100;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_file_info fi;
        fi.fh = -1;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        fileOperationsCloud_->Open(req, reinterpret_cast<fuse_ino_t>(&ino), &fi);
        EXPECT_TRUE(true);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest006 ERROR";
    }
    GTEST_LOG_(INFO) << "OpenTest006 End";
}

/**
 * @tc.name: OpenTest007
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, OpenTest007, TestSize.Level1)
{