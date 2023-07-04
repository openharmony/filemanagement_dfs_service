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

#include "cloud_sync_manager_napi.h"
#include "cloud_sync_manager_n_exporter.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
/***********************************************
 * Module export and register
 ***********************************************/

static napi_value Init(napi_env env, napi_value exports)
{
    CloudSyncManagerExport(env, exports);
    InitENumACtions(env, exports);

    return exports;
}

void CloudSyncManagerExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("changeAppCloudSwitch", ChangeAppCloudSwitch),
        DECLARE_NAPI_FUNCTION("clean", Clean),
        DECLARE_NAPI_FUNCTION("notifyDataChange", NotifyDataChange),
        DECLARE_NAPI_FUNCTION("enableCloud", EnableCloud),
        DECLARE_NAPI_FUNCTION("disableCloud", DisableCloud),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
}

void InitENumACtions(napi_env env, napi_value exports)
{
    char propertyName[] = "Action";
    napi_value actionObj = nullptr;
    napi_create_object(env, &actionObj);
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("RETAIN_DATA", NVal::CreateInt32(env, (int32_t)Action::RETAIN_DATA).val_),
        DECLARE_NAPI_STATIC_PROPERTY("CLEAR_DATA", NVal::CreateInt32(env, (int32_t)Action::CLEAR_DATA).val_),
    };
    napi_define_properties(env, actionObj, sizeof(desc) / sizeof(desc[0]), desc);
    napi_set_named_property(env, exports, propertyName, actionObj);
}

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "file.cloudSyncManager",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&_module);
}
} // namespace OHOS::FileManagement::CloudSync
