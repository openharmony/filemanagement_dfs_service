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
#include "cloud_report_cacher.h"
#include "hisysevent.h"
#include "nlohmann/json.hpp"
#include "cloud_file_log.h"
#include "cloud_file_fault_event.h"
 
namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;

constexpr int MAX_CACHE_SIZE = 20;
constexpr int REPORT_VERSION = 2;

CloudReportCacher::CloudReportCacher()
{
}

CloudReportCacher &CloudReportCacher::GetInstance()
{
    static CloudReportCacher instance_;
    return instance_;
}

void CloudReportCacher::Init(const string &bundleName, const string &traceId, const uint32_t scenarioCode)
{
    LOGI("init : %{public}s %{public}s %{public}d", bundleName.c_str(), traceId.c_str(), scenarioCode);
    if (bundleName.empty() || traceId.empty()) {
        return;
    }
    shared_ptr<ReportInfo> reportInfo;
    if (!GetReportInfo(bundleName, reportInfo)) {
        return;
    }
    reportInfo->traceId_ = traceId;
    reportInfo->scenarioCode_ = scenarioCode;
}

void CloudReportCacher::SetScenarioCode(const string &bundleName, const uint32_t scenarioCode)
{
    LOGI("set scenario_code : %{public}s %{public}d", bundleName.c_str(), scenarioCode);
    if (bundleName.empty() || scenarioCode <= 0) {
        return;
    }
    shared_ptr<ReportInfo> reportInfo;
    if (!GetReportInfo(bundleName, reportInfo)) {
        return;
    }
    reportInfo->scenarioCode_ = scenarioCode;
}

void CloudReportCacher::PushEvent(const string &bundleName, const CloudFaultInfo &event)
{
    shared_ptr<ReportInfo> reportInfo;
    if (!GetReportInfo(bundleName, reportInfo)) {
        return;
    }
    bool exceededLimit = false;
    {
        unique_lock<mutex> lock(eventsMutex_);
        reportInfo->events_.emplace_back(event);
        exceededLimit = reportInfo->events_.size() > MAX_CACHE_SIZE;
    }
    if (exceededLimit) {
        Report(bundleName, reportInfo->lastScheduleType_ + 1);
    }
}

void CloudReportCacher::Report(const string &bundleName, const int scheduleType)
{
    if (scheduleType <= 0) {
        return;
    }
    LOGI("report schedule : %{public}s %{public}d", bundleName.c_str(), scheduleType);
    shared_ptr<ReportInfo> reportInfo;
    if (!GetReportInfo(bundleName, reportInfo)) {
        return;
    }
    reportInfo->lastScheduleType_ = scheduleType;
    unique_lock<mutex> lock(eventsMutex_);
    if (reportInfo->events_.empty()) {
        return;
    }

    nlohmann::json jsonArr = nlohmann::json::array();
    CloudFaultInfo terminateEvent = reportInfo->events_[reportInfo->events_.size() - 1];
    terminateEvent.faultType_ = static_cast<uint32_t>(FaultType::WARNING);
    for (auto it = reportInfo->events_.begin(); it != reportInfo->events_.end(); ++it) {
        CloudFaultInfo& event = *it;
        nlohmann::json jsonItem;
        jsonItem["version"] = REPORT_VERSION;
        jsonItem["trace_id"] = reportInfo->traceId_;
        jsonItem["schedule"] = scheduleType;
        jsonItem["fault_error_code"] = event.faultErrorCode_ ;
        jsonItem["function_name"] = event.funcName_;
        jsonItem["fault_time"] = event.faultTime_;
        jsonItem["message"] = event.message_;
        jsonArr.push_back(jsonItem);
        if (event.terminate_) {
            terminateEvent = event;
            terminateEvent.faultType_ = static_cast<uint32_t>(FaultType::CLOUD_SYNC_ERROR);
        }
    }
    terminateEvent.message_ = jsonArr.dump();
    ReportFaultInfo(terminateEvent, bundleName, reportInfo->scenarioCode_);
    reportInfo->events_.clear();
}

void CloudReportCacher::ReportFaultInfo(const CloudFaultInfo &event, const string &bundleName,
    const uint32_t scenarioCode)
{
    HiSysEventWrite(
        HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, event.eventType_,
        HiviewDFX::HiSysEvent::EventType::FAULT,
        "bundle_name", bundleName,
        "fault_scenario", scenarioCode,
        "fault_type", event.faultType_,
        "fault_error_code", event.faultErrorCode_,
        "function_name", event.funcName_,
        "message", event.message_);
}

bool CloudReportCacher::GetReportInfo(const string &bundleName, shared_ptr<ReportInfo> &reportInfo)
{
    unique_lock<mutex> lock(reportMutex_);
    if (reportCache_.Find(bundleName, reportInfo)) {
        return true;
    }
    reportInfo = make_shared<ReportInfo>();
    if (reportInfo == nullptr) {
        LOGE("new report fail");
        return false;
    }
    reportCache_.EnsureInsert(bundleName, reportInfo);
    return true;
}
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS