/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "utils_directory.h"

#include <cstring>
#include <dirent.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

#include "dfs_error.h"
#include "directory_ex.h"
#include "hisysevent.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Utils {
using namespace std;

namespace {
    static const uint32_t STAT_MODE_DIR = 0771;
}

std::string GetAnonyString(const std::string &value)
{
    constexpr size_t shortIdLength = 20;
    constexpr size_t plaintextLength = 4;
    constexpr size_t minIdLength = 3;
    std::string res;
    std::string tmpStr("******");
    size_t strLen = value.length();
    if (strLen < minIdLength) {
        return tmpStr;
    }

    if (strLen <= shortIdLength) {
        res += value[0];
        res += tmpStr;
        res += value[strLen - 1];
    } else {
        res.append(value, 0, plaintextLength);
        res += tmpStr;
        res.append(value, strLen - plaintextLength, plaintextLength);
    }
    return res;
}

void SysEventWrite(string &uid)
{
    if (uid.empty()) {
        LOGE("uid is empty.");
        return;
    }
    int32_t ret = DEMO_SYNC_SYS_EVENT("PERMISSION_EXCEPTION",
        HiviewDFX::HiSysEvent::EventType::SECURITY,
        "CALLER_UID", uid,
        "PERMISSION_NAME", "account");
    if (ret != ERR_OK) {
        LOGE("report PERMISSION_EXCEPTION error %{public}d", ret);
    }
}

void SysEventFileParse(int64_t maxTime)
{
    int32_t ret = DEMO_SYNC_SYS_EVENT("INDEX_FILE_PARSE",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "MAX_TIME", maxTime);
    if (ret != ERR_OK) {
        LOGE("report INDEX_FILE_PARSE error %{public}d", ret);
    }
}

void RadarDotsReportOpenSession(struct RadarInfo &info)
{
    int32_t res = ERR_OK;
    if (info.state == StageRes::STAGE_SUCCESS) {
        res = DEMO_SYNC_SYS_EVENT(DISTRIBUTEDFILE_CONNECT_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "TO_CALL_PKG", SOFTBUSNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(BizScene::DFS_CONNECT),
            "BIZ_STAGE", static_cast<int32_t>(BizStage::DFS_OPEN_SESSION),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_START),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCCESS),
            "LOCAL_SESS_NAME", info.localSessionName,
            "PEER_SESS_NAME", info.peerSessionName);
    } else {
        res = DEMO_SYNC_SYS_EVENT(DISTRIBUTEDFILE_CONNECT_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "TO_CALL_PKG", SOFTBUSNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(BizScene::DFS_CONNECT),
            "BIZ_STAGE", static_cast<int32_t>(BizStage::DFS_OPEN_SESSION),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
            "LOCAL_SESS_NAME", info.localSessionName,
            "PEER_SESS_NAME", info.peerSessionName,
            "ERROR_CODE", std::abs(info.errCode));
    }
    if (res != ERR_OK) {
        LOGE("report RadarDotsReportOpenSession error %{public}d", res);
    }
}

void RadarDotsOpenSession(const std::string funcName, const std::string &sessionName,
    const std::string &peerSssionName, int32_t errCode, StageRes state)
{
    struct RadarInfo info = {
        .funcName = funcName,
        .localSessionName = sessionName,
        .peerSessionName = peerSssionName,
        .errCode = errCode,
        .state = state,
    };
    RadarDotsReportOpenSession(info);
}

void RadarDotsReportSendFile(struct RadarInfo &info)
{
    int32_t res = ERR_OK;
    if (info.state == StageRes::STAGE_SUCCESS) {
        res = DEMO_SYNC_SYS_EVENT(DISTRIBUTEDFILE_CONNECT_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "TO_CALL_PKG", SOFTBUSNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(BizScene::DFS_CONNECT),
            "BIZ_STAGE", static_cast<int32_t>(BizStage::DFS_SENDFILE),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_SUCCESS),
            "LOCAL_SESS_NAME", info.localSessionName,
            "PEER_SESS_NAME", info.peerSessionName);
    } else {
        res = DEMO_SYNC_SYS_EVENT(DISTRIBUTEDFILE_CONNECT_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            "ORG_PKG", ORGPKGNAME,
            "TO_CALL_PKG", SOFTBUSNAME,
            "FUNC", info.funcName,
            "BIZ_SCENE", static_cast<int32_t>(BizScene::DFS_CONNECT),
            "BIZ_STAGE", static_cast<int32_t>(BizStage::DFS_SENDFILE),
            "BIZ_STATE", static_cast<int32_t>(BizState::BIZ_STATE_END),
            "STAGE_RES", static_cast<int32_t>(StageRes::STAGE_FAIL),
            "LOCAL_SESS_NAME", info.localSessionName,
            "PEER_SESS_NAME", info.peerSessionName,
            "ERROR_CODE", std::abs(info.errCode));
    }
    if (res != ERR_OK) {
        LOGE("report RadarDotsReportSendFile error %{public}d", res);
    }
}

void RadarDotsSendFile(const std::string funcName, const std::string &sessionName,
    const std::string &peerSssionName, int32_t errCode, StageRes state)
{
    struct RadarInfo info = {
        .funcName = funcName,
        .localSessionName = sessionName,
        .peerSessionName = peerSssionName,
        .errCode = errCode,
        .state = state,
    };
    RadarDotsReportSendFile(info);
}

