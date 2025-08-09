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

#include "download_progress_ani.h"

#include <memory>

#include "error_handler.h"
#include "multi_download_progress_ani.h"

namespace OHOS::FileManagement::CloudSync {
using namespace arkts::ani_signature;
void SingleProgressAni::Update(const DownloadProgressObj &progress)
{
    if (taskId_ != progress.downloadId) {
        return;
    }
    std::lock_guard<std::mutex> lock(mtx_);
    uri_ = progress.path;
    totalSize_ = progress.totalSize;
    downloadedSize_ = progress.downloadedSize;
    state_ = progress.state;
    errorType_ = progress.downloadErrorType;
    needClean_ = (progress.state != DownloadProgressObj::RUNNING);
}

ani_object SingleProgressAni::ConvertToObject(ani_env *env)
{
    ani_class cls;
    ani_status ret;
    std::string classDesc = Builder::BuildClass("@ohos.file.cloudSync.cloudSync.DownloadProgressInner").Descriptor();
    if ((ret = env->FindClass(classDesc.c_str(), &cls)) != ANI_OK) {
        LOGE("Cannot find class %{private}s, err: %{public}d", classDesc.c_str(), ret);
        return nullptr;
    }

    ani_method ctor;
    std::string ct = Builder::BuildConstructorName();
    std::string argSign = Builder::BuildSignatureDescriptor(
        {Builder::BuildDouble(), Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.State"), Builder::BuildDouble(),
         Builder::BuildDouble(), Builder::BuildClass("std.core.String"),
         Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.DownloadErrorType")});
    ret = env->Class_FindMethod(cls, ct.c_str(), argSign.c_str(), &ctor);
    if (ret != ANI_OK) {
        LOGE("find ctor method failed. ret = %{public}d", ret);
        return nullptr;
    }
    ani_string uri = nullptr;
    ret = env->String_NewUTF8(uri_.c_str(), uri_.size(), &uri);
    if (ret != ANI_OK) {
        LOGE("get uri failed. ret = %{public}d", ret);
        return nullptr;
    }

    ani_enum stateEnum;
    Type stateSign = Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.State");
    env->FindEnum(stateSign.Descriptor().c_str(), &stateEnum);
    ani_enum downloadErrorEnum;
    Type errorSign = Builder::BuildEnum("@ohos.file.cloudSync.cloudSync.DownloadErrorType");
    env->FindEnum(errorSign.Descriptor().c_str(), &downloadErrorEnum);

    ani_enum_item stateEnumItem;
    ani_enum_item downloadErrorEnumItem;
    env->Enum_GetEnumItemByIndex(downloadErrorEnum, errorType_, &downloadErrorEnumItem);
    env->Enum_GetEnumItemByIndex(stateEnum, state_, &stateEnumItem);
    ani_object pg;
    ret = env->Object_New(cls, ctor, &pg, static_cast<double>(taskId_), stateEnumItem,
                          static_cast<double>(downloadedSize_), static_cast<double>(totalSize_), uri,
                          downloadErrorEnumItem);
    if (ret != ANI_OK) {
        LOGE("create new object failed. ret = %{public}d", ret);
    }
    return pg;
}

std::shared_ptr<DlProgressAni> SingleProgressAni::CreateNewObject()
{
    return std::make_shared<SingleProgressAni>(taskId_);
}

void BatchProgressAni::Update(const DownloadProgressObj &progress)
{
    if (taskId_ != progress.downloadId) {
        return;
    }
    std::lock_guard<std::mutex> lock(mtx_);
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

// no need to lock here, because it is called only once when a new object is copied out.
void BatchProgressAni::SetDownloadedFiles(const std::unordered_set<std::string> &fileList)
{
    downloadedFiles_ = fileList;
}

void BatchProgressAni::SetFailedFiles(const std::unordered_map<std::string, int32_t> &fileList)
{
    failedFiles_ = fileList;
}

std::shared_ptr<DlProgressAni> BatchProgressAni::CreateNewObject()
{
    auto resProgress = std::make_shared<BatchProgressAni>(taskId_);
    std::lock_guard<std::mutex> lock(mtx_);
    resProgress->SetDownloadedFiles(downloadedFiles_);
    resProgress->SetFailedFiles(failedFiles_);
    return resProgress;
}

ani_object BatchProgressAni::ConvertToObject(ani_env *env)
{
    ModuleFileIO::FsResult<MultiDlProgressCore *> data = MultiDlProgressCore::Constructor();
    if (!data.IsSuccess()) {
        return nullptr;
    }
    MultiDlProgressCore *multiProgress = data.GetData().value();
    multiProgress->downloadProgress_ = shared_from_this();

    ani_object pg = MultiDlProgressWrapper::Wrap(env, multiProgress);
    if (pg == nullptr) {
        multiProgress->downloadProgress_.reset();
        delete multiProgress;
    }
    return pg;
}
} // namespace OHOS::FileManagement::CloudSync
