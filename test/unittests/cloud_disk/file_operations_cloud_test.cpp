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
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_file_info fi;
        fi.fh = -1;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void*>(&data)));
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
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void*>(&data)));
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
        GTEST_LOG_(INFO) << "SetXattrTest005  ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest005 End";
}

/**
 * @tc.name: GetXattrTest001
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, GetXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = -1;
        size_t size = 0;
        string name = HMDFS_PERMISSION_XATTR;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->GetXattr(req, ino, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest001 End";
}

/**
 * @tc.name: GetXattrTest002
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, GetXattrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest002 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 0;
        string name = HMDFS_PERMISSION_XATTR;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_xattr(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->GetXattr(req, ino, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest002 End";
}

/**
 * @tc.name: GetXattrTest003
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, GetXattrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest003 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 0;
        string name = CLOUD_CLOUD_ID_XATTR;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_xattr(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->GetXattr(req, ino, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest003 End";
}

/**
 * @tc.name: GetXattrTest004
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, GetXattrTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest004 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 0;
        string name = IS_FAVORITE_XATTR;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_xattr(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->GetXattr(req, ino, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest004  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest004 End";
}

/**
 * @tc.name: GetXattrTest005
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, GetXattrTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest005 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 0;
        string name = IS_FILE_STATUS_XATTR;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_xattr(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->GetXattr(req, ino, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest005  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest005 End";
}

/**
 * @tc.name: GetXattrTest006
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, GetXattrTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest006 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 0;
        string name = CLOUD_FILE_LOCATION;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_xattr(_, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
        fileOperationsCloud_->GetXattr(req, ino, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest006  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest006 End";
}

/**
 * @tc.name: GetXattrTest007
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, GetXattrTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest007 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 0;
        string name = CLOUD_CLOUD_RECYCLE_XATTR;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fileOperationsCloud_->GetXattr(req, ino, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest007  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest007 End";
}

/**
 * @tc.name: GetXattrTest008
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, GetXattrTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest008 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = -999;
        string name = HMDFS_PERMISSION_XATTR;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_buf(_, _, _)).WillOnce(Return(E_OK)).WillOnce(Return(E_OK));
        fileOperationsCloud_->GetXattr(req, ino, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest008  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest008 End";
}

/**
 * @tc.name: GetXattrTest009
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, GetXattrTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest009 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 1;
        string name = HMDFS_PERMISSION_XATTR;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fileOperationsCloud_->GetXattr(req, ino, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest009  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest009 End";
}

/**
 * @tc.name: MkDirTest001
 * @tc.desc: Verify the MkDir function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, MkDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkDirTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t parent = -1;
        const char *name = nullptr;
        mode_t mode = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->MkDir(req, parent, name, mode);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkDirTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "MkDirTest001 End";
}

/**
 * @tc.name: MkDirTest002
 * @tc.desc: Verify the MkDir function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, MkDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkDirTest002 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 0;
        const char *name = nullptr;
        mode_t mode = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->MkDir(req, parent, name, mode);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkDirTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "MkDirTest002 End";
}

/**
 * @tc.name: RmDirTest001
 * @tc.desc: Verify the RmDir function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, RmDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmDirTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t parent = -1;
        const char *name = nullptr;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->RmDir(req, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RmDirTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "RmDirTest001 End";
}

/**
 * @tc.name: UnlinkTest001
 * @tc.desc: Verify the Unlink function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, UnlinkTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_ino_t parent = -1;
        fuse_req_t req = nullptr;
        const char *name = "";
        
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Unlink(req, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnlinkTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "UnlinkTest001 End";
}

/**
 * @tc.name: RenameTest001
 * @tc.desc: Verify the Rename function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, RenameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_ino_t parent = 0;
        fuse_ino_t newParent = 0;
        fuse_req_t req = nullptr;
        const char *name = "";
        const char *newName = "";
        unsigned int flags = 1;

        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Rename(req, parent, name, newParent, newName, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "RenameTest001 End";
}

/**
 * @tc.name: RenameTest002
 * @tc.desc: Verify the Rename function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, RenameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTest002 Start";
    try {
        CloudDiskFuseData data;
        fuse_ino_t parent = -1;
        fuse_ino_t newParent = -1;
        fuse_req_t req = nullptr;
        const char *name = "";
        const char *newName = "";
        unsigned int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Rename(req, parent, name, newParent, newName, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "RenameTest002 End";
}

/**
 * @tc.name: RenameTest003
 * @tc.desc: Verify the Rename function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, RenameTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTest003 Start";
    try {
        CloudDiskFuseData data;
        fuse_ino_t parent = 0;
        fuse_ino_t newParent = 0;
        fuse_req_t req = nullptr;
        const char *name = "";
        const char *newName = "";
        unsigned int flags = 0;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Rename(req, parent, name, newParent, newName, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "RenameTest003 End";
}

/**
 * @tc.name: ReadTest001
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, ReadTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        size_t size = 5 * 1024 *1024;
        off_t off = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Read(req, reinterpret_cast<fuse_ino_t>(&ino), size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadTest001 End";
}

/**
 * @tc.name: ReadTest002
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, ReadTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTest002 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        size_t size = 1024;
        off_t off = 0;
        struct fuse_file_info fi;
        fi.fh = -1;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Read(req, reinterpret_cast<fuse_ino_t>(&ino), size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadTest002 End";
}

/**
 * @tc.name: ReadTest003
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, ReadTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTest003 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        size_t size = 1024;
        off_t off = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_data(_, _, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Read(req, reinterpret_cast<fuse_ino_t>(&ino), size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadTest003 End";
}

/**
 * @tc.name: WriteBufTest001
 * @tc.desc: Verify the WriteBuf function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, WriteBufTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteBufTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_bufvec *bufv = nullptr;
        off_t offset = 0;
        struct fuse_file_info fi;
        fi.fh = -1;

        EXPECT_CALL(*insMock, fuse_buf_size(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->WriteBuf(req, reinterpret_cast<fuse_ino_t>(&ino), bufv, offset, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteBufTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "WriteBufTest001 End";
}

/**
 * @tc.name: WriteBufTest002
 * @tc.desc: Verify the WriteBuf function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, WriteBufTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteBufTest002 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_bufvec *bufv = nullptr;
        off_t offset = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_buf_size(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_buf_copy(_, _, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*insMock, fuse_reply_write(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->WriteBuf(req, reinterpret_cast<fuse_ino_t>(&ino), bufv, offset, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteBufTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "WriteBufTest002 End";
}

/**
 * @tc.name: WriteBufTest003
 * @tc.desc: Verify the WriteBuf function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, WriteBufTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteBufTest003 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_bufvec *bufv = nullptr;
        off_t offset = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_buf_size(_)).WillOnce(Return(E_OK));
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_buf_copy(_, _, _)).WillOnce(Return(-1));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->WriteBuf(req, reinterpret_cast<fuse_ino_t>(&ino), bufv, offset, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteBufTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "WriteBufTest003 End";
}

/**
 * @tc.name: ReleaseTest001
 * @tc.desc: Verify the Release function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, ReleaseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReleaseTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = -1;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        fileOperationsCloud_->Release(req, ino, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReleaseTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "ReleaseTest001 End";
}

/**
 * @tc.name: ReleaseTest002
 * @tc.desc: Verify the Release function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, ReleaseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReleaseTest002 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        struct fuse_file_info fi;
        fi.fh = -1;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillRepeatedly(Return(E_OK));
        fileOperationsCloud_->Release(req, ino, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReleaseTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "ReleaseTest002 End";
}

/**
 * @tc.name: ReleaseTest003
 * @tc.desc: Verify the Release function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, ReleaseTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReleaseTest003 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Release(req, reinterpret_cast<fuse_ino_t>(&ino), &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReleaseTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "ReleaseTest003 End";
}

/**
 * @tc.name: SetAttrTest001
 * @tc.desc: Verify the SetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetAttrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAttrTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct stat attr;
        int valid = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetAttr(req, -1, &attr, valid, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetAttrTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "SetAttrTest001 End";
}

/**
 * @tc.name: SetAttrTest002
 * @tc.desc: Verify the SetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetAttrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAttrTest002 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct stat attr;
        int valid = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_attr(_, _, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetAttr(req, 0, &attr, valid, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetAttrTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "SetAttrTest002 End";
}

/**
 * @tc.name: SetAttrTest003
 * @tc.desc: Verify the SetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetAttrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAttrTest003 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct stat attr;
        int valid = 13;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetAttr(req, 0, &attr, valid, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetAttrTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "SetAttrTest003 End";
}

/**
 * @tc.name: SetAttrTest004
 * @tc.desc: Verify the SetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, SetAttrTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAttrTest004 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        struct stat attr;
        int valid = 13;
        struct fuse_file_info fi;
        fi.fh = -1;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->SetAttr(req, 0, &attr, valid, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetAttrTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "SetAttrTest004 End";
}

/**
 * @tc.name: LseekTest001
 * @tc.desc: Verify the Lseek function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, LseekTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekTest001 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        off_t off = 0;
        int whence = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Lseek(req, -1, off, whence, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LseekTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "LseekTest001 End";
}

/**
 * @tc.name: LseekTest002
 * @tc.desc: Verify the Lseek function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, LseekTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekTest002 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        off_t off = 0;
        int whence = 0;
        struct fuse_file_info fi;
        fi.fh = -1;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Lseek(req, 0, off, whence, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LseekTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "LseekTest002 End";
}

/**
 * @tc.name: LseekTest003
 * @tc.desc: Verify the Lseek function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, LseekTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekTest003 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        off_t off = 0;
        int whence = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, lseek(_, _, _)).WillOnce(Return(E_OK));
        EXPECT_CALL(*insMock, fuse_reply_lseek(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Lseek(req, 0, off, whence, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LseekTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "LseekTest003 End";
}

/**
 * @tc.name: LseekTest004
 * @tc.desc: Verify the Lseek function
 * @tc.type: FUNC
 * @tc.require: issuesI91IOG
 */
HWTEST_F(FileOperationsCloudTest, LseekTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekTest004 Start";
    try {
        CloudDiskFuseData data;
        fuse_req_t req = nullptr;
        CloudDiskInode ino;
        off_t off = 0;
        int whence = 0;
        struct fuse_file_info fi;

        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        EXPECT_CALL(*insMock, lseek(_, _, _)).WillOnce(Return(-1));
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).WillOnce(Return(E_OK));
        fileOperationsCloud_->Lseek(req, 0, off, whence, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LseekTest004  ERROR";
    }
    GTEST_LOG_(INFO) << "LseekTest004 End";
}
}