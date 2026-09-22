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

#ifndef CLOUD_DISK_FUSE_CONTROLLER_H
#define CLOUD_DISK_FUSE_CONTROLLER_H

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace OHOS::FileManagement::CloudDiskService {

struct CloudDiskFuseLifecycleTrace {
    uint32_t startUserCalls{0};
    uint32_t addRootCalls{0};
    uint32_t removeRootCalls{0};
    uint32_t setIdleCallbackCalls{0};
    uint32_t stopCalls{0};
    uint32_t stopForServiceExitCalls{0};
    uint32_t canUnloadCalls{0};
    int32_t userId{-1};
    uint32_t syncFolderIndex{0};
    std::string path;
    std::unordered_map<uint32_t, std::string> roots;
    std::function<void()> idleCallback;
    std::vector<std::string> events;
    bool canUnload{true};
};

inline CloudDiskFuseLifecycleTrace &GetCloudDiskFuseLifecycleTrace()
{
    static CloudDiskFuseLifecycleTrace trace;
    return trace;
}

inline void ResetCloudDiskFuseLifecycleTrace()
{
    GetCloudDiskFuseLifecycleTrace() = {};
}

class CloudDiskFuseController {
public:
    static CloudDiskFuseController &GetInstance()
    {
        static CloudDiskFuseController instance;
        return instance;
    }

    void StartUser(int32_t userId, const std::unordered_map<uint32_t, std::string> &roots)
    {
        auto &trace = GetCloudDiskFuseLifecycleTrace();
        ++trace.startUserCalls;
        trace.userId = userId;
        trace.roots = roots;
        trace.events.emplace_back("StartUser");
    }

    void AddRoot(int32_t userId, uint32_t syncFolderIndex, const std::string &path)
    {
        auto &trace = GetCloudDiskFuseLifecycleTrace();
        ++trace.addRootCalls;
        trace.userId = userId;
        trace.syncFolderIndex = syncFolderIndex;
        trace.path = path;
        trace.events.emplace_back("AddRoot");
    }

    void RemoveRoot(int32_t userId, uint32_t syncFolderIndex)
    {
        auto &trace = GetCloudDiskFuseLifecycleTrace();
        ++trace.removeRootCalls;
        trace.userId = userId;
        trace.syncFolderIndex = syncFolderIndex;
        trace.events.emplace_back("RemoveRoot");
    }

    void SetIdleCallback(std::function<void()> callback)
    {
        auto &trace = GetCloudDiskFuseLifecycleTrace();
        ++trace.setIdleCallbackCalls;
        trace.idleCallback = std::move(callback);
        trace.events.emplace_back(trace.idleCallback ? "SetIdleCallback" : "ClearIdleCallback");
    }

    void Stop()
    {
        auto &trace = GetCloudDiskFuseLifecycleTrace();
        ++trace.stopCalls;
        trace.events.emplace_back("Stop");
    }

    void StopForServiceExit()
    {
        auto &trace = GetCloudDiskFuseLifecycleTrace();
        ++trace.stopForServiceExitCalls;
        trace.events.emplace_back("StopForServiceExit");
    }

    bool CanUnload() const
    {
        auto &trace = GetCloudDiskFuseLifecycleTrace();
        ++trace.canUnloadCalls;
        trace.events.emplace_back("CanUnload");
        return trace.canUnload;
    }
};

} // namespace OHOS::FileManagement::CloudDiskService

#endif // CLOUD_DISK_FUSE_CONTROLLER_H
