/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_CLOUD_DOWNLOAD_URI_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_DOWNLOAD_URI_MANAGER_H

#include "nocopyable.h"

#include <map>
#include <mutex>

namespace OHOS::FileManagement::CloudSync {
class CloudDownloadUriManager : public NoCopyable {
public:
    static CloudDownloadUriManager &GetInstance();

    int32_t AddPathToUri(const std::string& path, const std::string& uri);
    std::string GetUri(const std::string& path);
    void RemoveUri(const std::string& path);

private:
    CloudDownloadUriManager() = default;
    std::mutex mutex_;
    std::map<std::string, std::string> pathMap_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_DOWNLOAD_URI_MANAGER_H
