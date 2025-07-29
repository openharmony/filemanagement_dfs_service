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
std::shared_ptr<DataShareHelper> DataShareHelper::Creator(const std::string &strUri,
                                                          const CreateOptions &options,
                                                          const std::string &bundleName,
                                                          const int waitTime,
                                                          bool isSystem)
{
    return IDataHelper::ins->Creator();
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