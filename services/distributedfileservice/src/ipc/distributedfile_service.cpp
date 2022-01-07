/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "distributedfile_service.h"

#include <exception>
#include <system_ability_definition.h>

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "parcel.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

DistributedFileService::DistributedFileService() : SystemAbility(STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID, false) {}

DistributedFileService::~DistributedFileService() {}

void DistributedFileService::OnDump()
{
    LOGI("OnDump");
}

void DistributedFileService::OnStart()
{
    bool ret = SystemAbility::Publish(DelayedSingleton<DistributedFileService>::GetInstance().get());
    if (!ret) {
        LOGE("Leave, publishing DistributedFileService failed!");
        return;
    }
}

void DistributedFileService::OnStop()
{
    LOGI("DistributedFileService::OnStop start");
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
