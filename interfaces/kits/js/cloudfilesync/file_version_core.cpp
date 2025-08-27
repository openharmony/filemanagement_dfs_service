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

#include "file_version_core.h"

#include <sys/types.h>
#include <sys/xattr.h>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "uri.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

FsResult<FileVersionCore *> FileVersionCore::Constructor()
{
    auto fileVersionPtr = new FileVersionCore();
    if (fileVersionPtr == nullptr) {
        LOGE("Failed to create CloudSyncCore object on heap.");
        return FsResult<FileVersionCore *>::Error(ENOMEM);
    }

    return FsResult<FileVersionCore *>::Success(move(fileVersionPtr));
}

FsResult<vector<HistoryVersion>> FileVersionCore::GetHistoryVersionList(const string &uri, int32_t numLimit)
{
    vector<HistoryVersion> versionList;
    int32_t ret = CloudSyncManager::GetInstance().GetHistoryVersionList(uri, numLimit, versionList);
    if (ret != E_OK) {
        LOGE("get history version list faild, ret = %{public}d", ret);
        return FsResult<vector<HistoryVersion>>::Error(Convert2ErrNum(ret));
    }

    return FsResult<vector<HistoryVersion>>::Success(versionList);
}

FsResult<string> FileVersionCore::DownloadHistoryVersion(const string &uri, uint64_t versionId,
    shared_ptr<CloudDownloadCallback> callback)
{
    int64_t downloadId = 0;
    string versionUri;
    int32_t ret = CloudSyncManager::GetInstance()
            .DownloadHistoryVersion(uri, downloadId, versionId, callback, versionUri);
    if (ret != E_OK) {
        LOGE("Start download history version failed! ret = %{public}d", ret);
        return FsResult<string>::Error(Convert2ErrNum(ret));
    }

    return FsResult<string>::Success(versionUri);
}

FsResult<void> FileVersionCore::ReplaceFileWithHistoryVersion(const string &oriUri, const string &verUri)
{
    int32_t ret = CloudSyncManager::GetInstance().ReplaceFileWithHistoryVersion(oriUri, verUri);
    if (ret != E_OK) {
        LOGE("replace history version failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}

FsResult<bool> FileVersionCore::IsConflict(const string &uri)
{
    bool isConflict = false;
    int32_t ret = CloudSyncManager::GetInstance().IsFileConflict(uri, isConflict);
    if (ret != E_OK) {
        LOGE("is conflict check failed! ret = %{public}d", ret);
        return FsResult<bool>::Error(Convert2ErrNum(ret));
    }

    return FsResult<bool>::Success(isConflict);
}

FsResult<void> FileVersionCore::ClearFileConflict(const string &uri)
{
    int32_t ret = CloudSyncManager::GetInstance().ClearFileConflict(uri);
    if (ret != E_OK) {
        LOGE("clear conflict failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    return FsResult<void>::Success();
}
} // namespace OHOS::FileManagement::CloudSync