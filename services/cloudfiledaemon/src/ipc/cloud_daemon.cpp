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

#include "iremote_object.h"
#include "system_ability_definition.h"
#include "fuse_manager/fuse_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace CloudFile {
using namespace std;
using namespace OHOS::FileManagement;

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
    LOGI("Begin to start service");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        LOGD("Daemon has already started");
        return;
    }

    try {
        PublishSA();
        AddSystemAbilityListener(COMMON_EVENT_SERVICE_ID);
    } catch (const exception &e) {
        LOGE("%{public}s", e.what());
    }
    //FuseManager fuseMgr;
    //fuseMgr.Start("/data/cloud");

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

void CloudDaemon::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    (void)systemAbilityId;
    (void)deviceId;
}

void CloudDaemon::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    (void)deviceId;
    if (systemAbilityId != COMMON_EVENT_SERVICE_ID) {
        LOGE("systemAbilityId is not COMMON_EVENT_SERVICE_ID");
        return;
    }

}

int32_t CloudDaemon::StartFuse(int32_t devFd, const string &path)
{
    auto fuseMgr = make_shared<FuseManager>();
    LOGI("Start Fuse");

    std::thread([fusePtr{fuseMgr}, dev{devFd}, mnt{path}]() {
        int32_t ret = fusePtr->StartFuse(dev, mnt);
        LOGI("start fuse result %d", ret);
    }).detach();
    return E_OK;

}
} // namespace CloudFile
} // namespace Storage
} // namespace OHOS
