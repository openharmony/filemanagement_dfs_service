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
#include "file_operations_cloud.h"
#include "file_operations_helper.h"
#include "file_operations_base.h"
#include "parameters.h"
#include "utils_log.h"
#include "assistant.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FuseOperationsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<FuseOperations> fuseoperations_ = nullptr;
    static inline shared_ptr<AssistantMock> insMock = nullptr;
};

void FuseOperationsTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    fuseoperations_ = make_shared<FuseOperations>();
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
}

void FuseOperationsTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    fuseoperations_ = nullptr;
    Assistant::ins = nullptr;
    insMock = nullptr;
}

void FuseOperationsTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FuseOperationsTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: LookupTest001
 * @tc.desc: Verify the Lookup function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, LookupTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest001 Start";
    try {
        fuse_req_t req = nullptr;
        const char *name = "";

        fuseoperations_->Lookup(req, FUSE_ROOT_ID, name);
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
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, LookupTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = "";

        fuseoperations_->Lookup(req, 0, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookupTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "LookupTest002 End";
}

/**
 * @tc.name: LookupTest
 * @tc.desc: Verify the Lookup function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, LookupTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = "";
        fuse_ino_t parent = 0;

        fuseoperations_->Lookup(req, parent, name);
        EXPECT_NE(parent, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookupTest  ERROR";
    }
    GTEST_LOG_(INFO) << "LookupTest End";
}

/**
 * @tc.name:AccessTest001
 * @tc.desc: Verify the Access function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, AccessTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessTest001 Start";
    try {
        fuse_req_t req = nullptr;
        int mask = 0;

        fuseoperations_->Access(req, FUSE_ROOT_ID, mask);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AccessTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "AccessTest001 End";
}

/**
 * @tc.name:AccessTest002
 * @tc.desc: Verify the Access function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, AccessTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        int mask = 0;

        fuseoperations_->Access(req, 0, mask);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AccessTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "AccessTest002 End";
}

/**
 * @tc.name:AccessTest
 * @tc.desc: Verify the Access function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, AccessTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        int mask = 0;

        fuseoperations_->Access(req, ino, mask);
        EXPECT_NE(ino, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AccessTest ERROR";
    }
    GTEST_LOG_(INFO) << "AccessTest End";
}

/**
 * @tc.name:GetAttrTest001
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, GetAttrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest001 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_file_info *fi = nullptr;

        fuseoperations_->GetAttr(req, FUSE_ROOT_ID, fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "GetAttrTest001 End";
}

/**
 * @tc.name:GetAttrTest002
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, GetAttrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_file_info *fi = nullptr;

        fuseoperations_->GetAttr(req, 0, fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "GetAttrTest002 End";
}

/**
 * @tc.name:GetAttrTest
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, GetAttrTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        struct fuse_file_info *fi = nullptr;

        fuseoperations_->GetAttr(req, ino, fi);
        EXPECT_NE(ino, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest ERROR";
    }
    GTEST_LOG_(INFO) << "GetAttrTest End";
}

/**
 * @tc.name:OpenTest001
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, OpenTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenTest001 Start";
    try {
        fuse_req_t req = nullptr;
        struct fuse_file_info *fi = nullptr;

        fuseoperations_->Open(req, FUSE_ROOT_ID, fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "OpenTest001 End";
}

/**
 * @tc.name:OpenTest002
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, OpenTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_file_info *fi = nullptr;

        fuseoperations_->Open(req, 0, fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "OpenTest002 End";
}

/**
 * @tc.name:OpenTest
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, OpenTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        struct fuse_file_info fi;
        fi.fh = 1;

        fuseoperations_->Open(req, ino, &fi);
        EXPECT_NE(ino, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest ERROR";
    }
    GTEST_LOG_(INFO) << "OpenTest End";
}

/**
 * @tc.name:ForgetTest001
 * @tc.desc: Verify the Forget function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ForgetTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetTest001 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        uint64_t nLookup = 0;

        fuseoperations_->Forget(req, FUSE_ROOT_ID, nLookup);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetTest001 End";
}

/**
 * @tc.name:ForgetTest002
 * @tc.desc: Verify the Forget function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ForgetTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        uint64_t nLookup = 0;

        fuseoperations_->Forget(req, 0, nLookup);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetTest002 End";
}

/**
 * @tc.name:ForgetTest
 * @tc.desc: Verify the Forget function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ForgetTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        uint64_t nLookup = 0;

        fuseoperations_->Forget(req, ino, nLookup);
        EXPECT_NE(ino, FUSE_ROOT_ID);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetTest ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetTest End";
}

/**
 * @tc.name:ForgetMultiTest001
 * @tc.desc: Verify the ForgetMulti function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ForgetMultiTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetMultiTest001 Start";
    try {
        fuse_req_t req = nullptr;
        struct fuse_forget_data forgets = {
            FUSE_ROOT_ID,
            1,
        };

        fuseoperations_->ForgetMulti(req, 1, &forgets);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetMultiTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetMultiTest001 End";
}

/**
 * @tc.name:ForgetMultiTest002
 * @tc.desc: Verify the ForgetMulti function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ForgetMultiTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetMultiTest002 Start";
    try {
        fuse_req_t req = nullptr;
        struct fuse_forget_data forgets = {
            0,
            1,
        };

        fuseoperations_->ForgetMulti(req, 0, &forgets);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetMultiTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetMultiTest002 End";
}

/**
 * @tc.name:ForgetMultiTest01
 * @tc.desc: Verify the ForgetMulti function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ForgetMultiTest01, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetMultiTest01 Start";
    try {
        CloudDiskFuseData data1;
        CloudDiskFuseData data2;
        (data1.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data1.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data1)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data2)));
        fuse_req_t req = nullptr;
        struct fuse_forget_data forgets;
        forgets.ino = 0;
        size_t count = 1;

        fuseoperations_->ForgetMulti(req, count, &forgets);
        EXPECT_NE(count, 0);
        EXPECT_NE((data1.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetMultiTest01 ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetMultiTest01 End";
}

/**
 * @tc.name:ForgetMultiTest02
 * @tc.desc: Verify the ForgetMulti function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ForgetMultiTest02, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetMultiTest02 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_forget_data forgets;
        forgets.ino = 0;
        size_t count = 1;

        fuseoperations_->ForgetMulti(req, count, &forgets);
        EXPECT_NE(count, 0);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetMultiTest02 ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetMultiTest02 End";
}

/**
 * @tc.name:MkNodTest001
 * @tc.desc: Verify the MkNod function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, MkNodTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkNodTest001 Start";
    try {
        fuse_req_t req = nullptr;
        const char *name = "";
        mode_t mode = 0;
        dev_t rdev = 0;

        fuseoperations_->MkNod(req, FUSE_ROOT_ID, name, mode, rdev);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkNodTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "MkNodTest001 End";
}

/**
 * @tc.name:MkNodTest002
 * @tc.desc: Verify the MkNod function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, MkNodTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkNodTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = "";
        mode_t mode = 0;
        dev_t rdev = 0;

        fuseoperations_->MkNod(req, 0, name, mode, rdev);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkNodTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "MkNodTest002 End";
}

/**
 * @tc.name:MkNodTest
 * @tc.desc: Verify the MkNod function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, MkNodTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkNodTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 0;
        const char *name = "";
        mode_t mode = 0;
        dev_t rdev = 0;

        fuseoperations_->MkNod(req, parent, name, mode, rdev);
        EXPECT_NE(parent, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkNodTest ERROR";
    }
    GTEST_LOG_(INFO) << "MkNodTest End";
}

/**
 * @tc.name:CreateTest001
 * @tc.desc: Verify the Create function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, CreateTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateTest001 Start";
    try {
        fuse_req_t req = nullptr;
        const char *name = "";
        mode_t mode = 0;
        struct fuse_file_info fi;

        fuseoperations_->Create(req, FUSE_ROOT_ID, name, mode, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "CreateTest001 End";
}

/**
 * @tc.name:CreateTest002
 * @tc.desc: Verify the Create function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, CreateTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = "";
        mode_t mode = 0;
        struct fuse_file_info fi;

        fuseoperations_->Create(req, 0, name, mode, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "CreateTest002 End";
}

/**
 * @tc.name:CreateTest
 * @tc.desc: Verify the Create function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, CreateTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateTesT Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 0;
        const char *name = "";
        mode_t mode = 0;
        struct fuse_file_info fi;

        fuseoperations_->Create(req, parent, name, mode, &fi);
        EXPECT_NE(parent, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateTest ERROR";
    }
    GTEST_LOG_(INFO) << "CreateTest End";
}

/**
 * @tc.name:ReadDirTest001
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReadDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest001 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        fuseoperations_->ReadDir(req, FUSE_ROOT_ID, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest001 End";
}

/**
 * @tc.name:ReadDirTest002
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReadDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        fuseoperations_->ReadDir(req, 0, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest002 End";
}

/**
 * @tc.name:ReadDirTest
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReadDirTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        fuseoperations_->ReadDir(req, ino, size, off, &fi);
        EXPECT_NE(ino, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest End";
}

/**
 * @tc.name:SetXattrTest001
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, SetXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest001 Start";
    try {
        fuse_req_t req = nullptr;
        string name = HMDFS_PERMISSION_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        fuseoperations_->SetXattr(req, FUSE_ROOT_ID, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest001 End";
}

/**
 * @tc.name:SetXattrTest002
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, SetXattrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        string name = HMDFS_PERMISSION_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        fuseoperations_->SetXattr(req, 0, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest002 End";
}

/**
 * @tc.name:SetXattrTest
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, SetXattrTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        string name = HMDFS_PERMISSION_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        fuseoperations_->SetXattr(req, ino, name.c_str(), value, size, flags);
        EXPECT_NE(ino, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest  ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest End";
}

/**
 * @tc.name:GetXattrTest001
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, GetXattrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest001 Start";
    try {
        fuse_req_t req = nullptr;
        size_t size = 0;
        string name = HMDFS_PERMISSION_XATTR;

        fuseoperations_->GetXattr(req, FUSE_ROOT_ID, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest001 End";
}

/**
 * @tc.name:GetXattrTest002
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, GetXattrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        size_t size = 0;
        string name = HMDFS_PERMISSION_XATTR;

        fuseoperations_->GetXattr(req, 0, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest002 End";
}

/**
 * @tc.name:GetXattrTest
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, GetXattrTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 0;
        string name = HMDFS_PERMISSION_XATTR;

        fuseoperations_->GetXattr(req, ino, name.c_str(), size);
        EXPECT_NE(ino, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest End";
}

/**
 * @tc.name:MkDirTest001
 * @tc.desc: Verify the MkDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, MkDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkDirTest001 Start";
    try {
        fuse_req_t req = nullptr;
        const char *name = nullptr;
        mode_t mode = 0;


        fuseoperations_->MkDir(req, FUSE_ROOT_ID, name, mode);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkDirTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "MkDirTest001 End";
}

/**
 * @tc.name:MkDirTest002
 * @tc.desc: Verify the MkDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, MkDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkDirTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = nullptr;
        mode_t mode = 0;


        fuseoperations_->MkDir(req, 0, name, mode);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkDirTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "MkDirTest002 End";
}

/**
 * @tc.name:MkDirTest
 * @tc.desc: Verify the MkDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, MkDirTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkDirTest Start";
    try {
        CloudDiskFuseData data1;
        CloudDiskFuseData data2;
        (data1.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data1.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data1)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data2)));
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 0;
        const char *name = nullptr;
        mode_t mode = 0;


        fuseoperations_->MkDir(req, parent, name, mode);
        EXPECT_NE(parent, FUSE_ROOT_ID);
        EXPECT_NE((data1.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkDirTest ERROR";
    }
    GTEST_LOG_(INFO) << "MkDirTest End";
}

/**
 * @tc.name:RmDirTest001
 * @tc.desc: Verify the RmDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, RmDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmDirTest001 Start";
    try {
        fuse_req_t req = nullptr;
        const char *name = nullptr;

        fuseoperations_->RmDir(req, FUSE_ROOT_ID, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RmDirTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "RmDirTest001 End";
}

/**
 * @tc.name:RmDirTest002
 * @tc.desc: Verify the RmDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, RmDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmDirTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = nullptr;

        fuseoperations_->RmDir(req, 0, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RmDirTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "RmDirTest002 End";
}

/**
 * @tc.name:RmDirTest
 * @tc.desc: Verify the RmDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, RmDirTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmDirTest Start";
    try {
        CloudDiskFuseData data1;
        CloudDiskFuseData data2;
        (data1.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data1.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data1)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data2)));
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 0;
        const char *name = nullptr;

        fuseoperations_->RmDir(req, parent, name);
        EXPECT_NE(parent, FUSE_ROOT_ID);
        EXPECT_NE((data1.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RmDirTest ERROR";
    }
    GTEST_LOG_(INFO) << "RmDirTest End";
}

/**
 * @tc.name:UnlinkTest001
 * @tc.desc: Verify the Unlink function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, UnlinkTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkTest001 Start";
    try {
        fuse_req_t req = nullptr;
        const char *name = nullptr;

        fuseoperations_->Unlink(req, FUSE_ROOT_ID, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnlinkTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "UnlinkTest001 End";
}

/**
 * @tc.name:UnlinkTest002
 * @tc.desc: Verify the Unlink function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, UnlinkTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = nullptr;

        fuseoperations_->Unlink(req, 0, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnlinkTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "UnlinkTest002 End";
}

/**
 * @tc.name:UnlinkTest
 * @tc.desc: Verify the Unlink function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, UnlinkTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkTest Start";
    try {
        CloudDiskFuseData data1;
        CloudDiskFuseData data2;
        (data1.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data1.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data1)))
                                                   .WillOnce(Return(reinterpret_cast<void*>(&data2)))
                                                   .WillOnce(Return(reinterpret_cast<void*>(&data2)));
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 0;
        const char *name = nullptr;

        fuseoperations_->Unlink(req, parent, name);
        EXPECT_NE(parent, FUSE_ROOT_ID);
        EXPECT_NE((data1.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnlinkTest ERROR";
    }
    GTEST_LOG_(INFO) << "UnlinkTest End";
}

/**
 * @tc.name:RenameTest001
 * @tc.desc: Verify the Rename function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, RenameTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTest001 Start";
    try {
        fuse_req_t req = nullptr;
        const char *name = "";
        const char *newName = "";
        unsigned int flags = 1;

        fuseoperations_->Rename(req, FUSE_ROOT_ID, name, FUSE_ROOT_ID, newName, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "RenameTest001 End";
}

/**
 * @tc.name:RenameTest002
 * @tc.desc: Verify the Rename function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, RenameTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = "";
        const char *newName = "";
        unsigned int flags = 1;

        fuseoperations_->Rename(req, 0, name, FUSE_ROOT_ID, newName, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "RenameTest002 End";
}

/**
 * @tc.name:RenameTest
 * @tc.desc: Verify the Rename function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, RenameTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t parent = 0;
        const char *name = "";
        const char *newName = "";
        unsigned int flags = 1;

        fuseoperations_->Rename(req, parent, name, FUSE_ROOT_ID, newName, flags);
        EXPECT_NE(parent, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTest ERROR";
    }
    GTEST_LOG_(INFO) << "RenameTest End";
}

/**
 * @tc.name:ReadTest001
 * @tc.desc: Verify the Read function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReadTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTest001 Start";
    try {
        fuse_req_t req = nullptr;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        fuseoperations_->Read(req, FUSE_ROOT_ID, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadTest001 End";
}

/**
 * @tc.name:ReadTest002
 * @tc.desc: Verify the Read function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReadTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        fuseoperations_->Read(req, 0, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadTest002 End";
}

/**
 * @tc.name:ReadTest
 * @tc.desc: Verify the Read function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReadTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        fuseoperations_->Read(req, ino, size, off, &fi);
        EXPECT_NE(ino, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadTest ERROR";
    }
    GTEST_LOG_(INFO) << "ReadTest End";
}

/**
 * @tc.name:WriteBufTest001
 * @tc.desc: Verify the WriteBuf function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, WriteBufTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteBufTest001 Start";
    try {
        fuse_req_t req = nullptr;
        struct fuse_bufvec *bufv = nullptr;
        off_t offset = 0;
        struct fuse_file_info fi;

        fuseoperations_->WriteBuf(req, FUSE_ROOT_ID, bufv, offset, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteBufTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "WriteBufTest001 End";
}

/**
 * @tc.name:WriteBufTest002
 * @tc.desc: Verify the WriteBuf function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, WriteBufTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteBufTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_bufvec *bufv = nullptr;
        off_t offset = 0;
        struct fuse_file_info fi;

        fuseoperations_->WriteBuf(req, 0, bufv, offset, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteBufTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "WriteBufTest002 End";
}

/**
 * @tc.name:WriteBufTest
 * @tc.desc: Verify the WriteBuf function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, WriteBufTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteBufTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        struct fuse_bufvec *bufv = nullptr;
        off_t offset = 0;
        struct fuse_file_info fi;

        fuseoperations_->WriteBuf(req, ino, bufv, offset, &fi);
        EXPECT_NE(ino, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteBufTest ERROR";
    }
    GTEST_LOG_(INFO) << "WriteBufTest End";
}

/**
 * @tc.name:ReleaseTest001
 * @tc.desc: Verify the Release function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReleaseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReleaseTest001 Start";
    try {
        fuse_req_t req = nullptr;
        struct fuse_file_info fi;

        fuseoperations_->Release(req, FUSE_ROOT_ID, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReleaseTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "ReleaseTest001 End";
}

/**
 * @tc.name:ReleaseTest002
 * @tc.desc: Verify the Release function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReleaseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReleaseTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_file_info fi;

        fuseoperations_->Release(req, 0, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReleaseTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "ReleaseTest002 End";
}

/**
 * @tc.name:ReleaseTest
 * @tc.desc: Verify the Release function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReleaseTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReleaseTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        struct fuse_file_info fi;

        fuseoperations_->Release(req, ino, &fi);
        EXPECT_NE(ino, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReleaseTest ERROR";
    }
    GTEST_LOG_(INFO) << "ReleaseTest End";
}

/**
 * @tc.name:SetAttrTest001
 * @tc.desc: Verify the SetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, SetAttrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAttrTest001 Start";
    try {
        fuse_req_t req = nullptr;
        struct fuse_file_info fi;
        struct stat attr;
        int valid = 0;

        fuseoperations_->SetAttr(req, FUSE_ROOT_ID, &attr, valid, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetAttrTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "SetAttrTest001 End";
}

/**
 * @tc.name:SetAttrTest002
 * @tc.desc: Verify the SetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, SetAttrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAttrTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_file_info fi;
        struct stat attr;
        int valid = 0;

        fuseoperations_->SetAttr(req, 0, &attr, valid, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetAttrTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "SetAttrTest002 End";
}

/**
 * @tc.name:SetAttrTest
 * @tc.desc: Verify the SetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, SetAttrTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAttrTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        struct fuse_file_info fi;
        struct stat attr;
        int valid = 0;

        fuseoperations_->SetAttr(req, ino, &attr, valid, &fi);
        EXPECT_NE(ino, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetAttrTest ERROR";
    }
    GTEST_LOG_(INFO) << "SetAttrTest End";
}

/**
 * @tc.name:LseekTest001
 * @tc.desc: Verify the Lseek function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, LseekTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekTest001 Start";
    try {
        fuse_req_t req = nullptr;
        off_t off = 0;
        int whence = 0;
        struct fuse_file_info fi;

        fuseoperations_->Lseek(req, FUSE_ROOT_ID, off, whence, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LseekTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "LseekTest001 End";
}

/**
 * @tc.name:LseekTest002
 * @tc.desc: Verify the Lseek function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, LseekTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        off_t off = 0;
        int whence = 0;
        struct fuse_file_info fi;

        fuseoperations_->Lseek(req, 0, off, whence, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LseekTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "LseekTest002 End";
}

/**
 * @tc.name:LseekTest
 * @tc.desc: Verify the Lseek function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, LseekTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekTest Start";
    try {
        CloudDiskFuseData data;
        (data.inodeCache)[0] = make_shared<CloudDiskInode>();
        (data.inodeCache)[0]->ops = make_shared<FileOperationsCloud>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void *>(&data)))
                                                   .WillOnce(Return(reinterpret_cast<void *>(&data)));
        fuse_req_t req = nullptr;
        fuse_ino_t ino = 0;
        off_t off = 0;
        int whence = 0;
        struct fuse_file_info fi;

        fuseoperations_->Lseek(req, ino, off, whence, &fi);
        EXPECT_NE(ino, FUSE_ROOT_ID);
        EXPECT_NE((data.inodeCache)[0], nullptr);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LseekTest ERROR";
    }
    GTEST_LOG_(INFO) << "LseekTest End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test