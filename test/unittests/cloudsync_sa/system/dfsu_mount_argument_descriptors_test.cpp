/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include <sstream>
#include <sys/mount.h>

#include "dfsu_mount_argument_descriptors.h"

namespace OHOS::Storage::DistributedFile::Utils::Test {
using namespace testing;
using namespace testing::ext;
using namespace std;
class MountArgumentTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    shared_ptr<MountArgument> mountArgument_ = nullptr;
};

void MountArgumentTest::SetUpTestCase(void)
{
    GTEST_LOG_(INFO) << "SetUpTestCase";
}

void MountArgumentTest::TearDownTestCase(void)
{
    GTEST_LOG_(INFO) << "TearDownTestCase";
}

void MountArgumentTest::SetUp(void)
{
    mountArgument_ = make_shared<MountArgument>();
    GTEST_LOG_(INFO) << "SetUp";
}

void MountArgumentTest::TearDown(void)
{
    mountArgument_ = nullptr;
    GTEST_LOG_(INFO) << "TearDown";
}

/**
 * @tc.name: GetFullSrcTest
 * @tc.desc: Verify the GetFullSrc function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, GetFullSrcTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFullSrcTest Begin";
    try {
        std::string out = "/data/service/el2/0/hmdfs/";
        std::string ret = mountArgument_->GetFullSrc();
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFullSrcTest ERROR";
    }
    GTEST_LOG_(INFO) << "GetFullSrcTest End";
}

/**
 * @tc.name: GetFullDstTest
 * @tc.desc: Verify the GetFullDst function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, GetFullDstTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFullDstTest Begin";
    try {
        std::string out = "/mnt/hmdfs/0/";
        std::string ret = mountArgument_->GetFullDst();
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFullDstTest ERROR";
    }
    GTEST_LOG_(INFO) << "GetFullDstTest End";
}

/**
 * @tc.name: GetCachePathTest
 * @tc.desc: Verify the GetCachePath function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, GetCachePathTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCachePathTest Begin";
    try {
        std::string out = "/data/service/el2/0/hmdfs//cache/";
        std::string ret = mountArgument_->GetCachePath();
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCachePathTest ERROR";
    }
    GTEST_LOG_(INFO) << "GetCachePathTest End";
}

/**
 * @tc.name: GetCtrlPathTest
 * @tc.desc: Verify the GetCtrlPath function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, GetCtrlPathTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetCtrlPathTest Begin";
    try {
        std::string out = "/sys/fs/hmdfs/2989455826978503469/cmd";
        std::string ret = mountArgument_->GetCtrlPath();
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetCtrlPathTest ERROR";
    }
    GTEST_LOG_(INFO) << "GetCtrlPathTest End";
}

/**
 * @tc.name: OptionsToStringTest001
 * @tc.desc: Verify the OptionsToString function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, OptionsToStringTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OptionsToStringTest001 Begin";
    try {
        std::string out = "local_dst=/mnt/hmdfs/0/";
        std::string ret = mountArgument_->OptionsToString();
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OptionsToStringTest001 ERROR";
    }
    GTEST_LOG_(INFO) << "OptionsToStringTest001 End";
}

/**
 * @tc.name: OptionsToStringTest002
 * @tc.desc: Verify the OptionsToString function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, OptionsToStringTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OptionsToStringTest002 Begin";
    try {
        std::string out = "local_dst=/mnt/hmdfs/0/,cache_dir=/data/service/el2/0/hmdfs//cache/";
        mountArgument_->useCache_ = true;
        std::string ret = mountArgument_->OptionsToString();
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OptionsToStringTest002 ERROR";
    }
    GTEST_LOG_(INFO) << "OptionsToStringTest002 End";
}

/**
 * @tc.name: OptionsToStringTest003
 * @tc.desc: Verify the OptionsToString function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, OptionsToStringTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OptionsToStringTest003 Begin";
    try {
        std::string out = "local_dst=/mnt/hmdfs/0/,sensitive";
        mountArgument_->caseSensitive_ = true;
        std::string ret = mountArgument_->OptionsToString();
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OptionsToStringTest003 ERROR";
    }
    GTEST_LOG_(INFO) << "OptionsToStringTest003 End";
}

/**
 * @tc.name: OptionsToStringTest004
 * @tc.desc: Verify the OptionsToString function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, OptionsToStringTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OptionsToStringTest004 Begin";
    try {
        std::string out = "local_dst=/mnt/hmdfs/0/,merge";
        mountArgument_->enableMergeView_ = true;
        std::string ret = mountArgument_->OptionsToString();
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OptionsToStringTest004 ERROR";
    }
    GTEST_LOG_(INFO) << "OptionsToStringTest004 End";
}

/**
 * @tc.name: OptionsToStringTest005
 * @tc.desc: Verify the OptionsToString function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, OptionsToStringTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OptionsToStringTest005 Begin";
    try {
        std::string out = "local_dst=/mnt/hmdfs/0/,fixupownership";
        mountArgument_->enableFixupOwnerShip_ = true;
        std::string ret = mountArgument_->OptionsToString();
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OptionsToStringTest005 ERROR";
    }
    GTEST_LOG_(INFO) << "OptionsToStringTest005 End";
}

/**
 * @tc.name: OptionsToStringTest006
 * @tc.desc: Verify the OptionsToString function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, OptionsToStringTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OptionsToStringTest006 Begin";
    try {
        std::string out = "local_dst=/mnt/hmdfs/0/,no_offline_stash";
        mountArgument_->enableOfflineStash_ = false;
        std::string ret = mountArgument_->OptionsToString();
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OptionsToStringTest006 ERROR";
    }
    GTEST_LOG_(INFO) << "OptionsToStringTest006 End";
}

/**
 * @tc.name: OptionsToStringTest007
 * @tc.desc: Verify the OptionsToString function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, OptionsToStringTest007, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OptionsToStringTest006 Begin";
    try {
        std::string out = "local_dst=/mnt/hmdfs/0/,external_fs";
        mountArgument_->externalFS_ = true;
        std::string ret = mountArgument_->OptionsToString();
        EXPECT_STREQ(ret.c_str(), out.c_str());
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "OptionsToStringTest007 ERROR";
    }
    GTEST_LOG_(INFO) << "OptionsToStringTest007 End";
}

/**
 * @tc.name: GetFlagsTest
 * @tc.desc: Verify the GetFlags function
 * @tc.type: FUNC
 * @tc.require: I6JPKG
 */
HWTEST_F(MountArgumentTest, GetFlagsTest, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetFlagsTest Begin";
    try {
        auto ret = mountArgument_->GetFlags();
        EXPECT_EQ(ret, MS_NODEV);
    } catch (...) {
        EXPECT_TRUE(false);
        GTEST_LOG_(INFO) << "GetFlagsTest ERROR";
    }
    GTEST_LOG_(INFO) << "GetFlagsTest End";
}
} // namespace OHOS::Storage::DistributedFile::Utils::Test
