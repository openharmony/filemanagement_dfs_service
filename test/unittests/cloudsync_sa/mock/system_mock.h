/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef OHOS_FILEMGMT_CLOUD_SYNC_SA_SYSTEM_MOCK_H
#define OHOS_FILEMGMT_CLOUD_SYNC_SA_SYSTEM_MOCK_H

#include <sys/file.h>
#include <vector>
#include <gmock/gmock.h>

namespace OHOS::FileManagement::CloudSync {

class System {
public:
    static inline std::shared_ptr<System> ins = nullptr;
    static inline bool mockable = false;
    static inline bool exited = false;

public:
    virtual ~System() = default;
    virtual char *realpath(const char *path, char *resolved_path) = 0;
    virtual int getxattr(const char *path, const char *name, void *value, size_t size) = 0;
};

class SystemMock : public System {
public:
    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();
    MOCK_METHOD2(realpath, char*(const char *path, char *resolved_path));
    MOCK_METHOD4(getxattr, int(const char *path, const char *name, void *value, size_t size));
};
} // namespace OHOS::FileManagement::CloudSync
#endif // OHOS_FILEMGMT_CLOUD_SYNC_SA_SYSTEM_MOCK_H