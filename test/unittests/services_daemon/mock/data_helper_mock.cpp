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
#include "data_helper_mock.h"

namespace OHOS::DataShare {
using namespace OHOS::FileManagement::CloudFile;
std::pair<int, std::shared_ptr<DataShareHelper>> DataShareHelper::Create(const sptr<IRemoteObject> &token,
                                                                         const std::string &strUri,
                                                                         const std::string &extUri,
                                                                         const int waitTime)
{
    return IDataHelper::ins->Create();
}
} // namespace OHOS::DataShare

namespace OHOS::FileManagement::CloudSync {
using namespace OHOS::FileManagement::CloudFile;

SwitchStatus SettingsDataManager::GetSwitchStatus()
{
    return IDataHelper::ins->GetSwitchStatus();
}

int32_t SettingsDataManager::QuerySwitchStatus()
{
    return IDataHelper::ins->QuerySwitchStatus();
}

void SettingsDataManager::RegisterObserver(const std::string &key, sptr<AAFwk::DataAbilityObserverStub> dataObserver)
{
    return;
}
} // namespace OHOS::FileManagement::CloudSync

namespace OHOS {
using namespace OHOS::FileManagement::CloudFile;
ErrCode AccountSA::OsAccountManager::GetForegroundOsAccountLocalId(int32_t &localId)
{
    return IOsAccountManager::ins->GetForegroundOsAccountLocalId(localId);
}
} // namespace OHOS
