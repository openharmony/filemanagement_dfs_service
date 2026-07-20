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
#ifndef TEST_UNITTEST_CLOUD_DISK_SERVICE_SYNC_FOLDER_ASSISTANT_H
#define TEST_UNITTEST_CLOUD_DISK_SERVICE_SYNC_FOLDER_ASSISTANT_H

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gmock/gmock.h>

namespace OHOS::FileManagement::CloudDiskService {
class SyncFolderAssistant {
public:
    static inline std::shared_ptr<SyncFolderAssistant> ins = nullptr;

    virtual ~SyncFolderAssistant() = default;
    virtual char *realpath(const char *path, char *resolvedPath) = 0;
    virtual ssize_t getxattr(const char *path, const char *name, void *value, size_t size) = 0;
    virtual int unlink(const char *pathname) = 0;
    virtual int lstat(const char *path, struct stat *buf) = 0;
};

class SyncFolderAssistantMock : public SyncFolderAssistant {
public:
    MOCK_METHOD2(realpath, char *(const char *, char *));
    MOCK_METHOD4(getxattr, ssize_t(const char *, const char *, void *, size_t));
    MOCK_METHOD1(unlink, int(const char *));
    MOCK_METHOD2(lstat, int(const char *, struct stat *));
};
} // namespace OHOS::FileManagement::CloudDiskService

#endif // TEST_UNITTEST_CLOUD_DISK_SERVICE_SYNC_FOLDER_ASSISTANT_H
