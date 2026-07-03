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

#include "decompress_config_manager.h"

#include "nlohmann/json.hpp"
#include "utils_log.h"

#include <fstream>

namespace OHOS::FileManagement::CloudSync {

namespace {
    constexpr size_t MAX_UNSUPPORTED_LIST_SIZE = 1000;
}

DecompressConfigManager& DecompressConfigManager::GetInstance()
{
    static DecompressConfigManager instance;
    return instance;
}

void DecompressConfigManager::SetConfigDir(const std::string& configDir)
{
    configDir_ = configDir;
}

std::vector<std::string> DecompressConfigManager::GetUnsupportedList()
{
    std::vector<std::string> result;
    std::string filePath = configDir_ + "/" + UNSUPPORTED_LIST_FILE;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOGW("Unsupported list file not found: %{public}s", filePath.c_str());
        return result;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    if (!nlohmann::json::accept(content)) {
        LOGE("Invalid JSON format in unsupported list file");
        return result;
    }

    nlohmann::json jsonObj = nlohmann::json::parse(content, nullptr, false);
    if (jsonObj.is_discarded()) {
        LOGE("Failed to parse unsupported list JSON");
        return result;
    }
    
    if (!jsonObj.contains("unsupportedList") || !jsonObj["unsupportedList"].is_array()) {
        LOGE("Invalid unsupported list format");
        return result;
    }
    
    for (const auto &item : jsonObj["unsupportedList"]) {
        if (result.size() >= MAX_UNSUPPORTED_LIST_SIZE) {
            LOGW("Unsupported list exceed max size %{public}zu, truncated",
                 MAX_UNSUPPORTED_LIST_SIZE);
            break;
        }
        if (item.is_string()) {
            result.push_back(item.get<std::string>());
        }
    }
    
    LOGD("Loaded unsupported list: %{public}zu items", result.size());
    return result;
}

bool DecompressConfigManager::GetSystemFeature()
{
    std::string filePath = configDir_ + "/" + SYSTEM_FEATURE_FILE;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOGW("System feature file not found: %{public}s", filePath.c_str());
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    if (!nlohmann::json::accept(content)) {
        LOGE("Invalid JSON format in system feature file");
        return false;
    }

    nlohmann::json jsonObj = nlohmann::json::parse(content, nullptr, false);
    if (jsonObj.is_discarded()) {
        LOGE("Failed to parse system feature JSON");
        return false;
    }
    
    if (!jsonObj.contains("systemFeature") || !jsonObj["systemFeature"].is_boolean()) {
        LOGE("Invalid system feature format");
        return false;
    }
    
    bool systemFeature = jsonObj["systemFeature"].get<bool>();
    LOGD("Loaded system feature: %{public}s", systemFeature ? "true" : "false");
    return systemFeature;
}

} // namespace OHOS::FileManagement::CloudSync