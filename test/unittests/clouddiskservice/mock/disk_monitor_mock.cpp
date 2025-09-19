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
#include "disk_monitor.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;

DiskMonitor &DiskMonitor::GetInstance()
{
    static DiskMonitor instance_;
    return instance_;
}

bool DiskMonitor::StartMonitor(int32_t userId)
{
    return true;
}

void DiskMonitor::StopMonitor()
{
}

void DiskMonitor::InitStatus(int32_t userId)
{
}

bool DiskMonitor::InitFanotify()
{
    return true;
}

void DiskMonitor::CollectEvents()
{
}

void DiskMonitor::HandleEvents(int32_t mountFd, char eventsBuf[], size_t dataLen)
{
}

void DiskMonitor::EventProcess(struct fanotify_event_metadata *metaData, const string &filePath)
{
}

void DiskMonitor::HandleCreate(const string &filePath)
{
}

void DiskMonitor::HandleDelete(const string &filePath)
{
}

void DiskMonitor::HandleMoveFrom(const string &filePath)
{
}

void DiskMonitor::HandleMoveTo(const string &filePath)
{
}

void DiskMonitor::HandleCloseWrite(const string &filePath)
{
}

pair<uint32_t, bool> DiskMonitor::GetSyncFolderIndex(const string &filePath)
{
    return {0, false};
}

bool DiskMonitor::IsInBlockList(const string &path)
{
    return true;
}

void DiskMonitor::PostEvent(const EventInfo &eventInfo)
{
}
} // namespace OHOS::FileManagement::CloudDiskService
