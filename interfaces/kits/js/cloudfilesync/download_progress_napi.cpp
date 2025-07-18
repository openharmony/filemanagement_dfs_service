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

#include "download_progress_napi.h"

#include <memory>

#include "dfs_error.h"
#include "multi_download_progress_napi.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
void SingleProgressNapi::Update(const DownloadProgressObj &progress)
{
    if (taskId_ != progress.downloadId) {
        return;
    }
    uri_ = progress.path;
    totalSize_ = progress.totalSize;
    downloadedSize_ = progress.downloadedSize;
    state_ = progress.state;
    errorType_ = progress.downloadErrorType;
    needClean_ = (progress.state != DownloadProgressObj::RUNNING);
}

napi_value SingleProgressNapi::ConvertToValue(napi_env env)
{
    NVal obj = NVal::CreateObject(env);
    obj.AddProp("state", NVal::CreateInt32(env, state_).val_);
    obj.AddProp("processed", NVal::CreateInt64(env, downloadedSize_).val_);
    obj.AddProp("size", NVal::CreateInt64(env, totalSize_).val_);
    obj.AddProp("uri", NVal::CreateUTF8String(env, uri_).val_);
    obj.AddProp("error", NVal::CreateInt32(env, errorType_).val_);
    return obj.val_;
}

void BatchProgressNapi::Update(const DownloadProgressObj &progress)
{
    if (taskId_ != progress.downloadId) {
        return;
    }
    state_ = static_cast<int32_t>(progress.batchState);
    downloadedSize_ = progress.batchDownloadSize;
    totalSize_ = progress.batchTotalSize;
    totalNum_ = progress.batchTotalNum;
    errorType_ = static_cast<int32_t>(progress.downloadErrorType);
    if (progress.state == DownloadProgressObj::COMPLETED) {
        downloadedFiles_.insert(progress.path);
    } else if (progress.state != DownloadProgressObj::RUNNING) {
        failedFiles_.insert(std::make_pair(progress.path, static_cast<int32_t>(progress.downloadErrorType)));
    }
    needClean_ = ((progress.batchTotalNum == progress.batchFailNum + progress.batchSuccNum) &&
                  progress.batchState != DownloadProgressObj::RUNNING);
}

napi_value BatchProgressNapi::ConvertToValue(napi_env env)
{
    napi_value progressVal = NClass::InstantiateClass(env, MultiDlProgressNapi::className_, {});
    if (progressVal == nullptr) {
        LOGE("Failed to instantiate class");
        return nullptr;
    }
    auto progressEntity = NClass::GetEntityOf<MultiDlProgressEntity>(env, progressVal);
    if (progressEntity == nullptr) {
        LOGE("Failed to get progressEntity.");
        return nullptr;
    }
    progressEntity->downloadProgress = std::make_unique<BatchProgressNapi>(*this);
    return progressVal;
}
} // namespace OHOS::FileManagement::CloudSync
