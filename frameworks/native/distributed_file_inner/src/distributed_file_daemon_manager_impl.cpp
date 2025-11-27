/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "distributed_file_daemon_manager_impl.h"

#include "asset/asset_adapter_sa_client.h"
#include "copy/file_copy_manager.h"
#include "copy/file_size_utils.h"
#include "dfs_error.h"
#include "../../../../test/mock/cloudsyncservice/transport/softbus_adapter_mock.h"
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
#include "distributed_file_daemon_proxy.h"
#endif
#include "utils_log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD004315
#define LOG_TAG "distributedfile_daemon"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace OHOS::Storage;
DistributedFileDaemonManagerImpl &DistributedFileDaemonManagerImpl::GetInstance()
{
    static DistributedFileDaemonManagerImpl instance;
    return instance;
};

int32_t DistributedFileDaemonManagerImpl::ConnectDfs(const std::string &networkId)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (!distributedFileDaemonProxy) {
        LOGE("proxy is null");

        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    openP2PConnectionSem_.acquire();
    int32_t res = distributedFileDaemonProxy->ConnectDfs(networkId);
    openP2PConnectionSem_.release();
    return res;
#else
    return OHOS::FileManagement::OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::DisconnectDfs(const std::string &networkId)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (!distributedFileDaemonProxy) {
        LOGE("proxy is null");

        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->DisconnectDfs(networkId);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::OpenP2PConnectionEx(const std::string &networkId,
                                                              sptr<IFileDfsListener> remoteReverseObj)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (!distributedFileDaemonProxy) {
        LOGE("proxy is null.");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    openP2PConnectionExSem_.acquire();
    int32_t res = distributedFileDaemonProxy->OpenP2PConnectionEx(networkId, remoteReverseObj);
    openP2PConnectionExSem_.release();
    return res;
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::CloseP2PConnectionEx(const std::string &networkId)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (!distributedFileDaemonProxy) {
        LOGE("proxy is null.");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->CloseP2PConnectionEx(networkId);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::PrepareSession(const std::string &srcUri,
                                                         const std::string &dstUri,
                                                         const std::string &srcDeviceId,
                                                         const sptr<IRemoteObject> &listener,
                                                         HmdfsInfo &info)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->PrepareSession(srcUri, dstUri, srcDeviceId, listener, info);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::RequestSendFile(const std::string &srcUri,
                                                          const std::string &dstPath,
                                                          const std::string &remoteDeviceId,
                                                          const std::string &sessionName)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->RequestSendFile(srcUri, dstPath, remoteDeviceId, sessionName);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::GetRemoteCopyInfo(const std::string &srcUri, bool &isFile, bool &isDir)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->GetRemoteCopyInfo(srcUri, isFile, isDir);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::CancelCopyTask(const std::string &sessionName)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->CancelCopyTask(sessionName);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::CancelCopyTask(const std::string &srcUri, const std::string &dstUri)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->CancelCopyTask(srcUri, dstUri);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::PushAsset(int32_t userId,
                                                    const sptr<AssetObj> &assetObj,
                                                    const sptr<IAssetSendCallback> &sendCallback)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    LOGI("DistributedFileDaemonManagerImpl PushAsset enter.");
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->PushAsset(userId, assetObj, sendCallback);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::GetDfsSwitchStatus(const std::string &networkId, int32_t &switchStatus)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->GetDfsSwitchStatus(networkId, switchStatus);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::UpdateDfsSwitchStatus(int32_t switchStatus)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->UpdateDfsSwitchStatus(switchStatus);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::GetConnectedDeviceList(std::vector<DfsDeviceInfo> &deviceList)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->GetConnectedDeviceList(deviceList);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::RegisterFileDfsListener(const std::string &instanceId,
    const sptr<IFileDfsListener> &listener)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->RegisterFileDfsListener(instanceId, listener);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::UnregisterFileDfsListener(const std::string &instanceId)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->UnregisterFileDfsListener(instanceId);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::IsSameAccountDevice(const std::string &networkId, bool &isSameAccount)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->IsSameAccountDevice(networkId, isSameAccount);
#else
    return OHOS::FileManagement::E_OK;
#endif
}

int32_t DistributedFileDaemonManagerImpl::RegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    LOGI("DistributedFileDaemonManagerImpl registerAssetCallback enter.");

    if (recvCallback == nullptr) {
        LOGE("Register IAssetRecvCallback is null.");
        return OHOS::FileManagement::E_NULLPTR;
    }
    int32_t ret = AssetAdapterSaClient::GetInstance().AddListener(recvCallback);
    if (ret != OHOS::FileManagement::E_OK) {
        LOGE("AssetAdapterSaClient addListener fail.");
        return ret;
    }
#endif
    return OHOS::FileManagement::E_OK;
}

int32_t DistributedFileDaemonManagerImpl::UnRegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback)
{
#ifdef DFS_ENABLE_DISTRIBUTED_ABILITY
    LOGI("DistributedFileDaemonManagerImpl unRegisterAssetCallback enter.");
    if (recvCallback == nullptr) {
        LOGE("UnRegister IAssetRecvCallback is null.");
        return OHOS::FileManagement::E_NULLPTR;
    }
    int32_t ret = AssetAdapterSaClient::GetInstance().RemoveListener(recvCallback);
    if (ret != OHOS::FileManagement::E_OK) {
        LOGE("AssetAdapterSaClient removeListener fail.");
        return ret;
    }
#endif
    return OHOS::FileManagement::E_OK;
}

int32_t DistributedFileDaemonManagerImpl::GetSize(const std::string &uri, bool isSrcUri, uint64_t &size)
{
    return FileSizeUtils::GetSize(uri, isSrcUri, size);
}

int32_t DistributedFileDaemonManagerImpl::IsDirectory(const std::string &uri, bool isSrcUri, bool &isDirectory)
{
    return FileSizeUtils::IsDirectory(uri, isSrcUri, isDirectory);
}

int32_t DistributedFileDaemonManagerImpl::Copy(const std::string &srcUri,
    const std::string &destUri, ProcessCallback processCallback)
{
    return FileCopyManager::GetInstance().Copy(srcUri, destUri, processCallback);
}

int32_t DistributedFileDaemonManagerImpl::Cancel(const std::string &srcUri, const std::string &destUri)
{
    return FileCopyManager::GetInstance().Cancel(srcUri, destUri);
}

int32_t DistributedFileDaemonManagerImpl::Cancel()
{
    return FileCopyManager::GetInstance().Cancel();
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
