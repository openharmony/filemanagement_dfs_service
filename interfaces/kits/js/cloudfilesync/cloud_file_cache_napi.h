/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_CLOUD_FILE_CACHE_NAPI_H
#define OHOS_FILEMGMT_CLOUD_FILE_CACHE_NAPI_H

#include <mutex>
#include <unordered_set>
#include "cloud_file_napi.h"

namespace OHOS::FileManagement::CloudSync {
const std::string PROGRESS = "progress";
const std::string MULTI_PROGRESS = "multiProgress";
class CloudFileCacheNapi final : public LibN::NExporter {
public:
    CloudFileCacheNapi(napi_env env, napi_value exports) : NExporter(env, exports) {}
    ~CloudFileCacheNapi() = default;

    bool Export() override;
    bool ToExport(std::vector<napi_property_descriptor> props);
    std::string GetClassName() override;
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static napi_value StartFileCache(napi_env env, napi_callback_info info);
    static napi_value StartBatchFileCache(napi_env env, napi_callback_info info);
    static napi_value StopFileCache(napi_env env, napi_callback_info info);
    static napi_value StopBatchFileCache(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value CleanCloudFileCache(napi_env env, napi_callback_info info);

private:
    inline static std::string className_ = "CloudFileCache";
};
struct RegisterInfoArg {
    std::string eventType;
    std::shared_ptr<CloudDownloadCallbackImpl> callback;
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
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_FILE_DOWNLOAD_NAPI_H