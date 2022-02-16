/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sendfile.h"

#include <fcntl.h>
#include <unistd.h>
#include <tuple>

#include "dfs_filetransfer_callback.h"
#include "event_agent.h"
#include "filetransfer_callback_proxy.h"
#include "i_distributedfile_service.h"
#include "iservice_registry.h"
#include "service_proxy.h"
#include "system_ability_definition.h"
#include "system_ability_manager.h"
#include "trans_event.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
const std::string DfsAppUid { "SendFileTestUid" };
constexpr int32_t FILE_BLOCK_SIZE = 1024;
const std::string APP_PATH { "/data/accounts/account_0/appdata/" };
}

napi_value RegisterSendFileNotifyCallback()
{
    sptr<ISystemAbilityManager> systemAbilityMgr =
    SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        LOGE("SendFile napi Regist callback Get ISystemAbilityManager failed ... \n");
        return nullptr;
    }

    sptr<IRemoteObject> remote = systemAbilityMgr->GetSystemAbility(STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID);
    if (remote == nullptr) {
        LOGE("SendFile napi Regist callback Get SaId = %{public}d fail ... \n", STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID);
        return nullptr;
    }

    sptr<IDistributedFileService> distributedFileService = iface_cast<IDistributedFileService>(remote);
    if (distributedFileService == nullptr) {
        LOGE("SendFile napi Regist callback == nullptr\n");
        return nullptr;
    }

    sptr<IFileTransferCallback> callback = (std::make_unique<DfsFileTransferCallback>()).release();
    LOGI("INotifyCallback == callback %{public}p", callback->AsObject().GetRefPtr());
    distributedFileService->RegisterNotifyCallback(callback);
    return nullptr;
}

int32_t NapiDeviceOnline(const std::string &cid)
{
    for (std::unordered_map<std::string, EventAgent*>::iterator iter = g_mapUidToEventAgent.begin();
        iter != g_mapUidToEventAgent.end(); ++iter) {
        EventAgent* agent = iter->second;
        if (agent != nullptr && agent->FindDevice(cid)) {
            LOGI("NapiDeviceOnline, event agent for device[%{public}s] was found.", cid.c_str());
            agent->InsertDevice(cid);
            break;
        }
    }
    return 0;
}

int32_t NapiDeviceOffline(const std::string &cid)
{
    for (std::unordered_map<std::string, EventAgent*>::iterator iter = g_mapUidToEventAgent.begin();
        iter != g_mapUidToEventAgent.end(); ++iter) {
        EventAgent* agent = iter->second;
        if (agent != nullptr && agent->FindDevice(cid)) {
            LOGI("NapiDeviceOffline, event agent for device[%{public}s] was found.", cid.c_str());
            agent->RemoveDevice(cid);
            break;
        }
    }
    return 0;
}

int32_t NapiSendFinished(const std::string &cid, const std::string &fileName)
{
    EventAgent* agent = nullptr;
    std::unordered_map<std::string, EventAgent*>::iterator iter;
    for (iter = g_mapUidToEventAgent.begin(); iter != g_mapUidToEventAgent.end(); ++iter) {
        agent = iter->second;
        if (agent != nullptr && agent->FindDevice(cid)) {
            LOGI("DEBUG_SENDFILE:OnSendFinished, event agent for device[%{public}s] was found.", cid.c_str());
            break;
        } else {
            sptr<ISystemAbilityManager> systemAbilityMgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (systemAbilityMgr == nullptr) {
                return -1;
            }
            sptr<IRemoteObject> remote = systemAbilityMgr->GetSystemAbility(STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID);
            if (remote == nullptr) {
                return -1;
            }
            sptr<IDistributedFileService> distributedFileService = iface_cast<IDistributedFileService>(remote);
            if (distributedFileService == nullptr) {
                return -1;
            }
            if (distributedFileService->IsDeviceOnline(cid) == 1) {
                agent->InsertDevice(cid);
            }
            LOGI("DEBUG_SENDFILE:OnSendFinished, ------ no event agent for device[%{public}s].", cid.c_str());
        }
    }

    iter = g_mapUidToEventAgent.find(DfsAppUid);
    if (g_mapUidToEventAgent.end() != iter) {
        TransEvent event(TransEvent::TRANS_SUCCESS, fileName);
        agent->Emit("sendFinished", reinterpret_cast<Event*>(&event));
        LOGI("DEBUG_SENDFILE:OnSendFinished, [%{public}s] event was done.", cid.c_str());
        return 0;
    } else {
        LOGI("DEBUG_SENDFILE:OnSendFinished, [%{public}s] no event processor.", cid.c_str());
        return -1;
    }
}

int32_t NapiSendError(const std::string &cid)
{
    EventAgent* agent = nullptr;
    std::unordered_map<std::string, EventAgent*>::iterator iter;
    for (iter = g_mapUidToEventAgent.begin(); iter != g_mapUidToEventAgent.end(); ++iter) {
        agent = iter->second;
        if (agent != nullptr && agent->FindDevice(cid)) {
            LOGI("DEBUG_SENDFILE:OnSendError, event agent for device[%{public}s] was found.", cid.c_str());
            break;
        } else {
            sptr<ISystemAbilityManager> systemAbilityMgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (systemAbilityMgr == nullptr) {
                return -1;
            }
            sptr<IRemoteObject> remote = systemAbilityMgr->GetSystemAbility(STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID);
            if (remote == nullptr) {
                return -1;
            }
            sptr<IDistributedFileService> distributedFileService = iface_cast<IDistributedFileService>(remote);
            if (distributedFileService == nullptr) {
                return -1;
            }
            if (distributedFileService->IsDeviceOnline(cid) == 1) {
                agent->InsertDevice(cid);
            }
            LOGI("DEBUG_SENDFILE:OnSendError, no event agent for device[%{public}s].", cid.c_str());
        }
    }

    iter = g_mapUidToEventAgent.find(DfsAppUid);
    if (g_mapUidToEventAgent.end() != iter) {
        TransEvent event(TransEvent::TRANS_FAILURE);
        agent->Emit("sendFinished", reinterpret_cast<Event*>(&event));
        LOGI("DEBUG_SENDFILE:OnSendError, [%{public}s] event was done.", cid.c_str());
        return 0;
    } else {
        LOGI("DEBUG_SENDFILE:OnSendError, [%{public}s] no event processor.", cid.c_str());
        return -1;
    }
}

int32_t NapiReceiveFinished(const std::string &cid, const std::string &fileName, uint32_t num)
{
    EventAgent* agent = nullptr;
    std::unordered_map<std::string, EventAgent*>::iterator iter;
    for (auto iter = g_mapUidToEventAgent.begin(); iter != g_mapUidToEventAgent.end(); ++iter) {
        agent = iter->second;
        if (agent != nullptr && agent->FindDevice(cid)) {
            LOGI("DEBUG_SENDFILE:OnReceiveFinished, event agent for device[%{public}s] was found.", cid.c_str());
            break;
        } else {
            sptr<ISystemAbilityManager> systemAbilityMgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (systemAbilityMgr == nullptr) {
                return -1;
            }
            sptr<IRemoteObject> remote = systemAbilityMgr->GetSystemAbility(STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID);
            if (remote == nullptr) {
                return -1;
            }
            sptr<IDistributedFileService> distributedFileService = iface_cast<IDistributedFileService>(remote);
            if (distributedFileService == nullptr) {
                return -1;
            }
            if (distributedFileService->IsDeviceOnline(cid) == 1) {
                agent->InsertDevice(cid);
            }
            LOGI("DEBUG_SENDFILE:OnReceiveFinished, no event agent for device[%{public}s].", cid.c_str());
        }
    }

    iter = g_mapUidToEventAgent.find(DfsAppUid);
    if (g_mapUidToEventAgent.end() != iter) {
        TransEvent event(TransEvent::TRANS_SUCCESS, fileName, num);
        agent->Emit("receiveFinished", reinterpret_cast<Event*>(&event));
        LOGI("DEBUG_SENDFILE:OnReceiveFinished, [%{public}s] event was done.", cid.c_str());
        return 0;
    } else {
        LOGI("DEBUG_SENDFILE:OnReceiveFinished, [%{public}s] no event processor.", cid.c_str());
        return -1;
    }
}

