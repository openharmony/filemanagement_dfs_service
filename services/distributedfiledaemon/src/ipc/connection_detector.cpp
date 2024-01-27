/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "connection_detector.h"

#include <dirent.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
std::string ConnectionDetector::GetCellByIndex(const std::string &str, int targetIndex)
{
    size_t begin = 0;
    size_t end = 0;
    int count = 0;
    do {
        size_t i;
        for (i = begin; i < str.size(); ++i) {
            if (str[i] != '\t' && str[i] != ' ') {
                break;
            }
        }
        begin = i;
        for (i = begin; i < str.size(); ++i) {
            if (!(str[i] != '\t' && str[i] != ' ')) {
                break;
            }
        }
        end = i;
        if (end == str.size()) {
            break;
        }
        ++count;
        if (count == targetIndex) {
            break;
        }
        begin = end;
    } while (true);
    return str.substr(begin, end - begin);
}

bool ConnectionDetector::MatchConnectionStatus(ifstream &inputFile)
{
    string str;
    getline(inputFile, str);
    if (str.find("connection_status") == string::npos) {
        return false;
    }
    while (getline(inputFile, str)) {
        if (str.empty()) {
            return false;
        }
        auto connectionStatus = GetCellByIndex(str, 2);  // 2 indicates the position of connection status
        auto tcpStatus = GetCellByIndex(str, 3);  // 3 indicates the position of tcp status
        // "2"|"3" indicates socket status is connecting|connected;
        if (connectionStatus == "2" && (tcpStatus == "2" || tcpStatus == "3")) {
            return true;
        }
    }
    return false;
}

bool ConnectionDetector::MatchConnectionGroup(const std::string &fileName, const string &networkId)
{
    if (access(fileName.c_str(), F_OK) != 0) {
        LOGE("Cannot find the status file");
        return false;
    }
    ifstream statusFile(fileName);
    std::string str;
    getline(statusFile, str);
    bool result = false;
    while (getline(statusFile, str)) {
        if (str.find(networkId) == 0) {
            result = MatchConnectionStatus(statusFile);
            break;
        }
    }
    statusFile.close();
    return result;
}

static int FilterFunc(const struct dirent *filename)
{
    if (string_view(filename->d_name) == "." || string_view(filename->d_name) == "..") {
        return DISMATCH;
    }
    return MATCH;
}

struct NameList {
    struct dirent **namelist = {nullptr};
    int direntNum = 0;
};

static void Deleter(struct NameList *arg)
{
    for (int i = 0; i < arg->direntNum; i++) {
        free((arg->namelist)[i]);
        (arg->namelist)[i] = nullptr;
    }
    free(arg->namelist);
}

bool ConnectionDetector::CheckValidDir(const std::string &path)
{
    struct stat buf {
    };
    auto ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        LOGE("stat failed, errno = %{public}d", errno);
        return false;
    }
    if ((buf.st_mode & S_IFMT) != S_IFDIR) {
        LOGE("It is not a dir.");
        return false;
    }
    return true;
}

bool ConnectionDetector::GetConnectionStatus(const std::string &targetDir, const std::string &networkId)
{
    if (!CheckValidDir(SYS_HMDFS_PATH)) {
        return false;
    }
    unique_ptr<struct NameList, decltype(Deleter) *> pNameList = {new (nothrow) struct NameList, Deleter};
    if (pNameList == nullptr) {
        LOGE("Failed to request heap memory.");
        return false;
    }
    int num = scandir(SYS_HMDFS_PATH.c_str(), &(pNameList->namelist), FilterFunc, alphasort);
    pNameList->direntNum = num;
    for (int i = 0; i < num; i++) {
        if ((pNameList->namelist[i])->d_name != targetDir) {
            continue;
        }
        string dest = SYS_HMDFS_PATH + '/' + targetDir;
        if ((pNameList->namelist[i])->d_type == DT_DIR) {
            string statusFile = SYS_HMDFS_PATH + '/' + targetDir + '/' + CONNECTION_STATUS_FILE_NAME;
            if (MatchConnectionGroup(statusFile, networkId)) {
                LOGI("Parse connection status success.");
                return true;
            }
            break;
        }
    }
    return false;
}

uint64_t ConnectionDetector::MocklispHash(const string &str)
{
    uint64_t res = 0;
    constexpr int mocklispHashPos = 5;
    /* Mocklisp hash function. */
    for (auto ch : str) {
        res = (res << mocklispHashPos) - res + (uint64_t)ch;
    }
    return res;
}

int32_t ConnectionDetector::RepeatGetConnectionStatus(const std::string &targetDir, const std::string &networkId)
{
    int retryCount = 0;
    while (retryCount++ < MAX_RETRY - 1 && !GetConnectionStatus(targetDir, networkId)) {
        usleep(CHECK_SESSION_DELAY_TIME);
    }
    return retryCount == MAX_RETRY ? -1 : NO_ERROR;
}

int32_t ConnectionDetector::GetCurrentUserId()
{
    std::vector<int32_t> userIds{};
    auto ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(userIds);
    if (ret != NO_ERROR || userIds.empty()) {
        LOGE("query active os account id failed, ret = %{public}d", ret);
        return INVALID_USER_ID;
    }
    return userIds[0];
}

std::string ConnectionDetector::ParseHmdfsPath()
{
    auto userId = GetCurrentUserId();
    if (userId == INVALID_USER_ID) {
        return "";
    }
    std::string path = HMDFS_PATH;
    size_t pos = path.find(CURRENT_USER_ID_FLAG);
    if (pos == std::string::npos) {
        return "";
    }
    return path.replace(pos, CURRENT_USER_ID_FLAG.length(), std::to_string(userId));
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
