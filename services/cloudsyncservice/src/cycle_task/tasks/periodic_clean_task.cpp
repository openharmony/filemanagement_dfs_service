/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "periodic_clean_task.h"

#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <sys/types.h>
#include <unistd.h>

#include "battery_status.h"
#include "cloud_file_utils.h"
#include "data_sync_const.h"
#include "dfs_error.h"
#include "directory_ex.h"
#include "parameters.h"
#include "settings_data_manager.h"
#include "screen_status.h"
#include "utils_directory.h"
#include "utils_log.h"
#ifdef HICOLLIE_ENABLE
#include "xcollie_helper.h"
#endif

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;

static const std::string PHOTOS_KEY = "persist.kernel.bundle_name.photos";
static const std::string PERIODIC_CLEAN_KEY = "clean_dentry_file_once";
static const std::string GLOBAL_CONFIG_FILE_PATH = "globalConfig.xml";
static constexpr uint32_t STAT_MODE_DIR = 0771;
static constexpr int32_t FIVE_MINUTES = 5 * 60;

PeriodicCleanTask::PeriodicCleanTask(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
    : CycleTask(PeriodicCleanTaskName, {GALLERY_BUNDLE_NAME}, ONE_WEEK, dataSyncManager)
{
    runTaskForSwitchOff_ = true;
}

int32_t PeriodicCleanTask::RunTaskForBundle(int32_t userId, std::string bundleName)
{
    LOGI("PeriodicCleanTask start, userId: %{public}d", userId);
    if (ScreenStatus::IsScreenOn() || !BatteryStatus::IsCharging()) {
        LOGI("Screen state is On or BatteryStatus is disconnected, stop PeriodicCleanTask");
        return E_STOP;
    }

    int32_t ret = E_OK;
    dataSyncManager_->PeriodicCleanLock();
#ifdef HICOLLIE_ENABLE
    const int32_t TIMEOUT_S = 2;
    int32_t xcollieId = XCollieHelper::SetTimer("PeriodicCleanTask_StartRename", TIMEOUT_S, nullptr, nullptr, true);
#endif
    if (SettingsDataManager::GetSwitchStatus() == SwitchStatus::NONE) {
        if (periodicCleanConfig_ == nullptr || userId != userId_) {
            periodicCleanConfig_ = std::make_unique<CloudPrefImpl>(userId, "", GLOBAL_CONFIG_FILE_PATH);
        }
        bool cleanDentryFlag = false;
        periodicCleanConfig_->GetBool(PERIODIC_CLEAN_KEY, cleanDentryFlag);
        LOGI("cleanDentryFlag: %{public}d", cleanDentryFlag);
        if (!cleanDentryFlag) {
            ret = RenameCacheDir(userId);
            if (ret == E_OK) {
                periodicCleanConfig_->SetBool(PERIODIC_CLEAN_KEY, true);
            }
            LOGI("RenameCacheDir ret: %{public}d", ret);
        }
    }
#ifdef HICOLLIE_ENABLE
    XCollieHelper::CancelTimer(xcollieId);
#endif
    dataSyncManager_->PeriodicCleanUnlock();

    if (PeriodicCleanTempDir(userId) == E_STOP) {
        ret = E_STOP;
    }
    LOGI("PeriodicCleanTask end, ret: %{public}d", ret);
    return ret;
}

int32_t PeriodicCleanTask::PeriodicCleanTempDir(int32_t userId)
{
    std::string userIdStr = std::to_string(userId);
    std::string dentryDir = "/data/service/el2/" + userIdStr + "/hmdfs/cache/account_cache/dentry_cache";
    std::string tmpDentry = "cloud_temp_";
    if (CleanTempDir(dentryDir, tmpDentry) == E_STOP) {
        LOGE("clean temp dentry stop");
        return E_STOP;
    }

    std::string cacheDir = "/mnt/hmdfs/" + userIdStr + "/account/device_view/local/files/.cloud_cache";
    std::string tmpDkCache = "pread_cache_tmp_";
    std::string tmpDownloadCache = "download_cache_tmp_";
    if (CleanTempDir(cacheDir, tmpDkCache) == E_STOP) {
        LOGE("clean temp pread_cache stop");
        return E_STOP;
    }
    if (CleanTempDir(cacheDir, tmpDownloadCache) == E_STOP) {
        LOGE("clean temp download_cache stop");
        return E_STOP;
    }

    std::string photoBundleName = system::GetParameter(PHOTOS_KEY, "");
    if (photoBundleName.empty()) {
        LOGE("get photoBundleName failed");
        return E_STOP;
    }
    std::string videoCache = "/mnt/hmdfs/" + userIdStr + "/account/device_view/local/data/" + photoBundleName;
    std::string tmpVideoCache = ".video_cache_tmp_";
    if (CleanTempDir(videoCache, tmpVideoCache) == E_STOP) {
        LOGE("clean temp video_cache stop");
        return E_STOP;
    }

    return E_OK;
}

int32_t PeriodicCleanTask::CleanTempDir(const std::string &dir, const std::string &prefix)
{
    if (prefix.empty()) {
        LOGW("prefix is empty");
        return E_OK;
    }

    std::error_code ec;
    if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec)) {
        LOGW("dir: %{public}s is invalid, err: %{public}d", GetAnonyStringStrictly(dir).c_str(), ec.value());
        return E_OK;
    }

    for (const auto &entry: std::filesystem::directory_iterator(dir, ec)) {
        std::string childDirName = entry.path().filename();
        if (entry.is_directory() && childDirName.find(prefix) == 0) {
            std::string tmpDir = dir + "/" + childDirName;
            LOGI("clean tmpDir: %{public}s", GetAnonyStringStrictly(tmpDir).c_str());
#ifdef HICOLLIE_ENABLE
            int32_t xcollieId = XCollieHelper::SetTimer("PeriodicCleanTask_StartCleanTempDir", FIVE_MINUTES,
                nullptr, nullptr, true);
#endif
            int32_t ret = RemoveDirectoryByScreenOff(tmpDir);
#ifdef HICOLLIE_ENABLE
            XCollieHelper::CancelTimer(xcollieId);
#endif
            LOGI("clean tmpDir: %{public}s, ret: %{public}d", GetAnonyStringStrictly(tmpDir).c_str(), ret);
            if (ret == E_STOP) {
                return E_STOP;
            }
        }
    }

    return E_OK;
}

int32_t PeriodicCleanTask::RemoveDirectoryByScreenOff(const std::string& path)
{
    string subPath;
    int32_t ret = E_OK;
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        LOGE("opendir failed, path = %{public}s, err:%{public}d", GetAnonyStringStrictly(path).c_str(), errno);
        return E_DELETE_FAILED;
    }

    while (true) {
        if (ScreenStatus::IsScreenOn()) {
            LOGE("Screen state is On, stop remove path: %{public}s", GetAnonyStringStrictly(path).c_str());
            closedir(dir);
            return E_STOP;
        }

        struct dirent *ptr = readdir(dir);
        if (ptr == nullptr) {
            break;
        }

        // current dir OR parent dir
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        subPath = IncludeTrailingPathDelimiter(path) + string(ptr->d_name);
        if (ptr->d_type == DT_DIR) {
            int32_t tmpRet = RemoveDirectoryByScreenOff(subPath);
            if (tmpRet == E_STOP) {
                closedir(dir);
                return E_STOP;
            } else if (tmpRet != E_OK) {
                ret = E_DELETE_FAILED;
            }
        } else if (access(subPath.c_str(), F_OK) == 0) {
            if (remove(subPath.c_str()) != 0) {
                closedir(dir);
                LOGE("remove failed, subPath: %{public}s, err:%{public}d",
                    GetAnonyStringStrictly(subPath).c_str(), errno);
                return E_DELETE_FAILED;
            }
        }
    }
    closedir(dir);

    string currentPath = ExcludeTrailingPathDelimiter(path);
    if (access(currentPath.c_str(), F_OK) == 0) {
        if (remove(currentPath.c_str()) != 0) {
            LOGE("remove failed, currentPath = %{public}s, err:%{public}d",
                GetAnonyStringStrictly(currentPath).c_str(), errno);
            return E_DELETE_FAILED;
        }
    }

    return access(path.c_str(), F_OK) == 0 ? E_DELETE_FAILED : ret;
}

int32_t PeriodicCleanTask::RenameCacheDir(int32_t userId)
{
    int32_t ret = E_OK;
    std::string userIdStr = std::to_string(userId);
    std::string curTime = std::to_string(GetCurrentTimeStampMs());
    std::string dentryDir = "/data/service/el2/" + userIdStr + "/hmdfs/cache/account_cache/dentry_cache/cloud";
    ret = RenameTempDir(dentryDir, "_temp_" + curTime);
    if (ret != E_OK) {
        return E_RENAME_FAIL;
    }

    std::string dkCacheDir = "/mnt/hmdfs/" + userIdStr + "/account/device_view/local/files/.cloud_cache/pread_cache";
    ret = RenameTempDir(dkCacheDir, "_tmp_" + curTime);
    if (ret != E_OK) {
        return E_RENAME_FAIL;
    }

    std::string downloadCache =
        "/mnt/hmdfs/" + userIdStr + "/account/device_view/local/files/.cloud_cache/download_cache";
    ret = RenameTempDir(downloadCache, "_tmp_" + curTime);
    if (ret != E_OK) {
        return E_RENAME_FAIL;
    }

    std::string photoBundleName = system::GetParameter(PHOTOS_KEY, "");
    if (photoBundleName.empty()) {
        LOGE("get photoBundleName failed");
        return E_RENAME_FAIL;
    }
    std::string videoCache =
        "/mnt/hmdfs/" + userIdStr + "/account/device_view/local/data/" + photoBundleName + "/.video_cache";
    ret = RenameTempDir(videoCache, "_tmp_" + curTime);
    if (ret != E_OK) {
        return E_RENAME_FAIL;
    }

    return E_OK;
}

int32_t PeriodicCleanTask::RenameTempDir(const std::string &dir, const std::string &tempSuffix)
{
    if (tempSuffix.empty()) {
        LOGW("tempSuffix is empty");
        return E_OK;
    }

    std::error_code ec;
    if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec)) {
        LOGW("dir: %{public}s is invalid, err: %{public}d", GetAnonyStringStrictly(dir).c_str(), ec.value());
        return E_OK;
    }

    if (!std::filesystem::is_empty(dir)) {
        LOGI("dir: %{public}s not empty", GetAnonyStringStrictly(dir).c_str());
        std::string tempDir = dir + tempSuffix;
        if (rename(dir.c_str(), tempDir.c_str()) != E_OK) {
            LOGE("rename dir: %{public}s failed, errno: %{public}d", GetAnonyStringStrictly(dir).c_str(), errno);
            return E_RENAME_FAIL;
        }
        if (mkdir(dir.c_str(), STAT_MODE_DIR) != E_OK) {
            int32_t ret = rename(tempDir.c_str(), dir.c_str());
            LOGE("mkdir dir: %{public}s failed, errno: %{public}d, rollback result: %{public}d",
                GetAnonyStringStrictly(dir).c_str(), errno, ret);
            return E_RENAME_FAIL;
        }
    }

    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS