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
#include "io_message_listener.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <unordered_set>
#include "datetime_ex.h"
#include "hisysevent.h"
#include "utils_log.h"

using namespace std;
using namespace chrono;

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

const int32_t GET_FREQUENCY = 5;
const int32_t READ_THRESHOLD = 1000;
const int32_t OPEN_THRESHOLD = 1000;
const int32_t STAT_THRESHOLD = 1000;
const string IO_DATA_FILE_PATH = "/data/service/el1/public/cloudfile/io/";
const string IO_FILE_NAME = "io_message.csv";
const string IO_NEED_REPORT_PREFIX = "wait_report_";
const int32_t TYPE_FRONT = 2;
const int32_t TYPE_BACKGROUND = 4;
const int32_t MAX_IO_FILE_SIZE = 16 * 1024;
const size_t MAX_RECORD_IN_FILE = 10000;
const size_t MAX_IO_REPORT_NUMBER = 100;

const std::unordered_set<std::string> BUNDLE_NAME_CHECKLIST = {
    "com.ohos.sceneboard",
    "inputmethod",
};

IoMessageManager &IoMessageManager::GetInstance()
{
    static IoMessageManager instance;
    return instance;
}

bool IoMessageManager::CheckBundleName(const std::string &appBundleName)
{
    for (const auto& pattern : BUNDLE_NAME_CHECKLIST) {
        if (appBundleName.find(pattern) != std::string::npos) {
            return false;
        }
    }

    return true;
}

string IoMessageManager::GetIoDatetime()
{
    int64_t currentTime = GetSecondsSince1970ToNow();

    time_t time_t_seconds = static_cast<time_t>(currentTime);
    std::tm* localTime = std::localtime(&time_t_seconds);

    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");

    return oss.str();
}

bool IoMessageManager::ReadIoDataFromFile(const std::string &path)
{
    ifstream inFile(path);
    if (!inFile.is_open()) {
        LOGE("Failed to open io, err: %{public}d", errno);
        return false;
    }

    string line;
    while (getline(inFile, line)) {
        size_t colon = line.find(':');
        if (colon == string::npos) {
            continue;
        }

        string key = line.substr(0, colon);
        string value = line.substr(colon + 1);

        try {
            if (key == "rchar") currentData.rchar = stol(value);
            if (key == "syscr") currentData.syscr = stol(value);
            if (key == "read_bytes") currentData.readBytes = stol(value);
            if (key == "syscopen") currentData.syscopen = stol(value);
            if (key == "syscstat") currentData.syscstat = stol(value);
        } catch (const invalid_argument& e) {
            LOGE("Invalid argument: %{public}s", e.what());
            currentData = preData;
            return false;
        } catch (const out_of_range& e) {
            LOGE("Out of range: %{public}s", e.what());
            currentData = preData;
            return false;
        }
    }
    return true;
}


void IoMessageManager::RecordDataToFile(const string &path)
{
    ofstream outFile(path, ios::app);
    if (!outFile) {
        LOGE("Failed to open io file to write, err: %{public}d", errno);
        return;
    }

    outFile << bundleTimeMap[dataToWrite.bundleName] << ","
            << dataToWrite.bundleName << ","
            << dataToWrite.rcharDiff << ","
            << dataToWrite.syscrDiff << ","
            << dataToWrite.readBytesDiff << ","
            << dataToWrite.syscopenDiff << ","
            << dataToWrite.syscstatDiff << ","
            << dataToWrite.result << ","
            << dataToWrite.datetime << "\n";
    LOGD("Write io data success");
}

static vector<const char*> ConvertToCStringArray(const vector<string>& vec)
{
    vector<const char*> cstrVec;
    for (const auto& str : vec) {
        cstrVec.push_back(str.c_str());
    }
    return cstrVec;
}

template <typename T>
void PushField(const std::string &value, std::vector<T> &vec)
{
    if constexpr (std::is_same_v<T, int32_t>) {
        vec.push_back(std::stoi(value));
    } else if constexpr (std::is_same_v<T, int64_t>) {
        vec.push_back(std::stoll(value));
    } else if constexpr (std::is_same_v<T, double>) {
        vec.push_back(std::stod(value));
    } else {
        vec.push_back(value);
    }
}

bool CheckInt(const std::string &value)
{
    if (value.empty()) {
        return false;
    }
    if (!all_of(value.begin(), value.end(), ::isdigit)) {
        return false;
    }
    return true;
}

bool CheckDouble(const std::string &value)
{
    if (value.empty()) {
        return false;
    }

    errno = 0;
    char *endptr;
    std::strtod(value.c_str(), &endptr);
    if (errno != 0) {
        return false;
    }
    return *endptr == '\0' || *endptr == '\r';
}

struct PushBackVisitor {
    const std::string &value;

    template<typename T>
    void operator()(std::vector<T> &vec)
    {
        PushField(value, vec);
    }
};

struct CheckVisitor {
    const std::string &value;
    bool &checkType;

