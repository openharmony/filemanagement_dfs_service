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

#include "periodic_chown_task.h"

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <cstring>
#include <dirent.h>
#include <filesystem>
#include <sys/stat.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

#include "battery_status.h"
#include "cloud_file_fault_event.h"
#include "cloud_file_utils.h"
#include "data_sync_const.h"
#include "dfs_error.h"
#include "parameters.h"
#include "screen_status.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
using namespace std;
using namespace CloudFile;

static const std::string PERIODIC_CHOWN_TASK_NAME = "periodic_chown_task";
static const std::string SPACE_OCCUPY_FILE_PATH = "spaceOccupy.xml";
static const std::string DIRECTORY_CONFIG_KEY = "chown_directory_templates";
static const char DIRECTORY_SEPARATOR = ';';
static const int32_t MODE_MASK = 0777;
static const int32_t MODE_DIR = 0771;
static const int32_t MODE_REG = 0660;
static constexpr uid_t MEDIA_UID = 1008;
static constexpr uid_t DFS_UID = 1009;
static constexpr size_t USER_ID_PLACEHOLDER_LENGTH = 8;
constexpr const char *DATA_DEV_PATH = "/dev/block/by-name/userdata";

static const std::vector<std::string> DIRECTORY_TEMPLATES = {
    "/data/service/el2/{userId}/hmdfs/account/files/Photo",
    "/data/service/el2/{userId}/hmdfs/account/files/.thumbs",
    "/data/service/el2/{userId}/hmdfs/account/files/.editData"
};

PeriodicChownTask::PeriodicChownTask(std::shared_ptr<CloudFile::DataSyncManager> dataSyncManager)
    : CycleTask(PERIODIC_CHOWN_TASK_NAME, {GALLERY_BUNDLE_NAME}, ONE_WEEK, dataSyncManager)
{
    runTaskForSwitchOff_ = true;
}

int32_t PeriodicChownTask::RunTaskForBundle(int32_t userId, std::string bundleName)
{
    LOGI("PeriodicChownTask start, userId: %{public}d", userId);
    
    if (!CheckChownCondition()) {
        LOGI("CheckChownCondition failed, stop PeriodicChownTask");
        return E_STOP;
    }

    int32_t ret = E_OK;
    if (periodicChownConfig_ == nullptr || userId != userId_) {
        periodicChownConfig_ = std::make_unique<CloudPrefImpl>(userId, "", SPACE_OCCUPY_FILE_PATH);
    }
    
    InitializeDirectoryTemplates();
    if (CheckAllDirectoriesDone()) {
        LOGI("All directories already processed, skip PeriodicChownTask");
        return E_OK;
    }
    
    ret = ExecuteChownTask(userId);
    LOGI("PeriodicChownTask end, ret: %{public}d", ret);
    return ret;
}

bool PeriodicChownTask::CheckChownCondition()
{
    return !ScreenStatus::IsScreenOn() && BatteryStatus::IsCharging();
}

bool PeriodicChownTask::CheckAllDirectoriesDone()
{
    bool allDirsDone = true;
    int32_t dirCount = static_cast<int32_t>(directoryKeys_.size());
    for (int32_t i = 0; i < dirCount; i++) {
        bool dirDone = false;
        periodicChownConfig_->GetBool(directoryKeys_[i], dirDone);
        if (!dirDone) {
            allDirsDone = false;
            break;
        }
    }
    return allDirsDone;
}

