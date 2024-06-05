/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "plugin_loader.h"

#include <dlfcn.h>
#include <unistd.h>
#include <vector>

#include "utils_log.h"

namespace OHOS::FileManagement::CloudFile {
using namespace std;

PluginLoader &PluginLoader::GetInstance()
{
    static PluginLoader instance;
    return instance;
}

PluginLoader::~PluginLoader()
{
    if (cloudKitPulginHandle_ == nullptr) {
        return;
    }
    dlclose(cloudKitPulginHandle_);
    LOGI("succ to unload plugin");
    cloudKitPulginHandle_ = nullptr;
}

string GetPluginPath(const string &pluginFileName)
{
    const string searchDirs[] = {"/system/lib64/", "system/lib/"};
    string pluginFilePath = "";
    for (auto &searchDir : searchDirs) {
        std::string tmpPath = searchDir + pluginFileName;
        if (access(tmpPath.c_str(), F_OK) == 0) {
            pluginFilePath = tmpPath;
            break;
        }
    }
    return pluginFilePath;
}

void PluginLoader::LoadCloudKitPlugin(bool isSupportCloudSync)
{
    std::vector<std::string> pluginFileNames;
    pluginFileNames.emplace_back(isSupportCloudSync ? "libcloudfile_ext.z.so" : "libcloudfile_ext_core.z.so");
    pluginFileNames.emplace_back("libcloud_adapter.z.so");
    for (auto &pluginFileName : pluginFileNames) {
        auto pluginFilePath = GetPluginPath(pluginFileName);
        if (!pluginFilePath.empty()) {
            char resolvedPath[PATH_MAX] = {'\0'};
            if (realpath(pluginFilePath.c_str(), resolvedPath) == nullptr) {
                LOGE("realpath failed in line path: %s", pluginFilePath.c_str());
                reutrn;
            }
            cloudKitPulginHandle_ = dlopen(pluginFilePath.c_str(), RTLD_LAZY);
            if (cloudKitPulginHandle_ == nullptr) {
                LOGE("dlopen failed, path:%{public}s", pluginFilePath.c_str());
            } else {
                LOGI("succ to load plugin, path:%{public}s", pluginFilePath.c_str());
            }
            return;
        }
    }
    LOGE("Load CloudKit Plugin failed");
}
} // namespace OHOS::FileManagement::CloudFile