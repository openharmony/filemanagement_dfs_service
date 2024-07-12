/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "all_connect/all_connect_manager.h"

#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <thread>

#include "dfs_error.h"
#include "network/softbus/softbus_handler.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
#ifdef __LP64__
constexpr const char *ALL_CONNECT_SO_PATH = "/system/lib64/";
#else
constexpr const char *ALL_CONNECT_SO_PATH = "/system/lib/";
#endif
constexpr const char *ALL_CONNECT_SO_NAME = "libcfwk_allconnect_client.z.so";
std::shared_ptr<BlockObject<bool>> AllConnectManager::applyResultBlock_;
constexpr int32_t DFS_QOS_TYPE_MIN_BW = 90 * 1024 * 1024;
constexpr int32_t DFS_QOS_TYPE_MAX_LATENCY = 10000;
constexpr int32_t DFS_QOS_TYPE_MIN_LATENCY = 2000;
AllConnectManager::AllConnectManager()
{
    allConnectCallback_.OnStop = &AllConnectManager::OnStop;
    allConnectCallback_.ApplyResult = &AllConnectManager::ApplyResult;
}

AllConnectManager &AllConnectManager::GetInstance()
{
    static AllConnectManager instance;
    return instance;
}

int32_t AllConnectManager::InitAllConnectManager()
{
    LOGI("InitAllConnectManager begin");
    int32_t ret = GetAllConnectSoLoad();
    if (ret != FileManagement::ERR_OK) {
        LOGE("InitAllConnectManager failed, all connect so not exist or load so error, ret %{public}d", ret);
        return ret;
    }

    ret = RegisterLifecycleCallback();
    if (ret != FileManagement::ERR_OK) {
        LOGE("InitAllConnectManager failed, register lifecycle callback error, ret %{public}d", ret);
        return ret;
    }
    LOGI("InitAllConnectManager success");
    return FileManagement::ERR_OK;
}

int32_t AllConnectManager::UnInitAllConnectManager()
{
    LOGI("UnInitAllConnectManager begin");
    int32_t ret = UnRegisterLifecycleCallback();
    if (ret != FileManagement::ERR_OK) {
        LOGE("UnInitAllConnectManagerfailed, unregister lifecycle callback error, ret %{public}d", ret);
    }
    dlclose(dllHandle_);
    dllHandle_ = nullptr;
    return FileManagement::ERR_OK;
}

int32_t AllConnectManager::PublishServiceState(const std::string &peerNetworkId,
                                               ServiceCollaborationManagerBussinessStatus state)
{
    LOGI("PublishServiceState %{public}d begin", state);
    std::lock_guard<std::mutex> lock(allConnectLock_);
    if (dllHandle_ == nullptr) {
        LOGE("dllHandle_ is nullptr, all connect not support.");
        return FileManagement::ERR_OK;
    }
    if (allConnect_.ServiceCollaborationManager_PublishServiceState == nullptr) {
        LOGE("PublishServiceState is nullptr, all connect function not load.");
        return FileManagement::ERR_DLOPEN;
    }

    int32_t ret = allConnect_.ServiceCollaborationManager_PublishServiceState(peerNetworkId.c_str(),
                                                                              SERVICE_NAME.c_str(),
                                                                              "", state);
    if (ret != FileManagement::ERR_OK) {
        LOGE("PublishServiceState %{public}d fail, ret %{public}d", state, ret);
        return FileManagement::ERR_PUBLISH_STATE;
    }
    return FileManagement::ERR_OK;
}

int32_t AllConnectManager::ApplyAdvancedResource(const std::string &peerNetworkId,
    ServiceCollaborationManager_ResourceRequestInfoSets *resourceRequest)
{
    LOGI("ApplyAdvancedResource begin");
    std::lock_guard<std::mutex> lock(allConnectLock_);
    if (dllHandle_ == nullptr) {
        LOGE("dllHandle_ is nullptr, all connect not support.");
        return FileManagement::ERR_OK;
    }
    if (allConnect_.ServiceCollaborationManager_ApplyAdvancedResource == nullptr) {
        LOGE("PublishServiceState is nullptr, all connect function not load.");
        return FileManagement::ERR_DLOPEN;
    }

    if (applyResultBlock_ == nullptr) {
        applyResultBlock_ = std::make_shared<BlockObject<bool>>(BLOCK_INTERVAL_ALLCONNECT, false);
    }

    int32_t ret = allConnect_.ServiceCollaborationManager_ApplyAdvancedResource(peerNetworkId.c_str(),
                                                                                SERVICE_NAME.c_str(),
                                                                                resourceRequest,
                                                                                &allConnectCallback_);
    if (ret != FileManagement::ERR_OK) {
        LOGE("ApplyAdvancedResource fail, ret %{public}d", ret);
        return FileManagement::ERR_APPLY_RESULT;
    }
    auto success = applyResultBlock_->GetValue();
    if (!success) {
        LOGE("applyResult is reject");
        return FileManagement::ERR_APPLY_RESULT;
    }
    return FileManagement::ERR_OK;
}

int32_t AllConnectManager::GetAllConnectSoLoad()
{
    LOGI("GetAllConnectSoLoad begin");
    std::lock_guard<std::mutex> lock(allConnectLock_);
    char path[PATH_MAX + 1] = {0x00};
    std::string soPathName = std::string(ALL_CONNECT_SO_PATH) + std::string(ALL_CONNECT_SO_NAME);
    if (soPathName.empty() || (soPathName.length() > PATH_MAX) ||
        (realpath(soPathName.c_str(), path) == nullptr)) {
        LOGE("all connect so load failed, soPath=%{public}s not exist.", soPathName.c_str());
        return FileManagement::ERR_DLOPEN;
    }

    int32_t (*allConnectProxy)(ServiceCollaborationManager_API *exportapi) = nullptr;

    dllHandle_ = dlopen(path, RTLD_LAZY);
    if (dllHandle_ == nullptr) {
        LOGE("dlopen fail");
        return FileManagement::ERR_DLOPEN;
    }

    allConnectProxy = reinterpret_cast<int32_t (*)(ServiceCollaborationManager_API *exportapi)>(
        dlsym(dllHandle_, "ServiceCollaborationManager_Export"));
    if (allConnectProxy == nullptr) {
        dlclose(dllHandle_);
        dllHandle_ = nullptr;
        LOGE("dlsym allConnectProxy fail");
        return FileManagement::ERR_DLOPEN;
    }

    int32_t ret = allConnectProxy(&allConnect_);
    if (ret != FileManagement::ERR_OK) {
        dlclose(dllHandle_);
        dllHandle_ = nullptr;
        LOGE("get function struct fail, ret %{public}d", ret);
        return FileManagement::ERR_DLOPEN;
    }
    LOGI("so load success");
    return FileManagement::ERR_OK;
}

int32_t AllConnectManager::RegisterLifecycleCallback()
{
    LOGI("RegisterLifecycleCallback begin");
    std::lock_guard<std::mutex> lock(allConnectLock_);
    if (dllHandle_ == nullptr) {
        LOGE("dllHandle_ is nullptr, all connect so has not been loaded.");
        return FileManagement::ERR_DLOPEN;
    }
    if (allConnect_.ServiceCollaborationManager_RegisterLifecycleCallback == nullptr) {
        LOGE("RegisterLifecycleCallback is nullptr, all connect function not load.");
        return FileManagement::ERR_DLOPEN;
    }

    int32_t ret = allConnect_.ServiceCollaborationManager_RegisterLifecycleCallback(SERVICE_NAME.c_str(),
                                                                                    &allConnectCallback_);
    if (ret != FileManagement::ERR_OK) {
        LOGE("RegisterLifecycleCallback fail, ret %{public}d", ret);
        return FileManagement::ERR_ALLCONNECT;
    }
    return FileManagement::ERR_OK;
}

