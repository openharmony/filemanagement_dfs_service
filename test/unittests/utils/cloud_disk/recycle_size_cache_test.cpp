/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "recycle_size_cache.h"

#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <sys/xattr.h>
#include <unistd.h>

#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace testing::ext;
using namespace std;

namespace {
    const string POS_XATTR = "user.cloud.location";
    const int32_t POS_LOCAL = 1;
    const int32_t POS_CLOUD = 2;
    const int32_t POS_LOCAL_AND_CLOUD = 3;
    const int32_t TEST_USER_ID = 100;
    const string TEST_BUNDLE = "com.test.app";
    const string TEST_DATA_ROOT = "./rt_data/";
    const string TEST_TRASH_ROOT = "./rt_trash/";
}

class RecycleSizeCacheTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        std::filesystem::remove_all(TEST_DATA_ROOT);
        std::filesystem::remove_all(TEST_TRASH_ROOT);
    }
    void TearDown()
    {
        std::filesystem::remove_all(TEST_DATA_ROOT);
        std::filesystem::remove_all(TEST_TRASH_ROOT);
    }
};

static bool WriteFile(const string &path, const string &content)
{
    int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        return false;
    }
    ssize_t w = write(fd, content.data(), content.size());
    close(fd);
    return w == static_cast<ssize_t>(content.size());
}

static bool SetPosition(const string &path, int32_t position)
{
    string val = to_string(position);
    return setxattr(path.c_str(), POS_XATTR.c_str(), val.data(), val.size(), 0) == 0;
}

static string TrashDir()
{
    return TEST_TRASH_ROOT + to_string(TEST_USER_ID) + "/cloud/data/" + TEST_BUNDLE + "/.trash";
}

/**
 * @tc.name: GetRecycleBinSize_CacheAbsent_CreatesWithZero_001
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, GetRecycleBinSize_CacheAbsent_CreatesWithZero_001, TestSize.Level1)
{
    int64_t size = -1;
    int32_t ret = RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: IncreaseRecycleBinSize_AddsToCache_002
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, IncreaseRecycleBinSize_AddsToCache_002, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, 100), E_OK);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 100);

    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, 250), E_OK);
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 350);
}

/**
 * @tc.name: DecreaseRecycleBinSize_SubtractsAndClamps_003
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, DecreaseRecycleBinSize_SubtractsAndClamps_003, TestSize.Level1)
{
    ASSERT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, 300), E_OK);
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, 120), E_OK);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 180);

    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, 9999), E_OK);
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: InvalidArgs_ReturnsError_004
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, InvalidArgs_ReturnsError_004, TestSize.Level1)
{
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, "", size), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(-1, TEST_BUNDLE, size), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, "", 10), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, -5), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, -5), E_INVAL_ARG);
}

/**
 * @tc.name: VerifyRecycleBinSize_NoTrashDir_SetsZero_005
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, VerifyRecycleBinSize_NoTrashDir_SetsZero_005, TestSize.Level1)
{
    ASSERT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, 500), E_OK);
    EXPECT_EQ(RecycleSizeCache::VerifyRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: VerifyRecycleBinSize_SumsOnlyParticipatingFiles_006
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, VerifyRecycleBinSize_SumsOnlyParticipatingFiles_006, TestSize.Level1)
{
    string trash = TrashDir();
    std::filesystem::create_directories(trash);

    string fileA = trash + "/a";
    ASSERT_TRUE(WriteFile(fileA, string(100, 'x')));
    ASSERT_TRUE(SetPosition(fileA, POS_LOCAL));

    string fileB = trash + "/b";
    ASSERT_TRUE(WriteFile(fileB, string(50, 'x')));
    ASSERT_TRUE(SetPosition(fileB, POS_CLOUD));

    string fileC = trash + "/c";
    ASSERT_TRUE(WriteFile(fileC, string(200, 'x')));
    ASSERT_TRUE(SetPosition(fileC, POS_LOCAL_AND_CLOUD));

    string subDir = trash + "/subdir";
    ASSERT_TRUE(std::filesystem::create_directory(subDir));
    ASSERT_TRUE(SetPosition(subDir, POS_LOCAL));

    string fileD = trash + "/d";
    ASSERT_TRUE(WriteFile(fileD, string(80, 'x')));

    EXPECT_EQ(RecycleSizeCache::VerifyRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 300);
}

/**
 * @tc.name: EndToEnd_RecyclePurgeRestore_007
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, EndToEnd_RecyclePurgeRestore_007, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, 1000), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, 2000), E_OK);
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, 500), E_OK);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 2500);

    string trash = TrashDir();
    std::filesystem::create_directories(trash);
    string file = trash + "/only";
    ASSERT_TRUE(WriteFile(file, string(400, 'x')));
    ASSERT_TRUE(SetPosition(file, POS_LOCAL_AND_CLOUD));
    EXPECT_EQ(RecycleSizeCache::VerifyRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 400);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
