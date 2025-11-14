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
#include "setting_data_helper.h"

#include "cloud_daemon_stub.h"
#include "data_sync_const.h"
#include "datashare_helper.h"
#include "os_account_manager.h"
#include "settings_data_manager.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;
using namespace OHOS::FileManagement::CloudSync;
namespace {
static const string SETTINGS_DATA_COMMON_URI =
    "datashare:///com.ohos.settingsdata/entry/settingsdata/SETTINGSDATA?Proxy=true";
static const string SETTINGS_DATA_EXT_URI = "datashare:///com.ohos.settingsdata.DataAbility";
static const string SYNC_SWITCH_KEY = "photos_sync_options";
} // namespace

SettingDataHelper &SettingDataHelper::GetInstance()
{
    static SettingDataHelper instance_;
    return instance_;
}

void SettingDataHelper::SetUserData(int32_t userId, void *data)
{
    lock_guard<mutex> lck(dataMtx_);
    if (data != nullptr) {
        dataMap_[userId] = data;
        return;
    }
    if (dataMap_.find(userId) != dataMap_.end()) {
        dataMap_.erase(userId);
    }
}

bool SettingDataHelper::IsDataShareReady()
{
    if (isDataShareReady_) {
        return true;
    }
    // try get DataShareHelper
    auto remote = sptr<ICloudDaemonBroker>(new (std::nothrow) IRemoteStub<ICloudDaemonBroker>());
    if (remote == nullptr) {
        LOGE("remote is nullptr");
        return false;
    }
    auto remoteObj = remote->AsObject();
    if (remoteObj == nullptr) {
        LOGE("remoteObj is nullptr");
        return false;
    }

    pair<int, shared_ptr<DataShare::DataShareHelper>> ret =
        DataShare::DataShareHelper::Create(remoteObj, SETTINGS_DATA_COMMON_URI, SETTINGS_DATA_EXT_URI);
    LOGI("create datashare helper, ret=%{public}d", ret.first);
    if (ret.first == DataShare::E_DATA_SHARE_NOT_READY) {
        LOGE("create datashare helper faild");
        isDataShareReady_ = false;
        return false;
    }
    if (ret.first == DataShare::E_OK) {
        auto helper = ret.second;
        if (helper != nullptr) {
            bool releaseRet = helper->Release();
            LOGI("release datashare helper, ret=%{public}d", releaseRet);
        }
        isDataShareReady_ = true;
        return true;
    }
    return true;
}

string SettingDataHelper::GetActiveBundle()
{
    // get from datashare
    SwitchStatus status = SettingsDataManager::GetSwitchStatus();
    if (status == AI_FAMILY) {
        return HDC_BUNDLE_NAME;
    }
    return GALLERY_BUNDLE_NAME;
}

bool SettingDataHelper::InitActiveBundle()
{
    lock_guard<mutex> lck(initMtx_);
    if (isBundleInited_) {
        return true;
    }
    if (!IsDataShareReady()) {
        return false;
    }
    UpdateActiveBundle();
    RegisterObserver();
    isBundleInited_ = true;
    return true;
}

void SettingDataHelper::UpdateActiveBundle(int32_t userId)
{
    // get latest data
    string bundle = GetActiveBundle();
    // update FuseData
    SetActiveBundle(userId, bundle);
}

void SettingDataHelper::RegisterObserver()
{
    sptr<SyncSwitchObserver> observer(new (std::nothrow) SyncSwitchObserver());
    SettingsDataManager::RegisterObserver(SYNC_SWITCH_KEY, observer);
}

bool SettingDataHelper::GetForegroundUser(int32_t &userId)
{
    int32_t foregroundLocalId = -1;
    int32_t ret = AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(foregroundLocalId);
    if (ret != 0) {
        LOGE("Fail to get foregrount os account local id, errorCode = %{public}d", ret);
        return false;
    }
    userId = foregroundLocalId;
    return true;
}

void SyncSwitchObserver::OnChange()
{
    SettingDataHelper::GetInstance().UpdateActiveBundle();
}
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS