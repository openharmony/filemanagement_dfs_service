/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef UTILS_DIRECTORY_H
#define UTILS_DIRECTORY_H

#include <string>
#include <sys/types.h>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <dirent.h>
#include <iostream>

#include "nlohmann/json.hpp"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Utils {
const std::string ORGPKGNAME = "distributedfile_daemon";
const std::string SOFTBUSNAME = "dsoftbus";
const std::string DISTRIBUTEDFILE_CONNECT_BEHAVIOR = "DISTRIBUTEDFILE_DAEMON";

enum Uid {
    UID_ROOT = 0,
    UID_SYSTEM = 1000,
    UID_MEDIA_RW = 1023,
};

enum class BizScene : int32_t {
    DFS_CONNECT = 0x1,
};

enum class BizStage : int32_t {
    DFS_OPEN_SESSION = 0x1,
    DFS_SENDFILE = 0x2
};

enum class StageRes : int32_t {
    STAGE_SUCCESS = 0x1,
    STAGE_FAIL = 0x2
};

enum class BizState : int32_t {
    BIZ_STATE_START = 0x1,
    BIZ_STATE_END = 0x2,
    BIZ_STATE_CANCEL = 0x3
};

struct RadarInfo {
    std::string funcName;
    std::string localSessionName;
    std::string peerSessionName;
    int32_t errCode;
    StageRes state;
};


#define DEMO_SYNC_SYS_EVENT(eventName, type, ...)    \
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::FILEMANAGEMENT, eventName,   \
                    type, ##__VA_ARGS__)    \

void SysEventWrite(std::string &uid);
void SysEventFileParse(int64_t maxTime);

void RadarDotsReportOpenSession(struct RadarInfo &info);
void RadarDotsOpenSession(const std::string funcName, const std::string &sessionName,
    const std::string &peerSssionName, int32_t errCode, StageRes state);
void RadarDotsReportSendFile(struct RadarInfo &info);
void RadarDotsSendFile(const std::string funcName, const std::string &sessionName,
    const std::string &peerSssionName, int32_t errCode, StageRes state);

std::string GetAnonyString(const std::string &value);

void ForceCreateDirectory(const std::string &path);
void ForceCreateDirectory(const std::string &path, mode_t mode);
void ForceCreateDirectory(const std::string &path, mode_t mode, uid_t uid, gid_t gid);

void ForceRemoveDirectory(const std::string &path);
bool IsFile(const std::string &path);
bool IsFolder(const std::string &name);
std::vector<std::string> GetFilePath(const std::string &name);
int32_t ChangeOwnerRecursive(const std::string &path, uid_t uid, gid_t gid);
bool IsInt32(const nlohmann::json &jsonObj, const std::string &key);
} // namespace Utils
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // UTILS_DIRECTORY_H