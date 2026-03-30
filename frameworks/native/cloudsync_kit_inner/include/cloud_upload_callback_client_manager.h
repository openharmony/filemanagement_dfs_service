/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_CLOUD_UPLOAD_CALLBACK_CLIENT_MANAGER_H
#define OHOS_FILEMGMT_CLOUD_UPLOAD_CALLBACK_CLIENT_MANAGER_H

#include <mutex>
#include <vector>

#include "cloud_upload_callback.h"
#include "nocopyable.h"

namespace OHOS::FileManagement::CloudSync {
class CloudUploadCallbackClientManager : public NoCopyable {
public:
    static CloudUploadCallbackClientManager &GetInstance();
    int32_t AddCallback(const UploadCallbackInfo &callbackInfo);
    int32_t RemoveCallback(const UploadCallbackInfo &callbackInfo);
    int32_t GetAllCallback(std::vector<UploadCallbackInfo> &callbackInfos);
    bool IsEmpty();

private:
    CloudUploadCallbackClientManager() = default;
    std::mutex callbackListMutex_;
    std::vector<UploadCallbackInfo> callbackList_;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_UPLOAD_CALLBACK_CLIENT_MANAGER_H