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

#include <filesystem>

#include "cloud_file_utils.h"
#include "data_sync_const.h"
#include "parameters.h"
#include "settings_data_manager.h"
#include "utils_directory.h"
#include "utils_log.h"
#ifdef HICOLLIE_ENABLE
#include "xcollie_helper.h"
#endif

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
static const std::string PHOTOS_KEY = "persist.kernel.bundle_name.photos";
static const std::string PERIODIC_CLEAN_KEY = "clean_dentry_file_once";
static const std::string GLOBAL_CONFIG_FILE_PATH = "globalConfig.xml";
static constexpr uint32_t STAT_MODE_DIR = 0771;

PeriodicCleanTask::PeriodicCleanTask(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
    : CycleTask(PeriodicCleanTaskName, {GALLERY_BUNDLE_NAME}, THREE_DAY, dataSyncManager)
{
    runTaskForSwitchOff_ = true;
}

int32_t PeriodicCleanTask::RunTaskForBundle(int32_t userId, std::string bundleName)
{
    LOGI("PeriodicCleanTask start, userId: %{public}d", userId);
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
        }
    }
#ifdef HICOLLIE_ENABLE
    XCollieHelper::CancelTimer(xcollieId);
#endif
    dataSyncManager_->PeriodicCleanUnlock();

    PeriodicCleanTempDir(userId);
    return ret;
}

void PeriodicCleanTask::PeriodicCleanTempDir(int32_t userId)
{
    std::string userIdStr = std::to_string(userId);
    std::string dentryDir = "/data/service/el2/" + userIdStr + "/hmdfs/cache/account_cache/dentry_cache";
    std::string tmpDentry = "cloud_temp_";
    CleanTempDir(dentryDir, tmpDentry);

    std::string cacheDir = "/mnt/hmdfs/" + userIdStr + "/account/device_view/local/files/.cloud_cache";
    std::string tmpDkCache = "pread_cache_tmp_";
    std::string tmpDownloadCache = "download_cache_tmp_";
    CleanTempDir(cacheDir, tmpDkCache);
    CleanTempDir(cacheDir, tmpDownloadCache);

    std::string photoBundleName = system::GetParameter(PHOTOS_KEY, "");
    if (photoBundleName.empty()) {
        LOGE("get photoBundleName failed");
        return;
    }
    std::string videoCache = "/mnt/hmdfs/" + userIdStr + "/account/device_view/local/data/" + photoBundleName;
    std::string tmpVideoCache = ".video_cache_tmp_";
    CleanTempDir(videoCache, tmpVideoCache);
}

void PeriodicCleanTask::CleanTempDir(const std::string &dir, const std::string &prefix)
{
    if (prefix.empty()) {
        LOGE("prefix is empty");
        return;
    }

    std::error_code ec;
    if (!std::filesystem::exists(dir, ec) || !std::filesystem::is_directory(dir, ec)) {
        LOGE("dir: %{public}s is invalid, err: %{public}d", GetAnonyStringStrictly(dir).c_str(), ec.value());
        return;
    }

    for (const auto &entry: std::filesystem::directory_iterator(dir)) {
        std::string childDirName = entry.path().filename();
        if (entry.is_directory() && childDirName.find(prefix) == 0) {
            std::string tmpDir = dir + "/" + childDirName;
            LOGI("clean tmpDir: %{public}s", GetAnonyStringStrictly(tmpDir).c_str());
            if (!Storage::DistributedFile::Utils::ForceRemoveDirectoryDeepFirst(tmpDir)) {
                LOGE("clean tmp dir failed, err: %{public}d", errno);
            }
        }
    }
}

int32_t PeriodicCleanTask::RenameCacheDir(int32_t userId)
{
    int32_t ret = E_OK;
    std::string userIdStr = std::to_string(userId);
    std::string curTime = std::to_string(GetCurrentTimeStampMs());
    std::string dentryDir = "/data/service/el2/" + userIdStr + "/hmdfs/cache/account_cache/dentry_cache/cloud";
    ret = RenameTempDir(dentryDir, "_temp_" + curTime);
    if (ret != E_OK) {
        return E_STOP;
    }

    std::string dkCacheDir = "/mnt/hmdfs/" + userIdStr + "/account/device_view/local/files/.cloud_cache/pread_cache";
    ret = RenameTempDir(dkCacheDir, "_tmp_" + curTime);
    if (ret != E_OK) {
        return E_STOP;
    }

    std::string downloadCache =
        "/mnt/hmdfs/" + userIdStr + "/account/device_view/local/files/.cloud_cache/download_cache";
    ret = RenameTempDir(downloadCache, "_tmp_" + curTime);
    if (ret != E_OK) {
        return E_STOP;
    }

    std::string photoBundleName = system::GetParameter(PHOTOS_KEY, "");
    if (photoBundleName.empty()) {
        LOGE("get photoBundleName failed");
        return E_STOP;
    }
    std::string videoCache =
        "/mnt/hmdfs/" + userIdStr + "/account/device_view/local/data/" + photoBundleName + "/.video_cache";
    ret = RenameTempDir(videoCache, "_tmp_" + curTime);
    if (ret != E_OK) {
        return E_STOP;
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
            return E_STOP;
        }
        if (mkdir(dir.c_str(), STAT_MODE_DIR) != E_OK) {
            int32_t ret = rename(tempDir.c_str(), dir.c_str());
            LOGE("mkdir dir: %{public}s failed, errno: %{public}d, rollback result: %{public}d",
                GetAnonyStringStrictly(dir).c_str(), errno, ret);
            return E_STOP;
        }
    }

    return E_OK;
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS