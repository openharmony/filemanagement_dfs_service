 /*
 * Copyright (C) 2025 Huawei Device Co., Ltd. All rights reserved.
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
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <memory>

#include "dfs_error.h"
#include "fuse_manager/fuse_manager.h"
#include "fuse_manager.cpp"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudFile::Test {
using namespace testing;
using namespace testing::ext;
constexpr int32_t USER_ID = 100;
class FuseManagerStaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<FuseManager> fuseManager_;
};

void FuseManagerStaticTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void FuseManagerStaticTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void FuseManagerStaticTest::SetUp(void)
{
    GTEST_LOG_(INFO) << "SetUp";
    fuseManager_ = std::make_shared<FuseManager>();
}

void FuseManagerStaticTest::TearDown(void)
{
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: IsValidCachePathTest001
 * @tc.desc: Verify the IsValidCachePath function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, IsValidCachePathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidCachePathTest001 Begin";
    try {
        string path = "/./";
        auto ret = IsValidCachePath(path);

        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidCachePathTest001 Error";
    }
    GTEST_LOG_(INFO) << "IsValidCachePathTest001 End";
}

/**
 * @tc.name: IsValidCachePathTest002
 * @tc.desc: Verify the IsValidCachePath function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, IsValidCachePathTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidCachePathTest002 Begin";
    try {
        string path = "/mnt/data/100/cloud_fuse/../";
        auto ret = IsValidCachePath(path);

        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidCachePathTest002 Error";
    }
    GTEST_LOG_(INFO) << "IsValidCachePathTest002 End";
}

/**
 * @tc.name: IsValidCachePathTest003
 * @tc.desc: Verify the IsValidCachePath function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, IsValidCachePathTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidCachePathTest003 Begin";
    try {
        string path = "/mnt/data/100/cloud_fuse/..";
        auto ret = IsValidCachePath(path);

        EXPECT_EQ(ret, false);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidCachePathTest003 Error";
    }
    GTEST_LOG_(INFO) << "IsValidCachePathTest003 End";
}

/**
 * @tc.name: IsValidCachePathTest004
 * @tc.desc: Verify the IsValidCachePath function
 * @tc.type: FUNC
 * @tc.require: I6H5MH
 */
HWTEST_F(FuseManagerStaticTest, IsValidCachePathTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsValidCachePathTest004 Begin";
    try {
        string path = "/mnt/data/100/cloud_fuse/";
        auto ret = IsValidCachePath(path);

        EXPECT_EQ(ret, true);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "IsValidCachePathTest004 Error";
    }
    GTEST_LOG_(INFO) << "IsValidCachePathTest004 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test