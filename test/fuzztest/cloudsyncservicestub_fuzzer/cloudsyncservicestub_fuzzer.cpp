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
#include <map>
#include <iostream>

#include "cloud_sync_service_stub.h"
#include "securec.h"
#include "message_parcel.h"
#include "cloud_file_sync_service_interface_code.h"
#include "dfs_error.h"

namespace OHOS {

constexpr size_t FOO_MAX_LEN = 1024;
constexpr size_t U32_AT_SIZE = 4;

using namespace OHOS::FileManagement::CloudSync;
using namespace std;
using namespace OHOS::FileManagement;

const std::u16string CLOUD_SYNC_SERVICE_TOKEN = u"OHOS.Filemanagement.Dfs.ICloudSyncService";

class CloudSyncServiceStubImpl : public CloudSyncServiceStub {
public:
    CloudSyncServiceStubImpl() = default;
    ~CloudSyncServiceStubImpl() override {}
    int32_t UnRegisterCallbackInner(const std::string &bundleName = "") override
    {
        return E_OK;
    }
    int32_t RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject,
                                  const std::string &bundleName = "") override
    {
        return E_OK;
    }
    int32_t StartSyncInner(bool forceFlag, const std::string &bundleName = "") override
    {
        return E_OK;
    }
    int32_t TriggerSyncInner(const std::string &bundleName, const int32_t &userId) override
    {
        return E_OK;
    }
    int32_t StopSyncInner(const std::string &bundleName = "") override
    {
        return E_OK;
    }
    int32_t ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status) override
    {
        return E_OK;
    }
    int32_t Clean(const std::string &accountId, const CleanOptions &cleanOptions) override
    {
        return E_OK;
    }
    int32_t NotifyDataChange(const std::string &accoutId, const std::string &bundleName) override
    {
        return E_OK;
    }
    int32_t NotifyEventChange(int32_t userId, const std::string &eventId, const std::string &extraData) override
    {
        return E_OK;
    }
    int32_t EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData) override
    {
        return E_OK;
    }
    int32_t DisableCloud(const std::string &accoutId) override
    {
        return E_OK;
    }
    int32_t StartDownloadFile(const std::string &path) override
    {
        return E_OK;
    }
    int32_t StartFileCache(const std::string &path) override
    {
        return E_OK;
    }
    int32_t StopDownloadFile(const std::string &path) override
    {
        return E_OK;
    }
    int32_t RegisterDownloadFileCallback(const sptr<IRemoteObject> &downloadCallback) override
    {
        return E_OK;
    }
    int32_t UnregisterDownloadFileCallback() override
    {
        return E_OK;
    }
    int32_t UploadAsset(const int32_t userId, const std::string &request, std::string &result) override
    {
        return E_OK;
    }
    int32_t DownloadFile(const int32_t userId, const std::string &bundleName, AssetInfoObj &assetInfoObj) override
    {
        return E_OK;
    }
    int32_t DownloadFiles(const int32_t userId,
                          const std::string &bundleName,
                          const std::vector<AssetInfoObj> &assetInfoObj,
                          std::vector<bool> &assetResultMap) override
    {
        return E_OK;
    }
    int32_t DownloadAsset(const uint64_t taskId,
                         const int32_t userId,
                         const std::string &bundleName,
                         const std::string &networkId,
                         AssetInfoObj &assetInfoObj) override
    {
        return E_OK;
    }
    int32_t RegisterDownloadAssetCallback(const sptr<IRemoteObject> &remoteObject) override
    {
        return E_OK;
    }
    int32_t DeleteAsset(const int32_t userId, const std::string &uri) override
    {
        return E_OK;
    }
    int32_t GetSyncTimeInner(int64_t &syncTime, const std::string &bundleName = "") override
    {
        return E_OK;
    }
    int32_t CleanCacheInner(const std::string &uri) override
    {
        return E_OK;
    }
};

void HandleChangeAppSwitchFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_CHANGE_APP_SWITCH
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CHANGE_APP_SWITCH);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleCleanFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_CLEAN
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_CLEAN);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleDeleteAssetFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_DELETE_ASSET
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DELETE_ASSET);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleDisableCloudFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_DISABLE_CLOUD
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DISABLE_CLOUD);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleDownloadFileFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_DOWNLOAD_FILE
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_DOWNLOAD_FILE);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleEnableCloudFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_ENABLE_CLOUD
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_ENABLE_CLOUD);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleNotifyDataChangeFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_NOTIFY_DATA_CHANGE
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_NOTIFY_DATA_CHANGE);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleRegisterCallbackInnerFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_REGISTER_CALLBACK
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_CALLBACK);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleRegisterDownloadFileCallbackFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_REGISTER_DOWNLOAD_FILE_CALLBACK
    uint32_t code =
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_REGISTER_DOWNLOAD_FILE_CALLBACK);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleStartDownloadFileFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_START_DOWNLOAD_FILE
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_DOWNLOAD_FILE);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleStartSyncInnerFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_START_SYNC
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_SYNC);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleStopDownloadFileFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_STOP_DOWNLOAD_FILE
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_START_SYNC);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleStopSyncInnerFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_STOP_SYNC
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_STOP_SYNC);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleUnRegisterCallbackInnerFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_UNREGISTER_CALLBACK
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_CALLBACK);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleUnRegisterDownloadFileCallbackFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_UNREGISTER_DOWNLOAD_FILE_CALLBACK
    uint32_t code =
        static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UNREGISTER_DOWNLOAD_FILE_CALLBACK);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

void HandleUploadAssetFuzzTest(
    std::shared_ptr<CloudSyncServiceStub> cloudSyncServiceStubStr,
    std::unique_ptr<char[]> data, size_t size)
{
    // SERVICE_CMD_UPLOAD_ASSET
    uint32_t code = static_cast<uint32_t>(CloudFileSyncServiceInterfaceCode::SERVICE_CMD_UPLOAD_ASSET);
    MessageParcel datas;
    datas.WriteInterfaceToken(CLOUD_SYNC_SERVICE_TOKEN);
    datas.WriteBuffer(data.get(), size);
    datas.RewindRead(0);
    MessageParcel reply;
    MessageOption option;

    cloudSyncServiceStubStr->OnRemoteRequest(code, datas, reply, option);
}

} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr) {
        return 0;
    }

    /* Validate the length of size */
    if (size < OHOS::U32_AT_SIZE || size > OHOS::FOO_MAX_LEN) {
        return 0;
    }

    auto str = std::make_unique<char[]>(size + 1);
    (void)memset_s(str.get(), size + 1, 0x00, size + 1);
    if (memcpy_s(str.get(), size, data, size) != EOK) {
        return 0;
    }

    auto cloudSyncServiceStubStr = std::make_shared<OHOS::CloudSyncServiceStubImpl>();

    OHOS::HandleChangeAppSwitchFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleCleanFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleDeleteAssetFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleDisableCloudFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleDownloadFileFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleEnableCloudFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleNotifyDataChangeFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleRegisterCallbackInnerFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleRegisterDownloadFileCallbackFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleStartDownloadFileFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleStartSyncInnerFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleStopDownloadFileFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleStopSyncInnerFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleUnRegisterCallbackInnerFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleUnRegisterDownloadFileCallbackFuzzTest(cloudSyncServiceStubStr, move(str), size);
    OHOS::HandleUploadAssetFuzzTest(cloudSyncServiceStubStr, move(str), size);
    return 0;
}
