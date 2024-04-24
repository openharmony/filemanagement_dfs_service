/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "ipc/cloud_sync_service_stub.h"
#include "cloud_file_sync_service_interface_code.h"
#include "data_sync/task_state_manager.h"
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "dfsu_memory_guard.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

CloudSyncServiceStub::CloudSyncServiceStub()
{
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_CALLBACK)] =
        &CloudSyncServiceStub::HandleUnRegisterCallbackInner;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_CALLBACK)] =
        &CloudSyncServiceStub::HandleRegisterCallbackInner;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_SYNC)] =
        &CloudSyncServiceStub::HandleStartSyncInner;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_TRIGGER_SYNC)] =
        &CloudSyncServiceStub::HandleTriggerSyncInner;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_SYNC)] =
        &CloudSyncServiceStub::HandleStopSyncInner;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CHANGE_APP_SWITCH)] =
        &CloudSyncServiceStub::HandleChangeAppSwitch;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CLEAN)] =
        &CloudSyncServiceStub::HandleClean;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_NOTIFY_DATA_CHANGE)] =
        &CloudSyncServiceStub::HandleNotifyDataChange;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_NOTIFY_EVENT_CHANGE)] =
        &CloudSyncServiceStub::HandleNotifyEventChange;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_ENABLE_CLOUD)] =
        &CloudSyncServiceStub::HandleEnableCloud;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DISABLE_CLOUD)] =
        &CloudSyncServiceStub::HandleDisableCloud;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_DOWNLOAD_FILE)] =
        &CloudSyncServiceStub::HandleStartDownloadFile;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_DOWNLOAD_FILE)] =
        &CloudSyncServiceStub::HandleStopDownloadFile;
    opToInterfaceMap_[static_cast<uint32_t>(
        CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_DOWNLOAD_FILE_CALLBACK)] =
        &CloudSyncServiceStub::HandleRegisterDownloadFileCallback;
    opToInterfaceMap_[static_cast<uint32_t>(
        CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_DOWNLOAD_FILE_CALLBACK)] =
        &CloudSyncServiceStub::HandleUnregisterDownloadFileCallback;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UPLOAD_ASSET)] =
        &CloudSyncServiceStub::HandleUploadAsset;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_FILE)] =
        &CloudSyncServiceStub::HandleDownloadFile;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_FILES)] =
        &CloudSyncServiceStub::HandleDownloadFiles;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_ASSET)] =
        &CloudSyncServiceStub::HandleDownloadAsset;
    opToInterfaceMap_[static_cast<uint32_t>(
        CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_DOWNLOAD_ASSET_CALLBACK)] =
        &CloudSyncServiceStub::HandleRegisterDownloadAssetCallback;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DELETE_ASSET)] =
        &CloudSyncServiceStub::HandleDeleteAsset;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_GET_SYNC_TIME)] =
        &CloudSyncServiceStub::HandleGetSyncTime;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CLEAN_CACHE)] =
        &CloudSyncServiceStub::HandleCleanCache;
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_FILE_CACHE)] =
        &CloudSyncServiceStub::HandleStartFileCache;
}

int32_t CloudSyncServiceStub::OnRemoteRequest(uint32_t code,
                                              MessageParcel &data,
                                              MessageParcel &reply,
                                              MessageOption &option)
{
    DfsuMemoryGuard cacheGuard;
    TaskStateManager::GetInstance().StartTask();
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return E_SERVICE_DESCRIPTOR_IS_EMPTY;
    }
    auto interfaceIndex = opToInterfaceMap_.find(code);
    if (interfaceIndex == opToInterfaceMap_.end() || !interfaceIndex->second) {
        LOGE("Cannot response request %d: unknown tranction", code);
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return (this->*(interfaceIndex->second))(data, reply);
}

int32_t CloudSyncServiceStub::HandleUnRegisterCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin UnRegisterCallbackInner");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }

    string bundleName = data.ReadString();
    int32_t res = UnRegisterCallbackInner(bundleName);
    reply.WriteInt32(res);
    LOGI("End UnRegisterCallbackInner");
    return res;
}

int32_t CloudSyncServiceStub::HandleRegisterCallbackInner(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin RegisterCallbackInner");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    auto remoteObj = data.ReadRemoteObject();
    string bundleName = data.ReadString();
    int32_t res = RegisterCallbackInner(remoteObj, bundleName);
    reply.WriteInt32(res);
    LOGI("End RegisterCallbackInner");
    return res;
}

int32_t CloudSyncServiceStub::HandleStartSyncInner(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin StartSyncInner");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    auto forceFlag = data.ReadBool();
    string bundleName = data.ReadString();
    int32_t res = StartSyncInner(forceFlag, bundleName);
    reply.WriteInt32(res);
    LOGI("End StartSyncInner");
    return res;
}

int32_t CloudSyncServiceStub::HandleTriggerSyncInner(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin TriggerSyncInner");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    string bundleName = data.ReadString();
    int32_t userId = data.ReadInt32();
    int32_t res = TriggerSyncInner(bundleName, userId);
    reply.WriteInt32(res);
    LOGI("End TriggerSyncInner");
    return res;
}

int32_t CloudSyncServiceStub::HandleStopSyncInner(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin StopSyncInner");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }

    string bundleName = data.ReadString();
    int32_t res = StopSyncInner(bundleName);
    reply.WriteInt32(res);
    LOGI("End StopSyncInner");
    return res;
}

int32_t CloudSyncServiceStub::HandleChangeAppSwitch(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin ChangeAppSwitch");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC_MANAGER)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    string accountId = data.ReadString();
    string bundleName = data.ReadString();
    bool status = data.ReadBool();
    int32_t res = ChangeAppSwitch(accountId, bundleName, status);
    reply.WriteInt32(res);
    LOGI("End ChangeAppSwitch");
    return res;
}

int32_t CloudSyncServiceStub::HandleClean(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin Clean");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC_MANAGER)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    string accountId = data.ReadString();
    sptr<CleanOptions> options = data.ReadParcelable<CleanOptions>();
    if (!options) {
        LOGE("object of CleanOptions is nullptr");
        return E_INVAL_ARG;
    }
    int32_t res = Clean(accountId, *options);
    reply.WriteInt32(res);
    LOGI("End Clean");
    return res;
}

int32_t CloudSyncServiceStub::HandleNotifyDataChange(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin NotifyDataChange");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC_MANAGER)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    string accountId = data.ReadString();
    string bundleName = data.ReadString();
    int32_t res = NotifyDataChange(accountId, bundleName);
    reply.WriteInt32(res);
    LOGI("End NotifyDataChange");
    return res;
}

int32_t CloudSyncServiceStub::HandleNotifyEventChange(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin NotifyEventChange");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC_MANAGER)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    int32_t userId = data.ReadInt32();
    string eventIdStr = data.ReadString();
    string extraDataStr = data.ReadString();
    
    int32_t res = NotifyEventChange(userId, eventIdStr, extraDataStr);
    reply.WriteInt32(res);
    LOGI("End NotifyEventChange");
    return res;
}

int32_t CloudSyncServiceStub::HandleDisableCloud(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin DisableCloud");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC_MANAGER)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    string accountId = data.ReadString();
    int32_t res = DisableCloud(accountId);
    reply.WriteInt32(res);
    LOGI("End DisableCloud");
    return res;
}
int32_t CloudSyncServiceStub::HandleEnableCloud(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin EnableCloud");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC_MANAGER)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    string accountId = data.ReadString();
    sptr<SwitchDataObj> switchObj = data.ReadParcelable<SwitchDataObj>();
    if (!switchObj) {
        LOGE("object of SwitchDataObj is nullptr");
        return E_INVAL_ARG;
    }
    int32_t res = EnableCloud(accountId, *switchObj);
    reply.WriteInt32(res);
    LOGI("End EnableCloud");
    return res;
}

int32_t CloudSyncServiceStub::HandleStartDownloadFile(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleStartDownloadFile");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    string path = data.ReadString();

    int32_t res = StartDownloadFile(path);
    reply.WriteInt32(res);
    LOGI("End HandleStartDownloadFile");
    return res;
}

