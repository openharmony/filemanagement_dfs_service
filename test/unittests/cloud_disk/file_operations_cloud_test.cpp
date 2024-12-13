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
    GTEST_LOG_(INFO) << "OpenTest007 Start";
    try {
        CloudDiskFuseData data;
        data.userId = 100;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        fileOperationsCloud_->Open(req, reinterpret_cast<fuse_ino_t>(&ino), &fi);
        EXPECT_TRUE(true);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest007 ERROR";
    }
    GTEST_LOG_(INFO) << "OpenTest007 End";
}

/**
 * @tc.name: MkNodTest001
 * @tc.desc: Verify the MkNod function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, MkNodTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkNodTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        const char *name = "";
        mode_t mode = 0;
        dev_t rdev = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->MkNod(req, reinterpret_cast<fuse_ino_t>(&ino), name, mode, rdev);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkNodTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "MkNodTest001 End";
}

/**
 * @tc.name: CreateTest001
 * @tc.desc: Verify the Create function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, CreateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 0;
        const char *name = "";
        mode_t mode = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Create(req, parent, name, mode, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "CreateTest001 End";
}

/**
 * @tc.name: ReadDirTest001
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, ReadDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 4;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_buf(_, _, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->ReadDir(req, ino, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest001 End";
}

/**
 * @tc.name: ReadDirTest002
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, ReadDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest002 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = -1;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->ReadDir(req, ino, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest002 End";
}


/**
 * @tc.name: ReadDirTest003
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, ReadDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest003 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_buf(_, _, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->ReadDir(req, ino, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest003 End";
}

/**
 * @tc.name: SetXattrTest001
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        string name = HMDFS_PERMISSION_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest001 End";
}

/**
 * @tc.name: SetXattrTest002
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest002 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = -1;
        string name = CLOUD_FILE_LOCATION;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest002 End";
}

/**
 * @tc.name: SetXattrTest003
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest003 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        string name = CLOUD_FILE_LOCATION;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest003 End";
}

/**
 * @tc.name: SetXattrTest004
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest004 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 1;
        string name = CLOUD_FILE_LOCATION;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest004  ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest004 End";
}

/**
 * @tc.name: SetXattrTest005
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest005 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 1;
        string name = CLOUD_FILE_LOCATION;
        const char *value = "test";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest005  ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest005 End";
}

/**
 * @tc.name: SetXattrTest006
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest006 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = -1;
        string name = CLOUD_CLOUD_RECYCLE_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest006 ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest006 End";
}

/**
 * @tc.name: SetXattrTest007
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest007 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        string name = CLOUD_CLOUD_RECYCLE_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest007 ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest007 End";
}

/**
 * @tc.name: SetXattrTest008
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest008 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 1;
        string name = CLOUD_CLOUD_RECYCLE_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest008 ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest008 End";
}

/**
 * @tc.name: SetXattrTest009
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest009 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 2;
        string name = CLOUD_CLOUD_RECYCLE_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest009 ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest009 End";
}

/**
 * @tc.name: SetXattrTest010
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest010 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 2;
        string name = CLOUD_CLOUD_RECYCLE_XATTR;
        const char *value = "test";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest010 ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest010 End";
}

/**
 * @tc.name: SetXattrTest011
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest011 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = -1;
        string name = IS_FAVORITE_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest011 ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest011 End";
}

/**
 * @tc.name: SetXattrTest012
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest012, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest012 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        string name = IS_FAVORITE_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest012 ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest012 End";
}

/**
 * @tc.name: SetXattrTest013
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest013, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest013 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        string name = IS_FAVORITE_XATTR;
        const char *value = "test";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest013 ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest013 End";
}

/**
 * @tc.name: SetXattrTest014
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest014, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest014 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = -1;
        string name = IS_FILE_STATUS_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest014 ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest014 End";
}

/**
 * @tc.name: SetXattrTest015
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetXattrTest015, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest015 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        string name = IS_FILE_STATUS_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest015 ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest015 End";
}

/**
 * @tc.name: SetXattrTest016
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC