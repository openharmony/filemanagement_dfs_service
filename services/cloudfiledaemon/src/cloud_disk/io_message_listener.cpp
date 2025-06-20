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
const string IO_DATA_FILE_PATH = "/data/service/el1/public/cloudfile/rdb/io_message.csv";
const int32_t TYPE_FRONT = 2;
const int32_t TYPE_BACKGROUND = 4;

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

bool IoMessageManager::IsFirstLineHeader(const string &path)
{
    ifstream inFile(path);
    if (!inFile.is_open()) {
        return false;
    }

    string firstLine;
    getline(inFile, firstLine);
    return firstLine.find("time") != string::npos;
}

void IoMessageManager::RecordDataToFile(const string &path)
{
    ofstream outFile(path, ios::app);
    if (!outFile) {
        LOGE("Failed to open io file to write, err: %{public}d", errno);
        return;
    }

    if (!IsFirstLineHeader(path)) {
        outFile << "time, bundle_name, rchar_diff, syscr_diff, read_bytes_diff, syscopen_diff, syscstat_diff, result\n";
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
        RecordDataToFile(IO_DATA_FILE_PATH);
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
    if (appStateData.bundleName != DESK_BUNDLE_NAME) {
        if (appStateData.state == TYPE_FRONT) {
            lastestAppStateData = appStateData;
            if (!ioThread.joinable()) {
                isThreadRunning.store(true);
                ioThread = thread(&IoMessageManager::RecordIoData, this);
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
}
} // namespace CloudDisk
} // namespace FileManagement
} // namespace OHOS