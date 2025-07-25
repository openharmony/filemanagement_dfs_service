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

#ifndef OHOS_FILEMGMT_CLOUD_FILE_CORE_H
#define OHOS_FILEMGMT_CLOUD_FILE_CORE_H

#include <optional>

#include "download_callback_impl_ani.h"
#include "filemgmt_libfs.h"

namespace OHOS::FileManagement::CloudSync {
using namespace ModuleFileIO;

class CloudFileCore {
public:
    CloudFileCore() = default;
    ~CloudFileCore() = default;

    static FsResult<CloudFileCore *> Constructor();
    FsResult<void> DoStart(const std::string &uri);
    FsResult<void> DoOn(const std::string &event, const std::shared_ptr<CloudDownloadCallbackImplAni> callback);
    FsResult<void> DoOff(const std::string &event,
                         const std::optional<std::shared_ptr<CloudDownloadCallbackImplAni>> &callback = std::nullopt);
    FsResult<void> DoStop(const std::string &uri, bool needClean = false);
    std::shared_ptr<CloudDownloadCallbackImplAni> GetCallbackImpl(bool isInit);

private:
    std::shared_ptr<CloudDownloadCallbackImplAni> callback_;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_FILE_CORE_H