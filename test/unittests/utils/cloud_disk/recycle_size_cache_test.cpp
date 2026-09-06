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

#include <chrono>
#include <cstdlib>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <unistd.h>

#include "dfs_error.h"
#include "meta_file.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace testing::ext;
using namespace std;

namespace {
    const int32_t TEST_USER_ID = 100;
    const int32_t TEST_USER_ID_2 = 101;
    const string TEST_BUNDLE = "com.test.filemanager";
    const string TEST_BUNDLE_2 = "com.test.photo";

    static string GetBaseDir(int32_t userId, const string &bundle)
    {
        return "/data/service/el2/" + to_string(userId) + "/hmdfs/cloudfile_manager/" + bundle;
    }

    static string GetCachePath(int32_t userId, const string &bundle)
    {
        return GetBaseDir(userId, bundle) + "/RecycleSizeCache";
    }

    static void PrepareDir(int32_t userId, const string &bundle)
    {
        std::system(("mkdir -p " + GetBaseDir(userId, bundle)).c_str());
    }

    static void CleanRecycleFile(int32_t userId, const string &bundle)
    {
        std::system(("rm -f " + GetCachePath(userId, bundle)).c_str());
    }

    static void RemoveBaseDir(int32_t userId, const string &bundle)
    {
        std::system(("rm -rf " + GetBaseDir(userId, bundle)).c_str());
    }

    static bool CacheFileExists(int32_t userId, const string &bundle)
    {
        return access(GetCachePath(userId, bundle).c_str(), F_OK) == 0;
    }

    static MetaBase MakeMetaBase(uint64_t size, const string &name = "test")
    {
        MetaBase mb(name);
        mb.size = size;
        return mb;
    }
}

// 写操作是异步的，轮询等待 Get 收敛到期望值（带回溯超时）。
static constexpr int64_t POLL_INTERVAL_MS = 10;
static constexpr int64_t POLL_TIMEOUT_MS = 3000;
static int64_t WaitSizeFor(int32_t userId, const string &bundle, int64_t expect,
    int timeoutMs = POLL_TIMEOUT_MS)
{
    int64_t size = -1;
    auto deadline = chrono::steady_clock::now() + chrono::milliseconds(timeoutMs);
    do {
        if (RecycleSizeCache::GetRecycleBinSize(userId, bundle, size) == E_OK && size == expect) {
            return size;
        }
        this_thread::sleep_for(chrono::milliseconds(POLL_INTERVAL_MS));
    } while (chrono::steady_clock::now() < deadline);
    return size;
}

static int64_t WaitSize(int64_t expect, int timeoutMs = POLL_TIMEOUT_MS)
{
    return WaitSizeFor(TEST_USER_ID, TEST_BUNDLE, expect, timeoutMs);
}

class RecycleSizeCacheTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
        // 依次创建各用户/应用的缓存目录，并清理残留缓存文件。
        PrepareDir(TEST_USER_ID, TEST_BUNDLE);
        PrepareDir(TEST_USER_ID, TEST_BUNDLE_2);
        PrepareDir(TEST_USER_ID_2, TEST_BUNDLE);
        PrepareDir(TEST_USER_ID_2, TEST_BUNDLE_2);
        CleanRecycleFile(TEST_USER_ID, TEST_BUNDLE);
        CleanRecycleFile(TEST_USER_ID, TEST_BUNDLE_2);
        CleanRecycleFile(TEST_USER_ID_2, TEST_BUNDLE);
        CleanRecycleFile(TEST_USER_ID_2, TEST_BUNDLE_2);
    }
    void TearDown()
    {
        // 删除各缓存文件与缓存目录。
        CleanRecycleFile(TEST_USER_ID, TEST_BUNDLE);
        CleanRecycleFile(TEST_USER_ID, TEST_BUNDLE_2);
        CleanRecycleFile(TEST_USER_ID_2, TEST_BUNDLE);
        CleanRecycleFile(TEST_USER_ID_2, TEST_BUNDLE_2);
        RemoveBaseDir(TEST_USER_ID, TEST_BUNDLE);
        RemoveBaseDir(TEST_USER_ID, TEST_BUNDLE_2);
        RemoveBaseDir(TEST_USER_ID_2, TEST_BUNDLE);
        RemoveBaseDir(TEST_USER_ID_2, TEST_BUNDLE_2);
    }
};

/**
 * @tc.name: GetRecycleBinSize_CacheAbsent_ReturnsZero_001
 * @tc.desc: 目录存在且无缓存文件时，读侧按 0 返回。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, GetRecycleBinSize_CacheAbsent_ReturnsZero_001, TestSize.Level1)
{
    int64_t size = -1;
    int32_t ret = RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size);
    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(size, 0);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
}

/**
 * @tc.name: IncreaseRecycleBinSize_AddsToCache_002
 * @tc.desc: 增量异步落盘，最终收敛为各增量之和，并创建缓存文件。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, IncreaseRecycleBinSize_AddsToCache_002, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(250)), E_OK);
    EXPECT_EQ(WaitSize(350), 350);
    EXPECT_TRUE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
}

/**
 * @tc.name: DecreaseRecycleBinSize_SubtractsAndClamps_003
 * @tc.desc: 减量落盘并钳制为非负。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, DecreaseRecycleBinSize_SubtractsAndClamps_003, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(300)), E_OK);
    EXPECT_EQ(WaitSize(300), 300);

    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(120)), E_OK);
    EXPECT_EQ(WaitSize(180), 180);

    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(9999)), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
}

/**
 * @tc.name: InvalidArgs_ReturnsError_004
 * @tc.desc: 非法入参统一返回 E_INVAL_ARG，含路径穿越类 bundleName。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, InvalidArgs_ReturnsError_004, TestSize.Level1)
{
    int64_t size = -1;
    MetaBase mb = MakeMetaBase(100);
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, "", size), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(-1, TEST_BUNDLE, size), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, "", mb), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(-1, TEST_BUNDLE, mb), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, ""), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(-1, TEST_BUNDLE), E_INVAL_ARG);

    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, "com.test/../evil", size), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, "com.test..steal", mb), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, "../escape", mb), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, "com.test/../evil"), E_INVAL_ARG);
}

/**
 * @tc.name: ResetRecycleBinSize_ResetsCacheToZero_005
 * @tc.desc: Reset 版本换代并清零。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetRecycleBinSize_ResetsCacheToZero_005, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(500)), E_OK);
    // 等异步增加落盘。
    EXPECT_EQ(WaitSize(500), 500);
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
}

/**
 * @tc.name: MissingBaseDir_ReturnsError_006
 * @tc.desc: 删除缓存目录后：读按 0 返回，写路径返回 E_PATH，增量被丢弃且不重建文件。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, MissingBaseDir_ReturnsError_006, TestSize.Level1)
{
    RemoveBaseDir(TEST_USER_ID, TEST_BUNDLE);

    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 0);

    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_PATH);

    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    this_thread::sleep_for(chrono::milliseconds(200));

    int64_t after = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, after), E_OK);
    EXPECT_EQ(after, 0);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
}

/**
 * @tc.name: EndToEnd_RecycleAndReset_007
 * @tc.desc: 全流程：增量、减量、清零。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, EndToEnd_RecycleAndReset_007, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(1000)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(2000)), E_OK);
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(500)), E_OK);
    EXPECT_EQ(WaitSize(2500), 2500);

    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
}

/**
 * @tc.name: MultiUserMultiBundle_Isolation_008
 * @tc.desc: 不同用户/应用的缓存互相隔离。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, MultiUserMultiBundle_Isolation_008, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE_2, MakeMetaBase(60)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE, MakeMetaBase(200)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2, MakeMetaBase(300)), E_OK);

    EXPECT_EQ(WaitSizeFor(TEST_USER_ID, TEST_BUNDLE, 100), 100);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID, TEST_BUNDLE_2, 60), 60);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID_2, TEST_BUNDLE, 200), 200);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID_2, TEST_BUNDLE_2, 300), 300);
}

/**
 * @tc.name: ResetRecycleBinSize_CreatesCacheFileWithZero_009
 * @tc.desc: Reset 在目录存在且无缓存文件时创建文件并写 0。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetRecycleBinSize_CreatesCacheFileWithZero_009, TestSize.Level1)
{
    CleanRecycleFile(TEST_USER_ID, TEST_BUNDLE);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));

    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_TRUE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));

    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: IncreaseRecycleBinSize_CloudOnlyIgnored_010
 * @tc.desc: 纯云端数据不参与回收站大小统计。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, IncreaseRecycleBinSize_CloudOnlyIgnored_010, TestSize.Level1)
{
    MetaBase cloud = MakeMetaBase(500);
    cloud.position = POSITION_CLOUD;
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, cloud), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
}

/**
 * @tc.name: DecreaseRecycleBinSize_CloudOnlyIgnored_011
 * @tc.desc: 纯云端数据的减量不生效。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, DecreaseRecycleBinSize_CloudOnlyIgnored_011, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(WaitSize(100), 100);

    MetaBase cloud = MakeMetaBase(60);
    cloud.position = POSITION_CLOUD;
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, cloud), E_OK);
    EXPECT_EQ(WaitSize(100), 100);
}

/**
 * @tc.name: DirectoryMeta_Ignored_012
 * @tc.desc: 目录类 MetaBase 不参与统计。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, DirectoryMeta_Ignored_012, TestSize.Level1)
{
    MetaBase dir = MakeMetaBase(500, "dir");
    dir.mode = S_IFDIR | 0771;
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, dir), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
}

/**
 * @tc.name: DeleteCacheFile_ForcesRereadToZero_013
 * @tc.desc: 删除缓存文件后重新读取收敛为 0。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, DeleteCacheFile_ForcesRereadToZero_013, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(WaitSize(100), 100);
    EXPECT_TRUE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));

    CleanRecycleFile(TEST_USER_ID, TEST_BUNDLE);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));

    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: IncreaseRecycleBinSize_CreatesCacheFileWhenMissing_014
 * @tc.desc: 无缓存文件时首次增量创建文件。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, IncreaseRecycleBinSize_CreatesCacheFileWhenMissing_014, TestSize.Level1)
{
    CleanRecycleFile(TEST_USER_ID, TEST_BUNDLE);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));

    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(50)), E_OK);
    EXPECT_EQ(WaitSize(50), 50);
    EXPECT_TRUE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
}

/**
 * @tc.name: UnknownPosition_Ignored_015
 * @tc.desc: position 未知时不属于本地数据，不参与统计。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, UnknownPosition_Ignored_015, TestSize.Level1)
{
    MetaBase unknown = MakeMetaBase(500);
    unknown.position = POSITION_UNKNOWN;
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, unknown), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
}

/**
 * @tc.name: ZeroSizeIncreaseDecrease_IsNoop_016
 * @tc.desc: 大小为 0 的增/减为空操作，不写文件。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ZeroSizeIncreaseDecrease_IsNoop_016, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(0)), E_OK);
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(0)), E_OK);

    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 0);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS