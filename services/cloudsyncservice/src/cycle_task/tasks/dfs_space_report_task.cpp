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

#include "dfs_space_report_task.h"

#include <atomic>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <dirent.h>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "cloud_file_fault_event.h"
#include "battery_status.h"
#include "data_sync_const.h"
#include "dfsu_timer.h"
#include "dfs_error.h"
#include "screen_status.h"
#include "system_load.h"
#include "task_state_manager.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

namespace {
constexpr uint64_t BLOCK_SIZE = 512;
constexpr uint64_t BYTE_TO_MB = 1024 * 1024;
constexpr int32_t MB_DECIMAL_PRECISION = 2;
constexpr uint32_t DFS_SPACE_REPORT_TIMEOUT_MS = 10 * 60 * 1000;
constexpr int32_t MIN_BATTERY_CAPACITY = 20;
constexpr uid_t MEDIA_UID = 1008;
constexpr uid_t DFS_UID = 1009;
constexpr size_t MAX_BUCKETS_IN_REPORT = 64;
constexpr uint32_t MAX_SCAN_DEPTH = 10;
const string DATA_SERVICE_EL2 = "/data/service/el2/";
const string PUBLIC_CLOUDFILE = "/data/service/el1/public/cloudfile";
const string PUBLIC_CLOUDKIT = "/data/service/el1/public/cloudkit_service";
const string PUBLIC_CLOUD_FILE_SYNC_DB = "/data/service/el1/public/database/CloudFileSyncService";

enum DfsTargetIndex : uint32_t {
    PUBLIC_CLOUDFILE_INDEX = 0,
    PUBLIC_CLOUDKIT_INDEX,
    PUBLIC_CLOUD_FILE_SYNC_DB_INDEX,
    CLOUD_INDEX,
    CACHE_ACCOUNT_INDEX,
    CACHE_CLOUD_INDEX,
    CACHE_NON_ACCOUNT_INDEX,
    CLOUD_MANAGER_INDEX,
    DFS_TARGET_COUNT
};

enum MediaTargetIndex : uint32_t {
    PHOTO_INDEX = 0,
    THUMBS_INDEX,
    EDIT_DATA_INDEX,
    MEDIA_TARGET_COUNT
};

enum class ScanStopReason {
    NOT_STOPPED,
    TIMEOUT,
    CONDITION_CHANGED,
    DEPTH_LIMIT,
};

struct MediaTargetConfig {
    const char *pathSuffix;
    MediaTargetIndex index;
};

const vector<MediaTargetConfig> MEDIA_TARGET_CONFIGS = {
    MediaTargetConfig {"/Photo", PHOTO_INDEX},
    MediaTargetConfig {"/.thumbs", THUMBS_INDEX},
    MediaTargetConfig {"/.editData", EDIT_DATA_INDEX},
};

static constexpr uint32_t ToIndex(DfsTargetIndex index)
{
    return static_cast<uint32_t>(index);
}

static constexpr uint32_t ToIndex(MediaTargetIndex index)
{
    return static_cast<uint32_t>(index);
}

static constexpr uint32_t ErrorBit(DfsTargetIndex index)
{
    return 1U << ToIndex(index);
}

static constexpr uint32_t ErrorBit(MediaTargetIndex index)
{
    return 1U << ToIndex(index);
}

struct DfsDirStat {
    uint64_t blocksBytes{0};
    uint32_t errors{0};
    ScanStopReason stopped{ScanStopReason::NOT_STOPPED};
};

struct ScanTarget {
    string path;
    DfsTargetIndex index;
    uint32_t errorBit;
};

struct MediaScanTarget {
    string path;
    MediaTargetIndex index;
    uint32_t errorBit;
};

struct MediaWrongUidStat {
    uint64_t blocksBytes{0};
    uint32_t badFirstLevelDirs{0};
    uint32_t errors{0};
    uint32_t omittedBuckets{0};
    ScanStopReason stopped{ScanStopReason::NOT_STOPPED};
    string badBuckets;
};

struct ReportStats {
    ReportStats()
        : dfsStats(ToIndex(DFS_TARGET_COUNT)), mediaWrongUidStats(ToIndex(MEDIA_TARGET_COUNT))
    {
    }

    vector<DfsDirStat> dfsStats;
    vector<MediaWrongUidStat> mediaWrongUidStats;
    uint64_t dfsSpaceTotalBytes{0};
    uint64_t mediaWrongUidTotalBytes{0};
    uint32_t mediaWrongUidDirs{0};
    uint32_t dfsSpaceErrorMask{0};
    uint32_t mediaWrongUidErrorMask{0};
    ScanStopReason stopped{ScanStopReason::NOT_STOPPED};
};

static bool IsDotDir(const char *name)
{
    return name != nullptr && (strcmp(name, ".") == 0 || strcmp(name, "..") == 0);
}

static void AddUint64(uint64_t &value, uint64_t addValue)
{
    if (UINT64_MAX - value < addValue) {
        value = UINT64_MAX;
        return;
    }
    value += addValue;
}

template<typename Stat>
static void AddDfsUidFileBlocks(const struct stat &fileStat, Stat &stat)
{
    if (!S_ISREG(fileStat.st_mode) || fileStat.st_uid != DFS_UID) {
        return;
    }
    uint64_t blocks = 0;
    if (fileStat.st_blocks > 0) {
        blocks = static_cast<uint64_t>(fileStat.st_blocks) * BLOCK_SIZE;
    }
    AddUint64(stat.blocksBytes, blocks);
}

static void ResetStopScan(atomic_bool &stopScan)
{
    stopScan = false;
}

static void OnScanTimeout(atomic_bool &stopScan)
{
    stopScan = true;
    LOGE("dfs space report scan timeout");
}

static bool ShouldStopScan(const atomic_bool &stopScan)
{
    return stopScan.load();
}

static bool IsDfsSpaceReportConditionSatisfied()
{
    return !ScreenStatus::IsScreenOn() && BatteryStatus::IsCharging() &&
        BatteryStatus::GetCapacity() >= MIN_BATTERY_CAPACITY &&
        SystemLoadStatus::IsSystemLoadAllowed(STOPPED_IN_OTHER, PowerMgr::ThermalLevel::NORMAL);
}

static ScanStopReason GetScanStopReason(const atomic_bool &stopScan)
{
    if (ShouldStopScan(stopScan)) {
        return ScanStopReason::TIMEOUT;
    }
    if (!IsDfsSpaceReportConditionSatisfied()) {
        return ScanStopReason::CONDITION_CHANGED;
    }
    return ScanStopReason::NOT_STOPPED;
}

static bool IsScanStopped(ScanStopReason stopped)
{
    return stopped != ScanStopReason::NOT_STOPPED;
}

static int32_t StopReasonToReport(ScanStopReason stopped)
{
    return static_cast<int32_t>(stopped);
}

static void UnregisterScanTimer(const shared_ptr<atomic_uint> &timerId)
{
    if (timerId == nullptr) {
        return;
    }
    uint32_t id = timerId->exchange(0);
    if (id != 0) {
        DfsuTimer::GetInstance().Unregister(id);
    }
}

static void AddBadBucket(const char *bucketName, MediaWrongUidStat &uidStat)
{
    int32_t bucketId = -1;
    if (bucketName == nullptr || !OHOS::StrToInt(bucketName, bucketId) || bucketId < 0) {
        return;
    }
    string bucketIdStr = to_string(bucketId);
    if (uidStat.badBuckets.empty()) {
        uidStat.badBuckets = bucketIdStr;
        return;
    }
    if (uidStat.badBuckets.size() >= MAX_BUCKETS_IN_REPORT) {
        uidStat.omittedBuckets++;
        return;
    }
    uidStat.badBuckets += ",";
    uidStat.badBuckets += bucketIdStr;
}

static string JoinPath(const string &parent, const char *name)
{
    if (parent.empty() || parent.back() == '/') {
        return parent + name;
    }
    return parent + "/" + name;
}

template<typename Stat>
static void ScanDfsUidTree(const string &rootPath, Stat &stat, const atomic_bool &stopScan, uint32_t depth = 0)
{
    ScanStopReason stopReason = GetScanStopReason(stopScan);
    if (IsScanStopped(stopReason)) {
        stat.stopped = stopReason;
        return;
    }
    if (depth > MAX_SCAN_DEPTH) {
        stat.stopped = ScanStopReason::DEPTH_LIMIT;
        LOGE("exceeds deep limit, path:%{public}s", GetAnonyStringStrictly(rootPath).c_str());
        return;
    }

    DIR *dir = opendir(rootPath.c_str());
    if (dir == nullptr) {
        stat.errors++;
        LOGE("opendir failed, path:%{public}s, errno:%{public}d", GetAnonyStringStrictly(rootPath).c_str(), errno);
        return;
    }
    struct dirent *entry = nullptr;
    for (errno = 0; (entry = readdir(dir)) != nullptr; errno = 0) {
        if (IsDotDir(entry->d_name)) {
            continue;
        }
        stopReason = GetScanStopReason(stopScan);
        if (IsScanStopped(stopReason)) {
            stat.stopped = stopReason;
            break;
        }
        string childPath = JoinPath(rootPath, entry->d_name);
        struct stat childStat {};
        if (lstat(childPath.c_str(), &childStat) != 0) {
            stat.errors++;
            LOGE("lstat failed, path:%{public}s, err:%{public}d", GetAnonyStringStrictly(childPath).c_str(), errno);
            continue;
        }
        AddDfsUidFileBlocks(childStat, stat);
        if (S_ISDIR(childStat.st_mode)) {
            ScanDfsUidTree(childPath, stat, stopScan, depth + 1);
            if (IsScanStopped(stat.stopped)) {
                break;
            }
        }
    }
    if (!IsScanStopped(stat.stopped) && errno != 0) {
        stat.errors++;
        LOGE("readdir, path:%{public}s, err:%{public}d", GetAnonyStringStrictly(rootPath).c_str(), errno);
    }
    closedir(dir);
}

static DfsDirStat ScanDir(const string &rootPath, const atomic_bool &stopScan)
{
    DfsDirStat dirStat;
    ScanDfsUidTree(rootPath, dirStat, stopScan);
    return dirStat;
}

static MediaWrongUidStat ScanMediaWrongUidDirs(const string &parentPath, const atomic_bool &stopScan)
{
    MediaWrongUidStat uidStat;
    DIR *dir = opendir(parentPath.c_str());
    if (dir == nullptr) {
        uidStat.errors++;
        LOGE("opendir media parent failed, path:%{public}s, errno:%{public}d",
            GetAnonyStringStrictly(parentPath).c_str(), errno);
        return uidStat;
    }
    struct dirent *entry = nullptr;
    for (errno = 0; (entry = readdir(dir)) != nullptr; errno = 0) {
        if (IsDotDir(entry->d_name)) {
            continue;
        }
        ScanStopReason stopReason = GetScanStopReason(stopScan);
        if (IsScanStopped(stopReason)) {
            uidStat.stopped = stopReason;
            break;
        }
        string childPath = JoinPath(parentPath, entry->d_name);
        struct stat childStat {};
        if (lstat(childPath.c_str(), &childStat) != 0) {
            uidStat.errors++;
            LOGE("lstat media entry failed, path:%{public}s, errno:%{public}d",
                GetAnonyStringStrictly(childPath).c_str(), errno);
            continue;
        }
        // Only buckets owned by the dfs uid are the real anomaly (dfs service leaking into media
        // storage); descend into them and only them, ignoring root/system/other-uid directories.
        if (!S_ISDIR(childStat.st_mode) || childStat.st_uid != DFS_UID) {
            continue;
        }
        uidStat.badFirstLevelDirs++;
        AddBadBucket(entry->d_name, uidStat);
        ScanDfsUidTree(childPath, uidStat, stopScan);
        if (IsScanStopped(uidStat.stopped)) {
            break;
        }
    }
    if (!IsScanStopped(uidStat.stopped) && errno != 0) {
        uidStat.errors++;
        LOGE("readdir media parent failed, path:%{public}s, errno:%{public}d",
            GetAnonyStringStrictly(parentPath).c_str(), errno);
    }
    closedir(dir);
    return uidStat;
}

static double BytesToMb(uint64_t bytes)
{
    return static_cast<double>(bytes) / static_cast<double>(BYTE_TO_MB);
}

static vector<ScanTarget> BuildDfsSpaceScanTargets(int32_t userId)
{
    string hmdfsRoot = DATA_SERVICE_EL2 + to_string(userId) + "/hmdfs";
    return {
        ScanTarget {PUBLIC_CLOUDFILE, PUBLIC_CLOUDFILE_INDEX, ErrorBit(PUBLIC_CLOUDFILE_INDEX)},
        ScanTarget {PUBLIC_CLOUDKIT, PUBLIC_CLOUDKIT_INDEX, ErrorBit(PUBLIC_CLOUDKIT_INDEX)},
        ScanTarget {PUBLIC_CLOUD_FILE_SYNC_DB, PUBLIC_CLOUD_FILE_SYNC_DB_INDEX,
            ErrorBit(PUBLIC_CLOUD_FILE_SYNC_DB_INDEX)},
        ScanTarget {hmdfsRoot + "/cache/account_cache", CACHE_ACCOUNT_INDEX, ErrorBit(CACHE_ACCOUNT_INDEX)},
        ScanTarget {hmdfsRoot + "/cache/cloud_cache", CACHE_CLOUD_INDEX, ErrorBit(CACHE_CLOUD_INDEX)},
        ScanTarget {hmdfsRoot + "/cache/non_account_cache", CACHE_NON_ACCOUNT_INDEX,
            ErrorBit(CACHE_NON_ACCOUNT_INDEX)},
        ScanTarget {hmdfsRoot + "/cloudfile_manager", CLOUD_MANAGER_INDEX, ErrorBit(CLOUD_MANAGER_INDEX)},
    };
}

static vector<MediaScanTarget> BuildMediaScanTargets(int32_t userId)
{
    string filesRoot = DATA_SERVICE_EL2 + to_string(userId) + "/hmdfs/account/files";
    vector<MediaScanTarget> scanTargets;
    scanTargets.reserve(MEDIA_TARGET_CONFIGS.size());
    for (const auto &config : MEDIA_TARGET_CONFIGS) {
        scanTargets.emplace_back(MediaScanTarget {filesRoot + config.pathSuffix, config.index,
            ErrorBit(config.index)});
    }
    return scanTargets;
}

static string BuildDfsSpaceDetail(int32_t userId, const vector<DfsDirStat> &stats, double totalMb,
    uint32_t dfsSpaceErrorMask, ScanStopReason stopped)
{
    ostringstream oss;
    oss << std::fixed << std::setprecision(MB_DECIMAL_PRECISION);
    oss << "dfsSpace|" << userId << "|" << totalMb << "|"
        << BytesToMb(stats[ToIndex(PUBLIC_CLOUDFILE_INDEX)].blocksBytes) << ","
        << BytesToMb(stats[ToIndex(PUBLIC_CLOUDKIT_INDEX)].blocksBytes) << ","
        << BytesToMb(stats[ToIndex(PUBLIC_CLOUD_FILE_SYNC_DB_INDEX)].blocksBytes) << ","
        << BytesToMb(stats[ToIndex(CLOUD_INDEX)].blocksBytes) << ","
        << BytesToMb(stats[ToIndex(CACHE_ACCOUNT_INDEX)].blocksBytes) << ","
        << BytesToMb(stats[ToIndex(CACHE_CLOUD_INDEX)].blocksBytes) << ","
        << BytesToMb(stats[ToIndex(CACHE_NON_ACCOUNT_INDEX)].blocksBytes) << ","
        << BytesToMb(stats[ToIndex(CLOUD_MANAGER_INDEX)].blocksBytes)
        << "|" << dfsSpaceErrorMask
        << "|" << (IsScanStopped(stopped) ? 1 : 0)
        << "|" << StopReasonToReport(stopped);
    return oss.str();
}

static string BuildMediaWrongUidDetail(int32_t userId, const ReportStats &reportStats)
{
    const vector<MediaWrongUidStat> &mediaWrongUidStats = reportStats.mediaWrongUidStats;
    uint32_t omittedBuckets = mediaWrongUidStats[ToIndex(PHOTO_INDEX)].omittedBuckets +
        mediaWrongUidStats[ToIndex(THUMBS_INDEX)].omittedBuckets +
        mediaWrongUidStats[ToIndex(EDIT_DATA_INDEX)].omittedBuckets;
    ostringstream oss;
    oss << std::fixed << std::setprecision(MB_DECIMAL_PRECISION);
    oss << "mediaWrongUid|" << userId << "|" << BytesToMb(reportStats.mediaWrongUidTotalBytes) << "|"
        << BytesToMb(mediaWrongUidStats[ToIndex(PHOTO_INDEX)].blocksBytes) << ","
        << BytesToMb(mediaWrongUidStats[ToIndex(THUMBS_INDEX)].blocksBytes) << ","
        << BytesToMb(mediaWrongUidStats[ToIndex(EDIT_DATA_INDEX)].blocksBytes) << "|"
        << reportStats.mediaWrongUidDirs << "|" << mediaWrongUidStats[ToIndex(PHOTO_INDEX)].badBuckets << ";"
        << mediaWrongUidStats[ToIndex(THUMBS_INDEX)].badBuckets << ";"
        << mediaWrongUidStats[ToIndex(EDIT_DATA_INDEX)].badBuckets << "|"
        << omittedBuckets << "|" << reportStats.mediaWrongUidErrorMask << "|"
        << (IsScanStopped(reportStats.stopped) ? 1 : 0) << "|" << StopReasonToReport(reportStats.stopped);
    return oss.str();
}

static int32_t ReportDfsSpaceDetail(const string &bundleName, const string &detail)
{
    return CLOUD_SYNC_FAULT_REPORT({bundleName,
        CloudFile::FaultScenarioCode::CLOUD_OPTIMMIZE_STORAGE,
        CloudFile::FaultType::TRIGGER_INTERN,
        E_OK,
        detail});
}

static int32_t ReportMediaWrongUidDetail(const string &bundleName, const string &detail)
{
    return CLOUD_SYNC_FAULT_REPORT({bundleName,
        CloudFile::FaultScenarioCode::CLOUD_OPTIMMIZE_STORAGE,
        CloudFile::FaultType::TRIGGER_INTERN,
        E_OK,
        detail});
}

static int32_t CollectDfsSpaceStats(const vector<ScanTarget> &scanTargets, ReportStats &reportStats,
    const atomic_bool &stopScan)
{
    for (const auto &target : scanTargets) {
        ScanStopReason stopReason = GetScanStopReason(stopScan);
        if (IsScanStopped(stopReason)) {
            reportStats.stopped = stopReason;
            return stopReason == ScanStopReason::TIMEOUT ? E_OK : E_STOP;
        }
        uint32_t index = ToIndex(target.index);
        if (index >= reportStats.dfsStats.size()) {
            reportStats.dfsSpaceErrorMask |= target.errorBit;
            LOGE("dfs scan target index out of range, index:%{public}u", index);
            continue;
        }
        reportStats.dfsStats[index] = ScanDir(target.path, stopScan);
        AddUint64(reportStats.dfsSpaceTotalBytes, reportStats.dfsStats[index].blocksBytes);
        if (reportStats.dfsStats[index].errors != 0) {
            reportStats.dfsSpaceErrorMask |= target.errorBit;
        }
        if (IsScanStopped(reportStats.dfsStats[index].stopped)) {
            reportStats.stopped = reportStats.dfsStats[index].stopped;
            return reportStats.stopped == ScanStopReason::TIMEOUT ? E_OK : E_STOP;
        }
    }
    return E_OK;
}

static int32_t CollectMediaWrongUidStats(const vector<MediaScanTarget> &scanTargets, ReportStats &reportStats,
    const atomic_bool &stopScan)
{
    for (const auto &target : scanTargets) {
        ScanStopReason stopReason = GetScanStopReason(stopScan);
        if (IsScanStopped(stopReason)) {
            reportStats.stopped = stopReason;
            return stopReason == ScanStopReason::TIMEOUT ? E_OK : E_STOP;
        }
        uint32_t index = ToIndex(target.index);
        if (index >= reportStats.mediaWrongUidStats.size()) {
            reportStats.mediaWrongUidErrorMask |= target.errorBit;
            LOGE("media wrong uid target index out of range, index:%{public}u", index);
            continue;
        }
        reportStats.mediaWrongUidStats[index] = ScanMediaWrongUidDirs(target.path, stopScan);
        AddUint64(reportStats.mediaWrongUidTotalBytes, reportStats.mediaWrongUidStats[index].blocksBytes);
        reportStats.mediaWrongUidDirs += reportStats.mediaWrongUidStats[index].badFirstLevelDirs;
        if (reportStats.mediaWrongUidStats[index].errors != 0) {
            reportStats.mediaWrongUidErrorMask |= target.errorBit;
        }
        if (IsScanStopped(reportStats.mediaWrongUidStats[index].stopped)) {
            reportStats.stopped = reportStats.mediaWrongUidStats[index].stopped;
            return reportStats.stopped == ScanStopReason::TIMEOUT ? E_OK : E_STOP;
        }
    }
    return E_OK;
}

static int32_t ReportDfsSpaceStats(int32_t userId, const string &bundleName, const ReportStats &reportStats)
{
    double totalMb = BytesToMb(reportStats.dfsSpaceTotalBytes);
    string detail = BuildDfsSpaceDetail(userId, reportStats.dfsStats, totalMb, reportStats.dfsSpaceErrorMask,
        reportStats.stopped);
    int32_t ret = ReportDfsSpaceDetail(bundleName, detail);
    if (ret != E_OK) {
        return ret;
    }
    return E_OK;
}

static int32_t ReportMediaWrongUidStats(int32_t userId, const string &bundleName, const ReportStats &reportStats)
{
    string detail = BuildMediaWrongUidDetail(userId, reportStats);
    int32_t ret = ReportMediaWrongUidDetail(bundleName, detail);
    if (ret != E_OK) {
        return ret;
    }
    return E_OK;
}

static bool HandleScanTimerRegisterResult(uint32_t timerId, ReportStats &reportStats)
{
    if (timerId != 0) {
        return true;
    }
    reportStats.stopped = ScanStopReason::TIMEOUT;
    LOGE("dfs space report timer register failed");
    return false;
}
} // namespace

DfsSpaceReportTask::DfsSpaceReportTask(shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
    : CycleTask("dfs_space_report_task", {GALLERY_BUNDLE_NAME}, THREE_DAY, dataSyncManager)
{
    runTaskForSwitchOff_ = true;
}

int32_t DfsSpaceReportTask::RunTaskForBundle(int32_t userId, string bundleName)
{
    if (!IsDfsSpaceReportConditionSatisfied()) {
        LOGI("dfs space report condition is not satisfied");
        return E_OK;
    }
    TaskStateManager::GetInstance().StartTask(bundleName, TaskType::DFS_SPACE_REPORT_TASK);
    auto stopScan = make_shared<atomic_bool>(false);
    ResetStopScan(*stopScan);
    uint32_t timerId = 0;
    auto timerIdHolder = make_shared<atomic_uint>(0);
    DfsuTimer::GetInstance().Register([stopScan]() { OnScanTimeout(*stopScan); }, timerId,
        DFS_SPACE_REPORT_TIMEOUT_MS);
    timerIdHolder->store(timerId);
    auto scanTargets = BuildDfsSpaceScanTargets(userId);
    auto mediaScanTargets = BuildMediaScanTargets(userId);
    ReportStats reportStats;
    int32_t collectRet = E_OK;
    if (HandleScanTimerRegisterResult(timerId, reportStats)) {
        collectRet = CollectDfsSpaceStats(scanTargets, reportStats, *stopScan);
        if (!IsScanStopped(reportStats.stopped)) {
            collectRet = CollectMediaWrongUidStats(mediaScanTargets, reportStats, *stopScan);
        }
    }
    int32_t ret = ReportDfsSpaceStats(userId, bundleName, reportStats);
    if (ret != E_OK) {
        LOGE("report dfs space stats failed, ret:%{public}d", ret);
    }
    ret = ReportMediaWrongUidStats(userId, bundleName, reportStats);
    if (ret != E_OK) {
        LOGE("report media wrong uid stats failed, ret:%{public}d", ret);
    }
    UnregisterScanTimer(timerIdHolder);
    TaskStateManager::GetInstance().CompleteTask(bundleName, TaskType::DFS_SPACE_REPORT_TASK);
    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
