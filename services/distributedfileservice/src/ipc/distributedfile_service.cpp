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

#include <system_ability_definition.h>
#include "device_manager_agent.h"
#include "utils_exception.h"
#include "utils_log.h"
#include "softbus_agent.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;

REGISTER_SYSTEM_ABILITY_BY_ID(DistributedFileService, STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID, false);

void DistributedFileService::OnDump()
{
    LOGI("OnDump");
}

void DistributedFileService::PublishSA()
{
    LOGI("Begin to init, pull the service SA");
    bool ret = SystemAbility::Publish(this);
    if (!ret) {
        throw runtime_error("publishing DistributedFileService failed");
    }
    LOGI("Init finished successfully");
}

void DistributedFileService::StartManagers()
{
    DeviceManagerAgent::GetInstance();
}

void DistributedFileService::OnStart()
{
    LOGI("DistributedFileService::OnStart");
    try {
        PublishSA();
        StartManagers();
    } catch (const Exception &e) {
        LOGE("%{public}s", e.what());
    }
}

void DistributedFileService::OnStop()
{
    LOGI("DistributedFileService::OnStop");
}

int32_t DistributedFileService::sendTest()
{
    LOGI("DistributedFileService::sendTest");
    return 0;
}

int32_t DistributedFileService::SendFile(const std::string &cid,
                                         const std::vector<std::string> &sourceFileList,
                                         const std::vector<std::string> &destinationFileList,
                                         const uint32_t fileCount)
{
    return 0;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
