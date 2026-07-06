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

#ifdef lstat
#define DFS_SPACE_REPORT_TEST_MOCK_LSTAT
#undef lstat
#endif
#ifdef opendir
#define DFS_SPACE_REPORT_TEST_MOCK_OPENDIR
#undef opendir
#endif
#ifdef readdir
#define DFS_SPACE_REPORT_TEST_MOCK_READDIR
#undef readdir
#endif
#ifdef closedir
#define DFS_SPACE_REPORT_TEST_MOCK_CLOSEDIR
#undef closedir
#endif

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <dirent.h>
#include <fstream>
#include <gtest/gtest.h>
#include <limits>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "battery_status.h"
#include "cloud_file_fault_event.h"
#include "dfs_error.h"
#include "screen_status.h"
#include "system_load.h"

namespace {
static bool g_mockReadDirFail = false;
static int32_t g_mockReadDirFailAfter = -1;
static bool g_mockTimeout = false;
static int32_t g_mockTimeoutAfter = -1;
static int32_t g_mockReadDirCalls = 0;
static bool g_mockScreenOnInReadDir = false;
static int32_t g_mockScreenOnAfterReadDir = -1;
static std::atomic_bool g_mockStopScan = false;
static bool g_mockFaultReportFailFirst = false;
static int32_t g_mockFaultReportCalls = 0;
static std::vector<std::string> g_mockFaultReportDetails;
static std::string g_mockLstatFailPath;
static std::string g_mockOpenDirFailPath;
static int32_t g_mockLstatCalls = 0;
static int32_t g_mockOpenDirCalls = 0;
static int32_t g_mockCloseDirCalls = 0;

static int32_t MockCloudSyncFaultReport(const OHOS::FileManagement::CloudFile::CloudSyncFaultInfo &info)
{
    g_mockFaultReportCalls++;
    g_mockFaultReportDetails.emplace_back(info.message_);
    if (g_mockFaultReportFailFirst && g_mockFaultReportCalls == 1) {
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    return OHOS::FileManagement::E_OK;
}
} // namespace

namespace OHOS::FileManagement::CloudSync {
static struct dirent *MockReadDir(DIR *dir);
static int MockLstat(const char *path, struct stat *buf);
static DIR *MockOpenDir(const char *path);
static int MockCloseDir(DIR *dir);
} // namespace OHOS::FileManagement::CloudSync

#undef CLOUD_SYNC_FAULT_REPORT
#define CLOUD_SYNC_FAULT_REPORT(...) MockCloudSyncFaultReport(__VA_ARGS__)
#ifdef DFS_SPACE_REPORT_TEST_MOCK_LSTAT
#define lstat MockLstat
#endif
#ifdef DFS_SPACE_REPORT_TEST_MOCK_OPENDIR
#define opendir MockOpenDir
#endif
#ifdef DFS_SPACE_REPORT_TEST_MOCK_READDIR
#define readdir MockReadDir
#endif
#ifdef DFS_SPACE_REPORT_TEST_MOCK_CLOSEDIR
#define closedir MockCloseDir
#endif
#include "dfs_space_report_task.cpp"
#ifdef lstat
#undef lstat
#endif
#ifdef opendir
#undef opendir
#endif
#ifdef readdir
#undef readdir
#endif
#ifdef closedir
#undef closedir
#endif

namespace OHOS::FileManagement::CloudSync {
static int MockLstat(const char *path, struct stat *buf)
{
    g_mockLstatCalls++;
    if (!g_mockLstatFailPath.empty() && path != nullptr && g_mockLstatFailPath == path) {
        errno = ENOENT;
        return -1;
    }
    return ::lstat(path, buf);
}

static DIR *MockOpenDir(const char *path)
{
    g_mockOpenDirCalls++;
    if (!g_mockOpenDirFailPath.empty() && path != nullptr && g_mockOpenDirFailPath == path) {
        errno = EACCES;
        return nullptr;
    }
    return ::opendir(path);
}

static struct dirent *MockReadDir(DIR *dir)
{
    int32_t readDirCalls = g_mockReadDirCalls++;
    if (g_mockScreenOnInReadDir && readDirCalls >= g_mockScreenOnAfterReadDir) {
        ScreenStatus::SetScreenState(ScreenStatus::ScreenState::SCREEN_ON);
    }
    if (g_mockTimeout && readDirCalls >= g_mockTimeoutAfter) {
        OnScanTimeout(g_mockStopScan);
    }
    if (g_mockReadDirFail && readDirCalls >= g_mockReadDirFailAfter) {
        errno = EIO;
        return nullptr;
    }
    return ::readdir(dir);
}

static int MockCloseDir(DIR *dir)
{
    g_mockCloseDirCalls++;
    return ::closedir(dir);
}
} // namespace OHOS::FileManagement::CloudSync

namespace OHOS::FileManagement::CloudSync::Test {
using namespace testing::ext;
using namespace std;

namespace {
const string TEST_ROOT = "/data/test_tdd/dfs_space_report";

static void ResetMockReadDir()
{
    g_mockReadDirFail = false;
    g_mockReadDirFailAfter = -1;
    g_mockTimeout = false;
    g_mockTimeoutAfter = -1;
    g_mockScreenOnInReadDir = false;
    g_mockScreenOnAfterReadDir = -1;
    g_mockReadDirCalls = 0;
}

static void ResetMockTime()
{
    ResetStopScan(g_mockStopScan);
}

static void ResetMockFaultReport()
{
    g_mockFaultReportFailFirst = false;
    g_mockFaultReportCalls = 0;
    g_mockFaultReportDetails.clear();
}

static void ResetMockSysCalls()
{
    g_mockLstatFailPath.clear();
    g_mockOpenDirFailPath.clear();
    g_mockLstatCalls = 0;
    g_mockOpenDirCalls = 0;
    g_mockCloseDirCalls = 0;
}

static void CreateFile(const string &path, const string &content = "dfs-space-report")
{
    ofstream file(path);
    file << content;
    file.close();
}

static bool TryMakeDfsOwned(const string &path)
{
    return chown(path.c_str(), DFS_UID, DFS_UID) == 0;
}

static void SetDfsReportCondition(bool screenOn, bool charging, PowerMgr::ThermalLevel level, int32_t capacity = 100)
{
    ScreenStatus::SetScreenState(
        screenOn ? ScreenStatus::ScreenState::SCREEN_ON : ScreenStatus::ScreenState::SCREEN_OFF);
    BatteryStatus::SetChargingStatus(charging);
    BatteryStatus::SetCapacity(capacity);
    SystemLoadStatus::Setload(level);
}

} // namespace

class DfsSpaceReportTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void DfsSpaceReportTaskTest::SetUp(void)
{
    ResetMockReadDir();
    ResetMockTime();
    ResetMockFaultReport();
    ResetMockSysCalls();
    SetDfsReportCondition(false, true, PowerMgr::ThermalLevel::NORMAL);
    std::system(("rm -rf " + TEST_ROOT).c_str());
    std::system(("mkdir -p " + TEST_ROOT).c_str());
}

void DfsSpaceReportTaskTest::TearDown(void)
{
    ResetMockReadDir();
    ResetMockTime();
    ResetMockFaultReport();
    ResetMockSysCalls();
    SetDfsReportCondition(false, true, PowerMgr::ThermalLevel::NORMAL);
    std::system(("rm -rf " + TEST_ROOT).c_str());
}

/*
 * @tc.name: ScanDirTest001
 * @tc.desc: Verify that readdir failure is counted in scan errors for reporting.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanDirTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanDirTest001 Start";
    g_mockReadDirFail = true;
    g_mockReadDirFailAfter = 1;

    DfsDirStat stat = ScanDir(TEST_ROOT, g_mockStopScan);

    EXPECT_GT(stat.errors, 0U);
    GTEST_LOG_(INFO) << "ScanDirTest001 End";
}

/*
 * @tc.name: ScanDirTest002
 * @tc.desc: Verify that scanning stops while reading entries after timeout.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanDirTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanDirTest002 Start";
    for (int32_t i = 0; i < 32; ++i) {
        CreateFile(TEST_ROOT + "/file_" + to_string(i));
    }
    g_mockTimeout = true;
    g_mockTimeoutAfter = 1;

    DfsDirStat stat = ScanDir(TEST_ROOT, g_mockStopScan);

    EXPECT_TRUE(IsScanStopped(stat.stopped));
    GTEST_LOG_(INFO) << "ScanDirTest002 End";
}

/*
 * @tc.name: ScanDirTest003
 * @tc.desc: Verify that DFS quota scan ignores entries whose uid is not dfs.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanDirTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanDirTest003 Start";
    if (geteuid() == DFS_UID) {
        GTEST_SKIP() << "Current uid equals DFS_UID; non-DFS fixture cannot be constructed reliably.";
    }
    CreateFile(TEST_ROOT + "/non_dfs_file");

    DfsDirStat stat = ScanDir(TEST_ROOT, g_mockStopScan);

    EXPECT_EQ(stat.blocksBytes, 0U);
    GTEST_LOG_(INFO) << "ScanDirTest003 End";
}

/*
 * @tc.name: ScanDirTest004
 * @tc.desc: Verify that DFS quota scan only counts regular files owned by dfs uid.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanDirTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanDirTest004 Start";
    string dfsDir = TEST_ROOT + "/dfs_dir";
    std::system(("mkdir -p " + dfsDir).c_str());
    if (geteuid() != DFS_UID && !TryMakeDfsOwned(dfsDir)) {
        GTEST_SKIP() << "Cannot construct dfs-owned directory fixture.";
    }

    DfsDirStat stat = ScanDir(TEST_ROOT, g_mockStopScan);

    EXPECT_EQ(stat.blocksBytes, 0U);
    GTEST_LOG_(INFO) << "ScanDirTest004 End";
}

/*
 * @tc.name: ScanDirTest005
 * @tc.desc: Verify that DFS scan stops before opening pending root directory.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanDirTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanDirTest005 Start";
    OnScanTimeout(g_mockStopScan);

    DfsDirStat stat = ScanDir(TEST_ROOT, g_mockStopScan);

    EXPECT_TRUE(IsScanStopped(stat.stopped));
    EXPECT_EQ(g_mockOpenDirCalls, 0);
    GTEST_LOG_(INFO) << "ScanDirTest005 End";
}

/*
 * @tc.name: AddUint64Test001
 * @tc.desc: Verify that uint64 accumulation saturates on overflow.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, AddUint64Test001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddUint64Test001 Start";
    uint64_t value = std::numeric_limits<uint64_t>::max() - 1;

    AddUint64(value, 2);

    EXPECT_EQ(value, std::numeric_limits<uint64_t>::max());
    GTEST_LOG_(INFO) << "AddUint64Test001 End";
}

/*
 * @tc.name: AddDfsUidFileBlocksTest001
 * @tc.desc: Verify that DFS-owned regular file blocks are accumulated.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, AddDfsUidFileBlocksTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddDfsUidFileBlocksTest001 Start";
    struct stat fileStat {};
    fileStat.st_mode = S_IFREG;
    fileStat.st_uid = DFS_UID;
    fileStat.st_blocks = 3;
    DfsDirStat dirStat;
    MediaWrongUidStat uidStat;

    AddDfsUidFileBlocks(fileStat, dirStat);
    AddDfsUidFileBlocks(fileStat, uidStat);

    EXPECT_EQ(dirStat.blocksBytes, 3U * BLOCK_SIZE);
    EXPECT_EQ(uidStat.blocksBytes, 3U * BLOCK_SIZE);
    GTEST_LOG_(INFO) << "AddDfsUidFileBlocksTest001 End";
}

/*
 * @tc.name: AddBadBucketTest001
 * @tc.desc: Verify bucket id filtering, appending, and omission accounting.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, AddBadBucketTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "AddBadBucketTest001 Start";
    MediaWrongUidStat uidStat;

    AddBadBucket(nullptr, uidStat);
    AddBadBucket("abc", uidStat);
    AddBadBucket("-1", uidStat);
    AddBadBucket("12", uidStat);
    AddBadBucket("34", uidStat);

    EXPECT_EQ(uidStat.badBuckets, "12,34");
    EXPECT_EQ(uidStat.omittedBuckets, 0U);

    uidStat.badBuckets.assign(MAX_BUCKETS_IN_REPORT, '1');
    AddBadBucket("56", uidStat);

    EXPECT_EQ(uidStat.badBuckets.size(), MAX_BUCKETS_IN_REPORT);
    EXPECT_EQ(uidStat.omittedBuckets, 1U);
    GTEST_LOG_(INFO) << "AddBadBucketTest001 End";
}

/*
 * @tc.name: JoinPathTest001
 * @tc.desc: Verify path joining handles empty and slash-terminated parents.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, JoinPathTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JoinPathTest001 Start";

    EXPECT_EQ(JoinPath("", "child"), "child");
    EXPECT_EQ(JoinPath("/parent/", "child"), "/parent/child");
    EXPECT_EQ(JoinPath("/parent", "child"), "/parent/child");
    GTEST_LOG_(INFO) << "JoinPathTest001 End";
}

/*
 * @tc.name: ScanDfsUidTreeTest001
 * @tc.desc: Verify DFS UID tree scan records missing roots and non-directory DFS roots.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanDfsUidTreeTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest001 Start";
    DfsDirStat missingStat;
    DfsDirStat fileStat;
    string fileRoot = TEST_ROOT + "/not_dir";
    CreateFile(fileRoot);

    ScanDfsUidTree(TEST_ROOT + "/missing_child", missingStat, g_mockStopScan);
    ScanDfsUidTree(fileRoot, fileStat, g_mockStopScan);

    EXPECT_GT(missingStat.errors, 0U);
    EXPECT_GT(fileStat.errors, 0U);
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest001 End";
}

/*
 * @tc.name: ScanDfsUidTreeTest002
 * @tc.desc: Verify DFS UID tree scan stops before opening a root directory when timeout is set.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanDfsUidTreeTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest002 Start";
    DfsDirStat dirStat;
    OnScanTimeout(g_mockStopScan);

    ScanDfsUidTree(TEST_ROOT, dirStat, g_mockStopScan);

    EXPECT_TRUE(IsScanStopped(dirStat.stopped));
    EXPECT_EQ(g_mockOpenDirCalls, 0);
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest002 End";
}

/*
 * @tc.name: BuildDfsSpaceScanTargetsTest001
 * @tc.desc: Verify DFS quota scan targets keep the final fixed order.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, BuildDfsSpaceScanTargetsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildDfsSpaceScanTargetsTest001 Start";
    vector<ScanTarget> targets = BuildDfsSpaceScanTargets(100);

    auto findTarget = [&targets](DfsTargetIndex index) {
        return find_if(targets.begin(), targets.end(), [index](const ScanTarget &target) {
            return target.index == index;
        });
    };

    ASSERT_EQ(targets.size(), ToIndex(DFS_TARGET_COUNT) - 1);
    EXPECT_EQ(findTarget(CLOUD_INDEX), targets.end());
    auto target = findTarget(PUBLIC_CLOUDFILE_INDEX);
    ASSERT_NE(target, targets.end());
    EXPECT_EQ(target->path, "/data/service/el1/public/cloudfile");
    target = findTarget(PUBLIC_CLOUDKIT_INDEX);
    ASSERT_NE(target, targets.end());
    EXPECT_EQ(target->path, "/data/service/el1/public/cloudkit_service");
    target = findTarget(PUBLIC_CLOUD_FILE_SYNC_DB_INDEX);
    ASSERT_NE(target, targets.end());
    EXPECT_EQ(target->path,
        "/data/service/el1/public/database/CloudFileSyncService");
    target = findTarget(CACHE_ACCOUNT_INDEX);
    ASSERT_NE(target, targets.end());
    EXPECT_EQ(target->path, "/data/service/el2/100/hmdfs/cache/account_cache");
    target = findTarget(CACHE_CLOUD_INDEX);
    ASSERT_NE(target, targets.end());
    EXPECT_EQ(target->path, "/data/service/el2/100/hmdfs/cache/cloud_cache");
    target = findTarget(CACHE_NON_ACCOUNT_INDEX);
    ASSERT_NE(target, targets.end());
    EXPECT_EQ(target->path, "/data/service/el2/100/hmdfs/cache/non_account_cache");
    target = findTarget(CLOUD_MANAGER_INDEX);
    ASSERT_NE(target, targets.end());
    EXPECT_EQ(target->path, "/data/service/el2/100/hmdfs/cloudfile_manager");
    GTEST_LOG_(INFO) << "BuildDfsSpaceScanTargetsTest001 End";
}

/*
 * @tc.name: BuildMediaScanTargetsTest001
 * @tc.desc: Verify media wrong-UID scan targets keep the final fixed order.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, BuildMediaScanTargetsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildMediaScanTargetsTest001 Start";
    vector<MediaScanTarget> targets = BuildMediaScanTargets(100);

    ASSERT_EQ(targets.size(), ToIndex(MEDIA_TARGET_COUNT));
    EXPECT_EQ(targets[ToIndex(PHOTO_INDEX)].path, "/data/service/el2/100/hmdfs/account/files/Photo");
    EXPECT_EQ(targets[ToIndex(PHOTO_INDEX)].index, PHOTO_INDEX);
    EXPECT_EQ(targets[ToIndex(THUMBS_INDEX)].path, "/data/service/el2/100/hmdfs/account/files/.thumbs");
    EXPECT_EQ(targets[ToIndex(THUMBS_INDEX)].index, THUMBS_INDEX);
    EXPECT_EQ(targets[ToIndex(EDIT_DATA_INDEX)].path, "/data/service/el2/100/hmdfs/account/files/.editData");
    EXPECT_EQ(targets[ToIndex(EDIT_DATA_INDEX)].index, EDIT_DATA_INDEX);
    GTEST_LOG_(INFO) << "BuildMediaScanTargetsTest001 End";
}

/*
 * @tc.name: BuildDfsSpaceDetailTest001
 * @tc.desc: Verify DFS space detail reports ordered bucket values without paths.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, BuildDfsSpaceDetailTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildDfsSpaceDetailTest001 Start";
    vector<DfsDirStat> stats(ToIndex(DFS_TARGET_COUNT));
    for (uint32_t i = 0; i < ToIndex(DFS_TARGET_COUNT); ++i) {
        stats[i].blocksBytes = static_cast<uint64_t>(i + 1) * BYTE_TO_MB;
    }

    string detail = BuildDfsSpaceDetail(100, stats, 45.0, 7, ScanStopReason::TIMEOUT);

    EXPECT_EQ(detail, "dfsSpace|100|45.00|1.00,2.00,3.00,4.00,5.00,6.00,7.00,8.00|7|1|1");
    EXPECT_EQ(detail.find("/data/"), string::npos);
    EXPECT_EQ(detail.find("="), string::npos);
    GTEST_LOG_(INFO) << "BuildDfsSpaceDetailTest001 End";
}

/*
 * @tc.name: BuildMediaWrongUidDetailTest001
 * @tc.desc: Verify media wrong-UID detail reports ordered space, bucket, omitted, error, and stop values.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, BuildMediaWrongUidDetailTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildMediaWrongUidDetailTest001 Start";
    ReportStats reportStats;
    reportStats.mediaWrongUidTotalBytes = 10 * BYTE_TO_MB;
    reportStats.mediaWrongUidStats[ToIndex(PHOTO_INDEX)].blocksBytes = 1 * BYTE_TO_MB;
    reportStats.mediaWrongUidStats[ToIndex(THUMBS_INDEX)].blocksBytes = 2 * BYTE_TO_MB;
    reportStats.mediaWrongUidStats[ToIndex(EDIT_DATA_INDEX)].blocksBytes = 3 * BYTE_TO_MB;
    reportStats.mediaWrongUidStats[ToIndex(PHOTO_INDEX)].badBuckets = "1,2";
    reportStats.mediaWrongUidStats[ToIndex(THUMBS_INDEX)].badBuckets = "3";
    reportStats.mediaWrongUidStats[ToIndex(EDIT_DATA_INDEX)].badBuckets = "4";
    reportStats.mediaWrongUidStats[ToIndex(PHOTO_INDEX)].omittedBuckets = 5;
    reportStats.mediaWrongUidStats[ToIndex(THUMBS_INDEX)].omittedBuckets = 6;
    reportStats.mediaWrongUidStats[ToIndex(EDIT_DATA_INDEX)].omittedBuckets = 7;
    reportStats.mediaWrongUidDirs = 8;
    reportStats.mediaWrongUidErrorMask = 9;
    reportStats.stopped = ScanStopReason::CONDITION_CHANGED;

    string detail = BuildMediaWrongUidDetail(100, reportStats);

    EXPECT_EQ(detail, "mediaWrongUid|100|10.00|1.00,2.00,3.00|8|1,2;3;4|18|9|1|2");
    EXPECT_EQ(detail.find("/data/"), string::npos);
    GTEST_LOG_(INFO) << "BuildMediaWrongUidDetailTest001 End";
}

/*
 * @tc.name: BytesToMbTest001
 * @tc.desc: Verify that BytesToMb returns the exact double MB value and preserves sub-MB precision.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, BytesToMbTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BytesToMbTest001 Start";

    // Whole-MB byte counts are converted without truncation.
    EXPECT_DOUBLE_EQ(BytesToMb(0), 0.0);
    EXPECT_DOUBLE_EQ(BytesToMb(BYTE_TO_MB), 1.0);
    EXPECT_DOUBLE_EQ(BytesToMb(2 * BYTE_TO_MB), 2.0);
    EXPECT_DOUBLE_EQ(BytesToMb(45 * BYTE_TO_MB), 45.0);

    // Sub-MB precision survives the conversion: 1.5 MB should stay 1.5, not 1.
    EXPECT_DOUBLE_EQ(BytesToMb(BYTE_TO_MB + BYTE_TO_MB / 2), 1.5);

    // Right below the integer-MB boundary should yield a value strictly between 0 and 1.
    double justUnder = BytesToMb(BYTE_TO_MB - 1);
    EXPECT_GT(justUnder, 0.0);
    EXPECT_LT(justUnder, 1.0);
    GTEST_LOG_(INFO) << "BytesToMbTest001 End";
}

/*
 * @tc.name: BuildDfsSpaceDetailTest002
 * @tc.desc: Verify that BuildDfsSpaceDetail formats each MB field with exactly two digits after
 *           the decimal point, including fractional values produced by sub-MB byte counts.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, BuildDfsSpaceDetailTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildDfsSpaceDetailTest002 Start";
    vector<DfsDirStat> stats(ToIndex(DFS_TARGET_COUNT));
    // 1.50 MB, 0.25 MB, 1.00 MB, 0.00 MB, 0.99 MB (rounded from 0.99x MB),
    // 2.50 MB, 0.12 MB (rounded from 0.12x MB), 0.01 MB (very small).
    stats[ToIndex(PUBLIC_CLOUDFILE_INDEX)].blocksBytes = BYTE_TO_MB + BYTE_TO_MB / 2;
    stats[ToIndex(PUBLIC_CLOUDKIT_INDEX)].blocksBytes = BYTE_TO_MB / 4;
    stats[ToIndex(PUBLIC_CLOUD_FILE_SYNC_DB_INDEX)].blocksBytes = 1 * BYTE_TO_MB;
    stats[ToIndex(CLOUD_INDEX)].blocksBytes = 0;
    stats[ToIndex(CACHE_ACCOUNT_INDEX)].blocksBytes = BYTE_TO_MB - (BYTE_TO_MB / 100);
    stats[ToIndex(CACHE_CLOUD_INDEX)].blocksBytes = 2 * BYTE_TO_MB + BYTE_TO_MB / 2;
    stats[ToIndex(CACHE_NON_ACCOUNT_INDEX)].blocksBytes = BYTE_TO_MB / 8;
    stats[ToIndex(CLOUD_MANAGER_INDEX)].blocksBytes = BYTE_TO_MB / 100;

    // Total = 1.5 + 0.25 + 1.0 + 0 + 0.99 + 2.5 + 0.125 + 0.01 = 6.375 (caller-supplied).
    string detail = BuildDfsSpaceDetail(100, stats, 6.375, 7, ScanStopReason::NOT_STOPPED);

    EXPECT_EQ(detail, "dfsSpace|100|6.38|1.50,0.25,1.00,0.00,0.99,2.50,0.12,0.01|7|0|0");
    // Counting sanity: 8 per-target MB fields delimited by exactly 7 commas between the third and fourth '|'.
    size_t firstListBar = detail.find('|', detail.find('|', detail.find('|') + 1) + 1) + 1;
    size_t lastListBar = detail.find('|', firstListBar);
    std::string perTargetFields = detail.substr(firstListBar, lastListBar - firstListBar);
    EXPECT_EQ(std::count(perTargetFields.begin(), perTargetFields.end(), ','), ToIndex(DFS_TARGET_COUNT) - 1);
    EXPECT_EQ(perTargetFields.find('e'), std::string::npos);
    EXPECT_EQ(perTargetFields.find('E'), std::string::npos);
    GTEST_LOG_(INFO) << "BuildDfsSpaceDetailTest002 End";
}

/*
 * @tc.name: BuildMediaWrongUidDetailTest002
 * @tc.desc: Verify that BuildMediaWrongUidDetail formats total and per-target MB fields with two decimals.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, BuildMediaWrongUidDetailTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "BuildMediaWrongUidDetailTest002 Start";
    ReportStats reportStats;
    // Fractional byte counts: 0.5 MB, 1.25 MB, 0.01 MB (very small).
    reportStats.mediaWrongUidTotalBytes = (BYTE_TO_MB / 2) + (5 * BYTE_TO_MB / 4) + (BYTE_TO_MB / 100);
    reportStats.mediaWrongUidStats[ToIndex(PHOTO_INDEX)].blocksBytes = BYTE_TO_MB / 2;
    reportStats.mediaWrongUidStats[ToIndex(THUMBS_INDEX)].blocksBytes = 5 * BYTE_TO_MB / 4;
    reportStats.mediaWrongUidStats[ToIndex(EDIT_DATA_INDEX)].blocksBytes = BYTE_TO_MB / 100;
    reportStats.mediaWrongUidStats[ToIndex(PHOTO_INDEX)].badBuckets = "100";
    reportStats.mediaWrongUidStats[ToIndex(THUMBS_INDEX)].badBuckets = "200";
    reportStats.mediaWrongUidStats[ToIndex(EDIT_DATA_INDEX)].badBuckets = "300";
    reportStats.mediaWrongUidDirs = 2;
    reportStats.mediaWrongUidErrorMask = 5;
    reportStats.stopped = ScanStopReason::TIMEOUT;

    string detail = BuildMediaWrongUidDetail(100, reportStats);

    // 0.5 + 1.25 + 0.01 = 1.76 MB total (rounded to two decimals).
    EXPECT_EQ(detail, "mediaWrongUid|100|1.76|0.50,1.25,0.01|2|100;200;300|0|5|1|1");
    // All three per-target MB fields must use two decimals.
    EXPECT_NE(detail.find("0.50"), string::npos);
    EXPECT_NE(detail.find("1.25"), string::npos);
    EXPECT_NE(detail.find("0.01"), string::npos);
    GTEST_LOG_(INFO) << "BuildMediaWrongUidDetailTest002 End";
}

/*
 * @tc.name: CollectDfsSpaceStatsTest001
 * @tc.desc: Verify that a stopped scan still merges the current target statistics before reporting.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, CollectDfsSpaceStatsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest001 Start";
    for (int32_t i = 0; i < 32; ++i) {
        CreateFile(TEST_ROOT + "/counted_file_" + to_string(i));
        TryMakeDfsOwned(TEST_ROOT + "/counted_file_" + to_string(i));
    }
    std::system(("mkdir -p " + TEST_ROOT + "/next_dir").c_str());
    TryMakeDfsOwned(TEST_ROOT);
    TryMakeDfsOwned(TEST_ROOT + "/next_dir");
    vector<ScanTarget> targets = {ScanTarget {TEST_ROOT, CACHE_ACCOUNT_INDEX, ErrorBit(CACHE_ACCOUNT_INDEX)}};
    ReportStats reportStats;
    g_mockTimeout = true;
    g_mockTimeoutAfter = 10;

    int32_t ret = CollectDfsSpaceStats(targets, reportStats, g_mockStopScan);

    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(IsScanStopped(reportStats.stopped));
    EXPECT_EQ(reportStats.stopped, ScanStopReason::TIMEOUT);
    if (geteuid() == DFS_UID || TryMakeDfsOwned(TEST_ROOT)) {
        EXPECT_GT(reportStats.dfsSpaceTotalBytes, 0U);
        EXPECT_GT(reportStats.dfsStats[ToIndex(CACHE_ACCOUNT_INDEX)].blocksBytes, 0U);
    }
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest001 End";
}

/*
 * @tc.name: CollectDfsSpaceStatsTest002
 * @tc.desc: Verify that readdir failure is propagated to DFS report error mask.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, CollectDfsSpaceStatsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest002 Start";
    g_mockReadDirFail = true;
    g_mockReadDirFailAfter = 1;
    vector<ScanTarget> targets = {ScanTarget {TEST_ROOT, CACHE_ACCOUNT_INDEX, ErrorBit(CACHE_ACCOUNT_INDEX)}};
    ReportStats reportStats;

    int32_t ret = CollectDfsSpaceStats(targets, reportStats, g_mockStopScan);

    EXPECT_EQ(ret, E_OK);
    EXPECT_NE(reportStats.dfsSpaceErrorMask & ErrorBit(CACHE_ACCOUNT_INDEX), 0U);
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest002 End";
}

/*
 * @tc.name: CollectDfsSpaceStatsTest003
 * @tc.desc: Verify that out-of-range scan target index is reported without writing out of bounds.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, CollectDfsSpaceStatsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest003 Start";
    constexpr uint32_t invalidIndexErrorBit = 1U << 31;
    vector<ScanTarget> targets = {
        ScanTarget {TEST_ROOT, static_cast<DfsTargetIndex>(DFS_TARGET_COUNT), invalidIndexErrorBit}
    };
    ReportStats reportStats;

    int32_t ret = CollectDfsSpaceStats(targets, reportStats, g_mockStopScan);

    EXPECT_EQ(ret, E_OK);
    EXPECT_NE(reportStats.dfsSpaceErrorMask & invalidIndexErrorBit, 0U);
    EXPECT_EQ(reportStats.dfsSpaceTotalBytes, 0U);
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest003 End";
}

/*
 * @tc.name: CollectDfsSpaceStatsTest004
 * @tc.desc: Verify that a pre-stopped scan exits before scanning targets.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, CollectDfsSpaceStatsTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest004 Start";
    vector<ScanTarget> targets = {ScanTarget {TEST_ROOT, CACHE_ACCOUNT_INDEX, ErrorBit(CACHE_ACCOUNT_INDEX)}};
    ReportStats reportStats;
    OnScanTimeout(g_mockStopScan);

    int32_t ret = CollectDfsSpaceStats(targets, reportStats, g_mockStopScan);

    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(IsScanStopped(reportStats.stopped));
    EXPECT_EQ(reportStats.dfsSpaceTotalBytes, 0U);
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest004 End";
}

/*
 * @tc.name: CollectDfsSpaceStatsTest005
 * @tc.desc: Verify that DFS scan stops when runtime condition changes during readdir.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, CollectDfsSpaceStatsTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest005 Start";
    CreateFile(TEST_ROOT + "/file_entry");
    vector<ScanTarget> targets = {ScanTarget {TEST_ROOT, CACHE_ACCOUNT_INDEX, ErrorBit(CACHE_ACCOUNT_INDEX)}};
    ReportStats reportStats;
    g_mockScreenOnInReadDir = true;
    g_mockScreenOnAfterReadDir = 1;

    int32_t ret = CollectDfsSpaceStats(targets, reportStats, g_mockStopScan);

    EXPECT_EQ(ret, E_STOP);
    EXPECT_EQ(reportStats.stopped, ScanStopReason::CONDITION_CHANGED);
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest005 End";
}

/*
 * @tc.name: CollectDfsSpaceStatsTest006
 * @tc.desc: Verify DFS scan stops and reports depth limit when recursion exceeds ten levels.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, CollectDfsSpaceStatsTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest006 Start";
    string current = TEST_ROOT;
    for (int32_t i = 0; i < 11; ++i) {
        current = current + "/dir_" + to_string(i);
        std::system(("mkdir -p " + current).c_str());
    }
    vector<ScanTarget> targets = {ScanTarget {TEST_ROOT, CACHE_ACCOUNT_INDEX, ErrorBit(CACHE_ACCOUNT_INDEX)}};
    ReportStats reportStats;

    int32_t ret = CollectDfsSpaceStats(targets, reportStats, g_mockStopScan);

    EXPECT_EQ(ret, E_STOP);
    EXPECT_EQ(reportStats.stopped, ScanStopReason::DEPTH_LIMIT);
    string detail = BuildDfsSpaceDetail(100, reportStats.dfsStats, BytesToMb(reportStats.dfsSpaceTotalBytes),
        reportStats.dfsSpaceErrorMask, reportStats.stopped);
    EXPECT_NE(detail.find("|1|3"), string::npos);
    GTEST_LOG_(INFO) << "CollectDfsSpaceStatsTest006 End";
}

/*
 * @tc.name: ScanMediaWrongUidDirsTest001
 * @tc.desc: Verify that media parent readdir failure is counted in scan errors for reporting.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanMediaWrongUidDirsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest001 Start";
    g_mockReadDirFail = true;
    g_mockReadDirFailAfter = 1;

    MediaWrongUidStat stat = ScanMediaWrongUidDirs(TEST_ROOT, g_mockStopScan);

    EXPECT_GT(stat.errors, 0U);
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest001 End";
}

/*
 * @tc.name: ScanMediaWrongUidDirsTest002
 * @tc.desc: Verify that an abnormal media directory is reported for wrong-UID statistics.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanMediaWrongUidDirsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest002 Start";
    if (geteuid() == MEDIA_UID) {
        GTEST_SKIP() << "Current uid equals MEDIA_UID; wrong-UID directory fixture cannot be constructed reliably.";
    }
    std::system(("mkdir -p " + TEST_ROOT + "/123").c_str());
    // Only dfs-uid-owned buckets are descended into, so the bucket itself must be dfs-owned.
    if (geteuid() != DFS_UID && !TryMakeDfsOwned(TEST_ROOT + "/123")) {
        GTEST_SKIP() << "Cannot construct dfs-owned bucket fixture.";
    }

    MediaWrongUidStat stat = ScanMediaWrongUidDirs(TEST_ROOT, g_mockStopScan);

    EXPECT_GT(stat.badFirstLevelDirs, 0U);
    EXPECT_EQ(stat.badBuckets, "123");
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest002 End";
}

/*
 * @tc.name: ScanMediaWrongUidDirsTest003
 * @tc.desc: Verify that media wrong-UID space only counts files owned by dfs uid.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanMediaWrongUidDirsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest003 Start";
    if (geteuid() == MEDIA_UID) {
        GTEST_SKIP() << "Current uid equals MEDIA_UID; wrong-UID directory fixture cannot be constructed reliably.";
    }
    std::system(("mkdir -p " + TEST_ROOT + "/123").c_str());
    // Only dfs-uid-owned buckets are descended into, so the bucket itself must be dfs-owned.
    if (geteuid() != DFS_UID && !TryMakeDfsOwned(TEST_ROOT + "/123")) {
        GTEST_SKIP() << "Cannot construct dfs-owned bucket fixture.";
    }
    string nonDfsFile = TEST_ROOT + "/123/non_dfs_file";
    string dfsFile = TEST_ROOT + "/123/dfs_file";
    CreateFile(nonDfsFile);
    CreateFile(dfsFile);
    if (geteuid() != DFS_UID && !TryMakeDfsOwned(dfsFile)) {
        GTEST_SKIP() << "Cannot construct dfs-owned file fixture.";
    }
    struct stat dfsFileStat {};
    ASSERT_EQ(lstat(dfsFile.c_str(), &dfsFileStat), 0);
    uint64_t expectedBlocks = 0;
    if (dfsFileStat.st_blocks > 0) {
        expectedBlocks = static_cast<uint64_t>(dfsFileStat.st_blocks) * BLOCK_SIZE;
    }

    MediaWrongUidStat stat = ScanMediaWrongUidDirs(TEST_ROOT, g_mockStopScan);

    EXPECT_EQ(stat.blocksBytes, expectedBlocks);
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest003 End";
}

/*
 * @tc.name: ScanDfsUidTreeTest003
 * @tc.desc: Verify media subtree scan records missing roots and regular-root opendir failures.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanDfsUidTreeTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest003 Start";
    MediaWrongUidStat missingStat;
    MediaWrongUidStat fileStat;
    string fileRoot = TEST_ROOT + "/regular_root";
    CreateFile(fileRoot);

    ScanDfsUidTree(TEST_ROOT + "/missing_root", missingStat, g_mockStopScan);
    ScanDfsUidTree(fileRoot, fileStat, g_mockStopScan);

    EXPECT_GT(missingStat.errors, 0U);
    EXPECT_EQ(fileStat.stopped, ScanStopReason::NOT_STOPPED);
    EXPECT_GT(fileStat.errors, 0U);
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest003 End";
}

/*
 * @tc.name: ScanDfsUidTreeTest004
 * @tc.desc: Verify common DFS UID tree scan records readdir failures.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanDfsUidTreeTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest004 Start";
    CreateFile(TEST_ROOT + "/file_entry");
    MediaWrongUidStat uidStat;
    g_mockReadDirFail = true;
    g_mockReadDirFailAfter = 1;

    ScanDfsUidTree(TEST_ROOT, uidStat, g_mockStopScan);

    EXPECT_GT(uidStat.errors, 0U);
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest004 End";
}

/*
 * @tc.name: ScanDfsUidTreeTest006
 * @tc.desc: Verify common DFS UID tree scan records child lstat failures.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanDfsUidTreeTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest006 Start";
    string childPath = TEST_ROOT + "/lost_entry";
    CreateFile(childPath);
    g_mockLstatFailPath = childPath;
    DfsDirStat dirStat;

    ScanDfsUidTree(TEST_ROOT, dirStat, g_mockStopScan);

    EXPECT_GT(dirStat.errors, 0U);
    EXPECT_EQ(dirStat.stopped, ScanStopReason::NOT_STOPPED);
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest006 End";
}

/*
 * @tc.name: ScanDfsUidTreeTest005
 * @tc.desc: Verify media wrong-UID subtree stops before opening pending root directory.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanDfsUidTreeTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest005 Start";
    MediaWrongUidStat uidStat;
    OnScanTimeout(g_mockStopScan);

    ScanDfsUidTree(TEST_ROOT, uidStat, g_mockStopScan);

    EXPECT_TRUE(IsScanStopped(uidStat.stopped));
    EXPECT_EQ(g_mockOpenDirCalls, 0);
    GTEST_LOG_(INFO) << "ScanDfsUidTreeTest005 End";
}

/*
 * @tc.name: ScanMediaWrongUidDirsTest004
 * @tc.desc: Verify media wrong-UID parent reports opendir failure.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanMediaWrongUidDirsTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest004 Start";
    g_mockOpenDirFailPath = TEST_ROOT;

    MediaWrongUidStat stat = ScanMediaWrongUidDirs(TEST_ROOT, g_mockStopScan);

    EXPECT_GT(g_mockOpenDirCalls, 0);
    EXPECT_GT(stat.errors, 0U);
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest004 End";
}

/*
 * @tc.name: ScanMediaWrongUidDirsTest005
 * @tc.desc: Verify media parent scan records first-level child lstat failures.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanMediaWrongUidDirsTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest005 Start";
    string childPath = TEST_ROOT + "/lost_bucket";
    std::system(("mkdir -p " + childPath).c_str());
    g_mockLstatFailPath = childPath;

    MediaWrongUidStat stat = ScanMediaWrongUidDirs(TEST_ROOT, g_mockStopScan);

    EXPECT_GT(stat.errors, 0U);
    EXPECT_EQ(stat.badFirstLevelDirs, 0U);
    EXPECT_TRUE(stat.badBuckets.empty());
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest005 End";
}

/*
 * @tc.name: ScanMediaWrongUidDirsTest006
 * @tc.desc: Verify that a non-dfs-uid bucket is not descended into even when it contains dfs-uid files.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ScanMediaWrongUidDirsTest006, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest006 Start";
    if (geteuid() == DFS_UID) {
        GTEST_SKIP() << "Current uid equals DFS_UID; non-dfs bucket fixture cannot be constructed reliably.";
    }
    // Bucket "456" stays owned by the test runner (a non-dfs, non-media uid).
    std::system(("mkdir -p " + TEST_ROOT + "/456").c_str());
    string dfsFile = TEST_ROOT + "/456/dfs_file";
    CreateFile(dfsFile);
    // Best-effort: even if a dfs-uid file sits inside, a non-dfs bucket must not be scanned.
    TryMakeDfsOwned(dfsFile);

    MediaWrongUidStat stat = ScanMediaWrongUidDirs(TEST_ROOT, g_mockStopScan);

    EXPECT_EQ(stat.badFirstLevelDirs, 0U);
    EXPECT_TRUE(stat.badBuckets.empty());
    EXPECT_EQ(stat.blocksBytes, 0U);
    GTEST_LOG_(INFO) << "ScanMediaWrongUidDirsTest006 End";
}

/*
 * @tc.name: CollectMediaWrongUidStatsTest001
 * @tc.desc: Verify that a stopped media scan still merges current target statistics before reporting.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, CollectMediaWrongUidStatsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectMediaWrongUidStatsTest001 Start";
    if (geteuid() == MEDIA_UID) {
        GTEST_SKIP() << "Current uid equals MEDIA_UID; wrong-UID fixture cannot be constructed reliably.";
    }
    std::system(("mkdir -p " + TEST_ROOT + "/123").c_str());
    // Only dfs-uid-owned buckets are descended into, so the bucket itself must be dfs-owned.
    if (geteuid() != DFS_UID && !TryMakeDfsOwned(TEST_ROOT + "/123")) {
        GTEST_SKIP() << "Cannot construct dfs-owned bucket fixture.";
    }
    for (int32_t i = 0; i < 32; ++i) {
        CreateFile(TEST_ROOT + "/123/file_" + to_string(i));
        TryMakeDfsOwned(TEST_ROOT + "/123/file_" + to_string(i));
    }
    if (geteuid() != DFS_UID && !TryMakeDfsOwned(TEST_ROOT + "/123/file_0")) {
        GTEST_SKIP() << "Cannot construct dfs-owned file fixture.";
    }
    vector<MediaScanTarget> targets = {MediaScanTarget {TEST_ROOT, PHOTO_INDEX, ErrorBit(PHOTO_INDEX)}};
    ReportStats reportStats;
    g_mockTimeout = true;
    g_mockTimeoutAfter = 10;

    int32_t ret = CollectMediaWrongUidStats(targets, reportStats, g_mockStopScan);

    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(IsScanStopped(reportStats.stopped));
    EXPECT_EQ(reportStats.stopped, ScanStopReason::TIMEOUT);
    EXPECT_GT(reportStats.mediaWrongUidTotalBytes, 0U);
    EXPECT_GT(reportStats.mediaWrongUidDirs, 0U);
    EXPECT_GT(reportStats.mediaWrongUidStats[ToIndex(PHOTO_INDEX)].blocksBytes, 0U);
    GTEST_LOG_(INFO) << "CollectMediaWrongUidStatsTest001 End";
}

/*
 * @tc.name: CollectMediaWrongUidStatsTest002
 * @tc.desc: Verify that readdir failure is propagated to media wrong-UID report error mask.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, CollectMediaWrongUidStatsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectMediaWrongUidStatsTest002 Start";
    g_mockReadDirFail = true;
    g_mockReadDirFailAfter = 1;
    vector<MediaScanTarget> targets = {MediaScanTarget {TEST_ROOT, PHOTO_INDEX, ErrorBit(PHOTO_INDEX)}};
    ReportStats reportStats;

    int32_t ret = CollectMediaWrongUidStats(targets, reportStats, g_mockStopScan);

    EXPECT_EQ(ret, E_OK);
    EXPECT_NE(reportStats.mediaWrongUidErrorMask & ErrorBit(PHOTO_INDEX), 0U);
    GTEST_LOG_(INFO) << "CollectMediaWrongUidStatsTest002 End";
}

/*
 * @tc.name: CollectMediaWrongUidStatsTest003
 * @tc.desc: Verify that out-of-range media target index is reported without writing out of bounds.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, CollectMediaWrongUidStatsTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectMediaWrongUidStatsTest003 Start";
    constexpr uint32_t invalidIndexErrorBit = 1U << 31;
    vector<MediaScanTarget> targets = {
        MediaScanTarget {TEST_ROOT, static_cast<MediaTargetIndex>(MEDIA_TARGET_COUNT), invalidIndexErrorBit}
    };
    ReportStats reportStats;

    int32_t ret = CollectMediaWrongUidStats(targets, reportStats, g_mockStopScan);

    EXPECT_EQ(ret, E_OK);
    EXPECT_NE(reportStats.mediaWrongUidErrorMask & invalidIndexErrorBit, 0U);
    EXPECT_EQ(reportStats.mediaWrongUidTotalBytes, 0U);
    GTEST_LOG_(INFO) << "CollectMediaWrongUidStatsTest003 End";
}

/*
 * @tc.name: CollectMediaWrongUidStatsTest004
 * @tc.desc: Verify that a pre-stopped media scan exits before scanning targets.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, CollectMediaWrongUidStatsTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectMediaWrongUidStatsTest004 Start";
    vector<MediaScanTarget> targets = {MediaScanTarget {TEST_ROOT, PHOTO_INDEX, ErrorBit(PHOTO_INDEX)}};
    ReportStats reportStats;
    OnScanTimeout(g_mockStopScan);

    int32_t ret = CollectMediaWrongUidStats(targets, reportStats, g_mockStopScan);

    EXPECT_EQ(ret, E_OK);
    EXPECT_TRUE(IsScanStopped(reportStats.stopped));
    EXPECT_EQ(reportStats.mediaWrongUidTotalBytes, 0U);
    GTEST_LOG_(INFO) << "CollectMediaWrongUidStatsTest004 End";
}

/*
 * @tc.name: CollectMediaWrongUidStatsTest005
 * @tc.desc: Verify that media scan stops when runtime condition changes during readdir.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, CollectMediaWrongUidStatsTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "CollectMediaWrongUidStatsTest005 Start";
    std::system(("mkdir -p " + TEST_ROOT + "/123").c_str());
    vector<MediaScanTarget> targets = {MediaScanTarget {TEST_ROOT, PHOTO_INDEX, ErrorBit(PHOTO_INDEX)}};
    ReportStats reportStats;
    g_mockScreenOnInReadDir = true;
    g_mockScreenOnAfterReadDir = 1;

    int32_t ret = CollectMediaWrongUidStats(targets, reportStats, g_mockStopScan);

    EXPECT_EQ(ret, E_STOP);
    EXPECT_EQ(reportStats.stopped, ScanStopReason::CONDITION_CHANGED);
    GTEST_LOG_(INFO) << "CollectMediaWrongUidStatsTest005 End";
}

/*
 * @tc.name: ShouldStopScanTest001
 * @tc.desc: Verify that timeout callback stops scanning.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, ShouldStopScanTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ShouldStopScanTest001 Start";

    EXPECT_FALSE(ShouldStopScan(g_mockStopScan));
    OnScanTimeout(g_mockStopScan);
    EXPECT_TRUE(ShouldStopScan(g_mockStopScan));
    GTEST_LOG_(INFO) << "ShouldStopScanTest001 End";
}

/*
 * @tc.name: IsDfsSpaceReportConditionSatisfiedTest001
 * @tc.desc: Verify DFS space report requires screen off, charging, enough battery, and normal thermal level.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, IsDfsSpaceReportConditionSatisfiedTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsDfsSpaceReportConditionSatisfiedTest001 Start";
    SetDfsReportCondition(false, true, PowerMgr::ThermalLevel::NORMAL);
    EXPECT_TRUE(IsDfsSpaceReportConditionSatisfied());

    SetDfsReportCondition(true, true, PowerMgr::ThermalLevel::NORMAL);
    EXPECT_FALSE(IsDfsSpaceReportConditionSatisfied());

    SetDfsReportCondition(false, false, PowerMgr::ThermalLevel::NORMAL);
    EXPECT_FALSE(IsDfsSpaceReportConditionSatisfied());

    SetDfsReportCondition(false, true, PowerMgr::ThermalLevel::NORMAL, 19);
    EXPECT_FALSE(IsDfsSpaceReportConditionSatisfied());

    SetDfsReportCondition(false, true, PowerMgr::ThermalLevel::NORMAL, 20);
    EXPECT_TRUE(IsDfsSpaceReportConditionSatisfied());

    SetDfsReportCondition(false, true, PowerMgr::ThermalLevel::WARM);
    EXPECT_FALSE(IsDfsSpaceReportConditionSatisfied());
    GTEST_LOG_(INFO) << "IsDfsSpaceReportConditionSatisfiedTest001 End";
}

/*
 * @tc.name: GetScanStopReasonTest001
 * @tc.desc: Verify scan stop reason combines timeout and device condition checks.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, GetScanStopReasonTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetScanStopReasonTest001 Start";
    SetDfsReportCondition(false, true, PowerMgr::ThermalLevel::NORMAL, 100);
    EXPECT_EQ(GetScanStopReason(g_mockStopScan), ScanStopReason::NOT_STOPPED);

    OnScanTimeout(g_mockStopScan);
    EXPECT_EQ(GetScanStopReason(g_mockStopScan), ScanStopReason::TIMEOUT);

    ResetStopScan(g_mockStopScan);
    SetDfsReportCondition(false, true, PowerMgr::ThermalLevel::NORMAL, 19);
    EXPECT_EQ(GetScanStopReason(g_mockStopScan), ScanStopReason::CONDITION_CHANGED);
    GTEST_LOG_(INFO) << "GetScanStopReasonTest001 End";
}

/*
 * @tc.name: UnregisterScanTimerTest001
 * @tc.desc: Verify unregister timer handles null and zero timer holders.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, UnregisterScanTimerTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "UnregisterScanTimerTest001 Start";
    auto timerId = make_shared<atomic_uint>(0);

    UnregisterScanTimer(nullptr);
    UnregisterScanTimer(timerId);

    EXPECT_EQ(timerId->load(), 0U);
    GTEST_LOG_(INFO) << "UnregisterScanTimerTest001 End";
}

/*
 * @tc.name: OnScanTimeoutTest001
 * @tc.desc: Verify timeout callback stops scan.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, OnScanTimeoutTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "OnScanTimeoutTest001 Start";

    OnScanTimeout(g_mockStopScan);

    EXPECT_TRUE(ShouldStopScan(g_mockStopScan));
    GTEST_LOG_(INFO) << "OnScanTimeoutTest001 End";
}

/*
 * @tc.name: HandleScanTimerRegisterResultTest001
 * @tc.desc: Verify that timer registration failure marks report stats as stopped.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, HandleScanTimerRegisterResultTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleScanTimerRegisterResultTest001 Start";
    ReportStats reportStats;

    bool timerReady = HandleScanTimerRegisterResult(0, reportStats);

    EXPECT_FALSE(timerReady);
    EXPECT_TRUE(IsScanStopped(reportStats.stopped));
    GTEST_LOG_(INFO) << "HandleScanTimerRegisterResultTest001 End";
}

/*
 * @tc.name: HandleScanTimerRegisterResultTest002
 * @tc.desc: Verify successful timer registration keeps scan active.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, HandleScanTimerRegisterResultTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "HandleScanTimerRegisterResultTest002 Start";
    ReportStats reportStats;

    bool timerReady = HandleScanTimerRegisterResult(1, reportStats);

    EXPECT_TRUE(timerReady);
    EXPECT_EQ(reportStats.stopped, ScanStopReason::NOT_STOPPED);
    GTEST_LOG_(INFO) << "HandleScanTimerRegisterResultTest002 End";
}

/*
 * @tc.name: ReportStatsTest001
 * @tc.desc: Verify report helpers return fault report failures.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ReportStatsTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReportStatsTest001 Start";
    ReportStats reportStats;
    g_mockFaultReportFailFirst = true;

    int32_t dfsRet = ReportDfsSpaceStats(0, HDC_BUNDLE_NAME, reportStats);
    int32_t mediaRet = ReportMediaWrongUidStats(0, HDC_BUNDLE_NAME, reportStats);

    EXPECT_EQ(dfsRet, E_INVAL_ARG);
    EXPECT_EQ(mediaRet, E_OK);
    EXPECT_EQ(g_mockFaultReportCalls, 2);
    GTEST_LOG_(INFO) << "ReportStatsTest001 End";
}

/*
 * @tc.name: ReportStatsTest002
 * @tc.desc: Verify media wrong-UID report helper returns fault report failures.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, ReportStatsTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "ReportStatsTest002 Start";
    ReportStats reportStats;
    g_mockFaultReportFailFirst = true;

    int32_t mediaRet = ReportMediaWrongUidStats(0, HDC_BUNDLE_NAME, reportStats);

    EXPECT_EQ(mediaRet, E_INVAL_ARG);
    EXPECT_EQ(g_mockFaultReportCalls, 1);
    GTEST_LOG_(INFO) << "ReportStatsTest002 End";
}

/*
 * @tc.name: RunTaskForBundleTest001
 * @tc.desc: Verify that DfsSpaceReportTask returns E_OK.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, RunTaskForBundleTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 Start";
    DfsSpaceReportTask task(nullptr);

    int32_t ret = task.RunTaskForBundle(0, GALLERY_BUNDLE_NAME);

    EXPECT_EQ(ret, E_OK);
    GTEST_LOG_(INFO) << "RunTaskForBundleTest001 End";
}

/*
 * @tc.name: RunTaskForBundleTest005
 * @tc.desc: Verify DFS space report returns E_OK without scanning or reporting when start condition is not satisfied.
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(DfsSpaceReportTaskTest, RunTaskForBundleTest005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 Start";
    DfsSpaceReportTask task(nullptr);
    SetDfsReportCondition(true, true, PowerMgr::ThermalLevel::NORMAL);

    int32_t ret = task.RunTaskForBundle(0, GALLERY_BUNDLE_NAME);

    EXPECT_EQ(ret, E_OK);
    EXPECT_EQ(g_mockOpenDirCalls, 0);
    EXPECT_EQ(g_mockFaultReportCalls, 0);
    GTEST_LOG_(INFO) << "RunTaskForBundleTest005 End";
}

/*
 * @tc.name: DfsSpaceReportTaskBundleNamesTest001
 * @tc.desc: Verify that DFS space report task runs every three days for Gallery only.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, DfsSpaceReportTaskBundleNamesTest001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "DfsSpaceReportTaskBundleNamesTest001 Start";
    DfsSpaceReportTask task(nullptr);

    EXPECT_EQ(task.bundleNames_.find(HDC_BUNDLE_NAME), task.bundleNames_.end());
    EXPECT_NE(task.bundleNames_.find(GALLERY_BUNDLE_NAME), task.bundleNames_.end());
    EXPECT_EQ(task.intervalTime_, CycleTask::THREE_DAY);
    GTEST_LOG_(INFO) << "DfsSpaceReportTaskBundleNamesTest001 End";
}

/*
 * @tc.name: RunTaskForBundleTest002
 * @tc.desc: Verify that DFS space and media wrong-UID stats are both reported.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, RunTaskForBundleTest002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 Start";
    DfsSpaceReportTask task(nullptr);

    int32_t ret = task.RunTaskForBundle(0, GALLERY_BUNDLE_NAME);

    EXPECT_EQ(ret, E_OK);
    ASSERT_EQ(g_mockFaultReportCalls, 2);
    EXPECT_NE(g_mockFaultReportDetails[0].find("dfsSpace|"), string::npos);
    EXPECT_NE(g_mockFaultReportDetails[1].find("mediaWrongUid|"), string::npos);
    GTEST_LOG_(INFO) << "RunTaskForBundleTest002 End";
}

/*
 * @tc.name: RunTaskForBundleTest003
 * @tc.desc: Verify that missing root directories are still reported as scan failures.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, RunTaskForBundleTest003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 Start";
    string missingRoot = TEST_ROOT + "/missing_root";
    vector<ScanTarget> dfsTargets = {ScanTarget {missingRoot, CACHE_ACCOUNT_INDEX, ErrorBit(CACHE_ACCOUNT_INDEX)}};
    vector<MediaScanTarget> mediaTargets = {MediaScanTarget {missingRoot, PHOTO_INDEX, ErrorBit(PHOTO_INDEX)}};
    ReportStats reportStats;

    int32_t dfsRet = CollectDfsSpaceStats(dfsTargets, reportStats, g_mockStopScan);
    int32_t mediaRet = CollectMediaWrongUidStats(mediaTargets, reportStats, g_mockStopScan);
    int32_t reportDfsRet = ReportDfsSpaceStats(0, HDC_BUNDLE_NAME, reportStats);
    int32_t reportMediaRet = ReportMediaWrongUidStats(0, HDC_BUNDLE_NAME, reportStats);

    EXPECT_EQ(dfsRet, E_OK);
    EXPECT_EQ(mediaRet, E_OK);
    EXPECT_EQ(reportDfsRet, E_OK);
    EXPECT_EQ(reportMediaRet, E_OK);
    ASSERT_EQ(g_mockFaultReportCalls, 2);
    EXPECT_NE(reportStats.dfsSpaceErrorMask & ErrorBit(CACHE_ACCOUNT_INDEX), 0U);
    EXPECT_NE(reportStats.mediaWrongUidErrorMask & ErrorBit(PHOTO_INDEX), 0U);
    EXPECT_NE(g_mockFaultReportDetails[0].find("|" + to_string(ErrorBit(CACHE_ACCOUNT_INDEX)) + "|0"), string::npos);
    EXPECT_NE(g_mockFaultReportDetails[1].find("|1|0"), string::npos);
    GTEST_LOG_(INFO) << "RunTaskForBundleTest003 End";
}

/*
 * @tc.name: RunTaskForBundleTest004
 * @tc.desc: Verify that regular files used as scan roots are reported as scan failures.
 * @tc.type: FUNC
 */
HWTEST_F(DfsSpaceReportTaskTest, RunTaskForBundleTest004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 Start";
    string fileRoot = TEST_ROOT + "/regular_root";
    CreateFile(fileRoot);
    vector<ScanTarget> dfsTargets = {ScanTarget {fileRoot, CACHE_ACCOUNT_INDEX, ErrorBit(CACHE_ACCOUNT_INDEX)}};
    vector<MediaScanTarget> mediaTargets = {MediaScanTarget {fileRoot, PHOTO_INDEX, ErrorBit(PHOTO_INDEX)}};
    ReportStats reportStats;

    int32_t dfsRet = CollectDfsSpaceStats(dfsTargets, reportStats, g_mockStopScan);
    int32_t mediaRet = CollectMediaWrongUidStats(mediaTargets, reportStats, g_mockStopScan);

    EXPECT_EQ(dfsRet, E_OK);
    EXPECT_EQ(mediaRet, E_OK);
    EXPECT_NE(reportStats.dfsSpaceErrorMask & ErrorBit(CACHE_ACCOUNT_INDEX), 0U);
    EXPECT_NE(reportStats.mediaWrongUidErrorMask & ErrorBit(PHOTO_INDEX), 0U);
    GTEST_LOG_(INFO) << "RunTaskForBundleTest004 End";
}
} // namespace OHOS::FileManagement::CloudSync::Test
