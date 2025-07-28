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

#ifndef APPSTATE_OBSERVER_H
#define APPSTATE_OBSERVER_H

#include "app_mgr_client.h"
#include "io_message_listener.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {
class AppStateObserverManager {
    public:
        AppStateObserverManager() = default;
        ~AppStateObserverManager() = default;
        static AppStateObserverManager &GetInstance();
        void SubscribeAppState(const std::vector<std::string> &bundleNameList);
        void UnSubscribeAppState();

    protected:
        sptr<OHOS::AppExecFwk::ApplicationStateObserverStub> appStateObserver_ = nullptr;
};

class AppStateObserver : public OHOS::AppExecFwk::ApplicationStateObserverStub {
    public:
        AppStateObserver() {};
        ~AppStateObserver() override = default;

        void OnForegroundApplicationChanged(const AppExecFwk::AppStateData &appStateData) override;
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // APPSTATE_OBSERVER_H