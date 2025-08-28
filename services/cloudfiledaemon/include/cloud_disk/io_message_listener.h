/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef CLOUD_FILE_DAEMON_IO_MESSAGE_STATUS_LISTENER_H
#define CLOUD_FILE_DAEMON_IO_MESSAGE_STATUS_LISTENER_H

#include <chrono>
#include <condition_variable>
#include <fstream>
#include <thread>
#include <mutex>

#include "application_state_observer_stub.h"
#include "common_event_subscriber.h"

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

struct IoData {
    int64_t rchar = 0;
    int64_t syscr = 0;
    int64_t readBytes = 0;
    int64_t syscopen = 0;
    int64_t syscstat = 0;
};

struct IoDataToWrite {
    std::string bundleName;
    int64_t rcharDiff = 0;
    int64_t syscrDiff = 0;
    int64_t readBytesDiff = 0;
    int64_t syscopenDiff = 0;
    int64_t syscstatDiff = 0;
    double result = 0;
};

class IoMessageManager {
private:
    std::string currentBundleName = "";
    std::atomic<bool> isThreadRunning{false};
    std::atomic<bool> reportThreadRunning{false};
    std::thread ioThread;
    std::mutex sleepMutex;
    std::mutex cvMute;
    std::condition_variable sleepCv;
    std::map<std::string, int32_t> bundleTimeMap;
    IoData currentData;
    IoData preData;
    IoDataToWrite dataToWrite;
    AppExecFwk::AppStateData lastestAppStateData;

    std::vector<std::string> ioTimes;
    std::vector<std::string> ioBundleName;
    std::vector<std::string> ioReadCharDiff;
    std::vector<std::string> ioSyscReadDiff;
    std::vector<std::string> ioReadBytesDiff;
    std::vector<std::string> ioSyscOpenDiff;
    std::vector<std::string> ioSyscStatDiff;
    std::vector<std::string> ioResult;

    bool ReadIoDataFromFile(const std::string &path);
    void RecordDataToFile(const std::string &path);
    bool IsFirstLineHeader(const std::string &path);
    void RecordIoData();
    void ProcessIoData(const std::string &path);
    void PushData(const std::vector<std::string> &fields);
    void ReadAndReportIoMessage();
    void CheckMaxSizeAndReport();
    void Report();

public:
    static IoMessageManager &GetInstance();
    void OnReceiveEvent(const AppExecFwk::AppStateData &appStateData);
};
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS
#endif // CLOUD_FILE_DAEMON_IO_MESSAGE_LISTENER_H