int32_t NapiReceiveError(const std::string &cid)
{
    EventAgent* agent = nullptr;
    std::unordered_map<std::string, EventAgent*>::iterator iter;
    for (auto iter = g_mapUidToEventAgent.begin(); iter != g_mapUidToEventAgent.end(); ++iter) {
        agent = iter->second;
        if (agent != nullptr && agent->FindDevice(cid)) {
            LOGI("OnReceiveError : event agent for device[%{public}s] was found.", cid.c_str());
            break;
        } else {
            sptr<ISystemAbilityManager> systemAbilityMgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (systemAbilityMgr == nullptr) {
                return -1;
            }
            sptr<IRemoteObject> remote = systemAbilityMgr->GetSystemAbility(STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID);
            if (remote == nullptr) {
                return -1;
            }
            sptr<IDistributedFileService> distributedFileService = iface_cast<IDistributedFileService>(remote);
            if (distributedFileService == nullptr) {
                return -1;
            }
            if (distributedFileService->IsDeviceOnline(cid) == 1) {
                agent->InsertDevice(cid);
            }
            LOGI("OnReceiveError : no event agent for device[%{public}s].", cid.c_str());
        }
    }

    iter = g_mapUidToEventAgent.find(DfsAppUid);
    if (g_mapUidToEventAgent.end() != iter) {
        TransEvent event(TransEvent::TRANS_FAILURE);
        agent->Emit("receiveFinished", reinterpret_cast<Event*>(&event));
        LOGI("OnReceiveError : [%{public}s] event was done.", cid.c_str());
        return 0;
    } else {
        LOGI("OnReceiveError : [%{public}s] no event processor.", cid.c_str());
        return -1;
    }
}

int32_t NapiWriteFile(int32_t fd, const std::string &fileName)
{
    if (fd <= 0) {
        return -1;
    }
    std::string filePath = APP_PATH + fileName;
    int32_t flags = O_WRONLY;

    if (!Utils::IsFileExist(filePath)) {
        flags |= O_CREAT;
    }

    int32_t writeFd = open(filePath.c_str(), flags, (S_IREAD | S_IWRITE) | S_IRGRP | S_IROTH);
    if (writeFd <= 0) {
        close(fd);
        LOGE("NapiWriteFile open file failed %{public}d, %{public}s, %{public}d", writeFd, strerror(errno), errno);
        return -1;
    }
    auto buffer = std::make_unique<char[]>(FILE_BLOCK_SIZE);
    ssize_t actLen = 0;
    do {
        actLen = read(fd, buffer.get(), FILE_BLOCK_SIZE);
        if (actLen > 0) {
            write(writeFd, buffer.get(), actLen);
        } else if (actLen == 0) {
            break;
        } else {
            if (errno == EINTR) {
                actLen = FILE_BLOCK_SIZE;
            } else {
                close(fd);
                close(writeFd);
                return -1;
            }
        }
    } while (actLen > 0);

    close(fd);
    close(writeFd);

    return 0;
}

void SetEventAgentMap(const std::unordered_map<std::string, EventAgent*> &map)
{
    g_mapUidToEventAgent = map;
}

int32_t ExecSendFile(const std::string &deviceId, const std::vector<std::string>& srcList,
    const std::vector<std::string>& dstList, uint32_t num)
{
    if (deviceId.empty()) {
        LOGE("DeviceId can't be emptey.\n");
        return -1;
    }
    sptr<ISystemAbilityManager> systemAbilityMgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        LOGE("BundleService Get ISystemAbilityManager failed ... \n");
        return -1;
    }
    sptr<IRemoteObject> remote = systemAbilityMgr->CheckSystemAbility(STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID);
    if (remote == nullptr) {
        LOGE("DistributedFileService Get STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID = %{public}d fail ... \n",
            STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID);
        return -1;
    }

    sptr<IDistributedFileService> distributedFileService = iface_cast<IDistributedFileService>(remote);
    if (distributedFileService == nullptr) {
        LOGE("DistributedFileService == nullptr\n");
        return -1;
    }

    int32_t fd = open(srcList.at(0).c_str(), O_RDONLY);
    int32_t result = distributedFileService->OpenFile(fd, srcList.at(0), (S_IREAD | S_IWRITE) | S_IRGRP | S_IROTH);
    result = distributedFileService->SendFile(deviceId, srcList, dstList, num);
    LOGI(" ------------ DistributedFileService SendFile result %{public}d", result);
    return result;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS