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

#include "hisysevent.h"
#include "utils_log.h"

using namespace std;
using namespace chrono;

namespace OHOS {
namespace FileManagement {
namespace CloudDisk {

const string DESK_BUNDLE_NAME = "com.ohos.sceneboard";
const int32_t GET_FREQUENCY = 5;
const int32_t READ_THRESHOLD = 1000;
const int32_t OPEN_THRESHOLD = 1000;
const int32_t STAT_THRESHOLD = 1000;
const string IO_DATA_FILE_PATH = "/data/service/el1/public/cloudfile/io/";
const string IO_FILE_NAME = "io_message.csv";
const string IO_NEED_REPORT_PREFIX = "wait_report_";
const int32_t TYPE_FRONT = 2;
const int32_t TYPE_BACKGROUND = 4;
const int32_t MAX_IO_FILE_SIZE = 128 * 1024;
int32_t MAX_IO_REPORT_NUMBER = 100;

IoMessageManager &IoMessageManager::GetInstance()
{
    static IoMessageManager instance;
    return instance;
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
            << dataToWrite.result << "\n";
    LOGI("Write io data success");
}

static vector<const char*> ConvertToCStringArray(const vector<string>& vec)
{
    vector<const char*> cstrVec;
    for (const auto& str : vec) {
        cstrVec.push_back(str.c_str());
    }
    return cstrVec;
}

void IoMessageManager::Report()
{
    auto charIoBundleName = ConvertToCStringArray(ioBundleName);

    HiSysEventParam params[] = {
        { "time", HISYSEVENT_INT32_ARRAY, { .array = ioTimes.data() },
            static_cast<int>(ioTimes.size()) },
        { "BundleName", HISYSEVENT_STRING_ARRAY, { .array = charIoBundleName.data() },
            static_cast<int>(charIoBundleName.size()) },
        { "ReadCharDiff", HISYSEVENT_INT64_ARRAY, { .array = ioReadCharDiff.data() },
            static_cast<int>(ioReadCharDiff.size()) },
        { "SyscReadDiff", HISYSEVENT_INT64_ARRAY, { .array = ioSyscReadDiff.data() },
            static_cast<int>(ioSyscReadDiff.size()) },
        { "ReadBytesDiff", HISYSEVENT_INT64_ARRAY, { .array = ioReadBytesDiff.data() },
            static_cast<int>(ioReadBytesDiff.size()) },
        { "SyscOpenDiff", HISYSEVENT_INT64_ARRAY, { .array = ioSyscOpenDiff.data() },
            static_cast<int>(ioSyscOpenDiff.size()) },
        { "SyscStatDiff", HISYSEVENT_INT64_ARRAY, { .array = ioSyscStatDiff.data() },
            static_cast<int>(ioSyscStatDiff.size()) },
        { "Result", HISYSEVENT_DOUBLE_ARRAY, { .array = ioResult.data() },
            static_cast<int>(ioResult.size()) },
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

    ioTimes.clear();
    ioBundleName.clear();
    ioReadCharDiff.clear();
    ioSyscReadDiff.clear();
    ioReadBytesDiff.clear();
    ioSyscOpenDiff.clear();
    ioSyscStatDiff.clear();
    ioResult.clear();
}

void IoMessageManager::PushData(const vector<string> &fields)
{
    try{
        static const std::map<int32_t, std::function<void(const std::string&)>> fieldMap = {
            { 0, [this](const std::string& value) { ioTimes.push_back(std::stoi(value)); }},
            { 1, [this](const std::string& value) { ioBundleName.push_back(value); }},
            { 2, [this](const std::string& value) { ioReadCharDiff.push_back(std::stoll(value)); }},
            { 3, [this](const std::string& value) { ioSyscReadDiff.push_back(std::stoll(value)); }},
            { 4, [this](const std::string& value) { ioReadBytesDiff.push_back(std::stoll(value)); }},
            { 5, [this](const std::string& value) { ioSyscOpenDiff.push_back(std::stoll(value)); }},
            { 6, [this](const std::string& value) { ioSyscStatDiff.push_back(std::stoll(value)); }},
            { 7, [this](const std::string& value) { ioResult.push_back(std::stod(value)); }},
        };
        for (int i = 0; i < fields.size(); ++i) {
            auto it = fieldMap.find(i);
            if (it == fieldMap.end()) {
                LOGE("Unknow field index: %{public}d", i);
                continue;
            }
            it->second(fields[i]);
        }
    } catch (const invalid_argument& e) {
        LOGE("Invalid argument: %{public}s", e.what());
        return;
    } catch (const out_of_range& e) {
        LOGE("Out of range: %{public}s", e.what());
        return;
    }
}

void IoMessageManager::ReadAndReportIoMessage()
{
    ifstream localData(IO_DATA_FILE_PATH + IO_NEED_REPORT_PREFIX + IO_FILE_NAME);
    if (!localData) {
        LOGE("Open cloud data statistic local data fail : %{public}d", errno);
        return;
    }

    string line;
    int32_t reportCount = 0;
    while (getline(localData, line)) {
        vector<string> fields;
        istringstream iss(line);
        string token;

        while (getline(iss, token, ',')) {
            fields.push_back(token);
        }

        PushData(fields);
        reportCount++;

        if (reportCount >= MAX_IO_REPORT_NUMBER) {
            Report();
            reportCount = 0;
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
    try {
        auto fileSize = filesystem::file_size(IO_DATA_FILE_PATH + IO_FILE_NAME);
        if (fileSize >= MAX_IO_FILE_SIZE) {
            if (filesystem::exists(IO_DATA_FILE_PATH + IO_NEED_REPORT_PREFIX + IO_FILE_NAME)) {
                LOGI("Report file exist");
            }
            filesystem::rename(IO_DATA_FILE_PATH + IO_FILE_NAME,
                IO_DATA_FILE_PATH + IO_NEED_REPORT_PREFIX + IO_FILE_NAME);
            if (!reportThreadRunning.load()) {
                reportThreadRunning.store(true);
                LOGI("Start report io data");
                thread reportThread(&IoMessageManager::ReadAndReportIoMessage, this);
                reportThread.detach();
            }
        }
    } catch (const filesystem::filesystem_error& e) {
        LOGE("Rename or get file size failed, err: %{public}s", e.what());
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
        if (!lastestAppStateData.bundleName.empty()) {
            string path = "/proc/" + to_string(lastestAppStateData.pid) + "/sys_count";
            ProcessIoData(path);
        }
        unique_lock<mutex> lock(sleepMutex);
        sleepCv.wait_for(lock, chrono::seconds(GET_FREQUENCY), [&] {
            return !isThreadRunning.load();
        });
    }
}

void IoMessageManager::OnReceiveEvent(const AppExecFwk::AppStateData &appStateData)
{
    if (appStateData.bundleName == DESK_BUNDLE_NAME) {
        return;
    }
    if (appStateData.state == TYPE_FRONT) {
        lastestAppStateData = appStateData;
        if (!ioThread.joinable()) {
            try {
                isThreadRunning.store(true);
                ioThread = thread(&IoMessageManager::RecordIoData, this);
            } catch (const std::system_error &e) {
                LOGE("System error while creating thread: %{public}s", e.what());
                isThreadRunning.store(false);
            } catch (const std::exception &e) {
                LOGE("Exception while creating thread: %{public}s", e.what());
                isThreadRunning.store(false);
            }
        }
        return;
    }
    if (appStateData.state == TYPE_BACKGROUND) {
        if (ioThread.joinable()) {
            lock_guard<mutex> lock(cvMute);
            isThreadRunning.store(false);
            sleepCv.notify_all();
            ioThread.join();
            ioThread = thread();
            currentData = {};
            preData = {};
        }
    }
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS