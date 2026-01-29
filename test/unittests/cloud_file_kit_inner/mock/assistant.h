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

#ifndef TEST_UNITTESTS_CLOUD_FILE_KIT_INNER_ASSISTANT_H
#define TEST_UNITTESTS_CLOUD_FILE_KIT_INNER_ASSISTANT_H

#include <gmock/gmock.h>
#include <memory>

namespace OHOS::FileManagement::CloudFile {
class Assistant {
public:
    static inline std::shared_ptr<Assistant> ins = nullptr;

    virtual ~Assistant() = default;
    virtual int access(const char *name, int type) = 0;
    virtual int creat(const char *pathname, mode_t mode) = 0;
    virtual int close(int fd) = 0;

    static inline bool mockable = false;
};

class AssistantMock : public Assistant {
public:
    MOCK_METHOD2(access, int(const char *, int));
    MOCK_METHOD2(creat, int(const char *, mode_t));
    MOCK_METHOD1(close, int(int));

    static void EnableMock();
    static void DisableMock();
    static bool IsMockable();
};
} // namespace OHOS::FileManagement::CloudFile
#endif // TEST_UNITTESTS_CLOUD_FILE_KIT_INNER_ASSISTANT_H