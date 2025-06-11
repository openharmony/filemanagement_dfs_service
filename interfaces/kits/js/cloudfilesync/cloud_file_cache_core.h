/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_FILEMGMT_CLOUD_FILE_CACHE_CORE_H
#define OHOS_FILEMGMT_CLOUD_FILE_CACHE_CORE_H

#include <mutex>
#include <optional>
#include <unordered_set>

#include "cloud_download_callback_ani.h"
#include "filemgmt_libfs.h"

namespace OHOS::FileManagement::CloudSync {
using namespace ModuleFileIO;
using namespace std;
const std::string PROGRESS = "progress";
const std::string MULTI_PROGRESS = "multiProgress";

struct RegisterInfoArg {
    std::string eventType;
    std::shared_ptr<CloudDownloadCallbackMiddle> callback;
    ~RegisterInfoArg()
    {
        if (callback != nullptr) {
            callback->DeleteReference();
            callback = nullptr;
        }
    }
};

class RegisterManager {
public:
    RegisterManager() = default;
    ~RegisterManager() = default;
    bool AddRegisterInfo(std::shared_ptr<RegisterInfoArg> info);
    bool RemoveRegisterInfo(const std::string &eventType);
    bool HasEvent(const std::string &eventType);

private:
    std::mutex registerMutex_;
    std::unordered_set<std::shared_ptr<RegisterInfoArg>> registerInfo_;
};

struct FileCacheEntity {
    RegisterManager registerMgr;
};

class CloudFileCacheCore {
public:
    CloudFileCacheCore();
    ~CloudFileCacheCore() = default;

    static FsResult<CloudFileCacheCore *> Constructor();
    FsResult<void> DoOn(const std::string &event, const std::shared_ptr<CloudDownloadCallbackMiddle> callback);
    FsResult<void> DoOff(const string &event,
        const std::optional<std::shared_ptr<CloudDownloadCallbackMiddle>> &callback = std::nullopt);
    FsResult<void> DoStart(const string &uri);
    FsResult<void> DoStop(const string &uri, bool needClean = false);
    FsResult<void> CleanCache(const string &uri);

private:
    std::unique_ptr<FileCacheEntity> fileCacheEntity;
};

} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_FILE_CACHE_CORE_H