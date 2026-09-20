/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace {
constexpr int EXPECTED_ARGUMENT_COUNT = 2;
constexpr int EXIT_USAGE = 2;
constexpr int EXIT_VERIFY_FAILED = 1;

void PrintFileInfo(const char *operation, const struct stat &fileStat)
{
    std::printf("[%s] ok: mode=%#o dev=%llu ino=%llu size=%lld\n", operation,
                static_cast<unsigned int>(fileStat.st_mode), static_cast<unsigned long long>(fileStat.st_dev),
                static_cast<unsigned long long>(fileStat.st_ino), static_cast<long long>(fileStat.st_size));
}

void PrintError(const char *operation, const char *path, int error)
{
    std::fprintf(stderr, "[%s] failed: path=%s errno=%d (%s)\n", operation, path, error, std::strerror(error));
}

int VerifyFile(const char *path)
{
    struct stat pathStat {};
    if (lstat(path, &pathStat) != 0) {
        PrintError("lstat", path, errno);
        return EXIT_VERIFY_FAILED;
    }
    PrintFileInfo("lstat", pathStat);
    if (!S_ISREG(pathStat.st_mode)) {
        PrintError("regular-file-check", path, EINVAL);
        return EXIT_VERIFY_FAILED;
    }

    int fd = open(path, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        PrintError("open", path, errno);
        return EXIT_VERIFY_FAILED;
    }
    std::printf("[open] ok: path=%s fd=%d flags=O_RDONLY|O_CLOEXEC|O_NOFOLLOW\n", path, fd);

    int result = 0;
    struct stat fileStat {};
    if (fstat(fd, &fileStat) != 0) {
        PrintError("fstat", path, errno);
        result = EXIT_VERIFY_FAILED;
    } else {
        PrintFileInfo("fstat", fileStat);
        if (pathStat.st_dev != fileStat.st_dev || pathStat.st_ino != fileStat.st_ino) {
            std::fprintf(
                stderr, "[identity] failed: lstat(dev=%llu, ino=%llu) differs from fstat(dev=%llu, ino=%llu)\n",
                static_cast<unsigned long long>(pathStat.st_dev), static_cast<unsigned long long>(pathStat.st_ino),
                static_cast<unsigned long long>(fileStat.st_dev), static_cast<unsigned long long>(fileStat.st_ino));
            result = EXIT_VERIFY_FAILED;
        } else {
            std::printf("[identity] ok: lstat and fstat identify the same file\n");
        }
    }

    if (close(fd) != 0) {
        PrintError("close", path, errno);
        result = EXIT_VERIFY_FAILED;
    } else {
        std::printf("[close] ok: fd=%d\n", fd);
    }

    if (result == 0) {
        std::printf("PASS: %s\n", path);
    } else {
        std::fprintf(stderr, "FAIL: %s\n", path);
    }
    return result;
}
} // namespace

int main(int argc, char *argv[])
{
    if (argc != EXPECTED_ARGUMENT_COUNT) {
        std::fprintf(stderr, "Usage: %s <file-path>\n", argv[0]);
        return EXIT_USAGE;
    }
    return VerifyFile(argv[1]);
}
