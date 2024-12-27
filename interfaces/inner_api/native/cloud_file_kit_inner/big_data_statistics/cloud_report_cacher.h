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
#ifndef OHOS_CLOUD_SYNC_SERVICE_CLOUD_REPORT_CACHE_H
#define OHOS_CLOUD_SYNC_SERVICE_CLOUD_REPORT_CACHE_H

#include "safe_map.h"
#include <vector>
#include <mutex>

namespace OHOS {
namespace FileManagement {
namespace CloudFile {
using namespace std;

struct CloudFaultInfo {
    string eventType_;
    int64_t faultTime_;
    uint32_t faultType_;
    uint32_t faultErrorCode_;
    string funcName_;
    string message_;
    bool terminate_ = false;
};

struct ReportInfo {
    vector<CloudFaultInfo> events_;
    string traceId_;
    uint32_t scenarioCode_;
    int lastScheduleType_;
};

class CloudReportCacher {
public:
    static CloudReportCacher &GetInstance();
    void Init(const string &bundleNmae, const string &traceId, const uint32_t scenarioCode);
    void SetScenarioCode(const string &bundleName, const uint32_t scenarioCode);
    void PushEvent(const string &bundleName, const CloudFaultInfo &event);
    void Report(const string &bundleName, const int32_t scheduleType);
    void ReportFaultInfo(const CloudFaultInfo &event, const string &bundleName, const uint32_t scenarioCode);
private:
    CloudReportCacher();
    ~CloudReportCacher() = default;
    bool GetReportInfo(const string &bundleName, shared_ptr<ReportInfo> &reportInfo);

    SafeMap<const string, shared_ptr<ReportInfo>> reportCache_;
    mutex eventsMutex_;
    mutex reportMutex_;
};
} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS

#endif // OHOS_CLOUD_SYNC_SERVICE_CLOUD_REPORT_CACHE_H