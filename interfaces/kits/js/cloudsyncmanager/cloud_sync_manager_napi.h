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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_NAPI_H
#define OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_NAPI_H

#include "filemgmt_libn.h"

enum Action {
    RETAIN_DATA = 0,
    CLEAR_DATA
};

namespace OHOS::FileManagement::CloudSync {
void CloudSyncManagerExport(napi_env env, napi_value exports);
void InitENumACtions(napi_env env, napi_value exports);
void InitDownloadState(napi_env env, napi_value exports);
void InitDownloadStopReason(napi_env env, napi_value exports);
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_SYNC_MANAGER_NAPI_H