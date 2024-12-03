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
 * @tc.name: AddDirEntryTest001
 * @tc.desc: Verify the AddDirEntry function
 * @tc.type: FUNC
 * @tc.require: issuesIB4SSZ
 */
HWTEST_F(FuseOperationsHelperTest, AddDirEntryPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddDirEntryTest001 start";
    try {
        fuse_req_t req = nullptr;
        std::string buffer = "";
        size_t size = 0;
        const char *name = "testAddDir";
        auto ino = std::make_shared<CloudDiskInode>();
        fuseoperationshelper_->AddDirEntry(req, buffer, size, name, ino);
        EXPECT_GT(size, 0);
        EXPECT_EQ(buffer.size(), size);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "AddDirEntryTest001 failed";
    }
    GTEST_LOG_(INFO) << "AddDirEntryTest001 end";
}

/**
 * @tc.name: GetCloudDiskLocalPathTest001
 * @tc.desc: Verify the GetCloudDiskLocalPath function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetCloudDiskLocalPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudDiskLocalPathTest001 Start";
    try {
        int32_t userId = 100;
        string fileName = "data";

        string ret = fuseoperationshelper_->GetCloudDiskLocalPath(userId, fileName);
        EXPECT_EQ(ret, "/data/service/el2/100/hmdfs/cloud/data/");
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCloudDiskLocalPathTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCloudDiskLocalPathTest001 End";
}

/**
 * @tc.name: GetCloudDiskLocalPathTest002
 * @tc.desc: Verify the GetCloudDiskLocalPath function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetCloudDiskLocalPathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudDiskLocalPathTest002 Start";
    try {
        int32_t userId = 100;
        string fileName = "/";

        string ret = fuseoperationshelper_->GetCloudDiskLocalPath(userId, fileName);
        EXPECT_EQ(ret, "/data/service/el2/100/hmdfs/cloud/");
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCloudDiskLocalPathTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCloudDiskLocalPathTest002 End";
}

/**
 * @tc.name: GetCloudDiskLocalPathTest003
 * @tc.desc: Verify the GetCloudDiskLocalPath function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetCloudDiskLocalPathTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCloudDiskLocalPathTest003 Start";
    try {
        int32_t userId = 100;
        string fileName = "test";

        string ret = fuseoperationshelper_->GetCloudDiskLocalPath(userId, fileName);
        EXPECT_EQ(ret, "/data/service/el2/100/hmdfs/cloud/data/test");
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCloudDiskLocalPathTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetCloudDiskLocalPathTest003 End";
}

/**
 * @tc.name: GetInodeAttrTest001
 * @tc.desc: Verify the GetInodeAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetInodeAttrTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInodeAttrTest001 Start";
    try {
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        struct stat statBuf;
        inoPtr->stat.st_mode |= S_IFREG;

        fuseoperationshelper_->GetInodeAttr(inoPtr, &statBuf);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInodeAttrTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetInodeAttrTest001 End";
}

/**
 * @tc.name: GetInodeAttrTest002
 * @tc.desc: Verify the GetInodeAttr function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetInodeAttrTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInodeAttrTest002 Start";
    try {
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        struct stat statBuf;
        inoPtr->stat.st_mode |= S_IFMT;

        fuseoperationshelper_->GetInodeAttr(inoPtr, &statBuf);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInodeAttrTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetInodeAttrTest002 End";
}

/**
 * @tc.name: GetNextLayerTest001
 * @tc.desc: Verify the GetNextLayer function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetNextLayerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNextLayerTest001 Start";
    try {
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        fuse_ino_t ino = FUSE_ROOT_ID;

        int ret = fuseoperationshelper_->GetNextLayer(inoPtr, ino);
        EXPECT_EQ(ret, CLOUD_DISK_INODE_ZERO_LAYER);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNextLayerTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetNextLayerTest001 End";
}

/**
 * @tc.name: GetNextLayerTest002
 * @tc.desc: Verify the GetNextLayer function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetNextLayerTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNextLayerTest002 Start";
    try {
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->layer = CLOUD_DISK_INODE_OTHER_LAYER;
        fuse_ino_t ino = 0;

        int ret = fuseoperationshelper_->GetNextLayer(inoPtr, ino);
        EXPECT_EQ(ret, CLOUD_DISK_INODE_OTHER_LAYER);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNextLayerTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetNextLayerTest002 End";
}

/**
 * @tc.name: GetNextLayerTest003
 * @tc.desc: Verify the GetNextLayer function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetNextLayerTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetNextLayerTest003 Start";
    try {
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->layer = CLOUD_DISK_INODE_FIRST_LAYER;
        fuse_ino_t ino = 0;

        int ret = fuseoperationshelper_->GetNextLayer(inoPtr, ino);
        EXPECT_EQ(ret, CLOUD_DISK_INODE_OTHER_LAYER);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetNextLayerTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetNextLayerTest003 End";
}

/**
 * @tc.name: GetFixedLayerRootIdTest001
 * @tc.desc: Verify the GetFixedLayerRootId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetFixedLayerRootIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest001 Start";
    try {
        int32_t layer = CLOUD_DISK_INODE_ZERO_LAYER;

        int ret = fuseoperationshelper_->GetFixedLayerRootId(layer);
        EXPECT_EQ(ret, CLOUD_DISK_INODE_ZERO_LAYER_LOCALID);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest001 End";
}

/**
 * @tc.name: GetFixedLayerRootIdTest002
 * @tc.desc: Verify the GetFixedLayerRootId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetFixedLayerRootIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest002 Start";
    try {
        int32_t layer = CLOUD_DISK_INODE_FIRST_LAYER;

        int ret = fuseoperationshelper_->GetFixedLayerRootId(layer);
        EXPECT_EQ(ret, CLOUD_DISK_INODE_FIRST_LAYER_LOCALID);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest002 End";
}

/**
 * @tc.name: GetFixedLayerRootIdTest003
 * @tc.desc: Verify the GetFixedLayerRootId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GetFixedLayerRootIdTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest003 Start";
    try {
        int32_t layer = CLOUD_DISK_INODE_OTHER_LAYER;

        int ret = fuseoperationshelper_->GetFixedLayerRootId(layer);
        EXPECT_EQ(ret, CLOUD_DISK_INODE_LAYER_LOCALID_UNKNOWN);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "GetFixedLayerRootIdTest003 End";
}

/**
 * @tc.name: FindCloudDiskInodeTest001
 * @tc.desc: Verify the FindCloudDiskInode function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, FindCloudDiskInodeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindCloudDiskInodeTest001 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskInode> inode = std::make_shared<CloudDiskInode>();
        data->inodeCache.insert(std::make_pair(1, inode));
        int64_t key = 1;

        fuseoperationshelper_->FindCloudDiskInode(data, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindCloudDiskInodeTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "FindCloudDiskInodeTest001 End";
}

/**
 * @tc.name: FindCloudDiskInodeTest002
 * @tc.desc: Verify the FindCloudDiskInode function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, FindCloudDiskInodeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindCloudDiskInodeTest002 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        int64_t key = 0;

        fuseoperationshelper_->FindCloudDiskInode(data, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindCloudDiskInodeTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindCloudDiskInodeTest002 End";
}

/**
 * @tc.name: FindCloudDiskFileTest001
 * @tc.desc: Verify the FindCloudDiskFile function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, FindCloudDiskFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindCloudDiskFileTest001 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskFile> file = std::make_shared<CloudDiskFile>();
        data->fileCache.insert(std::make_pair(1, file));
        int64_t key = 1;

        fuseoperationshelper_->FindCloudDiskFile(data, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindCloudDiskFileTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "FindCloudDiskFileTest001 End";
}

/**
 * @tc.name: FindCloudDiskFileTest002
 * @tc.desc: Verify the FindCloudDiskFile function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, FindCloudDiskFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindCloudDiskFileTest002 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        int64_t key = 0;

        fuseoperationshelper_->FindCloudDiskFile(data, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindCloudDiskFileTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindCloudDiskFileTest002 End";
}

/**
 * @tc.name: FindLocalIdTest001
 * @tc.desc: Verify the FindLocalId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, FindLocalIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindLocalIdTest001 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        data->localIdCache.insert(std::make_pair("test", 1));
        string key = "test";

        int ret = fuseoperationshelper_->FindLocalId(data, key);
        delete data;
        EXPECT_EQ(ret, 1);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindLocalIdTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "FindLocalIdTest001 End";
}

/**
 * @tc.name: FindLocalIdTest002
 * @tc.desc: Verify the FindLocalId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, FindLocalIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FindLocalIdTest002 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        string key = "";

        int ret = fuseoperationshelper_->FindLocalId(data, key);
        delete data;
        EXPECT_EQ(ret, -1);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FindLocalIdTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FindLocalIdTest002 End";
}

/**
 * @tc.name: FuseReplyLimitedTest001
 * @tc.desc: Verify the FuseReplyLimited function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, FuseReplyLimitedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FuseReplyLimitedTest001 Start";
    try {
        fuse_req_t req = nullptr;
        const char *buf;
        size_t bufSize = 1024;
        off_t off = 0;
        size_t maxSize = 1024 * 4 * 4;
        EXPECT_CALL(*insMock, fuse_reply_buf(_, _, _)).WillOnce(Return(0));

        fuseoperationshelper_->FuseReplyLimited(req, buf, bufSize, off, maxSize);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FuseReplyLimitedTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "FuseReplyLimitedTest001 End";
}

/**
 * @tc.name: FuseReplyLimitedTest002
 * @tc.desc: Verify the FuseReplyLimited function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, FuseReplyLimitedTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "FuseReplyLimitedTest002 Start";
    try {
        fuse_req_t req = nullptr;
        const char *buf;
        size_t bufSize = 1024;
        off_t off = 1024 * 4;
        size_t maxSize = 1024 * 4 * 4;
        EXPECT_CALL(*insMock, fuse_reply_buf(_, _, _)).WillOnce(Return(0));

        fuseoperationshelper_->FuseReplyLimited(req, buf, bufSize, off, maxSize);
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "FuseReplyLimitedTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "FuseReplyLimitedTest002 End";
}

/**
 * @tc.name: GenerateCloudDiskInodeTest001
 * @tc.desc: Verify the FuseReplyLimited function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GenerateCloudDiskInodeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest001 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        fuse_ino_t parent = 0;
        string fileName = "";
        string path = "";

        fuseoperationshelper_->GenerateCloudDiskInode(data, parent, fileName, path);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest001 End";
}

/**
 * @tc.name: GenerateCloudDiskInodeTest002
 * @tc.desc: Verify the FuseReplyLimited function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GenerateCloudDiskInodeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest002 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        fuse_ino_t parent = 0;
        string fileName = "";
        string path = "/data";

        fuseoperationshelper_->GenerateCloudDiskInode(data, parent, fileName, path);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest002 End";
}

/**
 * @tc.name: GenerateCloudDiskInodeTest003
 * @tc.desc: Verify the FuseReplyLimited function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GenerateCloudDiskInodeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest003 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskInode> inode = std::make_shared<CloudDiskInode>();
        data->inodeCache.insert(std::make_pair(1, inode));
        fuse_ino_t parent = 1;
        string fileName = "";
        string path = "/data";

        fuseoperationshelper_->GenerateCloudDiskInode(data, parent, fileName, path);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest003 End";
}

/**
 * @tc.name: GenerateCloudDiskInodeTest004
 * @tc.desc: Verify the FuseReplyLimited function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, GenerateCloudDiskInodeTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest004 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskInode> inode = std::make_shared<CloudDiskInode>();
        data->inodeCache.insert(std::make_pair(0, inode));
        fuse_ino_t parent = 0;
        string fileName = "";
        string path = "/data";

        fuseoperationshelper_->GenerateCloudDiskInode(data, parent, fileName, path);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "GenerateCloudDiskInodeTest004 End";
}

/**
 * @tc.name: PutCloudDiskInodeTest001
 * @tc.desc: Verify the PutCloudDiskInode function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutCloudDiskInodeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCloudDiskInodeTest001 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskInode> inoPtr = nullptr;
        uint64_t num = 1;
        int64_t key = 0;

        fuseoperationshelper_->PutCloudDiskInode(data, inoPtr, num, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutCloudDiskInodeTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "PutCloudDiskInodeTest001 End";
}

/**
 * @tc.name: PutCloudDiskInodeTest002
 * @tc.desc: Verify the PutCloudDiskInode function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutCloudDiskInodeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCloudDiskInodeTest002 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->refCount.store(1);
        uint64_t num = 1;
        int64_t key = 0;

        fuseoperationshelper_->PutCloudDiskInode(data, inoPtr, num, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutCloudDiskInodeTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "PutCloudDiskInodeTest002 End";
}

/**
 * @tc.name: PutCloudDiskInodeTest003
 * @tc.desc: Verify the PutCloudDiskInode function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutCloudDiskInodeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCloudDiskInodeTest003 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->refCount.store(2);
        uint64_t num = 1;
        int64_t key = 0;

        fuseoperationshelper_->PutCloudDiskInode(data, inoPtr, num, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutCloudDiskInodeTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "PutCloudDiskInodeTest003 End";
}

/**
 * @tc.name: PutCloudDiskFileTest001
 * @tc.desc: Verify the PutCloudDiskFile function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutCloudDiskFileTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCloudDiskFileTest001 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskFile> file = nullptr;
        int64_t key = 0;

        fuseoperationshelper_->PutCloudDiskFile(data, file, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutCloudDiskFileTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "PutCloudDiskFileTest001 End";
}

/**
 * @tc.name: PutCloudDiskFileTest002
 * @tc.desc: Verify the PutCloudDiskFile function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutCloudDiskFileTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCloudDiskFileTest002 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskFile> file = make_shared<CloudDiskFile>();
        file->refCount.store(0);
        int64_t key = 0;

        fuseoperationshelper_->PutCloudDiskFile(data, file, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutCloudDiskFileTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "PutCloudDiskFileTest002 End";
}

/**
 * @tc.name: PutCloudDiskFileTest003
 * @tc.desc: Verify the PutCloudDiskFile function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutCloudDiskFileTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutCloudDiskFileTest003 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskFile> file = make_shared<CloudDiskFile>();
        file->refCount.store(1);
        int64_t key = 0;

        fuseoperationshelper_->PutCloudDiskFile(data, file, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutCloudDiskFileTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "PutCloudDiskFileTest003 End";
}

/**
 * @tc.name: PutLocalIdTest001
 * @tc.desc: Verify the PutLocalId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutLocalIdTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutLocalIdTest001 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskInode> inoPtr = nullptr;
        uint64_t num = 1;
        string key = "";

        fuseoperationshelper_->PutLocalId(data, inoPtr, num, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutLocalIdTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "PutLocalIdTest001 End";
}

/**
 * @tc.name: PutLocalIdTest002
 * @tc.desc: Verify the PutLocalId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutLocalIdTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutLocalIdTest002 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->refCount.store(1);
        uint64_t num = 1;
        string key = "";

        fuseoperationshelper_->PutLocalId(data, inoPtr, num, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutLocalIdTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "PutLocalIdTest002 End";
}

/**
 * @tc.name: PutLocalIdTest003
 * @tc.desc: Verify the PutLocalId function
 * @tc.type: FUNC
 * @tc.require: issuesI92WQP
 */
HWTEST_F(FuseOperationsHelperTest, PutLocalIdTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "PutLocalIdTest003 Start";
    try {
        struct CloudDiskFuseData *data = new CloudDiskFuseData;
        std::shared_ptr<CloudDiskInode> inoPtr = make_shared<CloudDiskInode>();
        inoPtr->refCount.store(2);
        uint64_t num = 1;
        string key = "";

        fuseoperationshelper_->PutLocalId(data, inoPtr, num, key);
        delete data;
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "PutLocalIdTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "PutLocalIdTest003 End";
}
} // namespace OHOS::FileManagement::CloudDisk::Test