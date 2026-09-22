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

#ifndef CLOUD_DISK_FUSE_CONTROLLER_MOCK_H
#define CLOUD_DISK_FUSE_CONTROLLER_MOCK_H

#include <cstdint>
#include <deque>
#include <functional>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

namespace OHOS::FileManagement::CloudDiskService::Test {

struct ControllerMetaBehavior {
    bool exists{false};
    int32_t lookupResult{0};
    uint32_t mode{S_IFREG | 0600};
    uint8_t placeholder{1};
    int32_t relativePathResult{0};
    std::string relativePath;
};

class CloudDiskFuseControllerMock final {
public:
    struct State {
        bool convertResult{true};
        std::string convertedPath{"/mnt/mock/root"};
        uint32_t convertCalls{0};

        int32_t warmupResult{0};
        uint32_t warmupCalls{0};
        bool warmupCancelValue{false};
        uint32_t scheduleFillCalls{0};

        int32_t mountResult{0};
        int mountFd{-1};
        uint32_t mountCalls{0};
        std::deque<int32_t> unmountResults;
        int32_t defaultUnmountResult{0};
        uint32_t unmountCalls{0};

        uint32_t forceCreateDirectoryCalls{0};
        std::string lastDirectory;
        mode_t lastDirectoryMode{0};

        std::function<void()> metaFileLookupHook;
        std::unordered_map<uint32_t, ControllerMetaBehavior> metaBehaviors;
    };

    static State &GetState();
    static void Reset();
    static int32_t NextUnmountResult();
};

} // namespace OHOS::FileManagement::CloudDiskService::Test

#endif // CLOUD_DISK_FUSE_CONTROLLER_MOCK_H