int32_t CloudSyncServiceStub::HandleStartFileCache(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleStartFileCache");
    string path = data.ReadString();
    if (!DfsuAccessTokenHelper::CheckUriPermission(path) &&
        !DfsuAccessTokenHelper::CheckCallerPermission(PERM_AUTH_URI)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    int32_t res = StartDownloadFile(path);
    reply.WriteInt32(res);
    LOGI("End HandleStartFileCache");
    return res;
}

int32_t CloudSyncServiceStub::HandleStopDownloadFile(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleStopDownloadFile");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    string path = data.ReadString();

    int32_t res = StopDownloadFile(path);
    reply.WriteInt32(res);
    LOGI("End HandleStopDownloadFile");
    return res;
}

int32_t CloudSyncServiceStub::HandleRegisterDownloadFileCallback(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleRegisterDownloadFileCallback");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }

    auto downloadCallback = data.ReadRemoteObject();

    int32_t res = RegisterDownloadFileCallback(downloadCallback);
    reply.WriteInt32(res);
    LOGI("End HandleRegisterDownloadFileCallback");
    return res;
}

int32_t CloudSyncServiceStub::HandleUnregisterDownloadFileCallback(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleUnregisterDownloadFileCallback");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }

    int32_t res = UnregisterDownloadFileCallback();
    reply.WriteInt32(res);
    LOGI("End HandleUnregisterDownloadFileCallback");
    return res;
}

int32_t CloudSyncServiceStub::HandleUploadAsset(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin UploadAsset");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    int32_t userId = data.ReadInt32();
    string request = data.ReadString();
    string result;
    int32_t res = UploadAsset(userId, request, result);
    reply.WriteInt32(res);
    reply.WriteString(result);
    LOGI("End UploadAsset");
    return res;
}

int32_t CloudSyncServiceStub::HandleDownloadFile(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin DownloadFile");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    int32_t userId = data.ReadInt32();
    string bundleName = data.ReadString();
    sptr<AssetInfoObj> assetInfoObj = data.ReadParcelable<AssetInfoObj>();
    if (!assetInfoObj) {
        LOGE("object of AssetInfoObj is nullptr");
        return E_INVAL_ARG;
    }
    int32_t res = DownloadFile(userId, bundleName, *assetInfoObj);
    reply.WriteInt32(res);
    LOGI("End DownloadFile");
    return res;
}

int32_t CloudSyncServiceStub::HandleDownloadFiles(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin DownloadFile");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    int32_t userId = data.ReadInt32();
    string bundleName = data.ReadString();
    int32_t size = data.ReadInt32();
    std::vector<AssetInfoObj> assetInfoObj;
    for (int i = 0; i < size; i++) {
        sptr<AssetInfoObj> obj = data.ReadParcelable<AssetInfoObj>();
        if (!obj) {
            LOGE("object of obj is nullptr");
            return E_INVAL_ARG;
        }
        assetInfoObj.emplace_back(*obj);
    }

    std::vector<bool> assetResultMap;
    int32_t res = DownloadFiles(userId, bundleName, assetInfoObj, assetResultMap);
    reply.WriteBoolVector(assetResultMap);
    reply.WriteInt32(res);
    LOGI("End DownloadFiles");
    return res;
}

int32_t CloudSyncServiceStub::HandleDownloadAsset(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin DownloadAsset");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    uint64_t taskId = data.ReadUint64();
    int32_t userId = data.ReadInt32();
    string bundleName = data.ReadString();
    string networkId = data.ReadString();
    sptr<AssetInfoObj> assetInfoObj = data.ReadParcelable<AssetInfoObj>();
    if (!assetInfoObj) {
        LOGE("object of AssetInfoObj is nullptr");
        return E_INVAL_ARG;
    }
    int32_t res = DownloadAsset(taskId, userId, bundleName, networkId, * assetInfoObj);
    reply.WriteInt32(res);
    LOGI("End DownloadAsset");
    return res;
}

int32_t CloudSyncServiceStub::HandleRegisterDownloadAssetCallback(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin RegisterDownloadAssetCallback");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    auto remoteObj = data.ReadRemoteObject();
    int32_t res = RegisterDownloadAssetCallback(remoteObj);
    reply.WriteInt32(res);
    LOGI("End RegisterDownloadAssetCallback");
    return res;
}

int32_t CloudSyncServiceStub::HandleDeleteAsset(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin DeleteAsset");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    int32_t userId = data.ReadInt32();
    string uri = data.ReadString();
    int32_t res = DeleteAsset(userId, uri);
    reply.WriteInt32(res);
    reply.WriteString(uri);
    LOGI("End DeleteAsset");
    return res;
}

int32_t CloudSyncServiceStub::HandleGetSyncTime(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin GetSyncTime");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
	
    int64_t syncTime = 0;
    string bundleName = data.ReadString();
    int32_t res = GetSyncTimeInner(syncTime, bundleName);
    reply.WriteInt64(syncTime);
    reply.WriteInt32(res);
    return res;
}

int32_t CloudSyncServiceStub::HandleCleanCache(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleCleanCache");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }

    string uri = data.ReadString();
    int32_t res = CleanCacheInner(uri);

    reply.WriteInt32(res);
    return res;
}
} // namespace OHOS::FileManagement::CloudSync
