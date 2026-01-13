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

#include "fuse_manager.h"
#include "setting_data_helper.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
FuseManager &FuseManager::GetInstance()
{
    static FuseManager instance_;
    return instance_;
}

int32_t FuseManager::StartFuse(int32_t userId, int32_t devFd, const string &path)
{
    return 0;
}

struct fuse_session* FuseManager::GetSession(std::string path)
{
    return nullptr;
}

void SettingDataHelper::SetActiveBundle(int32_t userId, string bundle)
{
}
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS