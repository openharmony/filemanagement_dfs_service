/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_SERVICE_PROXY_H
#define MOCK_SERVICE_PROXY_H

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <sys/stat.h>
#include <unistd.h>

#include "service_proxy.h"
#include "icloud_sync_service.h"

namespace OHOS {
namespace FileManagement::CloudSync {
using namespace std;

class CloudSyncServiceMock : public ICloudSyncService {
public:
    MOCK_METHOD0(AsObject, sptr<IRemoteObject>());
    MOCK_METHOD1(UnRegisterCallbackInner, int32_t(const std::string &bundleName));
    MOCK_METHOD1(UnRegisterFileSyncCallbackInner, int32_t(const std::string &bundleName));
    MOCK_METHOD2(RegisterCallbackInner,
                 int32_t(const sptr<IRemoteObject> &remoteObject, const std::string &bundleName));
    MOCK_METHOD2(RegisterFileSyncCallbackInner,
                 int32_t(const sptr<IRemoteObject> &remoteObject, const std::string &bundleName));
    MOCK_METHOD2(StartSyncInner, int32_t(bool forceFlag, const std::string &bundleName));
    MOCK_METHOD2(StartFileSyncInner, int32_t(bool forceFlag, const std::string &bundleName));
    MOCK_METHOD2(TriggerSyncInner, int32_t(const std::string &bundleName, int32_t userId));
    MOCK_METHOD2(StopSyncInner, int32_t(const std::string &bundleName, bool forceFlag));
    MOCK_METHOD2(StopFileSyncInner, int32_t(const std::string &bundleName, bool forceFlag));
    MOCK_METHOD2(ResetCursor, int32_t(bool flag, const std::string &bundleName));
    MOCK_METHOD3(ChangeAppSwitch, int32_t(const std::string &accoutId, const std::string &bundleName, bool status));
    MOCK_METHOD3(OptimizeStorage,
                 int32_t(const OptimizeSpaceOptions &optimizeOptions,
                         bool isCallbackValid,
                         const sptr<IRemoteObject> &optimizeCallback));
    MOCK_METHOD0(StopOptimizeStorage, int32_t());
    MOCK_METHOD2(Clean, int32_t(const std::string &accountId, const CleanOptions &cleanOptions));
    MOCK_METHOD2(NotifyDataChange, int32_t(const std::string &accoutId, const std::string &bundleName));
    MOCK_METHOD3(NotifyEventChange, int32_t(int32_t userId, const std::string &eventId, const std::string &extraData));
    MOCK_METHOD2(EnableCloud, int32_t(const std::string &accoutId, const SwitchDataObj &switchData));
    MOCK_METHOD1(DisableCloud, int32_t(const std::string &accoutId));
    MOCK_METHOD3(StartDownloadFile,
                 int32_t(const std::string &uri,
                         const sptr<IRemoteObject> &downloadCallback,
                         int64_t &downloadId));
    MOCK_METHOD5(StartFileCache,
                 int32_t(const std::vector<std::string> &uriVec,
                         int64_t &downloadId,
                         int32_t fieldkey,
                         const sptr<IRemoteObject> &downloadCallback,
                         int32_t timeout));
    MOCK_METHOD2(StopDownloadFile, int32_t(int64_t downloadId, bool needClean));
    MOCK_METHOD3(StopFileCache, int32_t(int64_t downloadId, bool needClean, int32_t timeout));
    MOCK_METHOD0(DownloadThumb, int32_t());
    MOCK_METHOD3(UploadAsset, int32_t(const int32_t userId, const std::string &request, std::string &result));
    MOCK_METHOD3(DownloadFile,
                 int32_t(const int32_t userId, const std::string &bundleName, const AssetInfoObj &assetInfoObj));
    MOCK_METHOD5(DownloadFiles,
                 int32_t(const int32_t userId,
                         const std::string &bundleName,
                         const std::vector<AssetInfoObj> &assetInfoObj,
                         std::vector<bool> &assetResultMap,
                         int32_t connectTime));
    MOCK_METHOD5(DownloadAsset,
                 int32_t(const uint64_t taskId,
                         const int32_t userId,
                         const std::string &bundleName,
                         const std::string &networkId,
                         const AssetInfoObj &assetInfoObj));
    MOCK_METHOD1(RegisterDownloadAssetCallback, int32_t(const sptr<IRemoteObject> &remoteObject));
    MOCK_METHOD2(DeleteAsset, int32_t(const int32_t userId, const std::string &uri));
    MOCK_METHOD2(GetSyncTimeInner, int32_t(int64_t &syncTime, const std::string &bundleName));
    MOCK_METHOD1(CleanCacheInner, int32_t(const std::string &uri));
    MOCK_METHOD1(CleanFileCacheInner, int32_t(const std::string &uri));
    MOCK_METHOD2(BatchCleanFile,
                 int32_t(const std::vector<CleanFileInfoObj> &fileInfo, std::vector<std::string> &failCloudId));
    MOCK_METHOD2(BatchDentryFileInsert,
                 int32_t(const std::vector<DentryFileInfoObj> &fileInfo, std::vector<std::string> &failCloudId));
    MOCK_METHOD2(StartDowngrade, int32_t(const std::string &bundleName, const sptr<IRemoteObject> &downloadCallback));
    MOCK_METHOD1(StopDowngrade, int32_t(const std::string &bundleName));
    MOCK_METHOD2(GetCloudFileInfo, int32_t(const std::string &bundleName, CloudFileInfo &cloudFileInfo));
    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));
    MOCK_METHOD3(GetHistoryVersionList,
                 int32_t(const string &uri, const int32_t versionNumLimit, vector<HistoryVersion> &historyVersionList));
    MOCK_METHOD5(DownloadHistoryVersion, int32_t(const string &uri, int64_t &downloadId, const uint64_t versionId,
                 const sptr<IRemoteObject> &downloadCallback, string &versionUri));
    MOCK_METHOD2(ReplaceFileWithHistoryVersion, int32_t(const string &uri, const string &versionUri));
    MOCK_METHOD2(IsFileConflict, int32_t(const string &uri, bool &isConflict));
    MOCK_METHOD1(ClearFileConflict, int32_t(const string &uri));

private:
    int32_t StartFileCacheWriteParcel(MessageParcel &data,
                                      const std::vector<std::string> &uriVec,
                                      std::bitset<FIELD_KEY_MAX_SIZE> &fieldkey,
                                      bool isCallbackValid,
                                      const sptr<IRemoteObject> &downloadCallback,
                                      int32_t timeout)
    {
        return 0;
    }
};

class IserviceProxy {
public:
    virtual sptr<ICloudSyncService> GetInstance() = 0;
    virtual ~IserviceProxy() = default;
    static inline std::shared_ptr<IserviceProxy> proxy_{nullptr};
};

class MockServiceProxy : public IserviceProxy {
public:
    MOCK_METHOD0(GetInstance, sptr<ICloudSyncService>());
};

sptr<ICloudSyncService> ServiceProxy::GetInstance()
{
    if (IserviceProxy::proxy_ == nullptr) {
        return nullptr;
    }
    return IserviceProxy::proxy_->GetInstance();
}

} // FileManagement::CloudSync
} // OHOS
#endif // MOCK_SERVICE_PROXY_H