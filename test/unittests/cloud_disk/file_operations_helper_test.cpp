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

class FuseOperationsHelperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    static inline shared_ptr<FileOperationsHelper> fuseoperationshelper_ = nullptr;
    static inline shared_ptr<AssistantMock> insMock = nullptr;
};

void FuseOperationsHelperTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
    fuseoperationshelper_ = make_shared<FileOperationsHelper>();
    insMock = make_shared<AssistantMock>();
    Assistant::ins = insMock;
}

void FuseOperationsHelperTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
    fuseoperationshelper_ = nullptr;
    Assistant::ins = nullptr;
    insMock = nullptr;
}

void FuseOperationsHelperTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
}

void FuseOperationsHelperTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name:GetNextLayerTest001
 * @tc.desc: Verify the GetNextLayer function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetNextLayerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNextLayerTest001 Start";
    try {
        static shared_ptr<CloudDiskInode> inoPtr;
        inoPtr = make_shared<CloudDiskInode>();

        fuseoperationshelper_->GetNextLayer(inoPtr, FUSE_ROOT_ID);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNextLayerTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "GetNextLayerTest001 End";
}

/**
 * @tc.name:GetNextLayerTest002
 * @tc.desc: Verify the GetNextLayer function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetNextLayerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNextLayerTest002 Start";
    try {
        static shared_ptr<CloudDiskInode> inoPtr;
        inoPtr = make_shared<CloudDiskInode>();

        fuseoperationshelper_->GetNextLayer(inoPtr, 0);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNextLayerTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "GetNextLayerTest002 End";
}

/**
 * @tc.name:GetNextLayerTest003
 * @tc.desc: Verify the GetNextLayer function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetNextLayerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNextLayerTest003 Start";
    try {
        static shared_ptr<CloudDiskInode> inoPtr;
        inoPtr = make_shared<CloudDiskInode>();
        inoPtr->layer = 10;

        fuseoperationshelper_->GetNextLayer(inoPtr, 0);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNextLayerTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "GetNextLayerTest003 End";
}

/**
 * @tc.name:GetFixedLayerRootIdTest001
 * @tc.desc: Verify the GetFixedLayerRootId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetFixedLayerRootIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest001 Start";
    try {
        fuseoperationshelper_->GetFixedLayerRootId(CLOUD_DISK_INODE_ZERO_LAYER);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest001 End";
}

/**
 * @tc.name:GetFixedLayerRootIdTest002
 * @tc.desc: Verify the GetFixedLayerRootId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetFixedLayerRootIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest002 Start";
    try {
        fuseoperationshelper_->GetFixedLayerRootId(CLOUD_DISK_INODE_FIRST_LAYER);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest002 End";
}

/**
 * @tc.name:GetFixedLayerRootIdTest003
 * @tc.desc: Verify the GetFixedLayerRootId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetFixedLayerRootIdTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest003 Start";
    try {
        fuseoperationshelper_->GetFixedLayerRootId(CLOUD_DISK_INODE_OTHER_LAYER);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest003  ERROR";
    }
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest003 End";
}

/**
 * @tc.name:FuseReplyLimitedTest001
 * @tc.desc: Verify the FuseReplyLimited function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, FuseReplyLimitedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FuseReplyLimitedTest001 Start";
    try {
        fuse_req_t req = nullptr;
        const char *buf = "";
        size_t bufSize = 1;
        off_t off = 0;
        size_t maxSize = 5;

        fuseoperationshelper_->FuseReplyLimited(req, buf, bufSize, off, maxSize);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FuseReplyLimitedTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "FuseReplyLimitedTest001 End";
}

/**
 * @tc.name:FuseReplyLimitedTest002
 * @tc.desc: Verify the FuseReplyLimited function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, FuseReplyLimitedTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FuseReplyLimitedTest002 Start";
    try {
        fuse_req_t req = nullptr;
        const char *buf = "";
        size_t bufSize = 1;
        off_t off = 1;
        size_t maxSize = 5;

        fuseoperationshelper_->FuseReplyLimited(req, buf, bufSize, off, maxSize);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FuseReplyLimitedTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "FuseReplyLimitedTest002 End";
}

/**
 * @tc.name:PutCloudDiskInodeTest001
 * @tc.desc: Verify the PutCloudDiskInode function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutCloudDiskInodeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCloudDiskInodeTest001 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> inoPtr = nullptr;
        uint64_t num = 0;
        int64_t key = 0;

        fuseoperationshelper_->PutCloudDiskInode(&data, inoPtr, num, key);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutCloudDiskInodeTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "PutCloudDiskInodeTest001 End";
}

/**
 * @tc.name:PutCloudDiskInodeTest002
 * @tc.desc: Verify the PutCloudDiskInode function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutCloudDiskInodeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCloudDiskInodeTest002 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> inoPtr;
        inoPtr = make_shared<CloudDiskInode>();
        inoPtr->refCount = 1;
        uint64_t num = 1;
        int64_t key = 0;

        fuseoperationshelper_->PutCloudDiskInode(&data, inoPtr, num, key);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutCloudDiskInodeTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "PutCloudDiskInodeTest002 End";
}

/**
 * @tc.name:PutCloudDiskFileTest001
 * @tc.desc: Verify the PutCloudDiskFile function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutCloudDiskFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCloudDiskFileTest001 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> inoPtr;
        inoPtr = make_shared<CloudDiskInode>();
        static shared_ptr<CloudDiskFile> filePtr;
        filePtr = make_shared<CloudDiskFile>();
        filePtr->refCount = 0;
        int64_t key = 0;

        fuseoperationshelper_->PutCloudDiskFile(&data, filePtr, key);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutCloudDiskFileTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "PutCloudDiskFileTest001 End";
}

/**
 * @tc.name:PutCloudDiskFileTest002
 * @tc.desc: Verify the PutCloudDiskFile function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutCloudDiskFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCloudDiskFileTest002 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> inoPtr = nullptr;
        static shared_ptr<CloudDiskFile> filePtr;
        filePtr = make_shared<CloudDiskFile>();
        filePtr->refCount = 0;
        int64_t key = 0;

        fuseoperationshelper_->PutCloudDiskFile(&data, filePtr, key);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutCloudDiskFileTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "PutCloudDiskFileTest002 End";
}

/**
 * @tc.name:PutLocalIdTest001
 * @tc.desc: Verify the PutLocalId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutLocalIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutLocalIdTest001 Start";
    try {
        CloudDiskFuseData data;
        static shared_ptr<CloudDiskInode> inoPtr = nullptr;
        uint64_t num = 0;
        string key = "test";

        fuseoperationshelper_->PutLocalId(&data, inoPtr, num, key);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutLocalIdTest001  ERROR";
    }
    GTEST_LOG_(INFO) << "PutLocalIdTest001 End";
}

/**
 * @tc.name:PutLocalIdTest002
 * @tc.desc: Verify the PutLocalId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutLocalIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutLocalIdTest002 Start";
    try {
        CloudDiskFuseData data;
        data.localIdCache["test"] = 1;
        static shared_ptr<CloudDiskInode> inoPtr;
        inoPtr = make_shared<CloudDiskInode>();
        inoPtr->refCount = 1;
        uint64_t num = 1;
        string key = "test";
        
        fuseoperationshelper_->PutLocalId(&data, inoPtr, num, key);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutLocalIdTest002  ERROR";
    }
    GTEST_LOG_(INFO) << "PutLocalIdTest002 End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test