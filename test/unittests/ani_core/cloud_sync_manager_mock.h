/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_MOCK
#define OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_MOCK

#include "cloud_sync_manager.h"

#include <gmock/gmock.h>

namespace OHOS::FileManagement::CloudSync {

class CloudSyncManagerImplMock : public CloudSyncManager {
public:
    static CloudSyncManagerImplMock &GetInstance()
    {
        static CloudSyncManagerImplMock instance;
        return instance;
    }
    MOCK_METHOD1(RegisterCallback, int32_t(const CallbackInfo &callbackInfo));
    MOCK_METHOD1(RegisterFileSyncCallback, int32_t(const CallbackInfo &callbackInfo));
    MOCK_METHOD1(UnRegisterCallback, int32_t(const CallbackInfo &callbackInfo));
    MOCK_METHOD1(UnRegisterFileSyncCallback, int32_t(const CallbackInfo &callbackInfo));
    MOCK_METHOD1(StartSync, int32_t(const std::string &bundleName));
    MOCK_METHOD1(StartFileSync, int32_t(const std::string &bundleName));
    MOCK_METHOD2(StartSync, int32_t(bool forceFlag, const std::shared_ptr<CloudSyncCallback> callback));
    MOCK_METHOD2(TriggerSync, int32_t(const std::string &bundleName, const int32_t &userId));
    MOCK_METHOD2(StopSync, int32_t(const std::string &bundleName, bool forceFlag));
    MOCK_METHOD2(StopFileSync, int32_t(const std::string &bundleName, bool forceFlag));
    MOCK_METHOD1(ResetCursor, int32_t(const std::string &bundleName));
    MOCK_METHOD2(ResetCursor, int32_t(bool flag, const std::string &bundleName));
    MOCK_METHOD3(ChangeAppSwitch, int32_t(const std::string &accoutId, const std::string &bundleName, bool status));
    MOCK_METHOD2(OptimizeStorage, int32_t(const OptimizeSpaceOptions &optimizeOptions,
        const std::shared_ptr<CloudOptimizeCallback> optimizeCallback));
    MOCK_METHOD0(StopOptimizeStorage, int32_t());
    MOCK_METHOD2(Clean, int32_t(const std::string &accountId, const CleanOptions &cleanOptions));
    MOCK_METHOD2(NotifyDataChange, int32_t(const std::string &accoutId, const std::string &bundleName));
    MOCK_METHOD3(NotifyEventChange, int32_t(int32_t userId, const std::string &eventId, const std::string &extraData));
    MOCK_METHOD2(EnableCloud, int32_t(const std::string &accoutId, const SwitchDataObj &switchData));
    MOCK_METHOD1(DisableCloud, int32_t(const std::string &accoutId));
    MOCK_METHOD3(StartDownloadFile, int32_t(const std::string &uri,
                              const std::shared_ptr<CloudDownloadCallback> downloadCallback,
                              int64_t &downloadId));
    MOCK_METHOD5(StartFileCache, int32_t(const std::vector<std::string> &uriVec,
                           int64_t &downloadId,
                           int32_t fieldkey,
                           const std::shared_ptr<CloudDownloadCallback> downloadCallback,
                           int32_t timeout));
    MOCK_METHOD2(StopDownloadFile, int32_t(int64_t downloadId, bool needClean));
    MOCK_METHOD3(StopFileCache, int32_t(int64_t downloadId, bool needClean, int32_t timeout));
    MOCK_METHOD0(DownloadThumb, int32_t());
    MOCK_METHOD2(GetSyncTime, int32_t(int64_t &syncTime, const std::string &bundleName));
    MOCK_METHOD1(CleanCache, int32_t(const std::string &uri));
    MOCK_METHOD1(CleanFileCache, int32_t(const std::string &uri));
    MOCK_METHOD0(CleanGalleryDentryFile, void());
    MOCK_METHOD1(CleanGalleryDentryFile, void(const std::string path));
    MOCK_METHOD2(BatchCleanFile, int32_t(const std::vector<CleanFileInfo> &fileInfo,
        std::vector<std::string> &failCloudId));
    MOCK_METHOD2(BatchDentryFileInsert, int32_t(const std::vector<DentryFileInfo> &fileInfo,
        std::vector<std::string> &failCloudId));
    MOCK_METHOD2(StartDowngrade, int32_t(const std::string &bundleName,
                           const std::shared_ptr<DowngradeDlCallback> downloadCallback));
    MOCK_METHOD1(StopDowngrade, int32_t(const std::string &bundleName));
    MOCK_METHOD2(GetCloudFileInfo, int32_t(const std::string &bundleName, CloudFileInfo &cloudFileInfo));
    // file version
    MOCK_METHOD3(GetHistoryVersionList, int32_t(const std::string &uri, const int32_t versionNumLimit,
                                  std::vector<CloudSync::HistoryVersion> &historyVersionList));
    MOCK_METHOD5(DownloadHistoryVersion, int32_t(const std::string &uri, int64_t &downloadId, const uint64_t versionId,
                                   const std::shared_ptr<CloudDownloadCallback> downloadCallback,
                                   std::string &versionUri));
    MOCK_METHOD2(ReplaceFileWithHistoryVersion, int32_t(const std::string &uri, const std::string &versionUri));
    MOCK_METHOD2(IsFileConflict, int32_t(const std::string &uri, bool &isConflict));
    MOCK_METHOD1(ClearFileConflict, int32_t(const std::string &uri));
    MOCK_METHOD2(RemovedClean, int32_t(const std::string &bundleName, int32_t userId));
    MOCK_METHOD2(GetBundlesLocalFilePresentStatus, int32_t(const std::vector<std::string> &bundleNames,
            std::vector<LocalFilePresentStatus> &localFilePresentStatusList));
    MOCK_METHOD1(IsFinishPull, int32_t(bool &finishFlag));
    MOCK_METHOD1(GetDentryFileOccupy, int32_t(int64_t &occupyNum));
};
}
#endif // OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_MOCK