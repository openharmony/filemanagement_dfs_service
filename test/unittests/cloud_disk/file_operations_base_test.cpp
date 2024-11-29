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

#include "assistant.h"
#include "cloud_disk_inode.h"
#include "dfs_error.h"
#include "file_operations_base.h"
#include "file_operations_helper.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;

class FileOperationBaseTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline FileOperationsBase* fileoperationBase_ = new FileOperationsBase();
    static inline std::shared_ptr<AssistantMock> insMock = nullptr;
};

void FileOperationBaseTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FileOperationBaseTest::TearDownTestCase(void)
{
    fileoperationBase_ = nullptr;
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FileOperationBaseTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock = std::make_shared<AssistantMock>();
    Assistant::ins = insMock;
}

void FileOperationBaseTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    Assistant::ins = nullptr;
    insMock = nullptr;
}

/**
 * @tc.name: ForgetTest001
 * @tc.desc: Verify the Forget function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationBaseTest, ForgetTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetTest001 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        uint64_t nLookup = 0;

        fileoperationBase_->Forget(req, FUSE_ROOT_ID, nLookup);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetTest001 End";
}

/**
 * @tc.name: ForgetTest002
 * @tc.desc: Verify the Forget function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationBaseTest, ForgetTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        uint64_t nLookup = 0;

        fileoperationBase_->Forget(req, 0, nLookup);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetTest002 End";
}

/**
 * @tc.name: ForgetMultiTest001
 * @tc.desc: Verify the ForgetMulti function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationBaseTest, ForgetMultiTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetMultiTest001 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        fuse_forget_data* forgets = nullptr;

        fileoperationBase_->ForgetMulti(req, 0, forgets);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetMultiTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetMultiTest001 End";
}

/**
 * @tc.name: ForgetMultiTest002
 * @tc.desc: Verify the ForgetMulti function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationBaseTest, ForgetMultiTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetMultiTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_forget_data forgets;
        forgets.ino = 2;

        fileoperationBase_->ForgetMulti(req, 1, &forgets);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetMultiTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetMultiTest002 End";
}

/**
 * @tc.name: ForgetMultiTest003
 * @tc.desc: Verify the ForgetMulti function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationBaseTest, ForgetMultiTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetMultiTest003 Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[2] = std::make_shared<CloudDiskInode>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_forget_data forgets;
        forgets.ino = 2;

        fileoperationBase_->ForgetMulti(req, 1, &forgets);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetMultiTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetMultiTest003 End";
}

/**
 * @tc.name: ForgetTest003
 * @tc.desc: Verify the Forget function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationBaseTest, ForgetTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetTest003 Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[1] = std::make_shared<CloudDiskInode>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        uint64_t nLookup = 1;

        fileoperationBase_->Forget(req, 1, nLookup);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetTest003  ERROR";
    }
}

/**
 * @tc.name: LookupTest001
 * @tc.desc: Verify the Lookup function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, LookupTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t parent = FUSE_ROOT_ID;
        const char *name = "testFile";
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->Lookup(req, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookupTest001 failed";
    }
    GTEST_LOG_(INFO) << "LookupTest001 end";
}

/**
 * @tc.name: AccessTest001
 * @tc.desc: Verify the Access function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, AccessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessTest001 start";
    try {
        fuse_ino_t ino = FUSE_ROOT_ID;
        fuse_req_t req = nullptr;
        int mask = 0;
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->Access(req, ino, mask);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AccessTest001 failed";
    }
    GTEST_LOG_(INFO) << "AccessTest001 end";
}

/**
 * @tc.name: GetAttrTest001
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, GetAttrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        auto fi = std::make_shared<fuse_file_info>();
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->GetAttr(req, ino, fi.get());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetAttrTest001 end";
}

/**
 * @tc.name: OpenTest001
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, OpenTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        auto fi = std::make_shared<fuse_file_info>();
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->Open(req, ino, fi.get());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest001 failed";
    }
    GTEST_LOG_(INFO) << "OpenTest001 end";
}

/**
 * @tc.name: MkDirTest001
 * @tc.desc: Verify the MkDir function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, MkDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkDirTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t parent = FUSE_ROOT_ID;
        const char *name = "testMkdir";
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->MkDir(req, parent, name, 0);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkDirTest001 failed";
    }
    GTEST_LOG_(INFO) << "MkDirTest001 end";
}

/**
 * @tc.name: RmDirTest001
 * @tc.desc: Verify the RmDir function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, RmDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmDirTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t parent = FUSE_ROOT_ID;
        const char *name = "testRmDir";
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->RmDir(req, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RmDirTest001 failed";
    }
    GTEST_LOG_(INFO) << "RmDirTest001 end";
}

/**
 * @tc.name: UnlinkTest001
 * @tc.desc: Verify the Unlink function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, UnlinkTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t parent = FUSE_ROOT_ID;
        const char *name = "testUnlink";
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->Unlink(req, parent, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnlinkTest001 failed";
    }
    GTEST_LOG_(INFO) << "UnlinkTest001 end";
}

/**
 * @tc.name: ReleaseTest001
 * @tc.desc: Verify the Release function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, ReleaseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReleaseTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t parent = FUSE_ROOT_ID;
        auto fi = std::make_shared<fuse_file_info>();
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->Release(req, parent, fi.get());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReleaseTest001 failed";
    }
    GTEST_LOG_(INFO) << "ReleaseTest001 end";
}

/**
 * @tc.name: MkNodTest001
 * @tc.desc: Verify the MkNod function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, MkNodTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkNodTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t parent = FUSE_ROOT_ID;
        const char *name = "testMkmod";
        mode_t mode = 0;
        dev_t rdev = 0;
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->MkNod(req, parent, name, mode, rdev);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkNodTest001 failed";
    }
    GTEST_LOG_(INFO) << "MkNodTest001 end";
}

/**
 * @tc.name: CreateTest001
 * @tc.desc: Verify the Create function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, CreateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t parent = FUSE_ROOT_ID;
        const char *name = "testMkmod";
        mode_t mode = 0;
        auto fi = std::make_shared<fuse_file_info>();
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->Create(req, parent, name, mode, fi.get());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkNodTest001 failed";
    }
    GTEST_LOG_(INFO) << "MkNodTest001 end";
}

/**
 * @tc.name: ReadDirTest001
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, ReadDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        size_t size = 0;
        off_t off = 0;
        auto fi = std::make_shared<fuse_file_info>();
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->ReadDir(req, ino, size, off, fi.get());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest001 failed";
    }
    GTEST_LOG_(INFO) << "ReadDirTest001 end";
}

/**
 * @tc.name: SetXattrTest001
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, SetXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        const char *name = "testSetXattr";
        const char *value = "";
        size_t size = 0;
        int flags = 0;
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->SetXattr(req, ino, name, value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest001 failed";
    }
    GTEST_LOG_(INFO) << "SetXattrTest001 end";
}

/**
 * @tc.name: GetXattrTest001
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, GetXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        const char *name = "testGetXattr";
        size_t size = 0;
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->GetXattr(req, ino, name, size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest001 failed";
    }
    GTEST_LOG_(INFO) << "GetXattrTest001 end";
}

/**
 * @tc.name: RenameTest001
 * @tc.desc: Verify the Rename function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, RenameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 0;
        const char *name = "testRename";
        fuse_ino_t newParent = FUSE_ROOT_ID;
        const char *newName = "renameTest";
        unsigned int flags = 0;
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->Rename(req, parent, name, newParent, newName, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTest001 failed";
    }
    GTEST_LOG_(INFO) << "RenameTest001 end";
}

/**
 * @tc.name: ReadTest001
 * @tc.desc: Verify the Read function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, ReadTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        size_t size = 0;
        off_t offset = 0;
        auto fi = std::make_shared<fuse_file_info>();
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->Read(req, ino, size, offset, fi.get());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadTest001 failed";
    }
    GTEST_LOG_(INFO) << "ReadTest001 end";
}

/**
 * @tc.name: WriteBufTest001
 * @tc.desc: Verify the WriteBuf function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, WriteBufTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteBufTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        std::shared_ptr<fuse_bufvec> buf = std::make_shared<fuse_bufvec>();
        off_t offset = 0;
        auto fi = std::make_shared<fuse_file_info>();
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->WriteBuf(req, ino, buf.get(), offset, fi.get());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteBufTest001 failed";
    }
    GTEST_LOG_(INFO) << "WriteBufTest001 end";
}

/**
 * @tc.name: SetAttrTest001
 * @tc.desc: Verify the SetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, SetAttrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAttrTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        std::shared_ptr<struct stat> attr = std::make_shared<struct stat>();
        int valid = 0;
        auto fi = std::make_shared<fuse_file_info>();
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->SetAttr(req, ino, attr.get(), valid, fi.get());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetAttrTest001 failed";
    }
    GTEST_LOG_(INFO) << "SetAttrTest001 end";
}

/**
 * @tc.name: LseekTest001
 * @tc.desc: Verify the Lseek function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FileOperationBaseTest, LseekTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekTest001 start";
    try {
        fuse_req_t req = nullptr;
        fuse_ino_t ino = FUSE_ROOT_ID;
        off_t offset = 0;
        int whence = 0;
        auto fi = std::make_shared<fuse_file_info>();
        EXPECT_CALL(*insMock, fuse_reply_err(_, _)).Times(1);
        fileoperationBase_->Lseek(req, ino, offset, whence, fi.get());
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LseekTest001 failed";
    }
    GTEST_LOG_(INFO) << "LseekTest001 end";
}
} // namespace OHOS::FileManagement::CloudDisk::Test