int32_t PeriodicChownTask::ChownAllDirectories(int32_t userId)
{
    int32_t dirCount = static_cast<int32_t>(directoryTemplates_.size());
    
    for (int32_t i = 0; i < dirCount; i++) {
        std::string dirPath = GetTargetDirectory(userId, i);
        if (dirPath.empty()) {
            LOGE("GetTargetDirectory failed for index: %{public}d", i);
            continue;
        }
        
        bool dirDone = false;
        periodicChownConfig_->GetBool(directoryKeys_[i], dirDone);
        if (dirDone) {
            LOGI("Directory %{public}s already processed", GetAnonyStringStrictly(dirPath).c_str());
            continue;
        }
        
        std::string timestampKey = GenerateTimestampKey(directoryTemplates_[i]);
        int64_t timestamp = 0;
        if (timestampKey.empty()) {
            LOGE("GenerateTimestampKey failed for index: %{public}d", i);
            continue;
        }
        periodicChownConfig_->GetLong(timestampKey, timestamp);
        if (timestamp > 0) {
            LOGI("Directory %{public}s already has timestamp, skip", GetAnonyStringStrictly(dirPath).c_str());
            continue;
        }
        
        int32_t dirRet = ProcessSingleDirectory(dirPath, i);
        if (dirRet != E_OK) {
            LOGE("ProcessSingleDirectory failed for dir: %{public}s, ret: %{public}d",
                GetAnonyStringStrictly(dirPath).c_str(), dirRet);
            return dirRet;
        }
        timestamp = static_cast<int64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
        periodicChownConfig_->SetLong(timestampKey, timestamp);
        LOGI("Saved timestamp for directory: %{public}s, timestamp: %{public}" PRId64,
            GetAnonyStringStrictly(dirPath).c_str(), timestamp);
    }
    
    return E_OK;
}

int32_t PeriodicChownTask::ChownFiles(const std::string& path, int32_t index)
{
    LOGI("Start chown files in directory: %{public}s", GetAnonyStringStrictly(path).c_str());
    fileCount_ = 0;
    totalSize_ = 0;
    
    std::string timestampKey = GenerateTimestampKey(directoryTemplates_[index]);
    int64_t dirTimestamp = 0;
    if (timestampKey.empty()) {
        LOGE("GenerateTimestampKey failed for index: %{public}d", index);
        return E_PATH;
    }
    periodicChownConfig_->GetLong(timestampKey, dirTimestamp);
    std::error_code ec;
    int32_t batchCount = 0;
    
    for (auto const& dirEntry : filesystem::recursive_directory_iterator(path, ec)) {
        if (ec) {
            LOGE("recursive_directory_iterator error: %{public}s", ec.message().c_str());
            continue;
        }
        
        std::string filePath = dirEntry.path().string();
        struct stat st{};
        if (stat(filePath.c_str(), &st) != 0) {
            LOGE("stat failed for file: %{public}s, errno: %{public}d",
                GetAnonyStringStrictly(filePath).c_str(), errno);
            continue;
        }
        
        if (!ShouldChownFile(st, dirTimestamp)) {
            continue;
        }
        if (batchCount >= (BATCH_SIZE - 1) && !CheckChownCondition()) {
            LOGI("CheckChownCondition failed, stop processing files");
            ReportChownFilesResult(path, E_STOP);
            return E_STOP;
        }
        
        ChangeUidToMedia(filePath, MODE_REG, st);
        batchCount = (batchCount + 1) % BATCH_SIZE;
    }
    
    ReportChownFilesResult(path, E_OK);
    return E_OK;
}

void PeriodicChownTask::ChangeUidToMedia(const std::string& path, mode_t mode, const struct stat& st)
{
    if ((st.st_mode & MODE_MASK) != mode) {
        if (chmod(path.c_str(), mode) != 0) {
            LOGE("ChangeUidToMedia chmod failed, err err is %{public}d", errno);
            return;
        }
    }

    if (chown(path.c_str(), MEDIA_UID, static_cast<gid_t>(-1)) != 0) {
        LOGE("ChangeUidToMedia chown failed, err err is %{public}d", errno);
    }

    if (mode == MODE_REG) {
        fileCount_++;
        totalSize_ += st.st_size;
    }
}

