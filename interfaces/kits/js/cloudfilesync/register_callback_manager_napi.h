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

#ifndef OHOS_FILEMGMT_REGISTER_CALLBACK_MANAGER_NAPI_H
#define OHOS_FILEMGMT_REGISTER_CALLBACK_MANAGER_NAPI_H

#include <list>
#include <mutex>

#include "filemgmt_libn.h"
#include "n_napi.h"

namespace OHOS::FileManagement::CloudSync {
class RegisterCallbackManagerNapi {
public:
    explicit RegisterCallbackManagerNapi(napi_env env) : env_(env) {}
    bool IsExisted(napi_value callback);
    napi_status RegisterCallback(napi_value callback);
    napi_status UnregisterCallback(napi_value callback);
    void OnJsCallback(napi_value *value, uint32_t argc);

protected:
    napi_env env_;
    std::recursive_mutex callbackMtx_;
    std::atomic<uint32_t> validRefNum_{0};
    std::list<std::pair<bool, napi_ref>> callbackList_;
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_REGISTER_CALLBACK_MANAGER_NAPI_H