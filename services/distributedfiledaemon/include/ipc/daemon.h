/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef DAEMON_H
#define DAEMON_H

#include <memory>
#include <mutex>
#include <string>

#include "accesstoken_kit.h"
#include "daemon_event.h"
#include "daemon_eventhandler.h"
#include "daemon_execute.h"
#include "daemon_stub.h"
#include "device/device_profile_adapter.h"
#include "dm_device_info.h"
#include "file_trans_listener_proxy.h"
#include "hmdfs_info.h"
#include "ipc/i_daemon.h"
#include "iremote_stub.h"
#include "multiuser/os_account_observer.h"
#include "nocopyable.h"
#include "refbase.h"
#include "remote_file_share.h"
#include "system_ability.h"
#include "accesstoken_kit.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using HapTokenInfo = OHOS::Security::AccessToken::HapTokenInfo;
enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

class Daemon final : public SystemAbility, public DaemonStub, protected NoCopyable {
    DECLARE_SYSTEM_ABILITY(Daemon);

public:
    explicit Daemon(int32_t saID, bool runOnCreate = true) : SystemAbility(saID, runOnCreate) {};
    virtual ~Daemon() = default;

    void OnStart() override;
    void OnStop() override;
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    ServiceRunningState QueryServiceState() const
    {
        return state_;
    }

    int32_t OpenP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
    int32_t CloseP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo) override;
    int32_t OpenP2PConnectionEx(const std::string &networkId, sptr<IFileDfsListener> remoteReverseObj) override;
    int32_t CloseP2PConnectionEx(const std::string &networkId) override;
    int32_t ConnectionCount(const DistributedHardware::DmDeviceInfo &deviceInfo);
    int32_t CleanUp(const std::string &networkId);
    int32_t ConnectionAndMount(const DistributedHardware::DmDeviceInfo &deviceInfo, const std::string &networkId,
        sptr<IFileDfsListener> remoteReverseObj);
    int32_t InnerCopy(const std::string &srcUri, const std::string &dstUri,
        const std::string &srcDeviceId, const sptr<IFileTransListener> &listener, HmdfsInfo &info);
    int32_t PrepareSession(const std::string &srcUri,
                           const std::string &dstUri,
                           const std::string &srcDeviceId,
                           const sptr<IRemoteObject> &listener,
                           HmdfsInfo &info) override;
    int32_t CancelCopyTask(const std::string &sessionName) override;
    int32_t CancelCopyTask(const std::string &srcUri, const std::string &dstUri) override;
    int32_t RequestSendFile(const std::string &srcUri,
                            const std::string &dstPath,
                            const std::string &dstDeviceId,
                            const std::string &sessionName) override;
    // for ACL verification after 6.0.1 version
    int32_t RequestSendFileACL(const std::string &srcUri,
                               const std::string &dstPath,
                               const std::string &dstDeviceId,
                               const std::string &sessionName,
                               const AccountInfo &callerAccountInfo) override;
    int32_t GetRemoteCopyInfo(const std::string &srcUri, bool &isFile, bool &isDir) override;
    // for ACL verification after 6.0.1 version
    int32_t GetRemoteCopyInfoACL(const std::string &srcUri,
                                 bool &isFile,
                                 bool &isDir,
                                 const AccountInfo &callerAccountInfo) override;

    int32_t PushAsset(int32_t userId,
                      const sptr<AssetObj> &assetObj,
                      const sptr<IAssetSendCallback> &sendCallback) override;
    int32_t RegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override;
    int32_t UnRegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback) override;

    int32_t GetDfsUrisDirFromLocal(const std::vector<std::string> &uriList,
                                   const int32_t userId,
                                   std::unordered_map<std::string, AppFileService::ModuleRemoteFileShare::HmdfsUriInfo>
                                   &uriToDfsUriMaps) override;
    static int32_t Copy(const std::string &srcUri,
                        const std::string &dstPath,
                        const sptr<IDaemon> &daemon,
                        const std::string &sessionName,
                        const std::string &srcNetworkId);
    int32_t GetDfsSwitchStatus(const std::string &networkId, int32_t &switchStatus) override;
    int32_t UpdateDfsSwitchStatus(int32_t switchStatus) override;
    int32_t GetConnectedDeviceList(std::vector<DfsDeviceInfo> &deviceList) override;
    int32_t RegisterFileDfsListener(const std::string &instanceId, const sptr<IFileDfsListener> &listener) override;
    int32_t UnregisterFileDfsListener(const std::string &instanceId) override;
    int32_t IsSameAccountDevice(const std::string &networkId, bool &isSameAccount) override;

private:
    Daemon();
    ServiceRunningState state_ { ServiceRunningState::STATE_NOT_START };
    static sptr<Daemon> instance_;
    static std::mutex instanceLock_;
    bool registerToService_ { false };
    std::shared_ptr<OsAccountObserver> subScriber_;
    void PublishSA();
    void RegisterOsAccount();
    sptr<IDaemon> GetRemoteSA(const std::string &remoteDeviceId);
    int32_t StoreSessionAndListener(const std::string &physicalPath,
                                    std::string &sessionName,
                                    const sptr<IFileTransListener> &listener);
    void DeleteSessionAndListener(const std::string &sessionName);
    int32_t GetRealPath(const std::string &srcUri,
                        const std::string &dstUri,
                        std::string &physicalPath,
                        HmdfsInfo &info,
                        const sptr<IDaemon> &daemon,
                        const std::string &networkId);
    int32_t CheckCopyRule(std::string &physicalPath,
                          const std::string &dstUri,
                          HapTokenInfo &hapTokenInfo,
                          const bool &isSrcFile,
                          HmdfsInfo &info);
    int32_t HandleDestinationPathAndPermissions(const std::string &dstUri,
                                                bool isSrcFile,
                                                HmdfsInfo &info,
                                                std::string &physicalPath);
    int32_t SendDfsDelayTask(const std::string &networkId);
    void RemoveDfsDelayTask(const std::string &networkId);
    void DisconnectDevice(const std::string networkId);
    bool IsCallingDeviceTrusted();

    class DfsListenerDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        DfsListenerDeathRecipient(){};
        ~DfsListenerDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    };
    static inline sptr<DfsListenerDeathRecipient> dfsListenerDeathRecipient_;
    int32_t CopyBaseOnRPC(const std::string &srcUri,
                          const std::string &dstUri,
                          const std::string &srcDeviceId,
                          const sptr<IFileTransListener> &listenerCallback,
                          HmdfsInfo &info);

    void DisconnectByRemote(const std::string &networkId);
    int32_t CreatControlLink(const std::string &networkId);
    int32_t CancelControlLink(const std::string &networkId);
    int32_t CheckRemoteAllowConnect(const std::string &networkId);
    int32_t NotifyRemotePublishNotification(const std::string &networkId);
    int32_t NotifyRemoteCancelNotification(const std::string &networkId);
private:
    std::mutex connectMutex_;
    std::mutex eventHandlerMutex_;
    std::shared_ptr<DaemonEventHandler> eventHandler_;
    std::shared_ptr<DaemonExecute> daemonExecute_;
    void StartEventHandler();
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DAEMON_H
