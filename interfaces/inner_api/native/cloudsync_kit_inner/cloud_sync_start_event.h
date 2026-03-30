/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_CLOUD_SYNC_START_EVENT_H
#define OHOS_CLOUD_SYNC_START_EVENT_H

#include <chrono>
#include <map>
#include <string>

#include "dfs_error.h"
#include "hisysevent.h"
#include "ffrt_inner.h"
#include "utils_log.h"

#ifndef CLOUD_SYNC_START_SYS_EVENT
#define CLOUD_SYNC_START_SYS_EVENT(eventName, type, ...)                                  \
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, eventName, type, \
                    ##__VA_ARGS__)
#endif

namespace OHOS {
namespace FileManagement {
namespace CloudSync {
static uint32_t CLOUD_INIT = 000;
/* triggered internally or externally */
static uint32_t TRIGGER = 110000000;
static uint32_t TRIGGER_INTERN = 110000001;
static uint32_t TRIGGER_EXTERN = 110000002;
static std::string LOAD = "load";

static void HandleServiceStart(const std::string &event)
{
    ffrt::submit([event]() {
        int32_t ret = CLOUD_SYNC_START_SYS_EVENT("CLOUD_FILE_SYNC_FAULT",
            HiviewDFX::HiSysEvent::EventType::FAULT,
            "bundle_name", "-",
            "fault_scenario", CLOUD_INIT,
            "fault_type", TRIGGER,
            "fault_error_code", TRIGGER_INTERN,
            "function_name", "",
            "message", event);
        if (ret != E_OK) {
            LOGE("report CLOUD_FILE_SYNC_FAULT error %{public}d", ret);
        }
    });
}

static void HandleLoadRequest(const std::string &bundleName,
                              const std::string &callerMethod)
{
    ffrt::submit([bundleName, callerMethod]() {
        int32_t ret = CLOUD_SYNC_START_SYS_EVENT("CLOUD_FILE_SYNC_FAULT",
            HiviewDFX::HiSysEvent::EventType::FAULT,
            "bundle_name", bundleName,
            "fault_scenario", CLOUD_INIT,
            "fault_type", TRIGGER,
            "fault_error_code", TRIGGER_EXTERN,
            "function_name", "",
            "message", callerMethod);
        if (ret != E_OK) {
            LOGE("report CLOUD_FILE_SYNC_FAULT error %{public}d", ret);
        }
    });
}

inline void CloudFileServiceStartReport(const std::string &event)
{
    if (event == LOAD) {
        return;
    }
    HandleServiceStart(event);
}

inline void CloudFileServiceLoadRequestReport(const std::string &bundleName, const std::string &callerMethod)
{
    HandleLoadRequest(bundleName, callerMethod);
}
} // namespace CloudSync
} // namespace FileManagement
} // namespace OHOS
#endif // OHOS_CLOUD_SYNC_START_EVENT_H