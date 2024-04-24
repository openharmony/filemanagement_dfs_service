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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_STUB_H
#define OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_STUB_H

#include <map>
#include "iremote_stub.h"
#include "i_cloud_sync_service.h"

namespace OHOS::FileManagement::CloudSync {
class __attribute__ ((visibility("default"))) CloudSyncServiceStub : public IRemoteStub<ICloudSyncService> {
public:
    CloudSyncServiceStub();
    virtual ~CloudSyncServiceStub() = default;
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using ServiceInterface = int32_t (CloudSyncServiceStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, ServiceInterface> opToInterfaceMap_;
    int32_t HandleUnRegisterCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterCallbackInner(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStartSyncInner(MessageParcel &data, MessageParcel &reply);
    int32_t HandleTriggerSyncInner(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStopSyncInner(MessageParcel &data, MessageParcel &reply);
    int32_t HandleChangeAppSwitch(MessageParcel &data, MessageParcel &reply);
    int32_t HandleClean(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyDataChange(MessageParcel &data, MessageParcel &reply);
    int32_t HandleNotifyEventChange(MessageParcel &data, MessageParcel &reply);
    int32_t HandleEnableCloud(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDisableCloud(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStartDownloadFile(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStartFileCache(MessageParcel &data, MessageParcel &reply);
    int32_t HandleStopDownloadFile(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterDownloadFileCallback(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUnregisterDownloadFileCallback(MessageParcel &data, MessageParcel &reply);
    int32_t HandleUploadAsset(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDownloadFile(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDownloadFiles(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDownloadAsset(MessageParcel &data, MessageParcel &reply);
    int32_t HandleRegisterDownloadAssetCallback(MessageParcel &data, MessageParcel &reply);
    int32_t HandleDeleteAsset(MessageParcel &data, MessageParcel &reply);
    int32_t HandleGetSyncTime(MessageParcel &data, MessageParcel &reply);
    int32_t HandleCleanCache(MessageParcel &data, MessageParcel &reply);
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_SYNC_SERVICE_STUB_H
