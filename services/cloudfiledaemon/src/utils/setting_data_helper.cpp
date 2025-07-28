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

#include "data_sync_const.h"
#include "datashare_helper.h"
#include "settings_data_manager.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;
using namespace OHOS::FileManagement::CloudSync;
namespace {
static const string SETTING_DATA_QUERY_URI = "datashareproxy://";
static const string SYNC_SWITCH_KEY = "photos_sync_options";
} // namespace

SettingDataHelper &SettingDataHelper::GetInstance()
{
    static SettingDataHelper instance_;
    return instance_;
}

void SettingDataHelper::SetUserData(void *data)
{
    lock_guard<mutex> lck(dataMtx_);
    data_ = data;
}

bool SettingDataHelper::IsDataShareReady()
{
    if (isDataShareReady_) {
        return true;
    }
    // try get DataShareHelper
    DataShare::CreateOptions options;
    options.enabled_ = true;
    auto dataShareHelper = DataShare::DataShareHelper::Creator(SETTING_DATA_QUERY_URI, options);
    if (dataShareHelper == nullptr) {
        LOGE("get data share helper fail, will retry again after recv ready event");
        return false;
    }
    bool ret = dataShareHelper->Release();
    LOGI("release data share helper, ret=%{public}d", ret);
    isDataShareReady_ = true;
    return true;
}

string SettingDataHelper::GetActiveBundle()
{
    // get from cache, or get from datashare if not exits
    SwitchStatus status = SettingsDataManager::GetSwitchStatus();
    if (status == AI_FAMILY) {
        return HDC_BUNDLE_NAME;
    }
    if (status == CLOUD_SPACE) {
        return GALLERY_BUNDLE_NAME;
    }
    return "";
}

bool SettingDataHelper::InitActiveBundle()
{
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

void SettingDataHelper::UpdateActiveBundle()
{
    // update cache data
    CloudSync::SettingsDataManager::QuerySwitchStatus();
    // get latest data
    string bundle = GetActiveBundle();
    // update FuseData
    SetActiveBundle(bundle);
}

void SettingDataHelper::RegisterObserver()
{
    sptr<SyncSwitchObserver> observer(new (std::nothrow) SyncSwitchObserver());
    SettingsDataManager::RegisterObserver(SYNC_SWITCH_KEY, observer);
}

void SyncSwitchObserver::OnChange()
{
    SettingDataHelper::GetInstance().UpdateActiveBundle();
}
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS