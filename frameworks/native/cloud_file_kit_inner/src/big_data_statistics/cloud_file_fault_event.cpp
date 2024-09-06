/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cloud_file_log.h"
#include "cloud_file_fault_event.h"
#include "dfs_error.h"

namespace OHOS {
namespace FileManagement {
namespace CloudFile {

int32_t CloudFileFaultEvent::CloudSyncFaultReport(const std::string &funcName,
                                                  const int lineNum,
                                                  const CloudSyncFaultInfo &event)
{
    int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_FILE_SYNC_FAULT",
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "bundle_name", event.bundleName_,
        "fault_scenario", static_cast<uint32_t>(event.faultScenario_),
        "fault_type", static_cast<uint32_t>(event.faultType_),
        "fault_error_code", event.faultErrorCode_,
        "function_name", funcName + ":" + std::to_string(lineNum),
        "message", event.message_);
    if (ret != E_OK) {
        LOGE("report CLOUD_FILE_SYNC_FAULT error %{public}d", ret);
    }
    LOGE("%{public}s", event.message_.c_str());
    return event.faultErrorCode_;
}

int32_t CloudFileFaultEvent::CloudFileFaultReport(const std::string &funcName,
                                                  const int lineNum,
                                                  const CloudFileFaultInfo &event)
{
    int32_t ret = CLOUD_SYNC_SYS_EVENT("CLOUD_FILE_ACCESS_FAULT",
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "bundle_name", event.bundleName_,
        "fault_operation", static_cast<uint32_t>(event.faultOperation_),
        "fault_type", static_cast<uint32_t>(event.faultType_),
        "fault_error_code", event.faultErrorCode_,
        "function_name", funcName + ":" + std::to_string(lineNum),
        "message", event.message_);
    if (ret != E_OK) {
        LOGE("report CLOUD_FILE_ACCESS_FAULT error %{public}d", ret);
    }
    LOGE("%{public}s", event.message_.c_str());
    return event.faultErrorCode_;
}
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS