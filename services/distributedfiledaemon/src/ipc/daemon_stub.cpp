/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "ipc/daemon_stub.h"

#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "dm_device_info.h"
#include "ipc/distributed_file_daemon_ipc_interface_code.h"
#include "ipc_skeleton.h"
#include "utils_log.h"
#include "securec.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace OHOS::FileManagement;
const int32_t UID = 1009;
const int32_t DATA_UID = 3012;
constexpr size_t MAX_IPC_RAW_DATA_SIZE = 128 * 1024 * 1024;

DaemonStub::DaemonStub()
{
    opToInterfaceMap_[static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION)] =
        &DaemonStub::HandleOpenP2PConnection;
    opToInterfaceMap_[static_cast<uint32_t>(
        DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION)] =
        &DaemonStub::HandleCloseP2PConnection;
    opToInterfaceMap_[static_cast<uint32_t>(
        DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION_EX)] =
        &DaemonStub::HandleOpenP2PConnectionEx;
    opToInterfaceMap_[static_cast<uint32_t>(
        DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION_EX)] =
        &DaemonStub::HandleCloseP2PConnectionEx;
    opToInterfaceMap_[static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_PREPARE_SESSION)] =
        &DaemonStub::HandlePrepareSession;
    opToInterfaceMap_[static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CANCEL_COPY_TASK)] =
        &DaemonStub::HandleCancelCopyTask;
    opToInterfaceMap_[static_cast<uint32_t>(
        DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CANCEL_INNER_COPY_TASK)] =
        &DaemonStub::HandleInnerCancelCopyTask;
    opToInterfaceMap_[static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REQUEST_SEND_FILE)] =
        &DaemonStub::HandleRequestSendFile;
    opToInterfaceMap_[static_cast<uint32_t>(
        DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_GET_REMOTE_COPY_INFO)] =
        &DaemonStub::HandleGetRemoteCopyInfo;
    opToInterfaceMap_[static_cast<uint32_t>(
        DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REGISTER_ASSET_CALLBACK)] =
        &DaemonStub::HandleRegisterRecvCallback;
    opToInterfaceMap_[static_cast<uint32_t>(
        DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_UN_REGISTER_ASSET_CALLBACK)] =
        &DaemonStub::HandleUnRegisterRecvCallback;
    opToInterfaceMap_[static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_PUSH_ASSET)] =
        &DaemonStub::HandlePushAsset;
    opToInterfaceMap_[static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::GET_DFS_URI_IS_DIR_FROM_LOCAL)] =
        &DaemonStub::HandleGetDfsUrisDirFromLocal;
}

int32_t DaemonStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return DFS_DAEMON_DESCRIPTOR_IS_EMPTY;
    }
    if (code != static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REQUEST_SEND_FILE) &&
        code != static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_GET_REMOTE_COPY_INFO) &&
        !IPCSkeleton::IsLocalCalling()) {
        LOGE("function is only allowed to be called locally.");
        return E_ALLOW_LOCAL_CALL_ONLY;
    }
    switch (code) {
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION):
            return HandleOpenP2PConnection(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION):
            return HandleCloseP2PConnection(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_OPEN_P2P_CONNECTION_EX):
            return HandleOpenP2PConnectionEx(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CLOSE_P2P_CONNECTION_EX):
            return HandleCloseP2PConnectionEx(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_PREPARE_SESSION):
            return HandlePrepareSession(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CANCEL_COPY_TASK):
            return HandleCancelCopyTask(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_CANCEL_INNER_COPY_TASK):
            return HandleInnerCancelCopyTask(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REQUEST_SEND_FILE):
            return HandleRequestSendFile(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_GET_REMOTE_COPY_INFO):
            return HandleGetRemoteCopyInfo(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_REGISTER_ASSET_CALLBACK):
            return HandleRegisterRecvCallback(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_UN_REGISTER_ASSET_CALLBACK):
            return HandleUnRegisterRecvCallback(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::DISTRIBUTED_FILE_PUSH_ASSET):
            return HandlePushAsset(data, reply);
        case static_cast<uint32_t>(DistributedFileDaemonInterfaceCode::GET_DFS_URI_IS_DIR_FROM_LOCAL):
            return HandleGetDfsUrisDirFromLocal(data, reply);
        default:
            LOGE("Cannot response request %d: unknown tranction", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

static bool GetData(void *&buffer, size_t size, const void *data)
{
    if (data == nullptr) {
        LOGE("null data");
        return false;
    }
    if (size == 0 || size > MAX_IPC_RAW_DATA_SIZE) {
        LOGE("size invalid: %{public}zu", size);
        return false;
    }
    buffer = malloc(size);
    if (buffer == nullptr) {
        LOGE("malloc buffer failed");
        return false;
    }
    if (memcpy_s(buffer, size, data, size) != E_OK) {
        free(buffer);
        LOGE("memcpy failed");
        return false;
    }
    return true;
}

static bool ReadBatchUriByRawData(MessageParcel &data, std::vector<std::string> &uriVec)
{
    size_t dataSize = static_cast<size_t>(data.ReadInt32());
    if (dataSize == 0) {
        LOGE("parcel no data");
        return false;
    }

    void *buffer = nullptr;
    if (!GetData(buffer, dataSize, data.ReadRawData(dataSize))) {
        LOGE("read raw data failed: %{public}zu", dataSize);
        return false;
    }

    MessageParcel tempParcel;
    if (!tempParcel.ParseFrom(reinterpret_cast<uintptr_t>(buffer), dataSize)) {
        LOGE("failed to parseFrom");
        return false;
    }
    tempParcel.ReadStringVector(&uriVec);
    return true;
}

int32_t ReadBatchUris(MessageParcel &data, std::vector<std::string> &uriVec)
{
    uint32_t size = data.ReadUint32();
    if (size == 0) {
        LOGE("out of range: %{public}u", size);
        return OHOS::FileManagement::E_INVAL_ARG;
    }
    if (!ReadBatchUriByRawData(data, uriVec)) {
        LOGE("read uris failed");
        return OHOS::FileManagement::E_SA_LOAD_FAILED;
    }
    return E_OK;
}

static bool WriteUriByRawData(MessageParcel &data, const std::vector<std::string> &uriVec)
{
    MessageParcel tempParcel;
    tempParcel.SetMaxCapacity(MAX_IPC_RAW_DATA_SIZE);
    if (!tempParcel.WriteStringVector(uriVec)) {
        LOGE("Write uris failed");
        return false;
    }
    size_t dataSize = tempParcel.GetDataSize();
    if (!data.WriteInt32(static_cast<int32_t>(dataSize))) {
        LOGE("Write data size failed");
        return false;
    }
    if (!data.WriteRawData(reinterpret_cast<uint8_t *>(tempParcel.GetData()), dataSize)) {
        LOGE("Write raw data failed");
        return false;
    }
    return true;
}

bool WriteBatchUris(MessageParcel &data, const std::vector<std::string> &uriVec)
{
    if (!data.WriteUint32(uriVec.size())) {
        LOGE("Write uri size failed");
        return false;
    }
    if (!WriteUriByRawData(data, uriVec)) {
        LOGE("Write uri by raw data failed");
        return false;
    }
    return true;
}

int32_t DaemonStub::HandleOpenP2PConnection(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin OpenP2PConnection");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_DISTRIBUTED_DATASYNC)) {
        LOGE("[HandleOpenP2PConnection] DATASYNC permission denied");
        return E_PERMISSION_DENIED;
    }
    DistributedHardware::DmDeviceInfo deviceInfo;
    auto ret = strcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source device id failed, ret is %{public}d", ret);
        return -1;
    }
    ret = strcpy_s(deviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source device name failed, ret is %{public}d", ret);
        return -1;
    }
    ret = strcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source network id failed, ret is %{public}d", ret);
        return -1;
    }
    deviceInfo.deviceTypeId = data.ReadUint16();
    deviceInfo.range = static_cast<int32_t>(data.ReadUint32());
    deviceInfo.authForm = static_cast<DistributedHardware::DmAuthForm>(data.ReadInt32());

    int32_t res = OpenP2PConnection(deviceInfo);
    reply.WriteInt32(res);
    LOGI("End OpenP2PConnection, res = %{public}d", res);
    return res;
}

int32_t DaemonStub::HandleCloseP2PConnection(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin CloseP2PConnection");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_DISTRIBUTED_DATASYNC)) {
        LOGE("[HandleCloseP2PConnection] DATASYNC permission denied");
        return E_PERMISSION_DENIED;
    }
    DistributedHardware::DmDeviceInfo deviceInfo;
    auto ret = strcpy_s(deviceInfo.deviceId, DM_MAX_DEVICE_ID_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source device id failed, ret is %{public}d", ret);
        return -1;
    }
    ret = strcpy_s(deviceInfo.deviceName, DM_MAX_DEVICE_NAME_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source device name failed, ret is %{public}d", ret);
        return -1;
    }
    ret = strcpy_s(deviceInfo.networkId, DM_MAX_DEVICE_ID_LEN, data.ReadCString());
    if (ret != 0) {
        LOGE("strcpy for source network id failed, ret is %{public}d", ret);
        return -1;
    }
    deviceInfo.deviceTypeId = data.ReadUint16();
    deviceInfo.range = static_cast<int32_t>(data.ReadUint32());
    deviceInfo.authForm = static_cast<DistributedHardware::DmAuthForm>(data.ReadInt32());

    int32_t res = CloseP2PConnection(deviceInfo);
    reply.WriteInt32(res);
    LOGI("End CloseP2PConnection");
    return res;
}

int32_t DaemonStub::HandleOpenP2PConnectionEx(MessageParcel &data, MessageParcel &reply)
{
    LOGI("DaemonStub::Begin OpenP2PConnectionEx");
    std::string networkId;
    if (!data.ReadString(networkId)) {
        LOGE("read networkId failed");
        return E_IPC_READ_FAILED;
    }
    auto remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        LOGE("read remoteObject failed");
        return E_IPC_READ_FAILED;
    }
    auto remoteReverseObj = iface_cast<IFileDfsListener>(remote);
    if (remoteReverseObj == nullptr) {
        LOGE("remoteReverseObj is null");
        return E_INVAL_ARG;
    }
    int32_t res = OpenP2PConnectionEx(networkId, remoteReverseObj);
    reply.WriteInt32(res);
    LOGI("DaemonStub::End OpenP2PConnection, res = %{public}d.", res);
    return res;
}

int32_t DaemonStub::HandleCloseP2PConnectionEx(MessageParcel &data, MessageParcel &reply)
{
    LOGI("DaemonStub::Begin CloseP2PConnection.");
    std::string networkId;
    if (!data.ReadString(networkId)) {
        LOGE("read networkId failed");
        return E_IPC_READ_FAILED;
    }

    int32_t res = CloseP2PConnectionEx(networkId);
    reply.WriteInt32(res);
    LOGI("DaemonStub::End CloseP2PConnection");
    return res;
}

int32_t DaemonStub::HandlePrepareSession(MessageParcel &data, MessageParcel &reply)
{
    LOGI("DaemonStub::Begin HandlePrepareSession.");
    std::string srcUri;
    if (!data.ReadString(srcUri)) {
        LOGE("read srcUri failed");
        return E_IPC_READ_FAILED;
    }
    if (!DfsuAccessTokenHelper::CheckUriPermission(srcUri)) {
        LOGE("permission verify failed");
        return E_PERMISSION_DENIED;
    }
    std::string dstUri;
    if (!data.ReadString(dstUri)) {
        LOGE("read dstUri failed");
        return E_IPC_READ_FAILED;
    }
    std::string srcDeviceId;
    if (!data.ReadString(srcDeviceId)) {
        LOGE("read srcDeviceId failed");
        return E_IPC_READ_FAILED;
    }
    auto listener = data.ReadRemoteObject();
    if (listener == nullptr) {
        LOGE("read listener failed");
        return E_IPC_READ_FAILED;
    }
    HmdfsInfo info{};
    if (!data.ReadString(info.copyPath)) {
        LOGE("read info.copyPath failed");
        return E_IPC_READ_FAILED;
    }
    if (!data.ReadBool(info.dirExistFlag)) {
        LOGE("read info.dirExistFlag failed");
        return E_IPC_READ_FAILED;
    }
    int32_t res = PrepareSession(srcUri, dstUri, srcDeviceId, listener, info);
    if (!reply.WriteString(info.sessionName)) {
        LOGE("Write sessionName failed");
        return E_IPC_WRITE_FAILED;
    }
    reply.WriteInt32(res);
    LOGD("End PrepareSession, ret = %{public}d.", res);
    return res;
}

int32_t DaemonStub::HandleRequestSendFile(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleRequestSendFile");
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid != UID) {
        LOGE("Permission denied, caller is not dfs!");
        return E_PERMISSION_DENIED;
    }
    std::string srcUri;
    if (!data.ReadString(srcUri)) {
        LOGE("read srcUri failed");
        return E_IPC_READ_FAILED;
    }
    std::string dstPath;
    if (!data.ReadString(dstPath)) {
        LOGE("read dstPath failed");
        return E_IPC_READ_FAILED;
    }
    std::string dstDeviceId;
    if (!data.ReadString(dstDeviceId)) {
        LOGE("read remoteDeviceId failed");
        return E_IPC_READ_FAILED;
    }
    std::string sessionName;
    if (!data.ReadString(sessionName)) {
        LOGE("read sessionName failed");
        return E_IPC_READ_FAILED;
    }
    auto res = RequestSendFile(srcUri, dstPath, dstDeviceId, sessionName);
    reply.WriteInt32(res);
    LOGD("End RequestSendFile, ret = %{public}d.", res);
    return res;
}

int32_t DaemonStub::HandleGetRemoteCopyInfo(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleGetRemoteCopyInfo");
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid != UID) {
        LOGE("Permission denied, caller is not dfs!");
        return E_PERMISSION_DENIED;
    }
    std::string srcUri;
    if (!data.ReadString(srcUri)) {
        LOGE("read srcUri failed");
        return E_IPC_READ_FAILED;
    }
    bool isFile = false;
    bool isDir = false;
    auto res = GetRemoteCopyInfo(srcUri, isFile, isDir);
    if (res != E_OK) {
        LOGE("GetRemoteCopyInfo failed");
        return E_IPC_READ_FAILED;
    }
    if (!reply.WriteBool(isFile)) {
        LOGE("Write isFile failed");
        return E_IPC_READ_FAILED;
    }
    if (!reply.WriteBool(isDir)) {
        LOGE("Write isDir failed");
        return E_IPC_READ_FAILED;
    }
    if (!reply.WriteInt32(res)) {
        LOGE("Write res failed");
        return E_IPC_READ_FAILED;
    }
    LOGD("End GetRemoteCopyInfo, ret = %{public}d.", res);
    return res;
}

int32_t DaemonStub::HandleCancelCopyTask(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleCancelCopyTask");
    std::string sessionName;
    if (!data.ReadString(sessionName)) {
        LOGE("read sessionName failed");
        return E_IPC_READ_FAILED;
    }
    return CancelCopyTask(sessionName);
}

int32_t DaemonStub::HandleInnerCancelCopyTask(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleCancelCopyTask");
    std::string srcUri;
    if (!data.ReadString(srcUri)) {
        LOGE("read srcUri failed");
        return E_IPC_READ_FAILED;
    }
    std::string dstUri;
    if (!data.ReadString(dstUri)) {
        LOGE("read dstUri failed");
        return E_IPC_READ_FAILED;
    }
    return CancelCopyTask(srcUri, dstUri);
}

int32_t DaemonStub::HandleRegisterRecvCallback(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin RegisterRecvCallback");
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid != DATA_UID) {
        LOGE("Permission denied, caller is not data!");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_DISTRIBUTED_DATASYNC)) {
        LOGE("[RegisterRecvCallback] DATASYNC permission denied");
        return E_PERMISSION_DENIED;
    }
    auto object = data.ReadRemoteObject();
    if (object == nullptr) {
        LOGE("RegisterRecvCallback failed, object is nullptr.");
        return E_IPC_READ_FAILED;
    }
    auto recvCallback = iface_cast<IAssetRecvCallback>(object);
    if (recvCallback == nullptr) {
        LOGE("RegisterRecvCallback failed, Callback is nullptr");
        return E_INVAL_ARG;
    }
    int32_t res = RegisterAssetCallback(recvCallback);
    if (!reply.WriteInt32(res)) {
        LOGE("RegisterRecvCallback write res failed, res is %{public}d", res);
        return E_IPC_READ_FAILED;
    }
    return res;
}

