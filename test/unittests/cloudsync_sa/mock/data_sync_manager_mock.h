/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef MOCK_DATA_SYNC_MANAGER_H
#define MOCK_DATA_SYNC_MANAGER_H

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "data_sync_manager.h"

namespace OHOS::FileManagement::CloudFile {
class MockDataSyncManager : public DataSyncManager {
public:
    MOCK_METHOD(int32_t, TriggerStartSync,
                (const std::string &bundleName, const int32_t userId, bool forceFlag,
                 SyncTriggerType triggerType, std::string prepareTraceId),
                (override));
    MOCK_METHOD(int32_t, TriggerStopSync,
                (const std::string &bundleName, const int32_t userId, bool forceFlag, SyncTriggerType triggerType),
                (override));
    MOCK_METHOD(int32_t, StopSyncSynced,
                (const std::string &bundleName, const int32_t userId, bool forceFlag, SyncTriggerType triggerType),
                (override));
    MOCK_METHOD(int32_t, TriggerRecoverySync, (SyncTriggerType triggerType), (override));
    MOCK_METHOD(int32_t, StopUploadTask, (const std::string &bundleName, const int32_t userId), (override));
    MOCK_METHOD(void, StopDownloadAndUploadTask, (), (override));
    MOCK_METHOD(int32_t, ResetCursor, (const std::string &bundleName, const int32_t &userId, bool flag), (override));
    MOCK_METHOD(void, RegisterCloudSyncCallback,
                (const std::string &bundleName, const BundleNameUserInfo &bundleNameUserInfo,
                 const std::string &callbackId, const sptr<CloudSync::ICloudSyncCallback> &callback),
                (override));
    MOCK_METHOD(void, UnRegisterCloudSyncCallback,
                (const std::string &bundleName, const BundleNameUserInfo &bundleNameUserInfo,
                 const std::string &callbackId),
                (override));
    MOCK_METHOD(int32_t, IsSkipSync,
                (const std::string &bundleName, const int32_t userId, bool forceFlag), (override));
    MOCK_METHOD(int32_t, StartFileCache,
                (const BundleNameUserInfo &bundleNameUserInfo, const std::vector<std::string> &uriVec,
                 int64_t &downloadId, int32_t fieldkey,
                 const sptr<CloudSync::ICloudDownloadCallback> &downloadCallback, int32_t timeout),
                (override));
    MOCK_METHOD(int32_t, StartDownloadFile,
                (const BundleNameUserInfo &bundleNameUserInfo, const std::string &uri, int64_t &downloadId,
                 const sptr<CloudSync::ICloudDownloadCallback> &downloadCallback),
                (override));
    MOCK_METHOD(int32_t, StopFileCache,
                (const BundleNameUserInfo &bundleNameUserInfo, int64_t downloadId, bool needClean, int32_t timeout),
                (override));
    MOCK_METHOD(int32_t, StopDownloadFile,
                (const BundleNameUserInfo &bundleNameUserInfo, int64_t downloadId, bool needClean), (override));
    MOCK_METHOD(int32_t, CleanCloudFile,
                (const int32_t userId, const std::string &bundleName, const int action), (override));
    MOCK_METHOD(int32_t, OptimizeStorage,
                (const std::string &bundleName, const int32_t userId, const int32_t agingDays), (override));
    MOCK_METHOD(int32_t, StartOptimizeStorage,
                (const BundleNameUserInfo &bundleNameUserInfo, const CloudSync::OptimizeSpaceOptions &optimizeOptions,
                 const sptr<CloudSync::ICloudOptimizeCallback> &optimizeCallback),
                (override));
    MOCK_METHOD(int32_t, StopOptimizeStorage, (const BundleNameUserInfo &bundleNameUserInfo), (override));
    MOCK_METHOD(int32_t, OptimizeCache, (const int32_t userId, const std::string &bundleName), (override));
    MOCK_METHOD(int32_t, BatchCleanFile,
                (const std::vector<CloudSync::CleanFileInfo> &fileInfo, std::vector<std::string> &failCloudId),
                (override));
    MOCK_METHOD(int32_t, DownloadThumb, (), (override));
    MOCK_METHOD(int32_t, TriggerDownloadThumb, (), (override));
    MOCK_METHOD(int32_t, CacheVideo, (), (override));
    MOCK_METHOD(int32_t, CleanVideoCache, (), (override));
    MOCK_METHOD(int32_t, CleanCache,
                (const std::string &bundleName, const int32_t userId, const std::string &uri), (override));
    MOCK_METHOD(int32_t, CleanCache, (const std::string &bundleName, const int32_t userId), (override));
    MOCK_METHOD(int32_t, UpdateCachedFileSize,
                (const std::string &bundleName, const int32_t userId), (override));
    MOCK_METHOD(int32_t, GetCachedTotalSize,
                (const std::string &bundleName, const int32_t userId, int64_t &totalSize), (override));
    MOCK_METHOD(int32_t, BatchDentryFileInsert,
                (const std::vector<CloudSync::DentryFileInfo> &fileInfo, std::vector<std::string> &failCloudId),
                (override));
    MOCK_METHOD(int32_t, DisableCloud, (const int32_t userId), (override));
    MOCK_METHOD(int32_t, GetUserId, (int32_t &userId), (override));
    MOCK_METHOD(int32_t, SaveSubscription, (const std::string &bundleName, const int32_t userId), (override));
    MOCK_METHOD(int32_t, ReportEntry, (const std::string &bundleName, const int32_t userId), (override));
    MOCK_METHOD(int32_t, ChangeAppSwitch,
                (const std::string &bundleName, const int32_t userId, bool status), (override));
    MOCK_METHOD(int32_t, StartDowngrade,
                (const std::string &bundleName, const sptr<CloudSync::IDowngradeDlCallback> &downloadCallback),
                (override));
    MOCK_METHOD(int32_t, StopDowngrade, (const std::string &bundleName), (override));
    MOCK_METHOD(int32_t, StartTransfer,
                (const std::string &bundleName, const std::string &targetUri,
                 const sptr<CloudSync::IDowngradeDlCallback> &downloadCallback),
                (override));
    MOCK_METHOD(int32_t, GetCloudFileInfo,
                (const std::string &bundleName, CloudSync::CloudFileInfo &cloudFileInfo), (override));
    MOCK_METHOD(int32_t, GetHistoryVersionList,
                (const BundleNameUserInfo &bundleNameUserInfo, const std::string &uri, const int32_t versionNumLimit,
                 std::vector<CloudSync::HistoryVersion> &historyVersionList),
                (override));
    MOCK_METHOD(int32_t, DownloadHistoryVersion,
                (const BundleNameUserInfo &bundleNameUserInfo, const std::string &uri, int64_t &downloadId,
                 const uint64_t versionId, const sptr<CloudSync::ICloudDownloadCallback> &downloadCallback,
                 std::string &versionUri),
                (override));
    MOCK_METHOD(int32_t, ReplaceFileWithHistoryVersion,
                (const BundleNameUserInfo &bundleNameUserInfo, const std::string &uri, const std::string &versionUri),
                (override));
    MOCK_METHOD(int32_t, IsFileConflict,
                (const BundleNameUserInfo &bundleNameUserInfo, const std::string &uri, bool &isConflict), (override));
    MOCK_METHOD(int32_t, ClearFileConflict,
                (const BundleNameUserInfo &bundleNameUserInfo, const std::string &uri), (override));
    MOCK_METHOD(int32_t, GetBundlesLocalFilePresentStatus,
                (const std::vector<std::string> &bundleNames, const int32_t &userId,
                 std::vector<CloudSync::LocalFilePresentStatus> &localFilePresentStatusList),
                (override));
    MOCK_METHOD(int32_t, IsFinishPull,
                (const int32_t userId, const std::string &bundleName, bool &finishFlag), (override));
    MOCK_METHOD(int32_t, GetDownloadList,
                (const BundleNameUserInfo &bundleNameUserInfo, const std::vector<std::string> &uriVec,
                 std::vector<CloudSync::DownloadProgressObj> &downloadList),
                (override));
    MOCK_METHOD(int32_t, GetUploadList,
                (const BundleNameUserInfo &bundleNameUserInfo, const std::vector<std::string> &uriVec,
                 std::vector<CloudSync::UploadProgressObj> &uploadList),
                (override));
    MOCK_METHOD(int32_t, RegisterUploadCallback,
                (const std::string &bundleName, const BundleNameUserInfo &bundleNameUserInfo,
                 const std::string &callbackId, const sptr<CloudSync::ICloudUploadCallback> &callback),
                (override));
    MOCK_METHOD(void, UnRegisterUploadCallback,
                (const std::string &bundleName, const BundleNameUserInfo &bundleNameUserInfo,
                 const std::string &callbackId),
                (override));
    MOCK_METHOD(void, PeriodicCleanLock, (), (override));
    MOCK_METHOD(void, PeriodicCleanUnlock, (), (override));
    MOCK_METHOD(int32_t, GetDowngradeDownloadTaskState,
                (const std::vector<std::string> &bundleNames, const int32_t &userId,
                 std::vector<CloudSync::DowngradeProgress> &downgradeProgressList),
                (override));
    MOCK_METHOD(int32_t, SetMediaPreShared,
                (const std::string &albumId, const std::string &albumName, const std::string &localPath), (override));
};
} // namespace OHOS::FileManagement::CloudFile

#endif // MOCK_DATA_SYNC_MANAGER_H