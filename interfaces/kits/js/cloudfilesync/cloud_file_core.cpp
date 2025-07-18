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

#include "cloud_file_core.h"

#include <memory>
#include <sys/types.h>

#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace std;

FsResult<CloudFileCore *> CloudFileCore::Constructor()
{
    std::unique_ptr<CloudFileCore> cloudfile = std::make_unique<CloudFileCore>();
    return FsResult<CloudFileCore *>::Success(cloudfile.release());
}

std::shared_ptr<CloudDownloadCallbackImplAni> CloudFileCore::GetCallbackImpl(bool isInit)
{
    if (callback_ == nullptr && isInit) {
        callback_ = std::make_shared<CloudDownloadCallbackImplAni>();
    }
    return callback_;
}

FsResult<void> CloudFileCore::DoStart(const string &uri)
{
    LOGI("Start begin");
    auto callbackImpl = GetCallbackImpl(true);
    int32_t ret = callbackImpl->StartDownloadInner(uri);
    if (ret != E_OK) {
        LOGE("Start Download failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    LOGI("Start Download Success!");
    return FsResult<void>::Success();
}

FsResult<void> CloudFileCore::DoOn(const string &event, const shared_ptr<CloudDownloadCallbackImplAni> callback)
{
    return FsResult<void>::Success();
}

FsResult<void> CloudFileCore::DoOff(const string &event,
                                    const optional<shared_ptr<CloudDownloadCallbackImplAni>> &callback)
{
    return FsResult<void>::Success();
}

FsResult<void> CloudFileCore::DoStop(const string &uri, bool needClean)
{
    LOGI("Stop begin");
    auto callbackImpl = GetCallbackImpl(false);
    if (callbackImpl == nullptr) {
        LOGE("Failed to stop download, callback is null!");
        return FsResult<void>::Error(E_INVAL_ARG);
    }
    int32_t ret = callbackImpl->StopDownloadInner(uri);
    if (ret != E_OK) {
        LOGE("Stop Download failed! ret = %{public}d", ret);
        return FsResult<void>::Error(Convert2ErrNum(ret));
    }

    LOGI("Stop Download Success!");
    return FsResult<void>::Success();
}
} // namespace OHOS::FileManagement::CloudSync