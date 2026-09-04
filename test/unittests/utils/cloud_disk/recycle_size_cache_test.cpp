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
#include <gtest/gtest.h>
#include <thread>

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

    static MetaBase MakeMetaBase(uint64_t size, const string &name = "test")
    {
        MetaBase mb(name);
        mb.size = size;
        return mb;
    }
}

class RecycleSizeCacheTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

// 写操作是异步的，轮询等待 Get 收敛到期望值（带回溯超时）。
static constexpr int64_t POLL_INTERVAL_MS = 10;
static constexpr int64_t POLL_TIMEOUT_MS = 3000;
static int64_t WaitSize(int64_t expect, int timeoutMs = POLL_TIMEOUT_MS)
{
    int64_t size = -1;
    auto deadline = chrono::steady_clock::now() + chrono::milliseconds(timeoutMs);
    do {
        if (RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, size) == E_OK && size == expect) {
            return size;
        }
        this_thread::sleep_for(chrono::milliseconds(POLL_INTERVAL_MS));
    } while (chrono::steady_clock::now() < deadline);
    return size;
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
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(250)), E_OK);
    EXPECT_EQ(WaitSize(350), 350);
}

/**
 * @tc.name: DecreaseRecycleBinSize_SubtractsAndClamps_003
 * @tc.desc: Verify the sub function.
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
 * @tc.desc: Verify the sub function.
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
}

/**
 * @tc.name: MultiUserMultiBundle_Isolation_008
 * @tc.desc: Verify the sub function.
 * @tc.type: FUNC
 * @tc.require: issueNumber
 */
HWTEST_F(RecycleSizeCacheTest, MultiUserMultiBundle_Isolation_008, TestSize.Level1)
{
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE, MakeMetaBase(100)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID, TEST_BUNDLE_2, MakeMetaBase(60)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE, MakeMetaBase(200)), E_OK);
    EXPECT_EQ(RecycleSizeCache::IncreaseRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2, MakeMetaBase(300)), E_OK);
    EXPECT_EQ(WaitSize(100), 100);

    int64_t size2 = -1;
    auto deadline = chrono::steady_clock::now() + chrono::milliseconds(POLL_TIMEOUT_MS);
    do {
        if (RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE, size2) == E_OK && size2 == 200) {
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(POLL_INTERVAL_MS));
    } while (chrono::steady_clock::now() < deadline);
    EXPECT_EQ(size2, 200);

    int64_t size3 = -1;
    do {
        if (RecycleSizeCache::GetRecycleBinSize(TEST_USER_ID_2, TEST_BUNDLE_2, size3) == E_OK && size3 == 300) {
            break;
        }
        this_thread::sleep_for(chrono::milliseconds(POLL_INTERVAL_MS));
    } while (chrono::steady_clock::now() < deadline);
    EXPECT_EQ(size3, 300);
}

/**
 * @tc.name: ResetRecycleBinSize_ResetsCacheToZero_005
 * @tc.desc: Verify the sub function.
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
 * @tc.name: EndToEnd_RecycleAndReset_007
 * @tc.desc: Verify the sub function.
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
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