void ForceCreateDirectory(const string &path, function<void(const string &)> onSubDirCreated)
{
    string::size_type index = 0;
    do {
        string subPath;
        index = path.find('/', index + 1);
        if (index == string::npos) {
            subPath = path;
        } else {
            subPath = path.substr(0, index);
        }

        if (access(subPath.c_str(), F_OK) != 0) {
            if (mkdir(subPath.c_str(), STAT_MODE_DIR) != 0) {
                LOGE("failed to mkdir, errno:%{public}d", errno);
                return;
            }
            onSubDirCreated(subPath);
        }
    } while (index != string::npos);
}

void ForceCreateDirectory(const string &path)
{
    ForceCreateDirectory(path, nullptr);
}

void ForceCreateDirectory(const string &path, mode_t mode)
{
    ForceCreateDirectory(path, [mode](const string &subPath) {
        if (chmod(subPath.c_str(), mode) == -1) {
            LOGE("failed to chmod, errno:%{public}d", errno);
        }
    });
}

void ForceCreateDirectory(const string &path, mode_t mode, uid_t uid, gid_t gid)
{
    ForceCreateDirectory(path, [mode, uid, gid](const string &subPath) {
        if (chmod(subPath.c_str(), mode) == -1 || chown(subPath.c_str(), uid, gid) == -1) {
            LOGE("failed to chmod or chown, errno:%{public}d", errno);
        }
    });
}

void ForceRemoveDirectory(const string &path)
{
    if (!OHOS::ForceRemoveDirectory(path)) {
        LOGE("failed to forcibly remove directory, errno:%{public}d", errno);
    }
}

bool ForceRemoveDirectoryDeepFirst(const string& path)
{
    string subPath;
    bool ret = true;
    DIR *dir = opendir(path.c_str());
    if (dir == nullptr) {
        LOGE("opendir failed, path = %{public}s, err:%{public}d", GetAnonyString(path).c_str(), errno);
        return false;
    }

    while (true) {
        struct dirent *ptr = readdir(dir);
        if (ptr == nullptr) {
            break;
        }

        // current dir OR parent dir
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        subPath = IncludeTrailingPathDelimiter(path) + string(ptr->d_name);
        if (ptr->d_type == DT_DIR) {
            if (!ForceRemoveDirectoryDeepFirst(subPath)) {
                ret = false;
            }
        } else if (access(subPath.c_str(), F_OK) == 0) {
            if (remove(subPath.c_str()) != 0) {
                closedir(dir);
                LOGE("remove failed, subPath = %{public}s, err:%{public}d",
                    GetAnonyString(subPath).c_str(), errno);
                return false;
            }
        }
    }
    closedir(dir);

    string currentPath = ExcludeTrailingPathDelimiter(path);
    if (access(currentPath.c_str(), F_OK) == 0) {
        if (remove(currentPath.c_str()) != 0) {
            LOGE("remove failed, currentPath = %{public}s, err:%{public}d",
                GetAnonyString(currentPath).c_str(), errno);
            return false;
        }
    }

    return ret && (access(path.c_str(), F_OK) != 0);
}

bool IsFile(const std::string &path)
{
    if (path.empty()) {
        return false;
    }
    struct stat buf = {};
    if (stat(path.c_str(), &buf) != 0) {
        LOGE("stat failed, errno = %{public}d", errno);
        return false;
    }
    return S_ISREG(buf.st_mode);
}

bool IsFolder(const std::string &name)
{
    if (name.empty()) {
        return false;
    }
    struct stat buf = {};
    if (stat(name.c_str(), &buf) != 0) {
        LOGE("stat failed, errno = %{public}d", errno);
        return false;
    }
    return S_ISDIR(buf.st_mode);
}

std::vector<std::string> GetFilePath(const std::string &name)
{
    std::vector<std::string> path;
    if (!IsFolder(name)) {
        path.emplace_back(name);
        return path;
    }
    auto dir = opendir(name.data());
    struct dirent *ent = nullptr;
    if (dir) {
        while ((ent = readdir(dir)) != nullptr) {
            auto tmpPath = std::string(name).append("/").append(ent->d_name);
            if (strcmp(ent->d_name, "..") == 0 || strcmp(ent->d_name, ".") == 0) {
                continue;
            } else if (IsFolder(tmpPath)) {
                auto dirPath = GetFilePath(tmpPath);
                path.insert(path.end(), dirPath.begin(), dirPath.end());
            } else {
                path.emplace_back(tmpPath);
            }
        }
        closedir(dir);
    }
    return path;
}

int32_t ChangeOwnerRecursive(const std::string &path, uid_t uid, gid_t gid)
{
    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(path.c_str()), &closedir);
    if (dir == nullptr) {
        LOGE("Directory is null");
        return -1;
    }

    struct dirent *entry = nullptr;
    while ((entry = readdir(dir.get())) != nullptr) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            std::string subPath = path + "/" + entry->d_name;
            if (chown(subPath.c_str(), uid, gid) == -1) {
                LOGE("Change owner recursive failed");
                return -1;
            }
            return ChangeOwnerRecursive(subPath, uid, gid);
        } else {
            std::string filePath = path + "/" + entry->d_name;
            if (chown(filePath.c_str(), uid, gid) == -1) {
                LOGE("Change owner recursive failed");
                return -1;
            }
        }
    }
    return 0;
}

bool IsInt32(const nlohmann::json &jsonObj, const std::string &key)
{
    bool res = jsonObj.contains(key) && jsonObj[key].is_number_integer() && jsonObj[key] >= INT32_MIN &&
        jsonObj[key] <= INT32_MAX;
    if (!res) {
        LOGE("the key %{public}s in jsonObj is invalid.", key.c_str());
    }
    return res;
}
} // namespace Utils
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS