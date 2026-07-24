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

#include "decompress_update.h"

#include "decompress_kit.h"
#include "decompress_kit_constant.h"
#include "utils_log.h"

namespace OHOS {
namespace FileManagement {
namespace Decompress {

DecompressUpdateManager &DecompressUpdateManager::GetInstance()
{
    static DecompressUpdateManager instance;
    return instance;
}

void DecompressUpdateManager::HandleDecompressUpdate()
{
    LOGI("Update enter");
    auto kitInstance = DecompressKit::GetInstance();
    if (kitInstance == nullptr) {
        LOGE("Get decompress kitInstance instance failed");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(copyMutex_);
        std::string dataPath = kitInstance->GetCloudVersionFilePath();
        if (dataPath.empty()) {
            LOGE("Get cloud version file failed");
            return;
        }
    
        if (!kitInstance->IsUpdateVersionCompatible(dataPath)) {
            LOGE("UpdateVersion not compatible");
            return;
        }
        
        if (!kitInstance->IsNeedCopy(dataPath, LOCAL_CFG_DIR)) {
            LOGI("No need to copy param data.");
            return;
        }

        if (!kitInstance->DoParamDirCopy(dataPath, LOCAL_CFG_DIR)) {
            LOGE("Copy param dir failed.");
        }
    }
}

} // namespace Decompress
} // namespace FileManagement
} // namespace OHOS
