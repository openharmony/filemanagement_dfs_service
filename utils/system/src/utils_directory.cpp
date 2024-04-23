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

#include "utils_directory.h"

#include <cstring>
#include <dirent.h>
#include <sys/types.h>
#include <system_error>
#include <unistd.h>

#include "directory_ex.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace Utils {
using namespace std;

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
            if (mkdir(subPath.c_str(), (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)) != 0 && errno != EEXIST) {
                throw system_error(errno, system_category());
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
            throw system_error(errno, system_category());
        }
    });
}

void ForceCreateDirectory(const string &path, mode_t mode, uid_t uid, gid_t gid)
{
    ForceCreateDirectory(path, [mode, uid, gid](const string &subPath) {
        if (chmod(subPath.c_str(), mode) == -1 || chown(subPath.c_str(), uid, gid) == -1) {
            throw system_error(errno, system_category());
        }
    });
}

void ForceRemoveDirectory(const string &path)
{
    if (!OHOS::ForceRemoveDirectory(path)) {
        throw system_error(errno, system_category());
    }
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
} // namespace Utils
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS