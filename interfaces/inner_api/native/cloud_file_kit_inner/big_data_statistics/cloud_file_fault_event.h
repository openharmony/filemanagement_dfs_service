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
#ifndef OHOS_CLOUD_SYNC_SERVICE_CLOUD_FILE_FAULT_EVENT_H
#define OHOS_CLOUD_SYNC_SERVICE_CLOUD_FILE_FAULT_EVENT_H

#include "hisysevent.h"
#include <map>

#define CLOUD_SYNC_FAULT_REPORT(...) \
    CloudFile::CloudFileFaultEvent::CloudSyncFaultReport(__FUNCTION__, __LINE__, ##__VA_ARGS__)
#define CLOUD_FILE_FAULT_REPORT(...) \
    CloudFile::CloudFileFaultEvent::CloudFileFaultReport(__FUNCTION__, __LINE__, ##__VA_ARGS__)
#define CLOUD_SYNC_CACHE_REPORT(...) \
    CloudFile::CloudFileFaultEvent::CloudSyncCacheReport(__FUNCTION__, __LINE__, ##__VA_ARGS__)

namespace OHOS {
namespace FileManagement {
namespace CloudFile {

#ifndef CLOUD_SYNC_SYS_EVENT
#define CLOUD_SYNC_SYS_EVENT(eventName, type, ...)                                  \
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, eventName, type, \
                    ##__VA_ARGS__)
#endif

#define MINIMUM_FAULT_REPORT_INTERVAL 3600

enum class FaultScenarioCode {
    CLOUD_INIT = 000,
    CLOUD_FULL_SYNC = 100,
    CLOUD_INC_SYNC = 200,
    CLOUD_CHECK_SYNC = 300,
    CLOUD_SWITCH_CLOSE = 400,
    CLOUD_LOAD_VIDEO_CACHE = 500,
    CLOUD_DOWNLOAD_FILE = 600,
    CLOUD_DOWNLOAD_THUM = 700,
    CLOUD_OPTIMMIZE_STORAGE = 800,
    CLOUD_PROCESS_EXIT_START = 900,
    CLOUD_ACCOUNT_EXIT = 1000
};

enum class FaultOperation {
    LOOKUP = 100,
    FORGET = 200,
    GETATTR = 300,
    OPEN = 400,
    READ = 500,
    READDIR = 600,
    RELEASE = 700,
    FORGETMULTI = 800,
    IOCTL = 900,
    UNLINK = 1000
};

enum class FaultType {
    /* file fault type */
    FILE = 10000000,
    DENTRY_FILE = 10000001,
    INODE_FILE = 10000002,

    /* driverkit fault type */
    DRIVERKIT = 20000000,
    DRIVERKIT_NETWORK = 20000001,
    DRIVERKIT_SERVER = 20000002,
    DRIVERKIT_LOCAL = 20000003,
    DRIVERKIT_DATABASE = 20000004,
    DRIVERKIT_READSESSION = 20000005,

    /* timeout fault type */
    TIMEOUT = 30000000,
    OPEN_CLOUD_FILE_TIMEOUT = 30000001,
    CLOUD_READ_FILE_TIMEOUT = 30000002,
    CLOUD_FILE_LOOKUP_TIMEOUT = 30000003,
    CLOUD_FILE_FORGET_TIMEOUT = 30000004,

    /* temperature fault type */
    TEMPERATURE = 40000000,

    /* database fault type */
    DATABASE = 50000000,
    QUERY_DATABASE = 50000001,
    INSERT_DATABASE = 50000002,
    DELETE_DATABASE = 50000003,
    MODIFY_DATABASE = 50000004,

    /* consistency fault type */
    CONSISTENCY = 60000000,
    FILE_CONSISTENCY = 60000001,
    META_CONSISTENCY = 60000002,

    /* process fault type */
    PROCESS = 70000000,

    /* cloud sync error */
    CLOUD_SYNC_ERROR = 80000000,

    /* inner error */
    INNER_ERROR = 90000000,

    /* warning */
    WARNING = 100000000
};

struct CloudSyncFaultInfo {
    std::string bundleName_;
    FaultScenarioCode faultScenario_;
    FaultType faultType_;
    int32_t faultErrorCode_;
    std::string message_;
};

struct CloudFileFaultInfo {
    std::string bundleName_;
    FaultOperation faultOperation_;
    FaultType faultType_;
    int32_t faultErrorCode_;
    std::string message_;
};

struct CloudCacheFaultInfo {
    std::string bundleName_;
    FaultType faultType_;
    int32_t faultErrorCode_;
    std::string message_;
    bool terminate_ = false;
};

const std::vector<FaultType> PERIODIC_REPORT_FAULT_TYPE = { FaultType::WARNING,
                                                            FaultType::TIMEOUT,
                                                            FaultType::OPEN_CLOUD_FILE_TIMEOUT,
                                                            FaultType::CLOUD_READ_FILE_TIMEOUT,
                                                            FaultType::CLOUD_FILE_LOOKUP_TIMEOUT,
                                                            FaultType::CLOUD_FILE_FORGET_TIMEOUT };

class CloudFaultReportStatus {
public:
    static CloudFaultReportStatus &GetInstance();

    bool IsAllowToReport(const FaultType &faultType);

private:
    CloudFaultReportStatus();

    CloudFaultReportStatus(const CloudFaultReportStatus&) = delete;

    CloudFaultReportStatus& operator=(const CloudFaultReportStatus&) = delete;

    std::mutex timeMapLock_;
    std::unordered_map<FaultType, int64_t> lastTimeMap_;
} ;

class CloudFileFaultEvent {
public:
    static int32_t CloudSyncFaultReport(const std::string &funcName,
                                        const int lineNum,
                                        const CloudSyncFaultInfo &event);

    static int32_t CloudFileFaultReport(const std::string &funcName,
                                        const int lineNum,
                                        const CloudFileFaultInfo &event);

    static int32_t CloudSyncCacheReport(const std::string &funcName,
                                        const int lineNum,
                                        const CloudCacheFaultInfo &event);
};

} // namespace CloudFile
} // namespace FileManagement
} // namespace OHOS

#endif // OHOS_CLOUD_SYNC_SERVICE_CLOUD_FILE_FAULT_EVENT_H