std::string PeriodicChownTask::GetTargetDirectory(int32_t userId, int32_t index)
{
    if (index < 0 || index >= static_cast<int32_t>(directoryTemplates_.size())) {
        return "";
    }
    
    std::string userIdStr = std::to_string(userId);
    std::string templatePath = directoryTemplates_[index];
    
    size_t pos = templatePath.find("{userId}");
    if (pos != std::string::npos) {
        templatePath.replace(pos, USER_ID_PLACEHOLDER_LENGTH, userIdStr);
    }
    
    return templatePath;
}

int32_t PeriodicChownTask::ValidateAndChownDirectory(const std::string& path)
{
    std::error_code ec;
    if (!filesystem::exists(path, ec)) {
        std::string errmsg = "Directory " + GetAnonyStringStrictly(path) + " does not exist";
        CLOUD_FILE_FAULT_REPORT(CloudFileFaultInfo{GALLERY_BUNDLE_NAME, FaultOperation::READDIR,
            FaultType::FILE, E_PATH, errmsg});
        LOGE("ValidateAndChownDirectory failed for dir: %{public}s, ret: %{public}d",
            GetAnonyStringStrictly(path).c_str(), E_PATH);
        return E_PATH;
    }
    struct stat st{};
    if (stat(path.c_str(), &st) != 0) {
        LOGE("ValidateAndChownDirectory stat failed for dir: %{public}s, errno: %{public}d",
            GetAnonyStringStrictly(path).c_str(), errno);
        return E_PATH;
    }
    if (st.st_uid == DFS_UID) {
        ChangeUidToMedia(path, MODE_DIR, st);
    }
    return E_OK;
}

int32_t PeriodicChownTask::ProcessSingleDirectory(const std::string& path, int32_t index)
{
    LOGI("Start chown directory: %{public}s", GetAnonyStringStrictly(path).c_str());
    
    int32_t ret = ValidateAndChownDirectory(path);
    if (ret != E_OK) {
        return ret;
    }

    std::error_code ec;
    int32_t batchCount = 0;
    for (auto const& dirEntry : filesystem::recursive_directory_iterator(path, ec)) {
        if (ec) {
            LOGE("recursive_directory_iterator error: %{public}s", ec.message().c_str());
            continue;
        }
        
        std::string entryPath = dirEntry.path().string();
        struct stat st{};
        if (stat(entryPath.c_str(), &st) != 0) {
            LOGE("stat failed for directory: %{public}s, errno: %{public}d",
                GetAnonyStringStrictly(entryPath).c_str(), errno);
            continue;
        }
        
        if (S_ISDIR(st.st_mode) && st.st_uid == DFS_UID) {
            if (batchCount >= (BATCH_SIZE - 1) && !CheckChownCondition()) {
                LOGI("CheckChownCondition failed, stop processing directories");
                return E_STOP;
            }
            ChangeUidToMedia(entryPath, MODE_DIR, st);
            batchCount = (batchCount + 1) % BATCH_SIZE;
        }
    }
    
    LOGI("Chown directory completed: %{public}s", GetAnonyStringStrictly(path).c_str());
    return E_OK;
}

int32_t PeriodicChownTask::ExecuteChownTask(int32_t userId)
{
    int32_t ret = E_OK;
    int32_t dirCount = static_cast<int32_t>(directoryTemplates_.size());
    
    ret = ChownAllDirectories(userId);
    if (ret != E_OK) {
        LOGE("ChownAllDirectories failed, ret: %{public}d", ret);
        return ret;
    }
    bool dirDone = false;
    for (int32_t i = 0; i < dirCount; i++) {
        std::string dirPath = GetTargetDirectory(userId, i);
        if (dirPath.empty()) {
            LOGE("GetTargetDirectory failed for index: %{public}d", i);
            continue;
        }
        periodicChownConfig_->GetBool(directoryKeys_[i], dirDone);
        if (dirDone) {
            continue;
        }
        int32_t dirRet = ChownFiles(dirPath, i);
        if (dirRet != E_OK) {
            ret = dirRet;
            LOGE("ChownFiles failed for dir: %{public}s, ret: %{public}d",
                GetAnonyStringStrictly(dirPath).c_str(), dirRet);
            return dirRet;
        }
        periodicChownConfig_->SetBool(directoryKeys_[i], true);
    }
    
    return ret;
}

std::string PeriodicChownTask::GenerateDoneKey(const std::string& dirPath)
{
    return GenerateKeyWithSuffix(dirPath, "_done");
}

std::string PeriodicChownTask::GenerateTimestampKey(const std::string& dirPath)
{
    return GenerateKeyWithSuffix(dirPath, "_timestamp");
}

std::string PeriodicChownTask::GenerateKeyWithSuffix(const std::string& dirPath, const std::string& suffix)
{
    std::string path = dirPath;
    size_t lastSlash = path.rfind('/');
    if (lastSlash == std::string::npos || lastSlash == 0) {
        return "";
    }
    
    size_t secondLastSlash = path.rfind('/', lastSlash - 1);
    if (secondLastSlash == std::string::npos || secondLastSlash == 0) {
        return "";
    }
    
    size_t thirdLastSlash = path.rfind('/', secondLastSlash - 1);
    if (thirdLastSlash == std::string::npos) {
        return "";
    }
    
    std::string key = path.substr(thirdLastSlash + 1);
    key.erase(std::remove(key.begin(), key.end(), '.'), key.end());
    std::replace(key.begin(), key.end(), '/', '_');
    key += suffix;
    return key;
}

void PeriodicChownTask::InitializeDirectoryTemplates()
{
    std::string dirString;
    periodicChownConfig_->GetString(DIRECTORY_CONFIG_KEY, dirString);
    if (dirString.empty()) {
        std::string defaultDirString;
        for (size_t i = 0; i < DIRECTORY_TEMPLATES.size(); i++) {
            if (i > 0) {
                defaultDirString += DIRECTORY_SEPARATOR;
            }
            defaultDirString += DIRECTORY_TEMPLATES[i];
        }
        periodicChownConfig_->SetString(DIRECTORY_CONFIG_KEY, defaultDirString);
        directoryTemplates_ = DIRECTORY_TEMPLATES;
        directoryKeys_.clear();
        for (const auto& dirTemplate : directoryTemplates_) {
            std::string key = GenerateDoneKey(dirTemplate);
            if (!key.empty()) {
                directoryKeys_.push_back(key);
            }
        }
    } else {
        directoryTemplates_.clear();
        directoryKeys_.clear();
        std::string currentDir;
        for (char c : dirString) {
            if (c == DIRECTORY_SEPARATOR && !currentDir.empty()) {
                directoryTemplates_.push_back(currentDir);
                currentDir.clear();
            } else if (c != DIRECTORY_SEPARATOR) {
                currentDir += c;
            }
        }
        if (!currentDir.empty()) {
            directoryTemplates_.push_back(currentDir);
        }
        for (const auto& dirTemplate : directoryTemplates_) {
            std::string key = GenerateDoneKey(dirTemplate);
            if (!key.empty()) {
                directoryKeys_.push_back(key);
            }
        }
    }
}

bool PeriodicChownTask::ShouldChownFile(const struct stat& st, int64_t dirTimestamp)
{
    if (!S_ISREG(st.st_mode) || st.st_uid != DFS_UID) {
        return false;
    }
    
    if (dirTimestamp > 0 && st.st_mtime >= dirTimestamp) {
        return false;
    }
    
    return true;
}

void PeriodicChownTask::ReportChownFilesResult(const std::string& path, int32_t result)
{
    std::string message = "ChownFiles " + std::string(result == E_OK ? "completed" : "interrupted") +
        ", chowned files: " + std::to_string(fileCount_) +
        ", total size: " + std::to_string(totalSize_) + " bytes";
    CLOUD_FILE_FAULT_REPORT(CloudFileFaultInfo{GALLERY_BUNDLE_NAME, FaultOperation::SETATTR,
        FaultType::FILE, result, message});
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
