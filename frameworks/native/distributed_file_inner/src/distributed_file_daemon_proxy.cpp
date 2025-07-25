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

#include "distributed_file_daemon_proxy.h"

#include <sstream>

#include "copy/ipc_wrapper.h"
#include "dfs_error.h"
#include "ipc/distributed_file_daemon_ipc_interface_code.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "utils_log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD004315
#define LOG_TAG "distributedfile_daemon"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
using namespace OHOS::Storage;
constexpr int32_t INDEX1 = 1;
constexpr int32_t INDEX2 = 2;
constexpr int32_t STEP = 3;

static int32_t ReadUriInfoFromResult(const std::vector<std::string> &result,
    std::unordered_map<std::string, AppFileService::ModuleRemoteFileShare::HmdfsUriInfo> &uriToDfsUriMaps)
{
    for (size_t i = 0; (i < result.size()) && (i + INDEX2 < result.size()); i += STEP) {
        AppFileService::ModuleRemoteFileShare::HmdfsUriInfo info;
        info.uriStr = result[i + INDEX1];
        std::istringstream iss(result[i + INDEX2]);
        iss >> info.fileSize;
        if (iss.fail()) {
            LOGE("read fileSize failed: %{public}s", result[i + INDEX2].c_str());
            return OHOS::FileManagement::E_INVAL_ARG;
        }
        uriToDfsUriMaps[result[i]] = info;
    }
    LOGI("ReadUriInfoFromResult success, uriToDfsUriMaps size:%{public}d", static_cast<int>(uriToDfsUriMaps.size()));
    return OHOS::FileManagement::E_OK;
}

void DistributedFileDaemonProxy::OnRemoteSaDied(const wptr<IRemoteObject> &remote)
{
    LOGI("dfs service death");
    unique_lock<mutex> lock(proxyMutex_);
    daemonProxy_ = nullptr;
}

void DistributedFileDaemonProxy::DaemonDeathRecipient::SetDeathRecipient(RemoteDiedHandler handler)
{
    handler_ = std::move(handler);
}

void DistributedFileDaemonProxy::DaemonDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    LOGI("start");
    if (handler_ != nullptr) {
        handler_(remote);
    }
}

sptr<IDaemon> DistributedFileDaemonProxy::GetInstance()
{
    LOGI("getinstance");
    unique_lock<mutex> lock(proxyMutex_);
    if (daemonProxy_ != nullptr) {
        return daemonProxy_;
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Samgr is nullptr");
        return nullptr;
    }

    auto object = samgr->CheckSystemAbility(FILEMANAGEMENT_DISTRIBUTED_FILE_DAEMON_SA_ID);
    if (object == nullptr) {
        LOGE("object == nullptr");
        return nullptr;
    }

    if (deathRecipient_ == nullptr) {
        deathRecipient_ = new (std::nothrow) DaemonDeathRecipient();
        if (deathRecipient_ == nullptr) {
            LOGE("new death recipient failed");
            return nullptr;
        }
    }
    deathRecipient_->SetDeathRecipient([](const wptr<IRemoteObject> &remote) { OnRemoteSaDied(remote); });
    if ((object->IsProxyObject()) && (!object->AddDeathRecipient(deathRecipient_))) {
        LOGE("failed to add death recipient.");
        return nullptr;
    }

    daemonProxy_ = iface_cast<IDaemon>(object);
    if (daemonProxy_ == nullptr) {
        LOGE("service == nullptr");
        return nullptr;
    }
    return daemonProxy_;
}

int32_t DistributedFileDaemonProxy::OpenP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("Open p2p connection");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteCString(deviceInfo.deviceId)) {
        LOGE("Failed to send device id");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteCString(deviceInfo.deviceName)) {
        LOGE("Failed to send device name");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteCString(deviceInfo.networkId)) {
        LOGE("Failed to send network id");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteUint16(deviceInfo.deviceTypeId)) {
        LOGE("Failed to send deviceTypeId");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteUint32(deviceInfo.range)) {
        LOGE("Failed to send range");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteInt32(deviceInfo.authForm)) {
        LOGE("Failed to send user id");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION),
        data, reply, option);
    if (ret != 0) {
        stringstream ss;
        ss << "Failed to send out the requeset, errno:" << ret;
        LOGE("%{public}s", ss.str().c_str());
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    LOGI("Open p2p connection Success");
    return reply.ReadInt32();
}

int32_t DistributedFileDaemonProxy::CloseP2PConnection(const DistributedHardware::DmDeviceInfo &deviceInfo)
{
    LOGI("Close p2p connection");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteCString(deviceInfo.deviceId)) {
        LOGE("Failed to send device id");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteCString(deviceInfo.deviceName)) {
        LOGE("Failed to send device name");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteCString(deviceInfo.networkId)) {
        LOGE("Failed to send network id");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteUint16(deviceInfo.deviceTypeId)) {
        LOGE("Failed to send deviceTypeId");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteUint32(deviceInfo.range)) {
        LOGE("Failed to send range");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteInt32(deviceInfo.authForm)) {
        LOGE("Failed to send user id");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION),
        data, reply, option);
    if (ret != 0) {
        stringstream ss;
        ss << "Failed to send out the requeset, errno:" << ret;
        LOGE("%{public}s", ss.str().c_str());
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    LOGI("Close p2p connection Success");
    return reply.ReadInt32();
}

