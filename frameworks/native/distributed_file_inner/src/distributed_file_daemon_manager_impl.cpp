/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "distributed_file_daemon_proxy.h"
#include "utils_log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD001600
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

int32_t DistributedFileDaemonManagerImpl::OpenP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (!distributedFileDaemonProxy) {
        LOGE("proxy is null");

        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->OpenP2PConnection(deviceInfo);
}

int32_t DistributedFileDaemonManagerImpl::CloseP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (!distributedFileDaemonProxy) {
        LOGE("proxy is null");

        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->CloseP2PConnection(deviceInfo);
}

int32_t DistributedFileDaemonManagerImpl::OpenP2PConnectionEx(const std::string &networkId,
                                                              sptr<IFileDfsListener> remoteReverseObj)
{
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (!distributedFileDaemonProxy) {
        LOGE("proxy is null.");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->OpenP2PConnectionEx(networkId, remoteReverseObj);
}

int32_t DistributedFileDaemonManagerImpl::CloseP2PConnectionEx(const std::string &networkId)
{
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (!distributedFileDaemonProxy) {
        LOGE("proxy is null.");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->CloseP2PConnectionEx(networkId);
}

int32_t DistributedFileDaemonManagerImpl::PrepareSession(const std::string &srcUri,
                                                         const std::string &dstUri,
                                                         const std::string &srcDeviceId,
                                                         const sptr<IRemoteObject> &listener,
                                                         HmdfsInfo &info)
{
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->PrepareSession(srcUri, dstUri, srcDeviceId, listener, info);
}

int32_t DistributedFileDaemonManagerImpl::RequestSendFile(const std::string &srcUri,
                                                          const std::string &dstPath,
                                                          const std::string &remoteDeviceId,
                                                          const std::string &sessionName)
{
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->RequestSendFile(srcUri, dstPath, remoteDeviceId, sessionName);
}

int32_t DistributedFileDaemonManagerImpl::GetRemoteCopyInfo(const std::string &srcUri, bool &isFile, bool &isDir)
{
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->GetRemoteCopyInfo(srcUri, isFile, isDir);
}

int32_t DistributedFileDaemonManagerImpl::CancelCopyTask(const std::string &sessionName)
{
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->CancelCopyTask(sessionName);
}

int32_t DistributedFileDaemonManagerImpl::PushAsset(int32_t userId,
                                                    const sptr<AssetObj> &assetObj,
                                                    const sptr<IAssetSendCallback> &sendCallback)
{
    LOGI("DistributedFileDaemonManagerImpl PushAsset enter.");
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->PushAsset(userId, assetObj, sendCallback);
}

int32_t DistributedFileDaemonManagerImpl::RegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback)
{
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
    return OHOS::FileManagement::E_OK;
}

int32_t DistributedFileDaemonManagerImpl::UnRegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback)
{
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
    return OHOS::FileManagement::E_OK;
}

int32_t GetSize(const std::string &uri, bool isSrcUri, uint64_t &size)
{
    return FileSizeUtils::GetSize(uri, isSrcUri, size);
}

int32_t IsDirectory(const std::string &uri, bool isSrcUri, bool &isDirectory)
{
    return FileSizeUtils::IsDirectory(uri, isSrcUri, isDirectory);
}

int32_t DistributedFileDaemonManagerImpl::Copy(const std::string &srcUri,
    const std::string &destUri, ProcessCallback processCallback)
{
    return FileCopyManager::GetInstance()->Copy(srcUri, destUri, processCallback);
}

int32_t DistributedFileDaemonManagerImpl::Cancel(const std::string &srcUri, const std::string &destUri)
{
    return FileCopyManager::GetInstance()->Cancel(srcUri, destUri);
}

int32_t DistributedFileDaemonManagerImpl::Cancel()
{
    return FileCopyManager::GetInstance()->Cancel();
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS