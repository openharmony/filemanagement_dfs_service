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

#ifndef OHOS_STORAGE_DISTRIBUTED_FILE_DAEMON_PROXY_H
#define OHOS_STORAGE_DISTRIBUTED_FILE_DAEMON_PROXY_H

#include <cstdint>

#include "dm_device_info.h"
#include "ipc/i_daemon.h"
#include "iremote_proxy.h"
#include "asset/i_asset_recv_callback.h"
#include "asset/i_asset_send_callback.h"
#include "asset/asset_obj.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class DistributedFileDaemonProxy : public IRemoteProxy<IDaemon> {
public:
    explicit DistributedFileDaemonProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IDaemon>(impl) {}
    ~DistributedFileDaemonProxy() override {}

    static sptr<IDaemon> GetInstance();
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
                            const std::string &dstDeviceId,
                            const std::string &sessionName) override;
    int32_t GetRemoteCopyInfo(const std::string &srcUri, bool &isFile, bool &isDir) override;

    int32_t PushAsset(int32_t userId,
                      const sptr<AssetObj> &assetObj,
                      const sptr<IAssetSendCallback> &sendCallback) override;
    int32_t RegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override;
    int32_t UnRegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override;
private:
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
    static inline BrokerDelegator<DistributedFileDaemonProxy> delegator_;
    static inline sptr<DaemonDeathRecipient> deathRecipient_;
};

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // OHOS_STORAGE_DISTRIBUTED_FILE_DAEMON_PROXY_H