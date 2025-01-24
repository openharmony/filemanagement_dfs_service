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
#include "dfs_error.h"
#include "dfsu_access_token_helper.h"
#include "dfsu_memory_guard.h"
#include "task_state_manager.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

static const int READ_SIZE = 100;
static const int MAX_READ_DENTRY_FILE_SIZE = 500;

CloudSyncServiceStub::CloudSyncServiceStub()
{
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_CALLBACK)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleUnRegisterCallbackInner(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_CALLBACK)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleRegisterCallbackInner(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_SYNC)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleStartSyncInner(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_TRIGGER_SYNC)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleTriggerSyncInner(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_SYNC)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleStopSyncInner(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CHANGE_APP_SWITCH)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleChangeAppSwitch(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CLEAN)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleClean(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_NOTIFY_DATA_CHANGE)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleNotifyDataChange(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_NOTIFY_EVENT_CHANGE)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleNotifyEventChange(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_ENABLE_CLOUD)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleEnableCloud(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DISABLE_CLOUD)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleDisableCloud(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_DOWNLOAD_FILE)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleStartDownloadFile(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_DOWNLOAD_FILE)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleStopDownloadFile(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(
        CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_DOWNLOAD_FILE_CALLBACK)] =
        [this](MessageParcel &data, MessageParcel &reply) {
            return this->HandleRegisterDownloadFileCallback(data, reply);
        };
    opToInterfaceMap_[static_cast<uint32_t>(
        CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_DOWNLOAD_FILE_CALLBACK)] =
        [this](MessageParcel &data, MessageParcel &reply) {
            return this->HandleUnregisterDownloadFileCallback(data, reply);
        };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UPLOAD_ASSET)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleUploadAsset(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_FILE)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleDownloadFile(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_FILES)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleDownloadFiles(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_ASSET)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleDownloadAsset(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(
        CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_DOWNLOAD_ASSET_CALLBACK)] =
        [this](MessageParcel &data, MessageParcel &reply) {
            return this->HandleRegisterDownloadAssetCallback(data, reply);
        };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DELETE_ASSET)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleDeleteAsset(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_GET_SYNC_TIME)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleGetSyncTime(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CLEAN_CACHE)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleCleanCache(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_FILE_CACHE)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleStartFileCache(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_RESET_CURSOR)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleResetCursor(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_FILE_CACHE)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleStopFileCache(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_OPTIMIZE_STORAGE)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleOptimizeStorage(data, reply); };
    opToInterfaceMap_[static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DENTRY_FILE_INSERT)] =
        [this](MessageParcel &data, MessageParcel &reply) { return this->HandleBatchDentryFileInsert(data, reply); };
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
    auto memberFunc = interfaceIndex->second;
    return memberFunc(data, reply);
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
    return E_OK;
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
    return E_OK;
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
    return E_OK;
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
    return E_OK;
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
    bool forceFlag = data.ReadBool();
    int32_t res = StopSyncInner(bundleName, forceFlag);
    reply.WriteInt32(res);
    LOGI("End StopSyncInner");
    return E_OK;
}

int32_t CloudSyncServiceStub::HandleResetCursor(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin ResetCursor");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
 
    string bundleName = data.ReadString();
    int32_t res = ResetCursor(bundleName);
    reply.WriteInt32(res);
    LOGI("End ResetCursor");
    return E_OK;
}

int32_t CloudSyncServiceStub::HandleOptimizeStorage(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleOptimizeStorage");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    int32_t agingDays = data.ReadInt32();

    int32_t res = OptimizeStorage(agingDays);
    reply.WriteInt32(res);
    LOGI("End HandleOptimizeStorage");
    return E_OK;
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
    return E_OK;
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
    return E_OK;
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
    return E_OK;
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
    return E_OK;
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
    return E_OK;
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
    return E_OK;
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
    return E_OK;
}

int32_t CloudSyncServiceStub::HandleStartFileCache(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleStartFileCache");
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    std::vector<std::string> pathVec;
    if (!data.ReadStringVector(&pathVec)) {
        LOGE("Failed to get the cloud id.");
        return E_INVAL_ARG;
    }
    int32_t fieldkey = data.ReadInt32();

    bool isCallbackValid = data.ReadBool();

    sptr<IRemoteObject> downloadCallback = nullptr;
    if (isCallbackValid) {
        downloadCallback = data.ReadRemoteObject();
    }

    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_AUTH_URI)) {
        for (auto &uri : pathVec) {
            if (!DfsuAccessTokenHelper::CheckUriPermission(uri)) {
                LOGE("permission denied");
                return E_PERMISSION_DENIED;
            }
        }
    }
    int64_t downloadId = 0;
    int32_t res = StartFileCache(pathVec, downloadId, fieldkey, isCallbackValid, downloadCallback);
    reply.WriteInt64(downloadId);
    reply.WriteInt32(res);
    LOGI("End HandleStartFileCache");
    return E_OK;
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
    bool needClean = data.ReadBool();

    int32_t res = StopDownloadFile(path, needClean);
    reply.WriteInt32(res);
    LOGI("End HandleStopDownloadFile");
    return E_OK;
}

int32_t CloudSyncServiceStub::HandleStopFileCache(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleStopFileCache");
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_AUTH_URI)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    int64_t downloadId = data.ReadInt64();
    bool needClean = data.ReadBool();

    int32_t res = StopFileCache(downloadId, needClean);
    reply.WriteInt32(res);
    LOGI("End HandleStopFileCache");
    return E_OK;
}

int32_t CloudSyncServiceStub::HandleRegisterDownloadFileCallback(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleRegisterDownloadFileCallback");
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }

    auto downloadCallback = data.ReadRemoteObject();

    int32_t res = RegisterDownloadFileCallback(downloadCallback);
    reply.WriteInt32(res);
    LOGI("End HandleRegisterDownloadFileCallback");
    return E_OK;
}

int32_t CloudSyncServiceStub::HandleUnregisterDownloadFileCallback(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleUnregisterDownloadFileCallback");
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }

    int32_t res = UnregisterDownloadFileCallback();
    reply.WriteInt32(res);
    LOGI("End HandleUnregisterDownloadFileCallback");
    return E_OK;
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
    return E_OK;
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
    return E_OK;
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
    if (size > READ_SIZE) {
        return E_INVAL_ARG;
    }
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
    return E_OK;
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
    int32_t res = DownloadAsset(taskId, userId, bundleName, networkId, *assetInfoObj);
    reply.WriteInt32(res);
    LOGI("End DownloadAsset");
    return E_OK;
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
    return E_OK;
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
    return E_OK;
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
    return E_OK;
}

int32_t CloudSyncServiceStub::HandleBatchDentryFileInsert(MessageParcel &data, MessageParcel &reply)
{
    LOGI("Begin HandleBatchDentryFileInsert");
    if (!DfsuAccessTokenHelper::CheckCallerPermission(PERM_CLOUD_SYNC)) {
        LOGE("permission denied");
        return E_PERMISSION_DENIED;
    }
    if (!DfsuAccessTokenHelper::IsSystemApp()) {
        LOGE("caller hap is not system hap");
        return E_PERMISSION_SYSTEM;
    }

    int32_t size = data.ReadInt32();
    std::vector<DentryFileInfoObj> dentryFileInfoObj;
    if (size > MAX_READ_DENTRY_FILE_SIZE) {
        return E_INVAL_ARG;
    }
    for (int i = 0; i < size; i++) {
        sptr<DentryFileInfoObj> obj = data.ReadParcelable<DentryFileInfoObj>();
        if (!obj) {
            LOGE("object of obj is nullptr");
            return E_INVAL_ARG;
        }
        dentryFileInfoObj.emplace_back(*obj);
    }

    std::vector<std::string> failCloudId;
    int32_t res = BatchDentryFileInsert(dentryFileInfoObj, failCloudId);
    reply.WriteStringVector(failCloudId);
    reply.WriteInt32(res);
    LOGI("End HandleBatchDentryFileInsert");
    return E_OK;
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
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync
