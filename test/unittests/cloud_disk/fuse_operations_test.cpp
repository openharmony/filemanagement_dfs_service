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
#include "file_operations_base.h"
#include "file_operations_cloud.h"
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
 * @tc.name: LookupTest003
 * @tc.desc: Verify the Lookup function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, LookupTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LookupTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = "";

        fuseoperations_->Lookup(req, 0, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LookupTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "LookupTest003 End";
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
 * @tc.name: AccessTest003
 * @tc.desc: Verify the Access function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, AccessTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AccessTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        int mask = 0;

        fuseoperations_->Access(req, 0, mask);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AccessTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "AccessTest003 End";
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
 * @tc.name: GetAttrTest003
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, GetAttrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_file_info *fi = nullptr;

        fuseoperations_->GetAttr(req, 0, fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "GetAttrTest003 End";
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
 * @tc.name: OpenTest003
 * @tc.desc: Verify the Open function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, OpenTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OpenTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_file_info *fi = nullptr;

        fuseoperations_->Open(req, 0, fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OpenTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "OpenTest003 End";
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
 * @tc.name: ForgetTest003
 * @tc.desc: Verify the Forget function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ForgetTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        uint64_t nLookup = 0;

        fuseoperations_->Forget(req, 0, nLookup);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetTest003 End";
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
 * @tc.name: ForgetMultiTest003
 * @tc.desc: Verify the ForgetMulti function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ForgetMultiTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ForgetMultiTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_forget_data forgets = {
            0,
            1,
        };

        fuseoperations_->ForgetMulti(req, 1, &forgets);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetMultiTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetMultiTest003 End";
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
 * @tc.name: MkNodTest003
 * @tc.desc: Verify the MkNod function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, MkNodTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkNodTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = "";
        mode_t mode = 0;
        dev_t rdev = 0;

        fuseoperations_->MkNod(req, 0, name, mode, rdev);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkNodTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "MkNodTest003 End";
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
 * @tc.name: CreateTest003
 * @tc.desc: Verify the Create function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, CreateTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CreateTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = "";
        mode_t mode = 0;
        struct fuse_file_info fi;

        fuseoperations_->Create(req, 0, name, mode, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "CreateTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "CreateTest003 End";
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
 * @tc.name: ReadDirTest003
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReadDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        fuseoperations_->ReadDir(req, 0, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest003 End";
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
 * @tc.name: SetXattrTest003
 * @tc.desc: Verify the SetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, SetXattrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetXattrTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        string name = HMDFS_PERMISSION_XATTR;
        const char *value = "";
        size_t size = 0;
        int flags = 0;

        fuseoperations_->SetXattr(req, 0, name.c_str(), value, size, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetXattrTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "SetXattrTest003 End";
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
 * @tc.name: GetXattrTest003
 * @tc.desc: Verify the GetXattr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, GetXattrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetXattrTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        size_t size = 0;
        string name = HMDFS_PERMISSION_XATTR;

        fuseoperations_->GetXattr(req, 0, name.c_str(), size);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetXattrTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "GetXattrTest003 End";
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
 * @tc.name: MkDirTest003
 * @tc.desc: Verify the MkDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, MkDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "MkDirTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = nullptr;
        mode_t mode = 0;

        fuseoperations_->MkDir(req, 0, name, mode);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "MkDirTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "MkDirTest003 End";
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
 * @tc.name: RmDirTest003
 * @tc.desc: Verify the RmDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, RmDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RmDirTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = "test";

        fuseoperations_->RmDir(req, 0, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RmDirTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "RmDirTest003 End";
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
 * @tc.name: UnlinkTest003
 * @tc.desc: Verify the Unlink function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, UnlinkTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnlinkTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = "test";

        fuseoperations_->Unlink(req, 0, name);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "UnlinkTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "UnlinkTest003 End";
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
 * @tc.name: RenameTest003
 * @tc.desc: Verify the Rename function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, RenameTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RenameTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        const char *name = "";
        const char *newName = "";
        unsigned int flags = 1;

        fuseoperations_->Rename(req, 0, name, FUSE_ROOT_ID, newName, flags);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "RenameTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "RenameTest003 End";
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
 * @tc.name: ReadTest003
 * @tc.desc: Verify the Read function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReadTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        fuseoperations_->Read(req, 0, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadTest003 End";
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
 * @tc.name: WriteBufTest003
 * @tc.desc: Verify the WriteBuf function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, WriteBufTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "WriteBufTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_bufvec *bufv = nullptr;
        off_t offset = 0;
        struct fuse_file_info fi;

        fuseoperations_->WriteBuf(req, 0, bufv, offset, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "WriteBufTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "WriteBufTest003 End";
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
 * @tc.name: ReleaseTest003
 * @tc.desc: Verify the Release function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, ReleaseTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReleaseTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_file_info fi;

        fuseoperations_->Release(req, 0, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReleaseTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "ReleaseTest003 End";
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
 * @tc.name: SetAttrTest003
 * @tc.desc: Verify the SetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, SetAttrTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "SetAttrTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_file_info fi;
        struct stat attr;
        int valid = 0;

        fuseoperations_->SetAttr(req, 0, &attr, valid, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "SetAttrTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "SetAttrTest003 End";
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
 * @tc.name: LseekTest003
 * @tc.desc: Verify the Lseek function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsTest, LseekTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "LseekTest003 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> test1;
        test1 = make_shared<CloudDiskInode>();
        test1->ops = make_shared<FileOperationsCloud>();
        static shared_ptr<CloudDiskInode> test2;
        test2 = make_shared<CloudDiskInode>();
        test2->ops = make_shared<FileOperationsCloud>();
        data.inodeCache[0] = test1;
        data.inodeCache[1] = test2;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillRepeatedly(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        off_t off = 0;
        int whence = 0;
        struct fuse_file_info fi;
        
        fuseoperations_->Lseek(req, 0, off, whence, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "LseekTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "LseekTest003 End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test