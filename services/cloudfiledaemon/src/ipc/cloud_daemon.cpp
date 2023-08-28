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

#include "iremote_object.h"
#include "system_ability_definition.h"

#include "dfs_error.h"
#include "fuse_manager/fuse_manager.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;

REGISTER_SYSTEM_ABILITY_BY_ID(CloudDaemon, FILEMANAGEMENT_CLOUD_DAEMON_SERVICE_SA_ID, true);

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

void CloudDaemon::OnStart()
{
    int ret = mallopt(M_SET_THREAD_CACHE, M_THREAD_CACHE_DISABLE);
    LOGI("Begin to start service M_SET_THREAD_CACHE:%{public}d", ret);
    ret = mallopt(M_OHOS_CONFIG, M_DISABLE_OPT_TCACHE);
    LOGI("Begin to start service M_OHOS_CONFIG:%{public}d", ret);
    ret = mallopt(M_DELAYED_FREE, M_DELAYED_FREE_DISABLE);
    LOGI("Begin to start service M_DELAYED_FREE:%{public}d", ret);
    LOGI("Begin to start service");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LOGI("Daemon has already started");
        return;
    }

    try {
        PublishSA();
        AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }

    state_ = ServiceRunningState::STATE_RUNNING;
    LOGI("Start service successfully");
}

void CloudDaemon::OnStop()
{
    LOGI("Begin to stop");
    state_ = ServiceRunningState::STATE_NOT_START;
    registerToService_ = false;
    LOGI("Stop finished successfully");
}

int32_t CloudDaemon::StartFuse(int32_t userId, int32_t devFd, const string &path)
{
    LOGI("Start Fuse");

    std::thread([=]() {
        int32_t ret = FuseManager::GetInstance().StartFuse(userId, devFd, path);
        LOGI("start fuse result %d", ret);
        }).detach();
    return E_OK;
}
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS
