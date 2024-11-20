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

#include "ipc/cloud_daemon.h"

#include <exception>
#include <stdexcept>
#include <thread>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include "iremote_object.h"
#include "system_ability_definition.h"
#include "parameters.h"
#include "plugin_loader.h"
#include "dfs_error.h"
#include "fuse_manager/fuse_manager.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;
using namespace CloudDisk;

namespace {
    static const string LOCAL_PATH_DATA_SERVICE_EL2 = "/data/service/el2/";
    static const string LOCAL_PATH_HMDFS_DENTRY_CACHE = "/hmdfs/cache/account_cache/dentry_cache/";
    static const string LOCAL_PATH_HMDFS_CACHE_CLOUD = "/hmdfs/cache/account_cache/dentry_cache/cloud";
    static const int32_t STAT_MODE_DIR = 0771;
    static const int32_t STAT_MODE_DIR_DENTRY_CACHE = 02771;
    static const int32_t OID_DFS = 1009;
}
REGISTER_SYSTEM_ABILITY_BY_ID(CloudDaemon, FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID, true);

CloudDaemon::CloudDaemon(int32_t saID, bool runOnCreate) : SystemAbility(saID, runOnCreate)
{
    accountStatusListener_ = make_shared<AccountStatusListener>();
}

void CloudDaemon::PublishSA()
{
    LOGI("Begin to init");
    if (!registerToService_) {
        bool ret = SystemAbility::Publish(this);
        if (!ret) {
            throw runtime_error("Failed to publish the daemon");
        }
        registerToService_ = true;
    }
    LOGI("Init finished successfully");
}

static bool CheckDeviceInLinux()
{
    struct utsname uts;
    if (uname(&uts) == -1) {
        LOGE("uname get failed.");
        return false;
    }
    if (strcmp(uts.sysname, "Linux") == 0) {
        LOGI("uname system is linux.");
        return true;
    }
    return false;
}

static void ModSysParam()
{
    if (CheckDeviceInLinux()) {
        const string photos = "persist.kernel.bundle_name.photos";
        const string clouddrive = "persist.kernel.bundle_name.clouddrive";
        system::SetParameter(photos, "");
        system::SetParameter(clouddrive, "");
    }
}

void CloudDaemon::OnStart()
{
    LOGI("Begin to start service");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LOGI("Daemon has already started");
        return;
    }

    try {
        PublishSA();
        AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
        mode_t mode = 002;
        umask(mode);
        ModSysParam();
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }

    state_ = ServiceRunningState::STATE_RUNNING;
    /* load cloud file ext plugin */
    CloudFile::PluginLoader::GetInstance().LoadCloudKitPlugin();
    LOGI("Start service successfully");
}

void HandleStartMove(int32_t userId)
{
    const std::string filemanagerKey = "persist.kernel.bundle_name.filemanager";
    string subList[] = {"com.ohos.photos", system::GetParameter(filemanagerKey, "")};
    string srcBase = "/data/service/el1/public/cloudfile/" + to_string(userId);
    string dstBase = "/data/service/el2/" + to_string(userId) + "/hmdfs/cloudfile_manager";
    string removePath = srcBase + "/" + subList[1] + "/backup";
    bool ret = Storage::DistributedFile::Utils::ForceRemoveDirectoryDeepFirst(removePath);
    if (!ret) {
        LOGE("remove failed path: %{public}s", GetAnonyString(removePath).c_str());
    }
    const auto copyOptions = filesystem::copy_options::overwrite_existing | filesystem::copy_options::recursive;
    for (auto sub : subList) {
        string srcPath = srcBase + '/' + sub;
        string dstPath = dstBase + '/' + sub;
        if (access(srcPath.c_str(), F_OK) != 0) {
            LOGI("srcPath %{public}s not found", GetAnonyString(srcPath).c_str());
            continue;
        }
        LOGI("Begin to move path: %{public}s", GetAnonyString(srcPath).c_str());
        error_code errCode;
        filesystem::copy(srcPath, dstPath, copyOptions, errCode);
        if (errCode.value() != 0) {
            LOGE("copy failed path: %{public}s, errCode: %{public}d",
                GetAnonyString(srcPath).c_str(), errCode.value());
        }
        LOGI("End move path: %{public}s", GetAnonyString(srcPath).c_str());
        bool ret = Storage::DistributedFile::Utils::ForceRemoveDirectoryDeepFirst(srcPath);
        if (!ret) {
            LOGE("remove failed path: %{public}s", GetAnonyString(srcPath).c_str());
        }
    }
    const string moveFile = "persist.kernel.move.finish";
    system::SetParameter(moveFile, "true");
}

void HandleDaemonStarted()
{
    const string serviceReady = "bootevent.cloudfiledaemon.ready";
    if (!system::GetBoolParameter(serviceReady, false)) {
        system::SetParameter(serviceReady, "true");
        LOGI("set cloudfiledaemon service started true");
    }
}

void CloudDaemon::OnStop()
{
    LOGI("Begin to stop");
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
    LOGI("Stop finished successfully");
}

void CloudDaemon::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    LOGI("OnAddSystemAbility systemAbilityId:%{public}d added!", systemAbilityId);
    accountStatusListener_->Start();
}

int32_t CloudDaemon::StartFuse(int32_t userId, int32_t devFd, const string &path)
{
    LOGI("Start Fuse");
    std::thread([=]() {
        int32_t ret = FuseManager::GetInstance().StartFuse(userId, devFd, path);
        LOGI("start fuse result %d", ret);
        }).detach();

    string dentryPath = LOCAL_PATH_DATA_SERVICE_EL2 + to_string(userId) + LOCAL_PATH_HMDFS_CACHE_CLOUD;
    if (access(dentryPath.c_str(), F_OK) != 0) {
        string cachePath = LOCAL_PATH_DATA_SERVICE_EL2 + to_string(userId) + LOCAL_PATH_HMDFS_DENTRY_CACHE;
        if (mkdir(cachePath.c_str(), STAT_MODE_DIR) != 0 && errno != EEXIST) {
            LOGE("create accout_cache path error %{public}d", errno);
            return E_PATH;
        }
        if (chmod(cachePath.c_str(), STAT_MODE_DIR_DENTRY_CACHE) != 0) {
            LOGE("chmod cachepath error %{public}d", errno);
        }
        if (chown(cachePath.c_str(), OID_DFS, OID_DFS) != 0) {
            LOGE("chown cachepath error %{public}d", errno);
        }
        if (mkdir(dentryPath.c_str(), STAT_MODE_DIR) != 0 && errno != EEXIST) {
            LOGE("create dentrypath %{public}d", errno);
            return E_PATH;
        }
        if (chown(dentryPath.c_str(), OID_DFS, OID_DFS) != 0) {
            LOGE("chown cachepath error %{public}d", errno);
        }
    }
    if (path.find("cloud_fuse") != string::npos) {
        std::thread([userId]() {
            HandleStartMove(userId);
        }).detach();
    } else {
        HandleDaemonStarted();
    }
    return E_OK;
}
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS
