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
#include <fcntl.h>
#include <gtest/gtest.h>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <unistd.h>
#include <vector>

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

    static void WriteRawFile(const string &path, const string &content)
    {
        int fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0660);
        if (fd < 0) {
            return;
        }
        write(fd, content.data(), content.size());
        close(fd);
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

static bool WaitCacheFileExists(int32_t userId, const string &bundle,
    int timeoutMs = POLL_TIMEOUT_MS)
{
    auto deadline = chrono::steady_clock::now() + chrono::milliseconds(timeoutMs);
    do {
        if (CacheFileExists(userId, bundle)) {
            return true;
        }
        this_thread::sleep_for(chrono::milliseconds(POLL_INTERVAL_MS));
    } while (chrono::steady_clock::now() < deadline);
    return false;
}

static int64_t WaitSize(int64_t expect, int timeoutMs = POLL_TIMEOUT_MS)
{
    return WaitSizeFor(TEST_USER_ID, TEST_BUNDLE, expect, timeoutMs);
}

static void WaitVersionBumped(int32_t userId, const string &bundle, int64_t verBefore,
    int timeoutMs = POLL_TIMEOUT_MS)
{
    string path = GetCachePath(userId, bundle);
    auto deadline = chrono::steady_clock::now() + chrono::milliseconds(timeoutMs);
    while (RecycleSizeCache::GetCacheVersion(path) == verBefore &&
        chrono::steady_clock::now() < deadline) {
        this_thread::sleep_for(chrono::milliseconds(POLL_INTERVAL_MS));
    }
}

static void WaitAsyncSettled(int32_t userId, const string &bundle,
    int timeoutMs = POLL_TIMEOUT_MS)
{
    this_thread::sleep_for(chrono::milliseconds(POLL_INTERVAL_MS));
    int64_t prev = -1;
    int stableCount = 0;
    auto deadline = chrono::steady_clock::now() + chrono::milliseconds(timeoutMs);
    while (chrono::steady_clock::now() < deadline) {
        int64_t cur = 0;
        if (RecycleSizeCache::GetRecycleBinSize(userId, bundle, cur) == E_OK && cur == prev) {
            ++stableCount;
            if (stableCount >= 3) {
                return;
            }
        } else {
            stableCount = 0;
        }
        prev = cur;
        this_thread::sleep_for(chrono::milliseconds(POLL_INTERVAL_MS));
    }
}

class RecycleSizeCacheTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp()
    {
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
        CleanRecycleFile(TEST_USER_ID, TEST_BUNDLE);
        CleanRecycleFile(TEST_USER_ID, TEST_BUNDLE_2);
        CleanRecycleFile(TEST_USER_ID_2, TEST_BUNDLE);
        CleanRecycleFile(TEST_USER_ID_2, TEST_BUNDLE_2);
        RemoveBaseDir(TEST_USER_ID, TEST_BUNDLE);
        RemoveBaseDir(TEST_USER_ID, TEST_BUNDLE_2);
        RemoveBaseDir(TEST_USER_ID_2, TEST_BUNDLE);
        RemoveBaseDir(TEST_USER_ID_2, TEST_BUNDLE_2);
        for (size_t i = 0; i < RecycleSizeCache::kVersionSlotCount; ++i) {
            RecycleSizeCache::gVersionSlots_[i].store(0);
        }
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

    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    WaitAsyncSettled(TEST_USER_ID, TEST_BUNDLE);

    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    WaitAsyncSettled(TEST_USER_ID, TEST_BUNDLE);

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
    EXPECT_TRUE(WaitCacheFileExists(TEST_USER_ID, TEST_BUNDLE));

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

/**
 * @tc.name: ReadCachedSize_EmptyContent_ReturnsZero_017
 * @tc.desc: 缓存文件内容为空时按 0 解析。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ReadCachedSize_EmptyContent_ReturnsZero_017, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    WriteRawFile(path, "");
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(path, size), E_OK);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: ReadCachedSize_WhitespaceOnly_ReturnsZero_018
 * @tc.desc: 仅空白字符时按 0 解析。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ReadCachedSize_WhitespaceOnly_ReturnsZero_018, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    WriteRawFile(path, "   \n\t ");
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(path, size), E_OK);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: ReadCachedSize_NonNumericContent_ReturnsZero_019
 * @tc.desc: 非数字内容触发 from_chars 失败，按 0 返回。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ReadCachedSize_NonNumericContent_ReturnsZero_019, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    WriteRawFile(path, "abc");
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(path, size), E_OK);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: ReadCachedSize_NegativeValueClampedToZero_020
 * @tc.desc: 解析得到负值时钳制为 0。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ReadCachedSize_NegativeValueClampedToZero_020, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    WriteRawFile(path, "-5");
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(path, size), E_OK);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: ReadCachedSize_TrailingGarbage_ParsesPrefix_021
 * @tc.desc: 数字后跟非数字时按前缀有效值解析。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ReadCachedSize_TrailingGarbage_ParsesPrefix_021, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    WriteRawFile(path, "123abc");
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(path, size), E_OK);
    EXPECT_EQ(size, 123);
}

/**
 * @tc.name: ReadCachedSize_LeadingWhitespace_ParsesValue_022
 * @tc.desc: 前导空白被跳过后正常解析。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ReadCachedSize_LeadingWhitespace_ParsesValue_022, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    WriteRawFile(path, "  456");
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(path, size), E_OK);
    EXPECT_EQ(size, 456);
}

/**
 * @tc.name: ReadCachedSize_DirectoryPath_ReturnsPathError_023
 * @tc.desc: 缓存路径为目录时 read 失败返回 E_PATH。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ReadCachedSize_DirectoryPath_ReturnsPathError_023, TestSize.Level1)
{
    string dirPath = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    std::system(("mkdir -p " + dirPath).c_str());
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(dirPath, size), E_PATH);
    std::system(("rm -rf " + dirPath + " 2>/dev/null").c_str());
}

/**
 * @tc.name: WriteCachedSize_NegativeSize_ClampedToZero_024
 * @tc.desc: 负数 size 被钳制为 0 后落盘。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, WriteCachedSize_NegativeSize_ClampedToZero_024, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    EXPECT_EQ(RecycleSizeCache::WriteCachedSize(path, -50), E_OK);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(path, size), E_OK);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: OpenAndCheck_Hardlink_AsyncWriteFails_025
 * @tc.desc: 缓存文件存在硬链接(nlink>1)时异步写失败，文件内容保持写之前的值不被破坏。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, OpenAndCheck_Hardlink_AsyncWriteFails_025, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_TRUE(WaitCacheFileExists(TEST_USER_ID, TEST_BUNDLE));
    string cachePath = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    EXPECT_EQ(RecycleSizeCache::WriteCachedSize(cachePath, 999), E_OK);
    string linkPath = GetBaseDir(TEST_USER_ID, TEST_BUNDLE) + "/RecycleSizeCache.hlnk";
    ASSERT_EQ(link(cachePath.c_str(), linkPath.c_str()), 0);
    std::vector<MetaBase> list = {MakeMetaBase(100)};
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, list), E_OK);
    WaitAsyncSettled(TEST_USER_ID, TEST_BUNDLE);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(cachePath, size), E_OK);
    EXPECT_EQ(size, 999);
    unlink(linkPath.c_str());
}

/**
 * @tc.name: OpenAndCheck_CachePathIsDirectory_AsyncWriteFails_026
 * @tc.desc: 缓存路径为目录时异步 Reset 写失败，不崩溃不修改目录。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, OpenAndCheck_CachePathIsDirectory_AsyncWriteFails_026, TestSize.Level1)
{
    std::system(("mkdir -p " + GetCachePath(TEST_USER_ID, TEST_BUNDLE)).c_str());
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    WaitAsyncSettled(TEST_USER_ID, TEST_BUNDLE);
    struct stat st;
    ASSERT_EQ(stat(GetCachePath(TEST_USER_ID, TEST_BUNDLE).c_str(), &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));
    std::system(("rm -rf " + GetCachePath(TEST_USER_ID, TEST_BUNDLE) + " 2>/dev/null").c_str());
}

/**
 * @tc.name: ResetInvalidates_InFlightIncrease_027
 * @tc.desc: Inc 落盘后 Reset 清零，再在途 Inc 被版本丢弃，文件保持 Reset 后的值。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetInvalidates_InFlightIncrease_027, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(WaitSize(100), 100);
    int64_t verBefore = RecycleSizeCache::GetCacheVersion(GetCachePath(TEST_USER_ID, TEST_BUNDLE));
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    WaitVersionBumped(TEST_USER_ID, TEST_BUNDLE, verBefore);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(50)), E_OK);
    EXPECT_EQ(WaitSize(50), 50);
}

/**
 * @tc.name: ResetInvalidates_InFlightDecrease_028
 * @tc.desc: Inc 落盘后 Reset 清零，再在途 Dec 被版本丢弃，文件保持 Reset 后的值。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetInvalidates_InFlightDecrease_028, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(200)), E_OK);
    EXPECT_EQ(WaitSize(200), 200);
    int64_t verBefore = RecycleSizeCache::GetCacheVersion(GetCachePath(TEST_USER_ID, TEST_BUNDLE));
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    WaitVersionBumped(TEST_USER_ID, TEST_BUNDLE, verBefore);
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(80)), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
}

/**
 * @tc.name: IncreaseAfterReset_SucceedsWithNewVersion_029
 * @tc.desc: Reset 后新版本下的 Increase 能正常落盘。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, IncreaseAfterReset_SucceedsWithNewVersion_029, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(WaitSize(100), 100);
    int64_t verBefore = RecycleSizeCache::GetCacheVersion(GetCachePath(TEST_USER_ID, TEST_BUNDLE));
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    WaitVersionBumped(TEST_USER_ID, TEST_BUNDLE, verBefore);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(50)), E_OK);
    EXPECT_EQ(WaitSize(50), 50);
}

/**
 * @tc.name: MultipleResets_VersionIncrements_030
 * @tc.desc: 连续多次 Reset 后版本正确换代，后续 Increase 正常落盘。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, MultipleResets_VersionIncrements_030, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(WaitSize(100), 100);
    int64_t ver1 = RecycleSizeCache::GetCacheVersion(GetCachePath(TEST_USER_ID, TEST_BUNDLE));
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    WaitVersionBumped(TEST_USER_ID, TEST_BUNDLE, ver1);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(WaitSize(100), 100);
    int64_t ver2 = RecycleSizeCache::GetCacheVersion(GetCachePath(TEST_USER_ID, TEST_BUNDLE));
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    WaitVersionBumped(TEST_USER_ID, TEST_BUNDLE, ver2);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(30)), E_OK);
    EXPECT_EQ(WaitSize(30), 30);
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
}

/**
 * @tc.name: ConcurrentIncreases_Converge_031
 * @tc.desc: 多线程并发 Increase 不丢更新，最终收敛到累加值。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ConcurrentIncreases_Converge_031, TestSize.Level1)
{
    const int threads = 8;
    const int perThread = 5;
    const int64_t unit = 10;
    vector<thread> pool;
    for (int i = 0; i < threads; ++i) {
        pool.emplace_back([&] {
            for (int j = 0; j < perThread; ++j) {
                RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(unit));
            }
        });
    }
    for (auto &t : pool) {
        t.join();
    }
    int64_t expect = static_cast<int64_t>(threads) * perThread * unit;
    EXPECT_EQ(WaitSize(expect, 5000), expect);
}

/**
 * @tc.name: ConcurrentMixedOperations_Converge_032
 * @tc.desc: 先建立基线后并发增减，无版本换代时不发生钳制丢值，收敛到确定净值。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ConcurrentMixedOperations_Converge_032, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(500)), E_OK);
    EXPECT_EQ(WaitSize(500), 500);
    const int inc = 8;
    const int dec = 4;
    const int perThread = 5;
    const int64_t unit = 10;
    vector<thread> pool;
    for (int i = 0; i < inc; ++i) {
        pool.emplace_back([&] {
            for (int j = 0; j < perThread; ++j) {
                RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(unit));
            }
        });
    }
    for (int i = 0; i < dec; ++i) {
        pool.emplace_back([&] {
            for (int j = 0; j < perThread; ++j) {
                RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(unit));
            }
        });
    }
    for (auto &t : pool) {
        t.join();
    }
    int64_t expect = 500 + (inc - dec) * perThread * unit;
    EXPECT_EQ(WaitSize(expect, 5000), expect);
}

/**
 * @tc.name: ConcurrentReset_NoCorruption_033
 * @tc.desc: 并发 Increase 与 Reset 不产生崩溃或损坏，末次 Reset 后收敛为 0。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ConcurrentReset_NoCorruption_033, TestSize.Level1)
{
    vector<thread> pool;
    for (int i = 0; i < 8; ++i) {
        pool.emplace_back([&] {
            for (int j = 0; j < 5; ++j) {
                RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(10));
            }
        });
    }
    for (int i = 0; i < 3; ++i) {
        pool.emplace_back([&] { RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE); });
    }
    for (auto &t : pool) {
        t.join();
    }
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
}

/**
 * @tc.name: NoCrossKeyInterference_ResetDoesNotDiscardOtherKey_034
 * @tc.desc: per-key 版本追踪：对 A 的 Reset 不会丢弃 B 的在途 Increase。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, NoCrossKeyInterference_ResetDoesNotDiscardOtherKey_034, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2), E_OK);
    EXPECT_EQ(WaitSize(100), 100);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID_2, TEST_BUNDLE_2, 0), 0);
}

/**
 * @tc.name: IncreaseAsync_ReadFails_Discarded_035
 * @tc.desc: 缓存路径为目录时 Increase 异步读失败，不写文件不崩溃。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, IncreaseAsync_ReadFails_Discarded_035, TestSize.Level1)
{
    string dirPath = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    std::system(("mkdir -p " + dirPath).c_str());
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    WaitAsyncSettled(TEST_USER_ID, TEST_BUNDLE);
    struct stat st;
    ASSERT_EQ(stat(dirPath.c_str(), &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));
    std::system(("rm -rf " + dirPath + " 2>/dev/null").c_str());
}

/**
 * @tc.name: DecreaseAsync_ReadFails_Discarded_036
 * @tc.desc: 缓存路径为目录时 Decrease 异步读失败，不写文件不崩溃。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, DecreaseAsync_ReadFails_Discarded_036, TestSize.Level1)
{
    string dirPath = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    std::system(("mkdir -p " + dirPath).c_str());
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    WaitAsyncSettled(TEST_USER_ID, TEST_BUNDLE);
    struct stat st;
    ASSERT_EQ(stat(dirPath.c_str(), &st), 0);
    EXPECT_TRUE(S_ISDIR(st.st_mode));
    std::system(("rm -rf " + dirPath + " 2>/dev/null").c_str());
}

/**
 * @tc.name: WriteCachedSize_LargeSize_PersistsCorrectly_037
 * @tc.desc: 大数值（接近 int64_t 上限）正确落盘并读回。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, WriteCachedSize_LargeSize_PersistsCorrectly_037, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    const int64_t large = 9223372036854775807LL;
    EXPECT_EQ(RecycleSizeCache::WriteCachedSize(path, large), E_OK);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(path, size), E_OK);
    EXPECT_EQ(size, large);
}

/**
 * @tc.name: WriteCachedSize_OverwriteExistingFile_038
 * @tc.desc: 已有文件内容时再次写入，覆盖旧值。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, WriteCachedSize_OverwriteExistingFile_038, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    WriteRawFile(path, "999");
    EXPECT_EQ(RecycleSizeCache::WriteCachedSize(path, 100), E_OK);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(path, size), E_OK);
    EXPECT_EQ(size, 100);
}

/**
 * @tc.name: WriteCachedSize_CreatesNewFileWithCorrectMode_039
 * @tc.desc: 首次写入创建新文件，验证 isNewFile 分支（fchmod/fchown 执行）。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, WriteCachedSize_CreatesNewFileWithCorrectMode_039, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    CleanRecycleFile(TEST_USER_ID, TEST_BUNDLE);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
    EXPECT_EQ(RecycleSizeCache::WriteCachedSize(path, 42), E_OK);
    struct stat st;
    ASSERT_EQ(stat(path.c_str(), &st), 0);
    EXPECT_TRUE(S_ISREG(st.st_mode));
    EXPECT_EQ(st.st_nlink, 1u);
}

/**
 * @tc.name: GetRecycleBinSize_MissingBaseDir_ReturnsZero_040
 * @tc.desc: 缓存目录不存在时 Get 按 0 返回（ENOENT 分支）。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, GetRecycleBinSize_MissingBaseDir_ReturnsZero_040, TestSize.Level1)
{
    RemoveBaseDir(TEST_USER_ID, TEST_BUNDLE);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name: DecreaseRecycleBinSize_NoCacheFile_CreatesAndWritesZero_041
 * @tc.desc: 无缓存文件时 Decrease 异步读取按 0，减后钳制为 0 并创建文件。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, DecreaseRecycleBinSize_NoCacheFile_CreatesAndWritesZero_041, TestSize.Level1)
{
    PrepareDir(TEST_USER_ID, TEST_BUNDLE);
    CleanRecycleFile(TEST_USER_ID, TEST_BUNDLE);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    EXPECT_TRUE(WaitCacheFileExists(TEST_USER_ID, TEST_BUNDLE));
}

/**
 * @tc.name: ConcurrentGetAndIncrease_NoCrash_042
 * @tc.desc: 并发 Get 与 Increase 不崩溃，Get 最终收敛到累加值。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ConcurrentGetAndIncrease_NoCrash_042, TestSize.Level1)
{
    vector<thread> pool;
    for (int i = 0; i < 4; ++i) {
        pool.emplace_back([&] {
            for (int j = 0; j < 10; ++j) {
                int64_t s = 0;
                RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, s);
            }
        });
    }
    for (int i = 0; i < 4; ++i) {
        pool.emplace_back([&] {
            for (int j = 0; j < 5; ++j) {
                RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(10));
            }
        });
    }
    for (auto &t : pool) {
        t.join();
    }
    EXPECT_EQ(WaitSize(200), 200);
}

/**
 * @tc.name: ConcurrentIncreaseDecreaseReset_NoCrashConverges_043
 * @tc.desc: 高并发 Inc + Dec + Reset 不崩溃，末次 Reset 后收敛为 0。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ConcurrentIncreaseDecreaseReset_NoCrashConverges_043, TestSize.Level1)
{
    vector<thread> pool;
    for (int i = 0; i < 6; ++i) {
        pool.emplace_back([&] {
            for (int j = 0; j < 10; ++j) {
                RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(10));
            }
        });
    }
    for (int i = 0; i < 4; ++i) {
        pool.emplace_back([&] {
            for (int j = 0; j < 5; ++j) {
                RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(10));
            }
        });
    }
    for (int i = 0; i < 3; ++i) {
        pool.emplace_back([&] { RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE); });
    }
    for (auto &t : pool) {
        t.join();
    }
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
}

/**
 * @tc.name: IncreaseThenResetThenIncrease_Sequence_044
 * @tc.desc: 增量→重置→再增量序列，每步收敛到正确值。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, IncreaseThenResetThenIncrease_Sequence_044, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(300)), E_OK);
    EXPECT_EQ(WaitSize(300), 300);
    int64_t verBefore = RecycleSizeCache::GetCacheVersion(GetCachePath(TEST_USER_ID, TEST_BUNDLE));
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    WaitVersionBumped(TEST_USER_ID, TEST_BUNDLE, verBefore);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(150)), E_OK);
    EXPECT_EQ(WaitSize(150), 150);
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(50)), E_OK);
    EXPECT_EQ(WaitSize(100), 100);
}

/**
 * @tc.name: ConcurrentMultiUserIncreaseReset_NoCrossInterference_045
 * @tc.desc: 多用户并发 Increase 与 Reset 互不干扰，各自收敛正确。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ConcurrentMultiUserIncreaseReset_NoCrossInterference_045, TestSize.Level1)
{
    vector<thread> pool;
    pool.emplace_back([&] {
        for (int i = 0; i < 5; ++i) {
            RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(20));
        }
    });
    pool.emplace_back([&] {
        for (int i = 0; i < 5; ++i) {
            RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2, MakeMetaBase(30));
        }
    });
    pool.emplace_back([&] { RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE); });
    pool.emplace_back([&] { RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2); });
    for (auto &t : pool) {
        t.join();
    }
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID_2, TEST_BUNDLE_2, 0), 0);
}

/**
 * @tc.name: BatchContinuousDecrease_Converges_046
 * @tc.desc: 批量连续减量后缓存值正确收敛到 0，不丢更新。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, BatchContinuousDecrease_Converges_046, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(1000)), E_OK);
    EXPECT_EQ(WaitSize(1000), 1000);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    }
    EXPECT_EQ(WaitSize(0, 5000), 0);
    EXPECT_TRUE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
}

/**
 * @tc.name: MissingBaseDir_DecreaseDiscarded_047
 * @tc.desc: 缓存目录不存在时 Decrease 异步失败被丢弃，不重建文件不崩溃。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, MissingBaseDir_DecreaseDiscarded_047, TestSize.Level1)
{
    RemoveBaseDir(TEST_USER_ID, TEST_BUNDLE);
    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    WaitAsyncSettled(TEST_USER_ID, TEST_BUNDLE);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 0);
    EXPECT_FALSE(CacheFileExists(TEST_USER_ID, TEST_BUNDLE));
}

/**
 * @tc.name: CorruptedFile_TruncatedContent_ParsesPrefix_048
 * @tc.desc: 缓存文件写入被截断（如写一半进程被杀），残留部分数字按前缀解析。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, CorruptedFile_TruncatedContent_ParsesPrefix_048, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    WriteRawFile(path, "123");
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(path, size), E_OK);
    EXPECT_EQ(size, 123);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(50)), E_OK);
    EXPECT_EQ(WaitSize(173), 173);
}

/**
 * @tc.name: CorruptedFile_BinaryGarbage_ResetToZero_049
 * @tc.desc: 缓存文件含二进制垃圾数据时解析失败归零，后续 Reset 正常。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, CorruptedFile_BinaryGarbage_ResetToZero_049, TestSize.Level1)
{
    string path = GetCachePath(TEST_USER_ID, TEST_BUNDLE);
    string garbage = "\xff\xfe\x01\x02garbage\xff";
    WriteRawFile(path, garbage);
    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::ReadCachedSize(path, size), E_OK);
    EXPECT_EQ(size, 0);
    int64_t verBefore = RecycleSizeCache::GetCacheVersion(path);
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    WaitVersionBumped(TEST_USER_ID, TEST_BUNDLE, verBefore);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(WaitSize(100), 100);
}

/**
 * @tc.name: FourCombos_ConcurrentReset_Isolation_050
 * @tc.desc: 4组(userId,bundle)同时并发 Reset，互不干扰，各自收敛 0。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, FourCombos_ConcurrentReset_Isolation_050, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE_2, MakeMetaBase(200)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE, MakeMetaBase(300)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2, MakeMetaBase(400)), E_OK);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID, TEST_BUNDLE, 100), 100);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID, TEST_BUNDLE_2, 200), 200);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID_2, TEST_BUNDLE, 300), 300);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID_2, TEST_BUNDLE_2, 400), 400);

    vector<thread> pool;
    pool.emplace_back([&] { RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE); });
    pool.emplace_back([&] { RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE_2); });
    pool.emplace_back([&] { RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE); });
    pool.emplace_back([&] { RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2); });
    for (auto &t : pool) {
        t.join();
    }
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID, TEST_BUNDLE, 0), 0);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID, TEST_BUNDLE_2, 0), 0);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID_2, TEST_BUNDLE, 0), 0);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID_2, TEST_BUNDLE_2, 0), 0);
}

/**
 * @tc.name: GroupCrossOps_IsolationMaintained_051
 * @tc.desc: 组内增减重置交叉操作后，另一组缓存不受影响，隔离保持。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, GroupCrossOps_IsolationMaintained_051, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(500)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2, MakeMetaBase(200)), E_OK);
    EXPECT_EQ(WaitSize(500), 500);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID_2, TEST_BUNDLE_2, 200), 200);

    EXPECT_EQ(RecycleSizeCache::DecreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(WaitSize(400), 400);
    int64_t verBefore = RecycleSizeCache::GetCacheVersion(GetCachePath(TEST_USER_ID, TEST_BUNDLE));
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    WaitVersionBumped(TEST_USER_ID, TEST_BUNDLE, verBefore);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(50)), E_OK);
    EXPECT_EQ(WaitSize(50), 50);

    int64_t other = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2, other), E_OK);
    EXPECT_EQ(other, 200);
}

/**
 * @tc.name: VersionConflict_ConvergesToLatestVersion_052
 * @tc.desc: 多次版本换代后，最终数据收敛到最新版本的结果。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, VersionConflict_ConvergesToLatestVersion_052, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(WaitSize(100), 100);

    int64_t ver1 = RecycleSizeCache::GetCacheVersion(GetCachePath(TEST_USER_ID, TEST_BUNDLE));
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    WaitVersionBumped(TEST_USER_ID, TEST_BUNDLE, ver1);

    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(50)), E_OK);
    EXPECT_EQ(WaitSize(50), 50);

    int64_t ver2 = RecycleSizeCache::GetCacheVersion(GetCachePath(TEST_USER_ID, TEST_BUNDLE));
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
    WaitVersionBumped(TEST_USER_ID, TEST_BUNDLE, ver2);

    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(200)), E_OK);
    EXPECT_EQ(WaitSize(200), 200);

    int64_t size = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size), E_OK);
    EXPECT_EQ(size, 200);
}

/**
 * @tc.name: ResetWithList_EmptyList_ResetsToZero_053
 * @tc.desc: 空列表时等同于普通 Reset，重置为 0。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetWithList_EmptyList_ResetsToZero_053, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(500)), E_OK);
    EXPECT_EQ(WaitSize(500), 500);
    std::vector<MetaBase> empty;
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, empty), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
}

/**
 * @tc.name: ResetWithList_SingleItem_ResetsAndReapplies_054
 * @tc.desc: 单个元素时先重置为 0 再应用该元素增量。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetWithList_SingleItem_ResetsAndReapplies_054, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(999)), E_OK);
    EXPECT_EQ(WaitSize(999), 999);
    std::vector<MetaBase> list = {MakeMetaBase(100)};
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, list), E_OK);
    EXPECT_EQ(WaitSize(100), 100);
}

/**
 * @tc.name: ResetWithList_MultipleItems_SumsAll_055
 * @tc.desc: 多个元素时先重置为 0 再累加所有元素。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetWithList_MultipleItems_SumsAll_055, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(999)), E_OK);
    EXPECT_EQ(WaitSize(999), 999);
    std::vector<MetaBase> list = {MakeMetaBase(100), MakeMetaBase(200), MakeMetaBase(300)};
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, list), E_OK);
    EXPECT_EQ(WaitSize(600), 600);
}

/**
 * @tc.name: ResetWithList_FilteredItemsIgnored_056
 * @tc.desc: 列表中含纯云端和目录类型时被过滤，只累加本地文件。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetWithList_FilteredItemsIgnored_056, TestSize.Level1)
{
    MetaBase cloud = MakeMetaBase(500);
    cloud.position = POSITION_CLOUD;
    MetaBase dir = MakeMetaBase(300, "dir");
    dir.mode = S_IFDIR | 0771;
    std::vector<MetaBase> list = {cloud, dir, MakeMetaBase(150)};
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, list), E_OK);
    EXPECT_EQ(WaitSize(150), 150);
}

/**
 * @tc.name: ResetWithList_MixedSizes_Converges_057
 * @tc.desc: 含零大小的元素被跳过，其余正常累加收敛。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetWithList_MixedSizes_Converges_057, TestSize.Level1)
{
    std::vector<MetaBase> list = {MakeMetaBase(0), MakeMetaBase(100), MakeMetaBase(0), MakeMetaBase(250)};
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, list), E_OK);
    EXPECT_EQ(WaitSize(350), 350);
}

/**
 * @tc.name: ResetWithList_InvalidArgs_ReturnsError_058
 * @tc.desc: 非法入参（userId 为负数、bundleName 为空）返回 E_INVAL_ARG。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetWithList_InvalidArgs_ReturnsError_058, TestSize.Level1)
{
    std::vector<MetaBase> list = {MakeMetaBase(100)};
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(-1, TEST_BUNDLE, list), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, "", list), E_INVAL_ARG);
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, "../evil", list), E_INVAL_ARG);
}

/**
 * @tc.name: ResetWithList_IsolationBetweenCombos_059
 * @tc.desc: 对 A 的带列表 Reset 不影响 B 的缓存，隔离保持。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetWithList_IsolationBetweenCombos_059, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(500)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2, MakeMetaBase(200)), E_OK);
    EXPECT_EQ(WaitSize(500), 500);
    EXPECT_EQ(WaitSizeFor(TEST_USER_ID_2, TEST_BUNDLE_2, 200), 200);

    std::vector<MetaBase> list = {MakeMetaBase(100), MakeMetaBase(50)};
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, list), E_OK);
    EXPECT_EQ(WaitSize(150), 150);

    int64_t other = -1;
    EXPECT_EQ(RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2, other), E_OK);
    EXPECT_EQ(other, 200);
}

/**
 * @tc.name: ResetWithList_AfterResetWithList_Converges_060
 * @tc.desc: 连续调用带列表的 Reset，每次先清零再累加，收敛到最新列表总和。
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, ResetWithList_AfterResetWithList_Converges_060, TestSize.Level1)
{
    std::vector<MetaBase> list1 = {MakeMetaBase(100), MakeMetaBase(200)};
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, list1), E_OK);
    EXPECT_EQ(WaitSize(300), 300);

    std::vector<MetaBase> list2 = {MakeMetaBase(50), MakeMetaBase(30), MakeMetaBase(20)};
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, list2), E_OK);
    EXPECT_EQ(WaitSize(100), 100);

    std::vector<MetaBase> empty;
    EXPECT_EQ(RecycleSizeCache::ResetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, empty), E_OK);
    EXPECT_EQ(WaitSize(0), 0);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS