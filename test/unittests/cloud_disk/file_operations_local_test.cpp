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
 * @tc.name:GetAttrTest001
 * @tc.desc: Verify the GetAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, GetAttrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest001 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_file_info *fi = nullptr;


        fileoperationslocal_->GetAttr(req, FUSE_ROOT_ID, fi);
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
HWTEST_F(FileOperationsLocalTest, GetAttrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetAttrTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        struct fuse_file_info *fi = nullptr;


        fileoperationslocal_->GetAttr(req, 0, fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetAttrTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "GetAttrTest002 End";
}

/**
 * @tc.name:ReadDirTest001
 * @tc.desc: Verify the ReadDir function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FileOperationsLocalTest, ReadDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest001 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        fileoperationslocal_->ReadDir(req, FUSE_ROOT_ID, size, off, &fi);
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
HWTEST_F(FileOperationsLocalTest, ReadDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReadDirTest002 Start";
    try {
        CloudDiskFuseData data;
        EXPECT_CALL(*insMock, fuse_req_userdata(_)).WillOnce(Return(reinterpret_cast<void*>(&data)));
        fuse_req_t req = nullptr;
        size_t size = 0;
        off_t off = 0;
        struct fuse_file_info fi;

        fileoperationslocal_->ReadDir(req, 0, size, off, &fi);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "ReadDirTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "ReadDirTest002 End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test