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

#include "dfs_error.h"
#include "distributed_file_daemon_proxy.h"
#include "utils_log.h"

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
<<<<<<< HEAD
=======

>>>>>>> 25c44d380113487e6f05ec126f2dc7d3185f0bd2
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->CloseP2PConnectionEx(networkId);
}

<<<<<<< HEAD

=======
>>>>>>> 25c44d380113487e6f05ec126f2dc7d3185f0bd2
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
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS