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

#ifndef FILEMANAGEMENT_DFS_SERVICE_CONNECTION_DETECTOR_H
#define FILEMANAGEMENT_DFS_SERVICE_CONNECTION_DETECTOR_H

#include "os_account_manager.h"

#include <fstream>
#include <string>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace std;
const std::string HMDFS_PATH = "/mnt/hmdfs/<currentUserId>/account";
const std::string SYS_HMDFS_PATH = "/sys/fs/hmdfs/";
const std::string CONNECTION_STATUS_FILE_NAME = "status";
const std::string CURRENT_USER_ID_FLAG = "<currentUserId>";
constexpr int32_t MAX_RETRY = 25;
constexpr int32_t CHECK_SESSION_DELAY_TIME = 200000;
constexpr int32_t NO_ERROR = 0;
constexpr int32_t INVALID_USER_ID = -1;
constexpr int DISMATCH = 0;
constexpr int MATCH = 1;
class ConnectionDetector {
public:
    static bool GetConnectionStatus(const std::string &targetDir, const std::string &networkId);
    static int32_t RepeatGetConnectionStatus(const std::string &targetDir, const std::string &networkId);
    static std::string ParseHmdfsPath();
    static uint64_t MocklispHash(const string &str);

private:
    static std::string GetCellByIndex(const std::string &str, int targetIndex);
    static bool MatchConnectionStatus(ifstream &inputFile);
    static bool MatchConnectionGroup(const std::string &fileName, const string &networkId);
    static bool CheckValidDir(const std::string &path);
    static int32_t GetCurrentUserId();
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_CONNECTION_DETECTOR_H
