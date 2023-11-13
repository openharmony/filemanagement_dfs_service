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

#include "file_sync_napi.h"

#include <memory>
#include <sys/types.h>

#include "async_work.h"
#include "cloud_sync_manager.h"
#include "dfs_error.h"
#include "utils_log.h"
#include "uv.h"

namespace OHOS::FileManagement::CloudSync {
using namespace FileManagement::LibN;
using namespace std;

bool FileSyncNapi::Export()
{
    std::vector<napi_property_descriptor> props = {
        NVal::DeclareNapiFunction("on", FileSyncNapi::OnCallback),
        NVal::DeclareNapiFunction("off", FileSyncNapi::OffCallback),
        NVal::DeclareNapiFunction("start", FileSyncNapi::Start),
        NVal::DeclareNapiFunction("stop", FileSyncNapi::Stop),
    };

    SetClassName("FileSync");
    return ToExport(props);
}
} // namespace OHOS::FileManagement::CloudSync