    template<typename T>
    void operator()(std::vector<T> &vec)
    {
        if constexpr (std::is_same_v<T, int32_t>) {
            checkType = CheckInt(value);
        } else if constexpr (std::is_same_v<T, int64_t>) {
            checkType = CheckInt(value);
        } else if constexpr (std::is_same_v<T, double>) {
            checkType = CheckDouble(value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            checkType = true;
        }
    }
};

template <typename T>
HiSysEventParam CreateParam(const std::string name, HiSysEventParamType type, std::vector<T> &data)
{
    HiSysEventParam param;
    std::copy(name.begin(), name.begin() + std::min(name.length(), sizeof(param.name) - 1), param.name);
    param.name[sizeof(param.name) - 1] = '\0';
    param.t = type;
    param.v.array = data.data();
    param.arraySize = data.size();
    return param;
}

void IoMessageManager::Report()
{
    auto sizeVector = [](auto &vec) {
        if (vec.size() == 0) {
            return;
        }
    };
    for (auto &variant : targetVectors) {
        std::visit(sizeVector, variant);
    }
    auto charIoBundleName = ConvertToCStringArray(GetVector<StringVector, VectorIndex::IO_BUNDLE_NAME>(targetVectors));
    auto charIoDatetime = ConvertToCStringArray(GetVector<StringVector, VectorIndex::IO_DATETIME>(targetVectors));
    HiSysEventParam params[] = {
        CreateParam("TIME", HISYSEVENT_INT32_ARRAY, GetVector<Int32Vector, VectorIndex::IO_TIMES>(targetVectors)),
        CreateParam("BUNDLENAME", HISYSEVENT_STRING_ARRAY, charIoBundleName),
        CreateParam("READCHAR", HISYSEVENT_INT64_ARRAY,
            GetVector<Int64Vector, VectorIndex::IO_READ_CHAR_DIFF>(targetVectors)),
        CreateParam("SYSCR", HISYSEVENT_INT64_ARRAY,
            GetVector<Int64Vector, VectorIndex::IO_SYSC_READ_DIFF>(targetVectors)),
        CreateParam("READBYTES", HISYSEVENT_INT64_ARRAY,
            GetVector<Int64Vector, VectorIndex::IO_READ_BYTES_DIFF>(targetVectors)),
        CreateParam("SYSCOPEN", HISYSEVENT_INT64_ARRAY,
            GetVector<Int64Vector, VectorIndex::IO_SYSC_OPEN_DIFF>(targetVectors)),
        CreateParam("SYSCSTAT", HISYSEVENT_INT64_ARRAY,
            GetVector<Int64Vector, VectorIndex::IO_SYSC_STAT_DIFF>(targetVectors)),
        CreateParam("RESULT", HISYSEVENT_DOUBLE_ARRAY,
            GetVector<DoubleVector, VectorIndex::IO_RESULT>(targetVectors)),
        CreateParam("DATETIME", HISYSEVENT_STRING_ARRAY, charIoDatetime)
    };

    auto ret = OH_HiSysEvent_Write(
        "HM_FS",
        "ABNORMAL_IO_STATISTICS_DATA",
        HISYSEVENT_STATISTIC,
        params,
        sizeof(params) / sizeof(params[0])
    );
    if (ret != 0) {
        LOGE("Report failed, err : %{public}d", ret);
    }
    auto clearVector = [](auto &vec) {
        vec.clear();
    };
    for (auto &variant : targetVectors) {
        std::visit(clearVector, variant);
    }
}

void IoMessageManager::PushData(const vector<string> &fields)
{
    if (fields.size() != targetVectors.size()) {
        return;
    }
    for (uint32_t i = 0; i < fields.size(); ++i) {
        bool checkType = false;
        CheckVisitor visitor{fields[i], checkType};
        std::visit(visitor, targetVectors[i]);
        if (!checkType) {
            LOGI("checkType failed. value = %{public}s", fields[i].c_str());
            return;
        }
    }
    for (uint32_t i = 0; i < fields.size(); ++i) {
        PushBackVisitor visitor{fields[i]};
        std::visit(visitor, targetVectors[i]);
    }
    return;
}

void IoMessageManager::ReadAndReportIoMessage()
{
    ifstream localData(IO_DATA_FILE_PATH + IO_NEED_REPORT_PREFIX + IO_FILE_NAME);
    if (!localData) {
        LOGE("Open cloud data statistic local data fail : %{public}d", errno);
        return;
    }

    string line;
    size_t reportCount = 0;
    size_t totalCount = 0;
    while (getline(localData, line)) {
        vector<string> fields;
        istringstream iss(line);
        string token;

        while (getline(iss, token, ',')) {
            fields.push_back(token);
        }

        PushData(fields);
        reportCount++;
        totalCount++;
        if (reportCount >= MAX_IO_REPORT_NUMBER) {
            Report();
            reportCount = 0;
        }
        if (totalCount >= MAX_RECORD_IN_FILE) {
            break;
        }
    }
    if (reportCount > 0) {
        Report();
    }
    bool ret = filesystem::remove(IO_DATA_FILE_PATH + IO_NEED_REPORT_PREFIX + IO_FILE_NAME);
    if (!ret) {
        LOGE("Failed to remove need_report_io_file, err:%{public}d", errno);
    }
    reportThreadRunning.store(false);
}

void IoMessageManager::CheckMaxSizeAndReport()
{
    std::error_code errCode;
    if (!filesystem::exists(IO_DATA_FILE_PATH + IO_FILE_NAME, errCode) || errCode.value() != 0) {
        LOGE("source file not exist");
        return;
    }
    auto fileSize = filesystem::file_size(IO_DATA_FILE_PATH + IO_FILE_NAME, errCode);
    if (fileSize < MAX_IO_FILE_SIZE || errCode.value() != 0) {
        return;
    }
    if (filesystem::exists(IO_DATA_FILE_PATH + IO_NEED_REPORT_PREFIX + IO_FILE_NAME)) {
        LOGI("Report file exist");
    }
    filesystem::rename(IO_DATA_FILE_PATH + IO_FILE_NAME,
        IO_DATA_FILE_PATH + IO_NEED_REPORT_PREFIX + IO_FILE_NAME, errCode);
    if (errCode.value() != 0) {
        LOGE("Failed to rename file, error code: %{public}d", errCode.value());
        return;
    }
    if (!reportThreadRunning.load()) {
        reportThreadRunning.store(true);
        LOGI("Start report io data");
        ffrt::submit([this] { ReadAndReportIoMessage(); }, {}, {}, ffrt::task_attr().qos(ffrt::qos_background));
    }
}

void IoMessageManager::ProcessIoData(const string &path)
{
    if (currentBundleName != lastestAppStateData.bundleName) {
        bundleTimeMap[lastestAppStateData.bundleName] = 0;
    }

    if (!ReadIoDataFromFile(path)) {
        return;
    }

    if (preData.rchar == 0) {
        preData = currentData;
        return;
    }

    currentBundleName = lastestAppStateData.bundleName;
    dataToWrite.datetime = GetIoDatetime();
    dataToWrite.bundleName = lastestAppStateData.bundleName;
    dataToWrite.rcharDiff = currentData.rchar - preData.rchar;
    dataToWrite.syscrDiff = currentData.syscr - preData.syscr;
    dataToWrite.readBytesDiff = currentData.readBytes - preData.readBytes;
    dataToWrite.syscopenDiff = currentData.syscopen - preData.syscopen;
    dataToWrite.syscstatDiff = currentData.syscstat - preData.syscstat;

    /* Prevent division by zero */
    if (dataToWrite.rcharDiff + 1 != 0) {
        dataToWrite.result = (dataToWrite.readBytesDiff) / (dataToWrite.rcharDiff + 1) * dataToWrite.syscrDiff;
    }

    if (dataToWrite.result >= READ_THRESHOLD ||
        dataToWrite.syscopenDiff >= OPEN_THRESHOLD ||
        dataToWrite.syscstatDiff >= STAT_THRESHOLD) {
        CheckMaxSizeAndReport();
        RecordDataToFile(IO_DATA_FILE_PATH + IO_FILE_NAME);
    }

    preData = currentData;
    bundleTimeMap[lastestAppStateData.bundleName] += GET_FREQUENCY;
}

void IoMessageManager::RecordIoData()
{
    while (isThreadRunning.load()) {
        unique_lock<ffrt::mutex> lock(sleepMutex);
        if (!lastestAppStateData.bundleName.empty()) {
            string path = "/proc/" + to_string(lastestAppStateData.pid) + "/sys_count";
            ProcessIoData(path);
        }
        sleepCv.wait_for(lock, chrono::seconds(GET_FREQUENCY), [&] {
            return !isThreadRunning.load();
        });
    }
}

void IoMessageManager::OnReceiveEvent(const AppExecFwk::AppStateData &appStateData)
{
    lock_guard<ffrt::mutex> lock(sleepMutex);
    if (!CheckBundleName(appStateData.bundleName)) {
        return;
    }

    if (appStateData.state == TYPE_FRONT) {
        lastestAppStateData = appStateData;
        if (currentBundleName != lastestAppStateData.bundleName) {
            bundleTimeMap[lastestAppStateData.bundleName] = 0;
            currentBundleName = lastestAppStateData.bundleName;
            preData = {};
            sleepCv.notify_all();
        }
        if (!isThreadRunning.load()) {
            isThreadRunning.store(true);
            ffrt::submit([this] {RecordIoData();}, {}, {}, ffrt::task_attr().qos(ffrt::qos_background));
        }
        return;
    }
    if (appStateData.bundleName == currentBundleName && appStateData.state == TYPE_BACKGROUND) {
        isThreadRunning.store(false);
        sleepCv.notify_all();
        currentData = {};
        preData = {};
    }
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS