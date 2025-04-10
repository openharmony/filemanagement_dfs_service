/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include <fcntl.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <memory>

#include "dfs_error.h"
#include "fuse_assistant.h"
#include "fuse_manager/fuse_manager.h"
#include "fuse_i.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
constexpr int32_t USER_ID = 100;
class FuseManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<FuseManager> fuseManager_;
    static inline shared_ptr<FuseAssistantMock> insMock_ = nullptr;
};

void FuseManagerTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FuseManagerTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FuseManagerTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    insMock_ = make_shared<FuseAssistantMock>();
    FuseAssistantMock::ins = insMock_;
    fuseManager_ = std::make_shared<FuseManager>();
}

void FuseManagerTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
    FuseAssistantMock::ins = nullptr;
    insMock_ = nullptr;
}

/**
 * @tc.name: GetInstanceTest
 * @tc.desc: Verify the GetInstance function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerTest, GetInstanceTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetInstanceTest start";
    try {
        FuseManager::GetInstance();
        EXPECT_TRUE(true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetInstanceTest failed";
    }
    GTEST_LOG_(INFO) << "GetInstanceTest end";
}

/**
 * @tc.name: StartFuseTest001
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(FuseManagerTest, StartFuseTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest001 start";
    try {
        std::unique_ptr<struct fuse_session> fs = std::make_unique<struct fuse_session>();
        EXPECT_CALL(*insMock_, fuse_opt_add_arg(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, fuse_session_new(_, _, _, _)).WillOnce(Return(fs.get()));
        EXPECT_CALL(*insMock_, fuse_session_destroy(fs.get())).WillOnce(Return());
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "/mnt/data/100/cloud_fuse";
        int ret = fuseManager_->StartFuse(USER_ID, devFd, path);
        close(devFd);
        EXPECT_EQ(ret, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest001 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest001 end";
}

/**
 * @tc.name: StartFuseTest002
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(FuseManagerTest, StartFuseTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest002 start";
    try {
        EXPECT_CALL(*insMock_, fuse_opt_add_arg(_, _)).WillOnce(Return(-1));
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "/mnt/data/100/cloud_fuse";
        int ret = fuseManager_->StartFuse(USER_ID, devFd, path);
        close(devFd);
        EXPECT_EQ(ret, -EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest002 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest002 end";
}

/**
 * @tc.name: StartFuseTest003
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(FuseManagerTest, StartFuseTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest003 start";
    try {
        EXPECT_CALL(*insMock_, fuse_opt_add_arg(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, fuse_session_new(_, _, _, _)).WillOnce(Return(nullptr));
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "/mnt/data/100/cloud_fuse";
        int ret = fuseManager_->StartFuse(USER_ID, devFd, path);
        close(devFd);
        EXPECT_EQ(ret, -EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest003 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest003 end";
}

/**
 * @tc.name: StartFuseTest004
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(FuseManagerTest, StartFuseTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest004 start";
    try {
        std::unique_ptr<struct fuse_session> fs = std::make_unique<struct fuse_session>();
        EXPECT_CALL(*insMock_, fuse_opt_add_arg(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, fuse_session_new(_, _, _, _)).WillOnce(Return(fs.get()));
        EXPECT_CALL(*insMock_, fuse_session_destroy(fs.get())).WillOnce(Return());
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "/mnt/data/100/cloud";
        int ret = fuseManager_->StartFuse(USER_ID, devFd, path);
        close(devFd);
        EXPECT_EQ(ret, -1);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest004 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest004 end";
}

/**
 * @tc.name: StartFuseTest005
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(FuseManagerTest, StartFuseTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest005 start";
    try {
        EXPECT_CALL(*insMock_, fuse_opt_add_arg(_, _)).WillOnce(Return(-1));
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "/mnt/data/100/cloud";
        int ret = fuseManager_->StartFuse(USER_ID, devFd, path);
        close(devFd);
        EXPECT_EQ(ret, -EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest005 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest005 end";
}

/**
 * @tc.name: StartFuseTest006
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(FuseManagerTest, StartFuseTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest006 start";
    try {
        EXPECT_CALL(*insMock_, fuse_opt_add_arg(_, _)).WillOnce(Return(0));
        EXPECT_CALL(*insMock_, fuse_session_new(_, _, _, _)).WillOnce(Return(nullptr));
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "/mnt/data/100/cloud";
        int ret = fuseManager_->StartFuse(USER_ID, devFd, path);
        close(devFd);
        EXPECT_EQ(ret, -EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest006 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest006 end";
}

/**
 * @tc.name: StartFuseTest007
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(FuseManagerTest, StartFuseTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest007 start";
    try {
        EXPECT_CALL(*insMock_, CheckPathForStartFuse(_)).WillOnce(Return(false));
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "/invalid/data/100/cloud_fuse";
        int ret = fuseManager_->StartFuse(USER_ID, devFd, path);
        close(devFd);
        EXPECT_EQ(ret, -EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest007 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest007 end";
}

/**
 * @tc.name: StartFuseTest008
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(FuseManagerTest, StartFuseTest008, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest008 start";
    try {
        EXPECT_CALL(*insMock_, CheckPathForStartFuse(_)).WillOnce(Return(false));
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "/mnt/other/100/cloud_fuse";
        int ret = fuseManager_->StartFuse(USER_ID, devFd, path);
        close(devFd);
        EXPECT_EQ(ret, -EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest008 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest008 end";
}

/**
 * @tc.name: StartFuseTest009
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(FuseManagerTest, StartFuseTest009, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest009 start";
    try {
        EXPECT_CALL(*insMock_, CheckPathForStartFuse(_)).WillOnce(Return(false));
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "/mnt/data/abcd/cloud_fuse";
        int ret = fuseManager_->StartFuse(USER_ID, devFd, path);
        close(devFd);
        EXPECT_EQ(ret, -EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest009 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest009 end";
}

/**
 * @tc.name: StartFuseTest010
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(FuseManagerTest, StartFuseTest010, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest010 start";
    try {
        EXPECT_CALL(*insMock_, CheckPathForStartFuse(_)).WillOnce(Return(false));
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "/mnt/data/100/invalid_suffix";
        int ret = fuseManager_->StartFuse(USER_ID, devFd, path);
        close(devFd);
        EXPECT_EQ(ret, -EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest010 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest010 end";
}

/**
 * @tc.name: StartFuseTest011
 * @tc.desc: Verify the StartFuse function
 * @tc.type: FUNC
 * @tc.require: issuesIB538J
 */
HWTEST_F(FuseManagerTest, StartFuseTest011, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartFuseTest011 start";
    try {
        EXPECT_CALL(*insMock_, CheckPathForStartFuse(_)).WillOnce(Return(false));
        int32_t devFd = open("/dev/fuse", O_RDWR);
        string path = "\0";
        int ret = fuseManager_->StartFuse(USER_ID, devFd, path);
        close(devFd);
        EXPECT_EQ(ret, -EINVAL);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "StartFuseTest011 failed";
    }
    GTEST_LOG_(INFO) << "StartFuseTest011 end";
}
} // namespace OHOS::FileManagement::CloudSync::Test