int32_t DaemonStub::HandleUnRegisterRecvCallback(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin UnRegisterRecvCallback");
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid != DATA_UID) {
        LOGE("Permission denied, caller is not data!");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_DISTRIBUTED_DATASYNC)) {
        LOGE("[UnRegisterRecvCallback] DATASYNC permission denied");
        return E_PERMISSION_DENIED;
    }
    auto object = data.ReadRemoteObject();
    if (object == nullptr) {
        LOGE("UnRegisterRecvCallback failed, object is nullptr.");
        return E_IPC_READ_FAILED;
    }
    auto recvCallback = iface_cast<IAssetRecvCallback>(object);
    if (recvCallback == nullptr) {
        LOGE("UnRegisterRecvCallback failed, Callback is nullptr");
        return E_INVAL_ARG;
    }
    int32_t res = UnRegisterAssetCallback(recvCallback);
    if (!reply.WriteInt32(res)) {
        LOGE("UnRegisterRecvCallback write res failed, res is %{public}d", res);
        return E_IPC_READ_FAILED;
    }
    return res;
}

int32_t DaemonStub::HandlePushAsset(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin PushAsset");
    auto uid = IPCSkeleton::GetCallingUid();
    if (uid != DATA_UID) {
        LOGE("Permission denied, caller is not data!");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_DISTRIBUTED_DATASYNC)) {
        LOGE("[PushAsset] DATASYNC permission denied");
        return E_PERMISSION_DENIED;
    }
    int32_t userId;
    if (!data.ReadInt32(userId)) {
        LOGE("read userId failed");
        return E_INVAL_ARG;
    }

    sptr<AssetObj> assetObj = data.ReadParcelable<AssetObj>();
    if (!assetObj) {
        LOGE("object of AssetObj is nullptr");
        return E_INVAL_ARG;
    }

    auto object = data.ReadRemoteObject();
    if (object == nullptr) {
        LOGE("PushAsset failed, object is nullptr.");
        return E_IPC_READ_FAILED;
    }
    auto sendCallback = iface_cast<IAssetSendCallback>(object);
    if (sendCallback == nullptr) {
        LOGE("PushAsset failed, Callback is nullptr");
        return E_INVAL_ARG;
    }

    int32_t res = PushAsset(userId, assetObj, sendCallback);
    if (!reply.WriteInt32(res)) {
        LOGE("PushAsset write res failed, res is %{public}d", res);
        return E_IPC_READ_FAILED;
    }
    return res;
}

int32_t DaemonStub::HandleGetDfsUrisDirFromLocal(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::string> uriList;
    if (ReadBatchUris(data, uriList) != E_OK) {
        LOGE("read uriList failed");
        return E_IPC_READ_FAILED;
    }

    LOGI("stub uriList.size(): %{public}d", static_cast<int>(uriList.size()));
    int32_t userId;
    if (!data.ReadInt32(userId)) {
        LOGE("read userId failed");
        return E_IPC_READ_FAILED;
    }

    std::unordered_map<std::string, AppFileService::ModuleRemoteFileShare::HmdfsUriInfo> uriToDfsUriMaps;
    int32_t res = GetDfsUrisDirFromLocal(uriList, userId, uriToDfsUriMaps);

    std::vector<std::string> total;
    for (auto it = uriToDfsUriMaps.begin(); it != uriToDfsUriMaps.end(); ++it) {
        total.push_back(it->first);
        total.push_back((it->second).uriStr);
        total.push_back(std::to_string((it->second).fileSize));
    }
    LOGI("stub total.size(): %{public}d", static_cast<int>(total.size()));

    if (!WriteBatchUris(reply, total)) {
        LOGE("Failed to send user uriStr");
        return E_IPC_WRITE_FAILED;
    }

    if (!reply.WriteInt32(res)) {
        LOGE("Failed to send result");
        return E_IPC_WRITE_FAILED;
    }
    LOGI("HandleGetDfsUrisDirFromLocal end");
    return res;
}
}  // namespace DistributedFile
}  // namespace Storage
}  // namespace OHOS