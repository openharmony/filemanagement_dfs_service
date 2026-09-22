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

#ifndef CLOUD_DISK_META_FILE_IO_MOCK_H
#define CLOUD_DISK_META_FILE_IO_MOCK_H

#include <algorithm>
#include <cerrno>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <deque>
#include <securec.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace OHOS::FileManagement::CloudDiskService::Test {

class MetaFileIoMock final {
public:
    struct OpenReply {
        int result{-1};
        int error{ENOENT};
    };

    struct FstatReply {
        int result{-1};
        int error{EIO};
        off_t size{0};
    };

    struct ReadReply {
        int64_t result{-1};
        std::vector<uint8_t> bytes;
    };

    static void Reset()
    {
        accessResult_ = -1;
        accessError_ = ENOENT;
        openReplies_.clear();
        fstatReplies_.clear();
        readReplies_.clear();
        openPaths_.clear();
        openFlags_.clear();
        forceCreateDirectoryCalls_ = 0;
        forceCreateDirectoryPath_.clear();
        writeResult_ = -1;
        truncateResult_ = 0;
    }

    static void SetAccessResult(int result, int error = 0)
    {
        accessResult_ = result;
        accessError_ = error;
    }

    static void PushOpenResult(int result, int error = 0)
    {
        openReplies_.push_back({result, error});
    }

    static void PushFstatResult(int result, off_t size, int error = 0)
    {
        fstatReplies_.push_back({result, error, size});
    }

    template<typename T>
    static void PushReadObject(const T &object)
    {
        ReadReply reply;
        reply.result = static_cast<int64_t>(sizeof(T));
        reply.bytes.resize(sizeof(T));
        (void)memcpy_s(reply.bytes.data(), sizeof(T), &object, sizeof(T));
        readReplies_.push_back(std::move(reply));
    }

    static void PushReadResult(int64_t result)
    {
        readReplies_.push_back({result, {}});
    }

    static void SetWriteResult(int64_t result)
    {
        writeResult_ = result;
    }

    static int Access(const char *, int)
    {
        errno = accessError_;
        return accessResult_;
    }

    static int Open(const char *path, int flags, ...)
    {
        openPaths_.emplace_back(path == nullptr ? "" : path);
        openFlags_.push_back(flags);
        if (openReplies_.empty()) {
            errno = ENOENT;
            return -1;
        }
        OpenReply reply = openReplies_.front();
        openReplies_.pop_front();
        errno = reply.error;
        return reply.result < 0 ? reply.result : ::dup(STDERR_FILENO);
    }

    static int Fstat(int, struct stat *buffer)
    {
        if (fstatReplies_.empty()) {
            errno = EIO;
            return -1;
        }
        FstatReply reply = fstatReplies_.front();
        fstatReplies_.pop_front();
        errno = reply.error;
        if (reply.result == 0 && buffer != nullptr) {
            *buffer = {};
            buffer->st_size = reply.size;
        }
        return reply.result;
    }

    static int Truncate(int, off_t)
    {
        return truncateResult_;
    }

    static int64_t ReadFile(int, off_t, size_t size, void *data)
    {
        if (readReplies_.empty()) {
            return -1;
        }
        ReadReply reply = std::move(readReplies_.front());
        readReplies_.pop_front();
        if (reply.result > 0 && data != nullptr && !reply.bytes.empty()) {
            size_t copySize = std::min({size, reply.bytes.size(), static_cast<size_t>(reply.result)});
            (void)memcpy_s(data, copySize, reply.bytes.data(), copySize);
        }
        return reply.result;
    }

    static int64_t WriteFile(int, const void *, off_t, size_t)
    {
        return writeResult_;
    }

    static void RecordForceCreateDirectory(const std::string &path)
    {
        ++forceCreateDirectoryCalls_;
        forceCreateDirectoryPath_ = path;
    }

    static const std::vector<std::string> &GetOpenPaths()
    {
        return openPaths_;
    }

    static const std::vector<int> &GetOpenFlags()
    {
        return openFlags_;
    }

    static uint32_t GetForceCreateDirectoryCalls()
    {
        return forceCreateDirectoryCalls_;
    }

    static const std::string &GetForceCreateDirectoryPath()
    {
        return forceCreateDirectoryPath_;
    }

private:
    static inline int accessResult_{-1};
    static inline int accessError_{ENOENT};
    static inline std::deque<OpenReply> openReplies_;
    static inline std::deque<FstatReply> fstatReplies_;
    static inline std::deque<ReadReply> readReplies_;
    static inline std::vector<std::string> openPaths_;
    static inline std::vector<int> openFlags_;
    static inline uint32_t forceCreateDirectoryCalls_{0};
    static inline std::string forceCreateDirectoryPath_;
    static inline int64_t writeResult_{-1};
    static inline int truncateResult_{0};
};

} // namespace OHOS::FileManagement::CloudDiskService::Test

namespace OHOS::FileManagement {

class MetaFileIoAdapter final {
public:
    static int64_t ReadFile(int fd, off_t offset, size_t size, void *data)
    {
        return CloudDiskService::Test::MetaFileIoMock::ReadFile(fd, offset, size, data);
    }

    static int64_t WriteFile(int fd, const void *data, off_t offset, size_t size)
    {
        return CloudDiskService::Test::MetaFileIoMock::WriteFile(fd, data, offset, size);
    }
};

} // namespace OHOS::FileManagement

namespace OHOS::Storage::DistributedFile::Utils {

void ForceCreateDirectoryMetaFileMock(const std::string &path, mode_t mode);

} // namespace OHOS::Storage::DistributedFile::Utils

#endif // CLOUD_DISK_META_FILE_IO_MOCK_H
