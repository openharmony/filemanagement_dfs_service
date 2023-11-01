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

#include "data_sync_notifier.h"

#include <mutex>
#include "dataobs_mgr_client.h"
#include "gallery_file_const.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "uri.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;
using ChangeType = AAFwk::ChangeInfo::ChangeType;
using NotifyDataMap = unordered_map<ChangeType, list<Uri>>;
unordered_map<ChangeType, list<Uri>> DataSyncNotifier::notifyListMap_ = {};
mutex DataSyncNotifier::mtx_{};
constexpr int NOTIFY_INTERVALS = 10;

DataSyncNotifier &DataSyncNotifier::GetInstance()
{
    static DataSyncNotifier instance;
    return instance;
}

static int32_t TryNotifyChange()
{
    static int32_t recordAdded = 0;
    recordAdded++;
    if (recordAdded % NOTIFY_INTERVALS == 0) {
        auto obsMgrClient = AAFwk::DataObsMgrClient::GetInstance();
        if (obsMgrClient == nullptr) {
            LOGE("%{public}s obsMgrClient is nullptr", __func__);
            return E_SA_LOAD_FAILED;
        }
        for (auto it = DataSyncNotifier::notifyListMap_.begin();
                  it != DataSyncNotifier::notifyListMap_.end(); ++it) {
            obsMgrClient->NotifyChangeExt({it->first, it->second});
        }
        DataSyncNotifier::notifyListMap_.clear();
        recordAdded = 0;
    }
    return E_OK;
}

static int32_t NotifyFileAssetChange(const string &uri, const ChangeType changeType)
{
    std::lock_guard<mutex> lock(DataSyncNotifier::mtx_);
    auto iterator = DataSyncNotifier::notifyListMap_.find(changeType);
    if (iterator != DataSyncNotifier::notifyListMap_.end()) {
        iterator ->second.emplace_back(Uri(uri));
    } else {
        list<Uri> newList;
        newList.emplace_back(Uri(uri));
        DataSyncNotifier::notifyListMap_.insert(make_pair(changeType, newList));
    }
    return TryNotifyChange();
}

static int32_t NotifyAlbumChange(const string &uri, const ChangeType changeType)
{
    return NotifyFileAssetChange(uri, changeType);
}

static int32_t NotifyAlbumMapChange(const string &uri, const ChangeType changeType, const string &fileAssetId)
{
    return E_OK;
}

int32_t DataSyncNotifier::TryNotify(const string &uri, const ChangeType changeType, const string &fileAssetId)
{
    int ret = E_NOTIFY;
    if (uri.find(PHOTO_URI_PREFIX) != string::npos) {
        ret = NotifyFileAssetChange(uri, changeType);
    } else if (uri.find(ALBUM_URI_PREFIX) != string::npos && fileAssetId == INVALID_ASSET_ID) {
        ret = NotifyAlbumChange(uri, changeType);
    } else {
        ret = NotifyAlbumMapChange(uri, changeType, fileAssetId);
    }
    return ret;
}

int32_t DataSyncNotifier::FinalNotify()
{
    auto obsMgrClient = AAFwk::DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        LOGE("%{public}s obsMgrClient is nullptr", __func__);
        return E_SA_LOAD_FAILED;
    }
    std::lock_guard<mutex> lock(DataSyncNotifier::mtx_);
    for (auto it = DataSyncNotifier::notifyListMap_.begin();
              it != DataSyncNotifier::notifyListMap_.end(); ++it) {
        obsMgrClient->NotifyChangeExt({it->first, it->second});
    }
    DataSyncNotifier::notifyListMap_.clear();
    return E_OK;
}
} // namespace OHOS::FileManagement::CloudSync