int32_t AllConnectManager::UnRegisterLifecycleCallback()
{
    LOGI("UnRegisterLifecycleCallback begin");
    std::lock_guard<std::mutex> lock(allConnectLock_);
    if (dllHandle_ == nullptr) {
        LOGE("dllHandle_ is nullptr, all connect so has not been loaded.");
        return FileManagement::ERR_DLOPEN;
    }
    if (allConnect_.ServiceCollaborationManager_UnRegisterLifecycleCallback == nullptr) {
        LOGE("RegisterLifecycleCallback is nullptr, all connect function not load.");
        return FileManagement::ERR_DLOPEN;
    }

    int32_t ret = allConnect_.ServiceCollaborationManager_UnRegisterLifecycleCallback(SERVICE_NAME.c_str());
    if (ret != FileManagement::ERR_OK) {
        LOGE("UnRegisterLifecycleCallback fail, ret %{public}d", ret);
        return FileManagement::ERR_ALLCONNECT;
    }
    return FileManagement::ERR_OK;
}

int32_t AllConnectManager::ApplyResult(int32_t errorcode, int32_t result, const char *reason)
{
    LOGI("ApplyResult begin");
    if (result != PASS) {
        LOGE("Apply Result is Reject, errorcode is %{}d, reason is %{public}s", errorcode, reason);
        applyResultBlock_->SetValue(false);
        return FileManagement::ERR_APPLY_RESULT;
    }
    applyResultBlock_->SetValue(true);
    return FileManagement::ERR_OK;
}

int32_t AllConnectManager::OnStop(const char *peerNetworkId)
{
    LOGI("OnStop begin, peerNetworkId:%{public}s", Utils::GetAnonyString(peerNetworkId).c_str());
    std::thread([peerNetworkId]() {
        SoftBusHandler::GetInstance().CloseSessionWithNetworkId(peerNetworkId);
    }).detach();
    return FileManagement::ERR_OK;
}

std::shared_ptr<ServiceCollaborationManager_ResourceRequestInfoSets> AllConnectManager::BuildResourceRequest()
{
    auto resourceRequest = std::make_shared<ServiceCollaborationManager_ResourceRequestInfoSets>();

    if (remoteHardwareList_ == nullptr) {
        remoteHardwareList_ = std::make_shared<ServiceCollaborationManager_HardwareRequestInfo>();
        remoteHardwareList_->hardWareType = ServiceCollaborationManagerHardwareType::SCM_DISPLAY;
        remoteHardwareList_->canShare = true;
    }
    resourceRequest->remoteHardwareListSize = 1;
    resourceRequest->remoteHardwareList = remoteHardwareList_.get();

    if (localHardwareList_ == nullptr) {
        localHardwareList_ = std::make_shared<ServiceCollaborationManager_HardwareRequestInfo>();
        localHardwareList_->hardWareType = ServiceCollaborationManagerHardwareType::SCM_DISPLAY;
        localHardwareList_->canShare = true;
    }
    resourceRequest->localHardwareListSize = 1;
    resourceRequest->localHardwareList = localHardwareList_.get();

    if (communicationRequest_ == nullptr) {
        communicationRequest_ = std::make_shared<ServiceCollaborationManager_CommunicationRequestInfo>();
        communicationRequest_->minBandwidth = DFS_QOS_TYPE_MIN_BW;
        communicationRequest_->maxLatency = DFS_QOS_TYPE_MAX_LATENCY;
        communicationRequest_->minLatency = DFS_QOS_TYPE_MIN_LATENCY;
        communicationRequest_->maxWaitTime = 0;
        communicationRequest_->dataType = "DATA_TYPE_FILE";
    }
    resourceRequest->communicationRequest = communicationRequest_.get();

    return resourceRequest;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS