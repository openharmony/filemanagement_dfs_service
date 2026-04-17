/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_CLOUD_UPLOAD_CALLBACK_H
#define OHOS_FILEMGMT_CLOUD_UPLOAD_CALLBACK_H

#include <functional>

#include "cloud_sync_common.h"

namespace OHOS::FileManagement::CloudSync {

class CloudUploadCallback;

struct UploadCallbackInfo {
    std::string callbackId;
    std::shared_ptr<CloudUploadCallback> callback{nullptr};
    std::string bundleName;
};

class CloudUploadCallback {
public:
    virtual ~CloudUploadCallback() = default;
    virtual void OnUploadProgress(const UploadProgressObj& progress) {};
    /**
     * @brief 死亡回调通知同步状态逻辑
     *
     */
    virtual void OnDeathRecipient() {};
    /**
     * @brief 退出清理信息逻辑
     *
     */
    virtual void DeleteReference() {};
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_CLOUD_DOWNLOAD_CALLBACK_H
