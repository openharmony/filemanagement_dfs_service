/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMANAGEMENT_DISK_MONITOR_H
#define OHOS_FILEMANAGEMENT_DISK_MONITOR_H

#include <dirent.h>
#include <fcntl.h>
#include <sys/fanotify.h>

#include "disk_types.h"
#include "ffrt_inner.h"
#include "nocopyable.h"

namespace OHOS::FileManagement::CloudDiskService {
class DiskMonitor : public NoCopyable {
public:
    static DiskMonitor &GetInstance();
    bool StartMonitor(int32_t userId);
    void StopMonitor();

private:
    DiskMonitor() = default;
    ~DiskMonitor() = default;
    bool InitFanotify();
    void InitStatus(int32_t userId);
    void CollectEvents();
    void HandleEvents(int32_t mountFd, char eventsBuf[], ssize_t dataLen);
    void EventProcess(struct fanotify_event_metadata *metaData, const std::string &filePath);
    // events handle
    void HandleCreate(const std::string &filePath);
    void HandleDelete(const std::string &filePath);
    void HandleMoveFrom(const std::string &filePath);
    void HandleMoveTo(const std::string &filePath);
    void HandleCloseWrite(const std::string &filePath);
    void PostEvent(const EventInfo &eventInfo);
    // events filter
    std::pair<uint32_t, bool> GetSyncFolderIndex(const std::string &filePath);
    bool IsInBlockList(const std::string &path);

private:
    // monitor status
    int32_t userId_{-1};
    int32_t fanotifyFd_{-1};
    DIR *mountFp_{nullptr};
    int32_t mountFd_{-1};   // will be closed automatically after close(DIR*)
    bool isRunning_{false};
    ffrt::mutex mutex_;
    // events handle
    EventInfo oldEventInfo_;
    bool oldIsSyncFolder_{false};
    // events filter
    std::string syncFolderPrefix_;
    std::vector<std::string> blockList_;
};
} // namespace OHOS::FileManagement::CloudDiskService
#endif
