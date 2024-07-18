/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "cloudsyncservicestub_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <fcntl.h>
#include <map>

#include "accesstoken_kit.h"
#include "cloud_file_sync_service_interface_code.h"
#include "cloud_fuzzer_helper.h"
#include "cloud_sync_service.h"
#include "cloud_sync_service_stub.h"
#include "i_cloud_download_callback.h"
#include "i_cloud_sync_callback.h"
#include "i_download_asset_callback.h"
#include "task_state_manager.h"

#include "message_parcel.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "utils_log.h"

namespace OHOS {
constexpr size_t U32_AT_SIZE = 4;
constexpr size_t U64_AT_SIZE = 8;
constexpr size_t BOOL_AT_SIZE = 1;
constexpr int SPLITE_SIZE = 5;
const std::u16string CLOUD_SYNC_SERVICE_TOKEN = u"OHOS.Filemanagement.Dfs.ICloudSyncService";
constexpr int32_t SERVICE_SA_ID = 5204;

using namespace OHOS::FileManagement::CloudSync;
using namespace std;
class ICloudSyncCallbackTest : public IRemoteStub<ICloudSyncCallback> {
public:
    void OnSyncStateChanged(SyncType type, SyncPromptState state) override {}
    void OnSyncStateChanged(CloudSyncState state, ErrorType error) override {}
};

class ICloudDownloadCallbackTest : public IRemoteStub<ICloudDownloadCallback> {
public:
    void OnDownloadProcess(const DownloadProgressObj &progress) override {}
};

class IDownloadAssetCallbackTest : public IRemoteStub<IDownloadAssetCallback> {
public:
    void OnFinished(const TaskId taskId, const std::string &uri, const int32_t result) override {}
};

void NativeTokenGet(bool isSystem)
{
    uint64_t tokenId;
    static const char *perms[] = {"ohos.permission.CLOUDFILE_SYNC", "ohos.permission.CLOUDFILE_SYNC_MANAGER",
                                  "ohos.permission.PROXY_AUTHORIZATION_URI"};
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 3,
        .aclsNum = 0,
        .dcaps = nullptr,
        .perms = perms,
        .acls = nullptr,
        .aplStr = "system_core",
    };

    infoInstance.processName = "CloudOnRemoteRequestFuzzerTest";
    tokenId = GetAccessTokenId(&infoInstance);
    if (isSystem) {
        const uint64_t systemAppMask = (static_cast<uint64_t>(1) << 32);
        tokenId |= systemAppMask;
    }
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

bool WriteInterfaceToken(MessageParcel &data)
{
    if (!data.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN)) {
        LOGE("Write token failed.");
        return false;
    }
    return true;
}

void HandleChangeAppSwitchFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                   FuzzData &fuzzData,
                                   size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    bool status = fuzzData.GetData<bool>();
    int pos = static_cast<int>((size - BOOL_AT_SIZE) >> 1);
    std::string accountId = fuzzData.GetStringFromData(pos);
    std::string bundleName = fuzzData.GetStringFromData(pos);
    datas.WriteString(accountId);
    datas.WriteString(bundleName);
    datas.WriteBool(status);
    datas.RewindRead(0);
    // SERVICE_CMD_CHANGE_APP_SWITCH
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CHANGE_APP_SWITCH);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleCleanFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr, FuzzData &fuzzData, size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    std::string uri = fuzzData.GetStringFromData(static_cast<int>(size));
    datas.WriteString(uri);
    datas.RewindRead(0);
    // SERVICE_CMD_CLEAN
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CLEAN);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleDeleteAssetFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                               FuzzData &fuzzData,
                               size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    int32_t userId = fuzzData.GetData<int32_t>();
    datas.WriteInt32(userId);
    int len = static_cast<int>(size - U32_AT_SIZE);
    std::string uri = fuzzData.GetStringFromData(len);
    datas.WriteString(uri);
    datas.RewindRead(0);
    // SERVICE_CMD_DELETE_ASSET
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DELETE_ASSET);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleDisableCloudFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                FuzzData &fuzzData,
                                size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    string accountId = fuzzData.GetStringFromData(static_cast<int>(size));
    datas.WriteString(accountId);
    datas.RewindRead(0);
    // SERVICE_CMD_DISABLE_CLOUD
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DISABLE_CLOUD);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleDownloadFileFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                FuzzData &fuzzData,
                                size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    int32_t userId = fuzzData.GetData<int32_t>();
    datas.WriteInt32(userId);
    int pos = static_cast<int>(size - U32_AT_SIZE) / (SPLITE_SIZE + 1);
    string bundleName = fuzzData.GetStringFromData(pos);
    datas.WriteString(bundleName);
    AssetInfo assetInfo = {.uri = fuzzData.GetStringFromData(pos),
                           .recordType = fuzzData.GetStringFromData(pos),
                           .recordId = fuzzData.GetStringFromData(pos),
                           .fieldKey = fuzzData.GetStringFromData(pos),
                           .assetName = fuzzData.GetStringFromData(pos)};
    AssetInfoObj assetInfoObj(assetInfo);
    datas.WriteParcelable(&assetInfoObj);
    datas.RewindRead(0);
    // SERVICE_CMD_DOWNLOAD_FILE
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_FILE);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleEnableCloudFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                               FuzzData &fuzzData,
                               size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    int pos = static_cast<int>(size >> 1);
    string accountId = fuzzData.GetStringFromData(pos);
    datas.WriteString(accountId);
    int itemStrLen = pos / SPLITE_SIZE;
    if (itemStrLen <= static_cast<int>(BOOL_AT_SIZE)) {
        return;
    }
    std::map<string, bool> switchData;
    for (int i = 0; i < SPLITE_SIZE; i++) {
        string itemStr = fuzzData.GetStringFromData(itemStrLen - static_cast<int>(BOOL_AT_SIZE));
        bool itemBool = fuzzData.GetData<bool>();
        switchData.insert(pair<string, bool>(itemStr, itemBool));
    }
    SwitchDataObj switchDataObj;
    switchDataObj.switchData = switchData;
    datas.WriteParcelable(&switchDataObj);
    datas.RewindRead(0);
    // SERVICE_CMD_ENABLE_CLOUD
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_ENABLE_CLOUD);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleNotifyDataChangeFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                    FuzzData &fuzzData,
                                    size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    int pos = static_cast<int>(size >> 1);
    string accountId = fuzzData.GetStringFromData(pos);
    datas.WriteString(accountId);
    string bundleName = fuzzData.GetStringFromData(pos);
    datas.WriteString(bundleName);
    datas.RewindRead(0);
    // SERVICE_CMD_NOTIFY_DATA_CHANGE
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_NOTIFY_DATA_CHANGE);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleRegisterCallbackInnerFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                         FuzzData &fuzzData,
                                         size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    sptr<ICloudSyncCallbackTest> callback = new (std::nothrow) ICloudSyncCallbackTest();
    if (callback == nullptr) {
        return;
    }
    datas.WriteRemoteObject(callback->AsObject().GetRefPtr());
    string bundleName = fuzzData.GetStringFromData(static_cast<int>(size));
    datas.WriteString(bundleName);
    datas.RewindRead(0);
    // SERVICE_CMD_REGISTER_CALLBACK
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_CALLBACK);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleRegisterDownloadFileCallbackFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                                FuzzData &fuzzData,
                                                size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    sptr<ICloudDownloadCallbackTest> callback = new (std::nothrow) ICloudDownloadCallbackTest();
    if (callback == nullptr) {
        return;
    }
    datas.WriteRemoteObject(callback->AsObject().GetRefPtr());
    datas.RewindRead(0);
    // SERVICE_CMD_REGISTER_DOWNLOAD_FILE_CALLBACK
    uint32_t code =
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_DOWNLOAD_FILE_CALLBACK);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleStartDownloadFileFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                     FuzzData &fuzzData,
                                     size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    string path = fuzzData.GetStringFromData(static_cast<int>(size));
    datas.WriteString(path);
    datas.RewindRead(0);
    // SERVICE_CMD_START_DOWNLOAD_FILE
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_DOWNLOAD_FILE);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleStartSyncInnerFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                  FuzzData &fuzzData,
                                  size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    auto forceFlag = fuzzData.GetData<bool>();
    datas.WriteBool(forceFlag);
    string bundleName = fuzzData.GetStringFromData(static_cast<int>(size - BOOL_AT_SIZE));
    datas.WriteString(bundleName);
    datas.RewindRead(0);
    // SERVICE_CMD_START_SYNC
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_SYNC);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleStopDownloadFileFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                    FuzzData &fuzzData,
                                    size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    string path = fuzzData.GetStringFromData(static_cast<int>(size));
    datas.WriteString(path);
    datas.RewindRead(0);
    // SERVICE_CMD_STOP_DOWNLOAD_FILE
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_DOWNLOAD_FILE);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleStopSyncInnerFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                 const uint8_t *data,
                                 size_t size)
{
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    // SERVICE_CMD_STOP_SYNC
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_SYNC);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleUnRegisterCallbackInnerFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                           FuzzData &fuzzData,
                                           size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    string bundleName = fuzzData.GetStringFromData(static_cast<int>(size));
    datas.WriteString(bundleName);
    datas.RewindRead(0);
    // SERVICE_CMD_UNREGISTER_CALLBACK
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_CALLBACK);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleUnRegisterDownloadFileCallbackFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                                  const uint8_t *data,
                                                  size_t size)
{
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    datas.WriteBuffer(data, size);
    datas.RewindRead(0);
    // SERVICE_CMD_UNREGISTER_DOWNLOAD_FILE_CALLBACK
    uint32_t code =
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_DOWNLOAD_FILE_CALLBACK);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleUploadAssetFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                               FuzzData &fuzzData,
                               size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    int32_t userId = fuzzData.GetData<int32_t>();
    datas.WriteInt32(userId);
    int len = static_cast<int32_t>(size - U32_AT_SIZE);
    string request = fuzzData.GetStringFromData(len);
    datas.WriteString(request);
    datas.RewindRead(0);
    // SERVICE_CMD_UPLOAD_ASSET
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UPLOAD_ASSET);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleTriggerSyncInnerFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                    FuzzData &fuzzData,
                                    size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    int len = static_cast<int32_t>(size - U32_AT_SIZE);
    string bundleName = fuzzData.GetStringFromData(len);
    datas.WriteString(bundleName);
    int32_t userId = fuzzData.GetData<int32_t>();
    datas.WriteInt32(userId);
    datas.RewindRead(0);
    // SERVICE_CMD_TRIGGER_SYNC
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_TRIGGER_SYNC);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleNotifyEventChangeFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                     FuzzData &fuzzData,
                                     size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    int32_t userId = fuzzData.GetData<int32_t>();
    datas.WriteInt32(userId);

    int pos = static_cast<int>((size - U32_AT_SIZE) >> 1);
    string eventIdStr = fuzzData.GetStringFromData(pos);
    string extraDataStr = fuzzData.GetStringFromData(pos);
    datas.WriteString(eventIdStr);
    datas.WriteString(extraDataStr);
    datas.RewindRead(0);
    // SERVICE_CMD_NOTIFY_EVENT_CHANGE
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_NOTIFY_EVENT_CHANGE);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleStartFileCacheFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                  FuzzData &fuzzData,
                                  size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    int32_t userId = fuzzData.GetData<int32_t>();
    datas.WriteInt32(userId);
    int pos = static_cast<int>((size - U32_AT_SIZE) >> 1);
    string eventIdStr = fuzzData.GetStringFromData(pos);
    string extraDataStr = fuzzData.GetStringFromData(pos);
    datas.WriteString(eventIdStr);
    datas.WriteString(extraDataStr);
    datas.RewindRead(0);
    // SERVICE_CMD_START_FILE_CACHE
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_FILE_CACHE);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleDownloadFilesFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                 FuzzData &fuzzData,
                                 size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    int32_t userId = fuzzData.GetData<int32_t>();
    datas.WriteInt32(userId);
    int32_t vecSize = fuzzData.GetData<int32_t>() % SPLITE_SIZE + 1;
    auto remainSize = fuzzData.GetRemainSize();
    if (static_cast<int>(remainSize) <= vecSize * SPLITE_SIZE + 1) {
        return;
    }
    int len = static_cast<int>(remainSize / (vecSize * SPLITE_SIZE + 1));
    string bundleName = fuzzData.GetStringFromData(len);
    datas.WriteString(bundleName);
    datas.WriteInt32(vecSize);
    for (auto i = 0; i < vecSize; i++) {
        AssetInfo assetInfo = {.uri = fuzzData.GetStringFromData(len),
                               .recordType = fuzzData.GetStringFromData(len),
                               .recordId = fuzzData.GetStringFromData(len),
                               .fieldKey = fuzzData.GetStringFromData(len),
                               .assetName = fuzzData.GetStringFromData(len)};
        AssetInfoObj assetInfoObj(assetInfo);
        datas.WriteParcelable(&assetInfoObj);
    }
    datas.RewindRead(0);
    // SERVICE_CMD_DOWNLOAD_FILES
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_FILES);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleDownloadAssetFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                 FuzzData &fuzzData,
                                 size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    uint64_t taskId = fuzzData.GetData<uint64_t>();
    datas.WriteUint64(taskId);
    int32_t userId = fuzzData.GetData<int32_t>();
    datas.WriteUint32(userId);
    int len = static_cast<int>(fuzzData.GetRemainSize() >> 1);
    string bundleName = fuzzData.GetStringFromData(len);
    datas.WriteString(bundleName);
    string networkId = fuzzData.GetStringFromData(len);
    datas.WriteString(networkId);

    fuzzData.ResetData(size);
    len = static_cast<int>(size) / SPLITE_SIZE;
    AssetInfo assetInfo = {.uri = fuzzData.GetStringFromData(len),
                           .recordType = fuzzData.GetStringFromData(len),
                           .recordId = fuzzData.GetStringFromData(len),
                           .fieldKey = fuzzData.GetStringFromData(len),
                           .assetName = fuzzData.GetStringFromData(len)};
    AssetInfoObj assetInfoObj(assetInfo);
    datas.WriteParcelable(&assetInfoObj);
    datas.RewindRead(0);
    // SERVICE_CMD_DOWNLOAD_ASSET
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_ASSET);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleRegisterDownloadAssetCallbackFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                                                 FuzzData &fuzzData,
                                                 size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    sptr<IDownloadAssetCallbackTest> callback = new (std::nothrow) IDownloadAssetCallbackTest();
    if (callback == nullptr) {
        return;
    }
    datas.WriteRemoteObject(callback->AsObject().GetRefPtr());
    datas.RewindRead(0);
    // SERVICE_CMD_REGISTER_DOWNLOAD_ASSET_CALLBACK
    uint32_t code =
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_DOWNLOAD_ASSET_CALLBACK);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleGetSyncTimeFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                               FuzzData &fuzzData,
                               size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    string bundleName = fuzzData.GetStringFromData(static_cast<int>(size));
    datas.WriteString(bundleName);
    datas.RewindRead(0);
    // SERVICE_CMD_GET_SYNC_TIME
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_GET_SYNC_TIME);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleCleanCacheFuzzTest(std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
                              FuzzData &fuzzData,
                              size_t size)
{
    fuzzData.ResetData(size);
    MessageParcel datas;
    if (!WriteInterfaceToken(datas)) {
        return;
    }
    string uri = fuzzData.GetStringFromData(static_cast<int>(size));
    datas.WriteString(uri);
    datas.RewindRead(0);
    // SERVICE_CMD_CLEAN_CACHE
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CLEAN_CACHE);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size < OHOS::U32_AT_SIZE * static_cast<size_t>(OHOS::SPLITE_SIZE)) {
        return 0;
    }

    OHOS::NativeTokenGet(true);
    auto cloudSyncServiceStubStr =
        std::make_shared<OHOS::FileManagement::CloudSync::CloudSyncService>(OHOS::SERVICE_SA_ID);
    if (cloudSyncServiceStubStr == nullptr) {
        return false;
    }
    if (cloudSyncServiceStubStr->dataSyncManager_ == nullptr) {
        cloudSyncServiceStubStr->dataSyncManager_ =
            std::make_shared<OHOS::FileManagement::CloudFile::DataSyncManager>();
    }

    OHOS::FuzzData fuzzData(data, size);
    OHOS::HandleChangeAppSwitchFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleCleanFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleDeleteAssetFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleDisableCloudFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleDownloadFileFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleEnableCloudFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleNotifyDataChangeFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleRegisterCallbackInnerFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleRegisterDownloadFileCallbackFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleStartDownloadFileFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleStartSyncInnerFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleStopDownloadFileFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleStopSyncInnerFuzzTest(cloudSyncServiceStubStr, data, size);
    OHOS::HandleUnRegisterCallbackInnerFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleUnRegisterDownloadFileCallbackFuzzTest(cloudSyncServiceStubStr, data, size);
    OHOS::HandleUploadAssetFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleTriggerSyncInnerFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleNotifyEventChangeFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleStartFileCacheFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleDownloadFilesFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleDownloadAssetFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleRegisterDownloadAssetCallbackFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleGetSyncTimeFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    OHOS::HandleCleanCacheFuzzTest(cloudSyncServiceStubStr, fuzzData, size);
    if (OHOS::FileManagement::CloudSync::TaskStateManager::GetInstance().unloadTaskHandle_ != nullptr) {
        OHOS::FileManagement::CloudSync::TaskStateManager::GetInstance().queue_.wait(
            OHOS::FileManagement::CloudSync::TaskStateManager::GetInstance().unloadTaskHandle_);
    }
    return 0;
}
