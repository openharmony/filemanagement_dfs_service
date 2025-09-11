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
#include "ffrt_inner.h"

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

using Int32Vector = std::vector<int32_t>;
using Int64Vector = std::vector<int64_t>;
using StringVector = std::vector<std::string>;
using DoubleVector = std::vector<double>;

enum class VectorIndex {
    IO_TIMES,
    IO_BUNDLE_NAME,
    IO_READ_CHAR_DIFF,
    IO_SYSC_READ_DIFF,
    IO_READ_BYTES_DIFF,
    IO_SYSC_OPEN_DIFF,
    IO_SYSC_STAT_DIFF,
    IO_RESULT
};
using VectorVariant = std::variant<
    Int32Vector,
    StringVector,
    Int64Vector,
    DoubleVector
>;
class IoMessageManager {
private:
    std::string currentBundleName = "";
    std::atomic<bool> isThreadRunning{false};
    std::atomic<bool> reportThreadRunning{false};
    ffrt::mutex sleepMutex;
    ffrt::condition_variable sleepCv;
    std::map<std::string, int32_t> bundleTimeMap;
    IoData currentData;
    IoData preData;
    IoDataToWrite dataToWrite;
    AppExecFwk::AppStateData lastestAppStateData;

    Int32Vector ioTimes;
    StringVector ioBundleName;
    Int64Vector ioReadCharDiff;
    Int64Vector ioSyscReadDiff;
    Int64Vector ioReadBytesDiff;
    Int64Vector ioSyscOpenDiff;
    Int64Vector ioSyscStatDiff;
    DoubleVector ioResult;

    std::vector<VectorVariant> targetVectors = {
        VectorVariant(std::in_place_type<Int32Vector>, std::move(ioTimes)),
        VectorVariant(std::in_place_type<StringVector>, std::move(ioBundleName)),
        VectorVariant(std::in_place_type<Int64Vector>, std::move(ioReadCharDiff)),
        VectorVariant(std::in_place_type<Int64Vector>, std::move(ioSyscReadDiff)),
        VectorVariant(std::in_place_type<Int64Vector>, std::move(ioReadBytesDiff)),
        VectorVariant(std::in_place_type<Int64Vector>, std::move(ioSyscOpenDiff)),
        VectorVariant(std::in_place_type<Int64Vector>, std::move(ioSyscStatDiff)),
        VectorVariant(std::in_place_type<DoubleVector>, std::move(ioResult))
    };

    template <typename T, VectorIndex Index>
    T& GetVector(std::vector<VectorVariant>& targetVectors) {
        return std::get<T>(targetVectors[static_cast<size_t>(Index)]);
    }
    bool ReadIoDataFromFile(const std::string &path);
    void RecordDataToFile(const std::string &path);
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