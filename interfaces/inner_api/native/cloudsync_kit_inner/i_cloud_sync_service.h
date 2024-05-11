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

#ifndef OHOS_FILEMGMT_I_CLOUD_SYNC_SERVICE_H
#define OHOS_FILEMGMT_I_CLOUD_SYNC_SERVICE_H

#include <map>

#include "iremote_broker.h"

#include "cloud_sync_common.h"

namespace OHOS::FileManagement::CloudSync {
class ICloudSyncService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.Filemanagement.Dfs.ICloudSyncService")

    virtual int32_t UnRegisterCallbackInner(const std::string &bundleName = "") = 0;
    virtual int32_t RegisterCallbackInner(const sptr<IRemoteObject> &remoteObject,
                                          const std::string &bundleName = "") = 0;
    virtual int32_t StartSyncInner(bool forceFlag, const std::string &bundleName = "") = 0;
    virtual int32_t TriggerSyncInner(const std::string &bundleName, const int32_t &userId) = 0;
    virtual int32_t StopSyncInner(const std::string &bundleName = "") = 0;
    virtual int32_t ChangeAppSwitch(const std::string &accoutId, const std::string &bundleName, bool status) = 0;
    virtual int32_t Clean(const std::string &accountId, const CleanOptions &cleanOptions) = 0;
    virtual int32_t NotifyDataChange(const std::string &accoutId, const std::string &bundleName) = 0;
    virtual int32_t NotifyEventChange(int32_t userId, const std::string &eventId, const std::string &extraData) = 0;
    virtual int32_t EnableCloud(const std::string &accoutId, const SwitchDataObj &switchData) = 0;
    virtual int32_t DisableCloud(const std::string &accoutId) = 0;
    virtual int32_t StartDownloadFile(const std::string &path) = 0;
    virtual int32_t StartFileCache(const std::string &path) = 0;
    virtual int32_t StopDownloadFile(const std::string &path) = 0;
    virtual int32_t RegisterDownloadFileCallback(const sptr<IRemoteObject> &downloadCallback) = 0;
    virtual int32_t UnregisterDownloadFileCallback() = 0;
    virtual int32_t UploadAsset(const int32_t userId, const std::string &request, std::string &result) = 0;
    virtual int32_t DownloadFile(const int32_t userId, const std::string &bundleName, AssetInfoObj &assetInfoObj) = 0;
    virtual int32_t DownloadFiles(const int32_t userId,
                                  const std::string &bundleName,
                                  const std::vector<AssetInfoObj> &assetInfoObj,
                                  std::vector<bool> &assetResultMap) = 0;
    virtual int32_t DownloadAsset(const uint64_t taskId,
                                  const int32_t userId,
                                  const std::string &bundleName,
                                  const std::string &networkId,
                                  AssetInfoObj &assetInfoObj) = 0;
    virtual int32_t RegisterDownloadAssetCallback(const sptr<IRemoteObject> &remoteObject) = 0;
    virtual int32_t DeleteAsset(const int32_t userId, const std::string &uri) = 0;
    virtual int32_t GetSyncTimeInner(int64_t &syncTime, const std::string &bundleName = "") = 0;
    virtual int32_t CleanCacheInner(const std::string &uri) = 0;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_I_CLOUD_SYNC_SERVICE_H
