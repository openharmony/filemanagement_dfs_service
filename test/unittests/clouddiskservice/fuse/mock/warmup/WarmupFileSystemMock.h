/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef CLOUD_DISK_WARMUP_FILE_SYSTEM_MOCK_H
#define CLOUD_DISK_WARMUP_FILE_SYSTEM_MOCK_H

#include <algorithm>
#include <cerrno>
#include <cstdarg>
#include <cstring>
#include <dirent.h>
#include <memory>
#include <mutex>
#include <securec.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <utility>
#include <vector>

#include "file_utils.h"

namespace OHOS::FileManagement::CloudDiskService::Test {

class WarmupFileSystemMock final {
public:
    struct StatReply {
        int result{-1};
        int error{ENOENT};
        struct stat value {};
    };

    struct DirectoryReply {
        bool openSucceeds{true};
        int openError{ENOENT};
        int endError{0};
        size_t index{0};
        std::vector<std::string> names;
        struct dirent entry {};
    };

    static void Reset()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        statReplies_.clear();
        lstatReplies_.clear();
        directories_.clear();
        activeDirectories_.clear();
        statCalls_ = 0;
        lstatCalls_ = 0;
        openDirCalls_ = 0;
        readDirCalls_ = 0;
        forceCreateDirectoryCalls_ = 0;
        readFileCalls_ = 0;
        writeFileCalls_ = 0;
        placeholderStateCalls_ = 0;
    }

    static void SetStat(const std::string &path, mode_t mode, ino_t inode = 1)
    {
        StatReply reply;
        reply.result = 0;
        reply.value.st_mode = mode;
        reply.value.st_ino = inode;
        std::lock_guard<std::mutex> lock(mutex_);
        statReplies_[path] = reply;
    }

    static void SetStatError(const std::string &path, int error)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        statReplies_[path] = {-1, error, {}};
    }

    static void SetLstat(const std::string &path, mode_t mode, ino_t inode = 1)
    {
        StatReply reply;
        reply.result = 0;
        reply.value.st_mode = mode;
        reply.value.st_ino = inode;
        std::lock_guard<std::mutex> lock(mutex_);
        lstatReplies_[path] = reply;
    }

    static void SetLstatError(const std::string &path, int error)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        lstatReplies_[path] = {-1, error, {}};
    }

    static void SetDirectory(const std::string &path, std::vector<std::string> names, int endError = 0)
    {
        auto reply = std::make_shared<DirectoryReply>();
        reply->names = std::move(names);
        reply->endError = endError;
        std::lock_guard<std::mutex> lock(mutex_);
        directories_[path] = std::move(reply);
    }

    static void SetOpenDirError(const std::string &path, int error)
    {
        auto reply = std::make_shared<DirectoryReply>();
        reply->openSucceeds = false;
        reply->openError = error;
        std::lock_guard<std::mutex> lock(mutex_);
        directories_[path] = std::move(reply);
    }

    static int Stat(const char *path, struct stat *buffer)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++statCalls_;
        return ApplyStatReply(statReplies_, path, buffer);
    }

    static int Lstat(const char *path, struct stat *buffer)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++lstatCalls_;
        return ApplyStatReply(lstatReplies_, path, buffer);
    }

    static DIR *OpenDir(const char *path)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++openDirCalls_;
        auto it = directories_.find(path == nullptr ? "" : path);
        if (it == directories_.end() || !it->second->openSucceeds) {
            errno = it == directories_.end() ? ENOENT : it->second->openError;
            return nullptr;
        }
        it->second->index = 0;
        DIR *handle = reinterpret_cast<DIR *>(it->second.get());
        activeDirectories_[handle] = it->second;
        return handle;
    }

    static struct dirent *ReadDir(DIR *dir)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++readDirCalls_;
        auto it = activeDirectories_.find(dir);
        if (it == activeDirectories_.end()) {
            errno = EBADF;
            return nullptr;
        }
        auto &reply = *it->second;
        if (reply.index >= reply.names.size()) {
            errno = reply.endError;
            return nullptr;
        }
        reply.entry = {};
        const std::string &name = reply.names[reply.index++];
        (void)strncpy_s(reply.entry.d_name, sizeof(reply.entry.d_name), name.c_str(),
                        std::min(name.size(), sizeof(reply.entry.d_name) - 1));
        return &reply.entry;
    }

    static int CloseDir(DIR *dir)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        activeDirectories_.erase(dir);
        return 0;
    }

    static int Access(const char *, int)
    {
        errno = ENOENT;
        return -1;
    }

    static int Open(const char *, int, ...)
    {
        return ::dup(STDERR_FILENO);
    }

    static int Truncate(int, off_t)
    {
        return 0;
    }

    static uint32_t GetStatCalls()
    {
        return statCalls_;
    }

    static uint32_t GetLstatCalls()
    {
        return lstatCalls_;
    }

    static uint32_t GetOpenDirCalls()
    {
        return openDirCalls_;
    }

    static void RecordForceCreateDirectory()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++forceCreateDirectoryCalls_;
    }

    static uint32_t GetForceCreateDirectoryCalls()
    {
        return forceCreateDirectoryCalls_;
    }

    static int64_t ReadFile(size_t size, void *data)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++readFileCalls_;
        if (data != nullptr) {
            (void)memset_s(data, size, 0, size);
        }
        return static_cast<int64_t>(size);
    }

    static int64_t WriteFile(size_t size)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++writeFileCalls_;
        return static_cast<int64_t>(size);
    }

    static void RecordPlaceholderState()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ++placeholderStateCalls_;
    }

    static uint32_t GetReadFileCalls()
    {
        return readFileCalls_;
    }

    static uint32_t GetWriteFileCalls()
    {
        return writeFileCalls_;
    }

    static uint32_t GetPlaceholderStateCalls()
    {
        return placeholderStateCalls_;
    }

private:
    static int
        ApplyStatReply(const std::unordered_map<std::string, StatReply> &replies, const char *path, struct stat *buffer)
    {
        auto it = replies.find(path == nullptr ? "" : path);
        if (it == replies.end()) {
            errno = ENOENT;
            return -1;
        }
        errno = it->second.error;
        if (it->second.result == 0 && buffer != nullptr) {
            *buffer = it->second.value;
        }
        return it->second.result;
    }

    static inline std::mutex mutex_;
    static inline std::unordered_map<std::string, StatReply> statReplies_;
    static inline std::unordered_map<std::string, StatReply> lstatReplies_;
    static inline std::unordered_map<std::string, std::shared_ptr<DirectoryReply>> directories_;
    static inline std::unordered_map<DIR *, std::shared_ptr<DirectoryReply>> activeDirectories_;
    static inline uint32_t statCalls_{0};
    static inline uint32_t lstatCalls_{0};
    static inline uint32_t openDirCalls_{0};
    static inline uint32_t readDirCalls_{0};
    static inline uint32_t forceCreateDirectoryCalls_{0};
    static inline uint32_t readFileCalls_{0};
    static inline uint32_t writeFileCalls_{0};
    static inline uint32_t placeholderStateCalls_{0};
};

} // namespace OHOS::FileManagement::CloudDiskService::Test

namespace OHOS::FileManagement {

class WarmupFileIoAdapter final {
public:
    static int64_t ReadFile(int fd, off_t offset, size_t size, void *data)
    {
        (void)fd;
        (void)offset;
        return CloudDiskService::Test::WarmupFileSystemMock::ReadFile(size, data);
    }

    static int64_t WriteFile(int fd, const void *data, off_t offset, size_t size)
    {
        (void)fd;
        (void)data;
        (void)offset;
        return CloudDiskService::Test::WarmupFileSystemMock::WriteFile(size);
    }
};

} // namespace OHOS::FileManagement

namespace OHOS::Storage::DistributedFile::Utils {

void ForceCreateDirectoryWarmupMock(const std::string &path, mode_t mode);

} // namespace OHOS::Storage::DistributedFile::Utils

namespace OHOS::FileManagement::CloudDiskService {

int32_t GetFilePlaceholderStateWarmupMock(const std::string &path, uint8_t &placeholderState);

} // namespace OHOS::FileManagement::CloudDiskService

#endif // CLOUD_DISK_WARMUP_FILE_SYSTEM_MOCK_H