int32_t DistributedFileDaemonProxy::OpenP2PConnectionEx(const std::string &networkId,
                                                        sptr<IFileDfsListener> remoteReverseObj)
{
    LOGI("DistributedFileDaemonProxy::OpenP2PConnectionEx start.");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token.");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteString(networkId)) {
        LOGE("Failed to send network id.");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (remoteReverseObj == nullptr) {
        LOGE("remoteReverseObj is nullptr.");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteRemoteObject(remoteReverseObj->AsObject())) {
        LOGE("fail to WriteRemoteObject remoteReverseObj");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr.");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION_EX),
        data, reply, option);
    if (ret != 0) {
        LOGE("SendRequest failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    LOGI("DistributedFileDaemonProxy::OpenP2PConnectionEx success.");
    return reply.ReadInt32();
}

int32_t DistributedFileDaemonProxy::CloseP2PConnectionEx(const std::string &networkId)
{
    LOGI("Close p2p connection");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token.");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteString(networkId)) {
        LOGE("Failed to send device id.");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr.");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION_EX),
        data, reply, option);
    if (ret != 0) {
        LOGE("SendRequest failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    LOGI("DistributedFileDaemonProxy::Close p2p connection Success");
    return reply.ReadInt32();
}

int32_t DistributedFileDaemonProxy::PrepareSession(const std::string &srcUri,
                                                   const std::string &dstUri,
                                                   const std::string &srcDeviceId,
                                                   const sptr<IRemoteObject> &listener,
                                                   HmdfsInfo &info)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteString(srcUri)) {
        LOGE("Failed to send srcUri");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteString(dstUri)) {
        LOGE("Failed to send dstUri");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteString(srcDeviceId)) {
        LOGE("Failed to send remoteDeviceId");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteRemoteObject(listener)) {
        LOGE("Failed to send the listener callback stub");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteString(info.copyPath)) {
        LOGE("Failed to send info.copyPath");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteBool(info.dirExistFlag)) {
        LOGE("Failed to send info.dirExistFlag");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret =
        remote->SendRequest(static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_PREPARE_SESSION),
                            data, reply, option);
    if (!reply.ReadString(info.sessionName)) {
        LOGE("Failed to receive info.sessionName");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (ret != 0) {
        LOGE("SendRequest failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    return reply.ReadInt32();
}

int32_t DistributedFileDaemonProxy::RequestSendFile(const std::string &srcUri,
                                                    const std::string &dstPath,
                                                    const std::string &remoteDeviceId,
                                                    const std::string &sessionName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteString(srcUri)) {
        LOGE("Failed to send srcUri");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteString(dstPath)) {
        LOGE("Failed to send dstPath");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteString(remoteDeviceId)) {
        LOGE("Failed to send remoteDeviceId");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteString(sessionName)) {
        LOGE("Failed to send sessionName");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REQUEST_SEND_FILE), data, reply,
        option);
    if (ret != 0) {
        LOGE("SendRequest failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    return reply.ReadInt32();
}

int32_t DistributedFileDaemonProxy::GetRemoteCopyInfo(const std::string &srcUri, bool &isFile, bool &isDir)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteString(srcUri)) {
        LOGE("Failed to send srcUri");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    auto ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_GET_REMOTE_COPY_INFO), data, reply,
        option);
    if (ret != 0) {
        LOGE("SendRequest failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!reply.ReadBool(isFile)) {
        LOGE("read isFile failed");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!reply.ReadBool(isDir)) {
        LOGE("read isDir failed");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!reply.ReadInt32(ret)) {
        LOGE("read res failed");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    return ret;
}

int32_t DistributedFileDaemonProxy::CancelCopyTask(const std::string &sessionName)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteString(sessionName)) {
        LOGE("Failed to send sessionName");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CANCEL_COPY_TASK), data, reply,
        option);
    if (ret != 0) {
        LOGE("SendRequest failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    return reply.ReadInt32();
}

int32_t DistributedFileDaemonProxy::CancelCopyTask(const std::string &srcUri, const std::string &dstUri)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteString(srcUri)) {
        LOGE("Failed to send srcUri");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteString(dstUri)) {
        LOGE("Failed to send dstUri");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CANCEL_INNER_COPY_TASK), data, reply,
        option);
    if (ret != 0) {
        LOGE("SendRequest failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    return reply.ReadInt32();
}

int32_t DistributedFileDaemonProxy::PushAsset(int32_t userId,
                                              const sptr<AssetObj> &assetObj,
                                              const sptr<IAssetSendCallback> &sendCallback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }

    if (!data.WriteInt32(userId)) {
        LOGE("Failed to send the user id");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteParcelable(assetObj)) {
        LOGE("Failed to send the assetInfoObj");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (sendCallback == nullptr) {
        LOGE("sendCallback is nullptr.");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteRemoteObject(sendCallback->AsObject())) {
        LOGE("Failed to send the listener callback stub");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_PUSH_ASSET), data, reply,
        option);
    if (ret != 0) {
        LOGE("UnRegisterAssetCallback failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    return reply.ReadInt32();
}

int32_t DistributedFileDaemonProxy::RegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (recvCallback == nullptr) {
        LOGE("recvCallback is nullptr.");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteRemoteObject(recvCallback->AsObject())) {
        LOGE("Failed to send the listener callback stub");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REGISTER_ASSET_CALLBACK), data,
        reply, option);
    if (ret != 0) {
        LOGE("RegisterAssetCallback failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    return reply.ReadInt32();
}

int32_t DistributedFileDaemonProxy::UnRegisterAssetCallback(const sptr<IAssetRecvCallback> &recvCallback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (recvCallback == nullptr) {
        LOGE("recvCallback is nullptr.");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteRemoteObject(recvCallback->AsObject())) {
        LOGE("Failed to send the listener callback stub");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_UN_REGISTER_ASSET_CALLBACK), data,
        reply, option);
    if (ret != 0) {
        LOGE("UnRegisterAssetCallback failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    return reply.ReadInt32();
}

int32_t DistributedFileDaemonProxy::GetDfsUrisDirFromLocal(const std::vector<std::string> &uriList,
                                                           const int32_t userId,
                                                           std::unordered_map<std::string,
                                                           AppFileService::ModuleRemoteFileShare::HmdfsUriInfo>
                                                           &uriToDfsUriMaps)
{
    LOGI("GetDfsUrisDirFromLocal begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!IpcWrapper::WriteBatchUris(data, uriList)) {
        LOGE("Failed to send uriList");
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!data.WriteInt32(userId)) {
        LOGE("Failed to send userId");
        return OHOS::FileManagement::E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }

    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::GET_DFS_URI_IS_DIR_FROM_LOCAL),
        data, reply, option);
    if (ret != 0) {
        stringstream ss;
        ss << "Failed to send out the requeset, errno:" << ret;
        LOGE("%{public}s", ss.str().c_str());
        return OHOS::FileManagement::E_BROKEN_IPC;
    }

    if (!reply.ReadInt32(ret)) {
        LOGE("Failed to read res");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }

    if (ret != OHOS::FileManagement::E_OK) {
        LOGE("Failed to GetDfsUrisDirFromLocal, res: %{public}d", ret);
        return ret;
    }
    std::vector<std::string> total;
    if (IpcWrapper::ReadBatchUris(reply, total) != FileManagement::E_OK) {
        LOGE("read total failed");
        return OHOS::FileManagement::E_IPC_READ_FAILED;
    }

    LOGI("proxy total.size(): %{public}d", static_cast<int>(total.size()));
    return ReadUriInfoFromResult(total, uriToDfsUriMaps);
}

int32_t DistributedFileDaemonProxy::GetDfsSwitchStatus(const std::string &networkId, int32_t &switchStatus)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteString(networkId)) {
        LOGE("Failed to write network id.");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("Remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_GET_DFS_SWITCH_STATUS), data,
        reply, option);
    if (ret != FileManagement::E_OK) {
        LOGE("GetDfsSwitchStatus ipc failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!reply.ReadInt32(ret)) {
        LOGE("Read ret failed");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (ret != FileManagement::E_OK) {
        LOGE("GetDfsSwitchStatus failed, ret = %{public}d", ret);
        return ret;
    }
    if (!reply.ReadInt32(switchStatus)) {
        LOGE("Read switchStatus failed");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    return ret;
}

int32_t DistributedFileDaemonProxy::UpdateDfsSwitchStatus(int32_t switchStatus)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!data.WriteInt32(switchStatus)) {
        LOGE("Failed to write the switch status");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("Remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_UPDATE_DFS_SWITCH_STATUS), data,
        reply, option);
    if (ret != FileManagement::E_OK) {
        LOGE("UpdateDfsSwitchStatus failed, ret = %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!reply.ReadInt32(ret)) {
        LOGE("Read ret failed");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    return ret;
}

int32_t DistributedFileDaemonProxy::GetConnectedDeviceList(std::vector<DfsDeviceInfo> &deviceList)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    auto remote = Remote();
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_GET_CONNECTED_DEVICE_LIST), data,
        reply, option);
    if (ret != FileManagement::E_OK) {
        LOGE("GetConnectedDeviceList ipc failed, ret: %{public}d", ret);
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (!reply.ReadInt32(ret)) {
        LOGE("Read res failed");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    if (ret != FileManagement::E_OK) {
        LOGE("GetConnectedDeviceList failed, ret: %{public}d", ret);
        return ret;
    }
    int32_t len = 0;
    if (!reply.ReadInt32(len)) {
        LOGE("Read devicelist length failed");
        return OHOS::FileManagement::E_BROKEN_IPC;
    }
    deviceList.clear();
    for (int32_t i = 0; i < len; ++i) {
        DfsDeviceInfo deviceInfo;
        if (!reply.ReadString(deviceInfo.networkId_)) {
            LOGE("Failed to read networkId");
            return OHOS::FileManagement::E_BROKEN_IPC;
        }
        if (!reply.ReadString(deviceInfo.path_)) {
            LOGE("Failed to read path");
            return OHOS::FileManagement::E_BROKEN_IPC;
        }
        deviceList.emplace_back(deviceInfo);
    }
    return ret;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS