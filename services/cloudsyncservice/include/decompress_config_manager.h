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

#ifndef OHOS_FILEMGMT_DECOMPRESS_CONFIG_MANAGER_H
#define OHOS_FILEMGMT_DECOMPRESS_CONFIG_MANAGER_H

#include <string>
#include <vector>

namespace OHOS::FileManagement::CloudSync {

class DecompressConfigManager {
public:
    static DecompressConfigManager& GetInstance();
    
    std::vector<std::string> GetUnsupportedList();
    bool GetSystemFeature();
    
    void SetConfigDir(const std::string& configDir);

private:
    DecompressConfigManager() = default;
    ~DecompressConfigManager() = default;
    
    static constexpr const char* DEFAULT_CONFIG_DIR = "/system/etc/decompress_config";
    static constexpr const char* UNSUPPORTED_LIST_FILE = "unsupported_list.json";
    static constexpr const char* SYSTEM_FEATURE_FILE = "system_feature.json";
    
    std::string configDir_ = DEFAULT_CONFIG_DIR;
};

} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_DECOMPRESS_CONFIG_MANAGER_H