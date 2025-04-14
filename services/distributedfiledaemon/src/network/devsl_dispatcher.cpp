/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "network/devsl_dispatcher.h"

#include "device_manager.h"
#include "device/device_manager_agent.h"
#include "ipc/i_daemon.h"
#include "securec.h"
#include "security_label.h"
#include "softbus_bus_center.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
std::map<std::string, std::vector<std::weak_ptr<KernelTalker>>> DevslDispatcher::talkersMap_;
std::map<std::string, std::string> DevslDispatcher::idMap_;
std::mutex DevslDispatcher::mutex;
std::mutex DevslDispatcher::devslMapMutex_;
std::map<std::string, int32_t> DevslDispatcher::devslMap_;

int32_t DevslDispatcher::Start()
{
    int32_t status = DATASL_OnStart();
    if (status != 0) {
        LOGE("devsl dispatcher start error %{public}d", status);
    }

    return status;
}

void DevslDispatcher::Stop()
{
    DATASL_OnStop();
}

DEVSLQueryParams DevslDispatcher::MakeDevslQueryParams(const std::string &udid)
{
    DEVSLQueryParams queryParams;
    if (memcpy_s(queryParams.udid, MAX_UDID_LENGTH, udid.c_str(), udid.size())) {
        LOGE("devsl dispatcher memcpy error");
    }
    queryParams.udidLen = udid.size();

    return queryParams;
}

uint32_t DevslDispatcher::DevslGetRegister(const std::string &cid, std::weak_ptr<KernelTalker> talker)
{
    std::string udid;
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    deviceManager.GetUdidByNetworkId(IDaemon::SERVICE_NAME, cid, udid);

    std::lock_guard<std::mutex> lock(mutex);
    DEVSLQueryParams queryParams = MakeDevslQueryParams(udid);
    int status = DATASL_GetHighestSecLevelAsync(&queryParams, &DevslDispatcher::DevslGottonCallback);
    if (status != 0) {
        LOGE("devsl dispatcher register callback error %{public}d", status);
        return 0;
    }

    idMap_[udid] = cid;
    auto iter = talkersMap_.find(udid);
    if (iter == talkersMap_.end()) {
        std::vector<std::weak_ptr<KernelTalker>> talkers;
        talkers.push_back(talker);
        talkersMap_.emplace(udid, talkers);
    } else {
        iter->second.push_back(talker);
    }

    return 0;
}

void DevslDispatcher::DevslGottonCallbackAsync(const std::string udid, uint32_t devsl)
{
    std::lock_guard<std::mutex> lock(mutex);

    auto it = talkersMap_.find(udid);
    if (it == talkersMap_.end()) {
        LOGE("devsl dispatcher callback, there is no talker");
        return;
    }

    for (auto talker : it->second) {
        auto realTalker = talker.lock();
        if (!realTalker) {
            continue;
        }

        auto iter = idMap_.find(udid);
        if (iter != idMap_.end()) {
            realTalker->SinkDevslTokernel(iter->second, devsl);
        }
    }

    talkersMap_.erase(it);
}

void DevslDispatcher::DevslGottonCallback(DEVSLQueryParams *queryParams, int32_t result, uint32_t levelInfo)
{
    LOGI("devsl dispatcher callback");
    if (result != 0) {
        levelInfo = DATA_SEC_LEVEL1;
        LOGE("devsl dispatcher dsl get callback result : %{public}d", result);
    }

    std::string udid(reinterpret_cast<char *>(queryParams->udid), queryParams->udidLen);
    std::thread callbackThread =
        std::thread([udid, levelInfo]() { DevslGottonCallbackAsync(udid, levelInfo); });
    callbackThread.detach();
}

int32_t DevslDispatcher::GetSecurityLabel(const std::string &path)
{
    if (path.empty()) {
        LOGE("GetSecurityLabel path is empty");
        return -1;
    }

    static std::map<std::string, SecurityLabel> labels = {
        {"s1", SecurityLabel::S1},
        {"s2", SecurityLabel::S2},
        {"s3", SecurityLabel::S3},
        {"s4", SecurityLabel::S4},
    };

    std::string current = OHOS::FileManagement::ModuleSecurityLabel::SecurityLabel::GetSecurityLabel(path);
    auto iter = labels.find(current);
    if (iter == labels.end()) {
        LOGE("GetSecurityLabel cannot find label= %{public}s", current.c_str());
        return -1;
    }
    return static_cast<int32_t>(iter->second);
}

bool DevslDispatcher::CompareDevslWithLocal(const std::string &peerNetworkId, const std::vector<std::string> &paths)
{
    if (paths.size() == 0) {
        return false;
    }

    auto remoteDevsl = GetDeviceDevsl(peerNetworkId);
    if (remoteDevsl < 0) {
        LOGE("remoteDevsl < 0");
        return false;
    }

    for (auto path : paths) {
        auto securityLabel = GetSecurityLabel(path);
        if (securityLabel < 0) {
            LOGE("localDevsl < 0");
            return false;
        }
        if (remoteDevsl < securityLabel) {
            LOGE("remoteDevsl=%{public}d, securityLabel=%{public}d, path=%{public}s}",
                 remoteDevsl, securityLabel, Utils::GetAnonyString(path).c_str());
            return false;
        }
    }
    return true;
}

int32_t DevslDispatcher::GetDeviceDevsl(const std::string &networkId)
{
    if (networkId.empty()) {
        LOGE("GetDeviceDevsl networkId is empty");
        return -1;
    }
    std::lock_guard<std::mutex> lock(mutex);
    auto iter = devslMap_.find(networkId);
    if (iter != devslMap_.end()) {
        LOGI("find devsl in map");
        return iter->second;
    }

    std::string udid;
    auto &deviceManager = DistributedHardware::DeviceManager::GetInstance();
    deviceManager.GetUdidByNetworkId(IDaemon::SERVICE_NAME, networkId, udid);

    DEVSLQueryParams queryParams = MakeDevslQueryParams(udid);
    uint32_t levelInfo;
    auto ret = DATASL_GetHighestSecLevel(&queryParams, &levelInfo);
    if (ret != 0) {
        LOGE("devsl dispatcher register callback error %{public}d", ret);
        return -1;
    }
    devslMap_[networkId] = static_cast<int32_t>(levelInfo);
    return devslMap_[networkId];
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
