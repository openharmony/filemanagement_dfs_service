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

#ifndef DISTRIBUTED_FILE_DAEMON_MANAGER_IMPL_H
#define DISTRIBUTED_FILE_DAEMON_MANAGER_IMPL_H

#include "asset/asset_obj.h"
#include "asset/i_asset_recv_callback.h"
#include "asset/i_asset_send_callback.h"
#include "dm_device_info.h"
#include "dm_device_info_ext.h"
#include "ipc/distributed_file_daemon_manager.h"
#include "ipc/i_file_dfs_listener.h"
#include "nocopyable.h"
#include "refbase.h"
#include "idaemon.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class DistributedFileDaemonManagerImpl final : public DistributedFileDaemonManager, public NoCopyable {
public:
    static DistributedFileDaemonManagerImpl &GetInstance();
    static sptr<IDaemon> GetDaemonInterface();
    static void InvaildInstance();

    int32_t OpenP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
    int32_t CloseP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
    int32_t OpenP2PConnectionEx(const std::string &networkId, sptr<IFileDfsListener> remoteReverseObj) override;
    int32_t CloseP2PConnectionEx(const std::string &networkId) override;
    int32_t PrepareSession(const std::string &srcUri,
                           const std::string &dstUri,
                           const std::string &srcDeviceId,
                           const sptr<IRemoteObject> &listener,
                           HmdfsInfo &info) override;
    int32_t CancelCopyTask(const std::string &sessionName) override;
    int32_t RequestSendFile(const std::string &srcUri,
                            const std::string &dstPath,
                            const std::string &remoteDeviceId,
                            const std::string &sessionName);
    int32_t GetRemoteCopyInfo(const std::string &srcUri, bool &isFile, bool &isDir);

    int32_t PushAsset(int32_t userId,
                      const sptr<AssetObj> &assetObj,
                      const sptr<IAssetSendCallback> &sendCallback) override;
    int32_t RegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override;
    int32_t UnRegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override;

    int32_t GetSize(const std::string &uri, bool isSrcUri, uint64_t &size) override;
    int32_t IsDirectory(const std::string &uri, bool isSrcUri, bool &isDirectory) override;
    int32_t Copy(const std::string &srcUri, const std::string &destUri, ProcessCallback processCallback) override;
    int32_t Cancel(const std::string &srcUri, const std::string &destUri) override;
    int32_t Cancel() override;
private:
    DistributedFileDaemonManagerImpl() = default;
    class DaemonDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        DaemonDeathRecipient(){};
        ~DaemonDeathRecipient() = default;
        using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
        void SetDeathRecipient(RemoteDiedHandler handler);
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        RemoteDiedHandler handler_{ nullptr };
    };
    static void OnRemoteSaDied(const wptr<IRemoteObject> &remote);
    static inline std::mutex proxyMutex_;
    static inline sptr<IDaemon> daemonProxy_;
    static inline sptr<DaemonDeathRecipient> deathRecipient_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // DISTRIBUTED_FILE_DAEMON_MANAGER_IMPL_H