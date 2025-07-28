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
#include "appstate_observer.h"
#include "dfs_error.h"
#include "singleton.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
using namespace std;

AppStateObserverManager &AppStateObserverManager::GetInstance()
{
    static AppStateObserverManager instance;
    return instance;
}

void AppStateObserverManager::SubscribeAppState(const std::vector<std::string> &bundleNameList)
{
    auto appMgrClient = DelayedSingleton<OHOS::AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        LOGE("appMgrClient_ is nullptr");
        return;
    }
    if (appStateObserver_ != nullptr) {
        LOGE("appStateObserver has been registed");
        return;
    }
    appStateObserver_ = sptr<AppStateObserver>(new (std::nothrow) AppStateObserver());
    if (appStateObserver_ == nullptr) {
        LOGE("get appStateObserver failed");
        return;
    }

    int32_t result = appMgrClient->RegisterApplicationStateObserver(appStateObserver_, bundleNameList);
    if (result != E_OK) {
        LOGE("RegistApplicationStateObserver failed result = %{public}d", result);
        appStateObserver_ = nullptr;
    }
}

void AppStateObserverManager::UnSubscribeAppState()
{
    if (appStateObserver_ == nullptr) {
        LOGE("appStateObserver is nullptr");
        return;
    }

    auto appMgrClient = DelayedSingleton<OHOS::AppExecFwk::AppMgrClient>::GetInstance();
    if (appMgrClient == nullptr) {
        LOGE("appMgrClient_ is nullptr");
        return;
    }
    int32_t result = appMgrClient->UnregisterApplicationStateObserver(appStateObserver_);
    if (result != E_OK) {
        LOGE("UnregisterApplicationStateObserver failed result = %{public}d", result);
    }
    appStateObserver_ = nullptr;
}

void AppStateObserver::OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData)
{
    IoMessageManager::GetInstance().OnReceiveEvent(appStateData);
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS