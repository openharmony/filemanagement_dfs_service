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

#include "file_operations_base.h"
#include "cloud_disk_inode.h"
#include "file_operations_helper.h"
#include "dfs_error.h"
#include "assistant.h"

namespace OHOS::FileManagement::CloudDisk::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;

class FileOperationBaseTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline FileOperationsBase* fileoperationBase_ = new FileOperationsBase();
    static inline shared_ptr<AssistantMock> insMock = nullptr;
};

void FileOperationBaseTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
}

void FileOperationBaseTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    fileoperationBase_ = nullptr;
    Assistant::ins = nullptr;
    insMock = nullptr;
}

void FileOperationBaseTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FileOperationBaseTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
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
        (data.inodeCache)[2] = make_shared<CloudDiskInode>();
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
        (data.inodeCache)[1] = make_shared<CloudDiskInode>();
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        uint64_t nLookup = 1;

        fileoperationBase_->Forget(req, 1, nLookup);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ForgetTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "ForgetTest003 End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test