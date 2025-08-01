/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_CLOUD_SYNC_N_EXPORTER_H
#define OHOS_FILEMGMT_CLOUD_SYNC_N_EXPORTER_H

#include "filemgmt_libn.h"

namespace OHOS::FileManagement::CloudSync {
void InitCloudSyncState(napi_env env, napi_value exports);
void InitFileSyncState(napi_env env, napi_value exports);
void InitFileState(napi_env env, napi_value exports);
void InitErrorType(napi_env env, napi_value exports);
void InitCloudSyncApi(napi_env env, napi_value exports);
void InitDownloadErrorType(napi_env env, napi_value exports);
void InitNotifyType(napi_env env, napi_value exports);
void InitCloudSyncFuncs(napi_env env, napi_value exports);
void InitOptimizeState(napi_env env, napi_value exports);
void InitDownloadFileType(napi_env env, napi_value exports);
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_SYNC_N_EXPORTER_H