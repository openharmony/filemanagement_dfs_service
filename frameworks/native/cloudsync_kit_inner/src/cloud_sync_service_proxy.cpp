/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "cloud_sync_service_proxy.h"

#include "cloud_download_uri_manager.h"
#include "cloud_file_sync_service_interface_code.h"

#include <sstream>

#include "dfs_error.h"
#include "iservice_registry.h"
#ifdef SUPPORT_MEDIA_LIBRARY
#include "media_file_uri.h"
#endif
#include "system_ability_definition.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

constexpr int LOAD_SA_TIMEOUT_MS = 4000;

int32_t CloudSyncServiceProxy::UnRegisterCallbackInner(const std::string &bundleName)
{
    LOGI("Start UnRegisterCallbackInner");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_CALLBACK), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("UnRegisterCallbackInner Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject,
                                                     const std::string &bundleName)
{
    LOGI("Start RegisterCallbackInner");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!remoteObject) {
        LOGI("Empty callback stub");
        return E_INVAL_ARG;
    }

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteRemoteObject(remoteObject)) {
        LOGE("Failed to send the callback stub");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_CALLBACK), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("RegisterCallbackInner Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::StartSyncInner(bool forceFlag, const std::string &bundleName)
{
    LOGI("Start Sync");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteBool(forceFlag)) {
        LOGE("Failed to send the force flag");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_SYNC),
                                      data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("StartSyncInner Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::TriggerSyncInner(const std::string &bundleName, const int32_t &userId)
{
    LOGI("Trigger Sync");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    if (!data.WriteInt32(userId)) {
        LOGE("Failed to send the user id");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_TRIGGER_SYNC), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("TriggerSyncInner Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::GetSyncTimeInner(int64_t &syncTime, const std::string &bundleName)
{
    LOGI("Start GetSyncTimeInner");
    LOGI("Start Sync");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_GET_SYNC_TIME), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }

    syncTime = reply.ReadInt64();

    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::CleanCacheInner(const std::string &uri)
{
    LOGI("Start CleanCacheInner");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(uri)) {
        LOGE("Failed to send the uri");
        return E_BROKEN_IPC;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CLEAN_CACHE),
                                      data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }

    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::StopSyncInner(const std::string &bundleName)
{
    LOGI("StopSync");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    int32_t ret = remote->SendRequest(static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_SYNC),
                                      data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("StopSyncInner Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status)
{
    LOGI("ChangeAppSwitch");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(accoutId)) {
        LOGE("Failed to send the account id");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    if (!data.WriteBool(status)) {
        LOGE("Failed to send the switch status");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CHANGE_APP_SWITCH), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("ChangeAppSwitch Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::Clean(const std::string &accountId, const CleanOptions &cleanOptions)
{
    LOGI("Clean");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(accountId)) {
        LOGE("Failed to send the account id");
        return E_INVAL_ARG;
    }

    if (!data.WriteParcelable(&cleanOptions)) {
        LOGE("failed to write cleanOptions");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CLEAN), data,
                                      reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("Clean Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData)
{
    LOGI("EnableCloud");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(accoutId)) {
        LOGE("Failed to send the account id");
        return E_INVAL_ARG;
    }

    if (!data.WriteParcelable(&switchData)) {
        LOGE("Failed to send the bundle switch");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_ENABLE_CLOUD), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("EnableCloud Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::DisableCloud(const std::string &accoutId)
{
    LOGI("DisableCloud");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(accoutId)) {
        LOGE("Failed to send the account id");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DISABLE_CLOUD), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("DisableCloud Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::NotifyDataChange(const std::string &accoutId, const std::string &bundleName)
{
    LOGI("NotifyDataChange");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(accoutId)) {
        LOGE("Failed to send the account id");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_NOTIFY_DATA_CHANGE), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("NotifyDataChange Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::NotifyEventChange(
    int32_t userId, const std::string &eventId, const std::string &extraData)
{
    LOGI("NotifyEventChange");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteInt32(userId)) {
        LOGE("Failed to send the user id");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(eventId)) {
        LOGE("Failed to send the event id");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(extraData)) {
        LOGE("Failed to send the extra data");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_NOTIFY_EVENT_CHANGE), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("NotifyEventChange Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::StartDownloadFile(const std::string &uri)
{
#ifdef SUPPORT_MEDIA_LIBRARY
    LOGI("StartDownloadFile Start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    string path = uri;
    if (uri.find("file://media") == 0) {
        OHOS::Media::MediaFileUri mediaUri(uri);
        path = mediaUri.GetFilePath();
    }

    LOGI("StartDownloadFile Start, uri: %{public}s, path: %{public}s", uri.c_str(), path.c_str());

    CloudDownloadUriManager &uriMgr = CloudDownloadUriManager::GetInstance();
    if (uriMgr.AddPathToUri(path, uri) == E_STOP) {
        return E_OK;
    }

    if (!data.WriteString(path)) {
        LOGE("Failed to send the cloud id");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_DOWNLOAD_FILE), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("StartDownloadFile Success");
    return reply.ReadInt32();
#else
    LOGE("Function StartDownloadFile is undefined");
    return 0;
#endif
}

int32_t CloudSyncServiceProxy::StartFileCache(const std::string &uri)
{
    LOGI("StartFileCache Start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteString(uri)) {
        LOGE("Failed to send the cloud id");
        return E_INVAL_ARG;
    }

    CloudDownloadUriManager &uriMgr = CloudDownloadUriManager::GetInstance();
    if (uriMgr.AddPathToUri(uri, uri) == E_STOP) {
        return E_OK;
    }
    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_FILE_CACHE), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the request, errno %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("StartFileCache Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::StopDownloadFile(const std::string &uri)
{
    LOGI("StopDownloadFile Start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    string path = uri;
#ifdef SUPPORT_MEDIA_LIBRARY
    if (uri.find("file://media") == 0) {
        OHOS::Media::MediaFileUri Muri(uri);
        string path = Muri.GetFilePath();
    }
#endif
    LOGI("StopDownloadFile Start, uri: %{public}s, path: %{public}s", uri.c_str(), path.c_str());

    if (!data.WriteString(path)) {
        LOGE("Failed to send the cloud id");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_DOWNLOAD_FILE), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("StopDownloadFile Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::RegisterDownloadFileCallback(const sptr<IRemoteObject> &downloadCallback)
{
    LOGI("RegisterDownloadFileCallback Start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!downloadCallback) {
        LOGI("Empty callback stub");
        return E_INVAL_ARG;
    }

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteRemoteObject(downloadCallback)) {
        LOGE("Failed to send the callback stub");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_DOWNLOAD_FILE_CALLBACK), data,
        reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("RegisterDownloadFileCallback Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::UnregisterDownloadFileCallback()
{
    LOGI("UnregisterDownloadFileCallback Start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_DOWNLOAD_FILE_CALLBACK), data,
        reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("UnregisterDownloadFileCallback Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::UploadAsset(const int32_t userId, const std::string &request, std::string &result)
{
    LOGD("UploadAsset");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteInt32(userId)) {
        LOGE("Failed to send the user id");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(request)) {
        LOGE("Failed to send the request");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UPLOAD_ASSET), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    ret = reply.ReadInt32();
    result = reply.ReadString();
    LOGD("UploadAsset Success");
    return ret;
}

int32_t CloudSyncServiceProxy::DownloadFile(const int32_t userId,
                                            const std::string &bundleName,
                                            AssetInfoObj &assetInfoObj)
{
    LOGI("DownloadFile");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteInt32(userId)) {
        LOGE("Failed to send the user id");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    if (!data.WriteParcelable(&assetInfoObj)) {
        LOGE("Failed to send the bundle assetInfo");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_FILE), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("DownloadFile Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::DownloadFiles(const int32_t userId,
                                             const std::string &bundleName,
                                             const std::vector<AssetInfoObj> &assetInfoObj,
                                             std::vector<bool> &assetResultMap)
{
    LOGI("DownloadFiles");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteInt32(userId)) {
        LOGE("Failed to send the user id");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    if (assetInfoObj.size() > INT_MAX ||
        assetInfoObj.size() == 0 ||
        !data.WriteInt32(assetInfoObj.size())) {
        LOGE("Failed to send the vector size");
        return E_INVAL_ARG;
    }

    for (const auto &obj : assetInfoObj) {
        if (!data.WriteParcelable(&obj)) {
            LOGE("Failed to send the assetInfoObj");
            return E_INVAL_ARG;
        }
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_FILES), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("DownloadFile Success");
    bool readParcel = reply.ReadBoolVector(&assetResultMap);
    if (readParcel != true) {
        LOGE("Failed to ReadBoolVector");
        return E_INVAL_ARG;
    }
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::DownloadAsset(const uint64_t taskId,
                                             const int32_t userId,
                                             const std::string &bundleName,
                                             const std::string &networkId,
                                             AssetInfoObj &assetInfoObj)
{
    LOGI("DownloadFile");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteUint64(taskId)) {
        LOGE("Failed to send the task id");
        return E_INVAL_ARG;
    }

    if (!data.WriteInt32(userId)) {
        LOGE("Failed to send the user id");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(bundleName)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(networkId)) {
        LOGE("Failed to send the bundle name");
        return E_INVAL_ARG;
    }

    if (!data.WriteParcelable(&assetInfoObj)) {
        LOGE("Failed to send the bundle assetInfo");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_ASSET), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    LOGI("DownloadFile Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::RegisterDownloadAssetCallback(const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Start RegisterDownloadAssetCallback");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!remoteObject) {
        LOGI("Empty callback stub");
        return E_INVAL_ARG;
    }

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteRemoteObject(remoteObject)) {
        LOGE("Failed to send the callback stub");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_DOWNLOAD_ASSET_CALLBACK), data,
        reply, option);
    if (ret != E_OK) {
        stringstream ss;
        ss << "Failed to send out the requeset, errno:" << ret;
        LOGE("%{public}s", ss.str().c_str());
        return E_BROKEN_IPC;
    }
    LOGI("RegisterDownloadAssetCallback Success");
    return reply.ReadInt32();
}

int32_t CloudSyncServiceProxy::DeleteAsset(const int32_t userId, const std::string &uri)
{
    LOGD("DeleteAsset");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        LOGE("Failed to write interface token");
        return E_BROKEN_IPC;
    }

    if (!data.WriteInt32(userId)) {
        LOGE("Failed to send the user id");
        return E_INVAL_ARG;
    }

    if (!data.WriteString(uri)) {
        LOGE("Failed to send the uri");
        return E_INVAL_ARG;
    }

    auto remote = Remote();
    if (!remote) {
        LOGE("remote is nullptr");
        return E_BROKEN_IPC;
    }
    int32_t ret = remote->SendRequest(
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DELETE_ASSET), data, reply, option);
    if (ret != E_OK) {
        LOGE("Failed to send out the requeset, errno: %{public}d", ret);
        return E_BROKEN_IPC;
    }
    ret = reply.ReadInt32();
    LOGI("DeleteAsset Success");
    return ret;
}

sptr<ICloudSyncService> CloudSyncServiceProxy::GetInstance()
{
    LOGD("getinstance");
    unique_lock<mutex> lock(instanceMutex_);
    if (serviceProxy_ != nullptr) {
        return serviceProxy_;
    }

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        LOGE("Samgr is nullptr");
        return nullptr;
    }
    sptr<ServiceProxyLoadCallback> cloudSyncLoadCallback = new ServiceProxyLoadCallback();
    if (cloudSyncLoadCallback == nullptr) {
        LOGE("cloudSyncLoadCallback is nullptr");
        return nullptr;
    }
    int32_t ret = samgr->LoadSystemAbility(FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, cloudSyncLoadCallback);
    if (ret != E_OK) {
        LOGE("Failed to Load systemAbility, systemAbilityId:%{pulbic}d, ret code:%{pulbic}d",
             FILEMANAGEMENT_CLOUD_SYNC_SERVICE_SA_ID, ret);
        return nullptr;
    }
    unique_lock<mutex> proxyLock(proxyMutex_);
    auto waitStatus = cloudSyncLoadCallback->proxyConVar_.wait_for(
        proxyLock, std::chrono::milliseconds(LOAD_SA_TIMEOUT_MS),
        [cloudSyncLoadCallback]() { return cloudSyncLoadCallback->isLoadSuccess_.load(); });
    if (!waitStatus) {
        LOGE("Load CloudSynd SA timeout");
        return nullptr;
    }
    return serviceProxy_;
}

void CloudSyncServiceProxy::InvaildInstance()
{
    LOGI("invalid instance");
    unique_lock<mutex> lock(instanceMutex_);
    serviceProxy_ = nullptr;
}

void CloudSyncServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilitySuccess(
    int32_t systemAbilityId,
    const sptr<IRemoteObject> &remoteObject)
{
    LOGI("Load CloudSync SA success,systemAbilityId:%{public}d, remoteObj result:%{private}s", systemAbilityId,
         (remoteObject == nullptr ? "false" : "true"));
    unique_lock<mutex> lock(proxyMutex_);
    if (serviceProxy_ != nullptr) {
        LOGE("CloudSync SA proxy has been loaded");
    } else {
        serviceProxy_ = iface_cast<ICloudSyncService>(remoteObject);
    }
    isLoadSuccess_.store(true);
    proxyConVar_.notify_one();
}

void CloudSyncServiceProxy::ServiceProxyLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    LOGI("Load CloudSync SA failed,systemAbilityId:%{public}d", systemAbilityId);
    unique_lock<mutex> lock(proxyMutex_);
    serviceProxy_ = nullptr;
    isLoadSuccess_.store(false);
    proxyConVar_.notify_one();
}
} // namespace OHOS::FileManagement::CloudSync
