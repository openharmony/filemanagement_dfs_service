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

#ifndef OHOS_FILEMGMT_FILE_SYNC_NAPI_H
#define OHOS_FILEMGMT_FILE_SYNC_NAPI_H

#include "gallery_sync_napi.h"
#include "cloud_sync_napi.h"

namespace OHOS::FileManagement::CloudSync {
class FileSyncNapi final : public CloudSyncNapi {
public:
    virtual bool Export() override;
    static napi_value GetLastSyncTime(napi_env env, napi_callback_info info);
    FileSyncNapi(napi_env env, napi_value exports) : CloudSyncNapi(env, exports) {};
    ~FileSyncNapi() = default;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_FILE_SYNC_NAPI_H
