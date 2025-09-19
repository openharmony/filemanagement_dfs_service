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

#include <map>
#include <unistd.h>
#include <vector>

#include "cloud_disk_service_syncfolder.h"
#include "cloud_disk_sync_folder.h"
#include "disk_utils.h"
#include "ffrt_inner.h"
#include "utils_log.h"

namespace OHOS::FileManagement::CloudDiskService {
using namespace std;
namespace {
constexpr uint32_t INIT_FLAGS = FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME | FAN_UNLIMITED_QUEUE;
constexpr uint32_t INIT_EVENT_FLAGS = O_RDONLY | O_LARGEFILE;
constexpr uint32_t MARK_FLAGS = FAN_MARK_ADD | FAN_MARK_FILESYSTEM;
constexpr uint64_t MARK_MASK = FAN_CREATE | FAN_DELETE | FAN_MOVED_FROM | FAN_MOVED_TO | FAN_ONDIR | FAN_CLOSE_WRITE;
constexpr int32_t INVALID_SYNC_FOLDER = 0;
constexpr uint32_t EVENTS_BUF_SIZE = 4096;
const string MOUNT_PATH = "/data/service";
const string DATA_SERVICE_EL2 = "/data/service/el2/";
const string HMDFS_DOCS = "/hmdfs/account/files/Docs";
const vector<string> BLOCK_DIRS = {"/.Trash", "/.Recent", "/.thumbs", "/HO_DATA_EXT_MISC"};
} // namespace

DiskMonitor &DiskMonitor::GetInstance()
{
    static DiskMonitor instance_;
    return instance_;
}

bool DiskMonitor::StartMonitor(int32_t userId)
{
    {
        lock_guard<ffrt::mutex> lck(mutex_);
        if (isRunning_) {
            LOGE("monitor is running, do not start again");
            return false;
        }
        LOGI("Start monitor: %{public}d", userId);
        if (!InitFanotify()) {
            return false;
        }
        InitStatus(userId);
    }
    ffrt::submit([this] { this->CollectEvents(); });
    LOGI("Start monitor finished");
    return true;
}

void DiskMonitor::StopMonitor()
{
    lock_guard<ffrt::mutex> lck(mutex_);
    userId_ = -1;
    isRunning_ = false;
    syncFolderPrefix_.clear();
    blockList_.clear();
    // clean fd
    DiskUtils::CloseFd(fanotifyFd_);
    DiskUtils::CloseDir(mountFp_);
}

void DiskMonitor::InitStatus(int32_t userId)
{
    userId_ = userId;
    isRunning_ = true;
    syncFolderPrefix_ = DATA_SERVICE_EL2 + to_string(userId) + HMDFS_DOCS;
    for (auto &it : BLOCK_DIRS) {
        blockList_.push_back(DATA_SERVICE_EL2 + to_string(userId) + HMDFS_DOCS + it);
    }
}

bool DiskMonitor::InitFanotify()
{
    LOGI("init start");
    fanotifyFd_ = fanotify_init(INIT_FLAGS, INIT_EVENT_FLAGS);
    if (fanotifyFd_ == -1) {
        LOGE("disk monitor init failed, errno: %{public}d", errno);
        return false;
    }
    if (fanotify_mark(fanotifyFd_, MARK_FLAGS, MARK_MASK, AT_FDCWD, MOUNT_PATH.c_str()) == -1) {
        LOGE("fanotify_mark failed, errno: %{public}d", errno);
        DiskUtils::CloseFd(fanotifyFd_);
        return false;
    }
    mountFp_ = opendir(MOUNT_PATH.c_str());
    if (mountFp_ == nullptr) {
        LOGE("open dir failed, errno: %{public}d", errno);
        DiskUtils::CloseFd(fanotifyFd_);
        return false;
    }
    mountFd_ = dirfd(mountFp_);
    if (mountFd_ == -1) {
        LOGE("get dirfd failed, errno: %{public}d", errno);
        DiskUtils::CloseFd(fanotifyFd_);
        DiskUtils::CloseDir(mountFp_);
        return false;
    }
    LOGI("Init end");
    return true;
}

void DiskMonitor::CollectEvents()
{
    while (isRunning_) {
        char eventsBuf[EVENTS_BUF_SIZE]{};
        ssize_t dataLen = read(fanotifyFd_, eventsBuf, sizeof(eventsBuf));
        if (dataLen == -1 && errno != EAGAIN) {
            LOGE("read failed, errno: %{public}d", errno);
            continue;
        }
        if (dataLen == 0) {
            LOGI("no data to read");
            continue;
        }

        HandleEvents(mountFd_, eventsBuf, dataLen);
    }
}

void DiskMonitor::HandleEvents(int32_t mountFd, char eventsBuf[], size_t dataLen)
{
    for (struct fanotify_event_metadata *metaData = reinterpret_cast<struct fanotify_event_metadata *>(eventsBuf);
         FAN_EVENT_OK(metaData, dataLen); metaData = FAN_EVENT_NEXT(metaData, dataLen)) {
        struct fanotify_event_info_fid *fID = reinterpret_cast<struct fanotify_event_info_fid *>(metaData + 1);
        struct file_handle *fileHandle = reinterpret_cast<struct file_handle *>(fID->handle);
        if (fileHandle == nullptr) {
            LOGE("fileHandle is null");
            continue;
        }

        // get file name
        string fileName;
        if (!DiskUtils::ExtractFileName(fID->hdr.info_type, fileName, fileHandle)) {
            continue;
        }

        // get file path
        int eventFd = open_by_handle_at(mountFd, fileHandle, O_RDONLY);
        if (eventFd == -1) {
            continue;
        }
        string filePath = DiskUtils::GetFilePath(eventFd, fileName);
        if (filePath.empty()) {
            close(eventFd);
            continue;
        }

        EventProcess(metaData, filePath);
        close(eventFd);
    }
}

void DiskMonitor::EventProcess(struct fanotify_event_metadata *metaData, const string &filePath)
{
    if (metaData->mask & FAN_CREATE) {
        HandleCreate(filePath);
    } else if (metaData->mask & FAN_DELETE) {
        HandleDelete(filePath);
    } else if (metaData->mask & FAN_MOVED_FROM) {
        HandleMoveFrom(filePath);
    } else if (metaData->mask & FAN_MOVED_TO) {
        HandleMoveTo(filePath);
    } else if (metaData->mask & FAN_CLOSE_WRITE) {
        HandleCloseWrite(filePath);
    }
}

void DiskMonitor::HandleCreate(const string &filePath)
{
    auto [syncFolderIndex, _] = GetSyncFolderIndex(filePath);
    if (syncFolderIndex == INVALID_SYNC_FOLDER) {
        return;
    }
    auto eventInfo = EventInfo(userId_, syncFolderIndex, OperationType::CREATE, filePath);
    PostEvent(eventInfo);
}

void DiskMonitor::HandleDelete(const string &filePath)
{
    auto [syncFolderIndex, isSyncFolder] = GetSyncFolderIndex(filePath);
    if (syncFolderIndex == INVALID_SYNC_FOLDER) {
        return;
    }
    OperationType type = isSyncFolder ? OperationType::SYNC_FOLDER_INVALID : OperationType::DELETE;
    auto eventInfo = EventInfo(userId_, syncFolderIndex, type, filePath);
    PostEvent(eventInfo);
}

void DiskMonitor::HandleMoveFrom(const string &filePath)
{
    auto [syncFolderIndex, isSyncFolder] = GetSyncFolderIndex(filePath);
    if (syncFolderIndex == INVALID_SYNC_FOLDER) {
        return;
    }
    {
        lock_guard<ffrt::mutex> lck(mutex_);
        oldEventInfo_ = EventInfo(userId_, syncFolderIndex, OperationType::MOVE_FROM, filePath);
        oldIsSyncFolder_ = isSyncFolder;
    }
}

/*
 * old is not in sync folder, new is not in sync folder, skip
 * old is not in sync folder, new is in sync folder, create->new
 * old is in sync folder, new is not in sync folder:
 *  1. old is sync folder, sync_folder_invalid->old
 *  2. old is not sync folder, delete->old
 * old is in sync folder, new is in sync folder, sync root is the same, move_from->old + move_to->new
 * old is in sync folder, new is in sync folder, sync root is not the same
 *  1. old is sync folder, sync_folder_invalid->old + create->new
 *  2. old is not sync folder, delete->old + create->new
 */
void DiskMonitor::HandleMoveTo(const string &filePath)
{
    auto [syncFolderIndex, _] = GetSyncFolderIndex(filePath);
    auto eventInfo = EventInfo(userId_, syncFolderIndex, OperationType::MOVE_TO, filePath);
    do {
        // old is not in sync folder
        if (oldEventInfo_.syncFolderIndex == INVALID_SYNC_FOLDER) {
            if (eventInfo.syncFolderIndex == INVALID_SYNC_FOLDER) {
                break;
            }
            eventInfo.operateType = OperationType::CREATE;
            PostEvent(eventInfo);
            break;
        }

        // old is in sync folder, new is not in sync folder
        if (eventInfo.syncFolderIndex == INVALID_SYNC_FOLDER) {
            OperationType type = oldIsSyncFolder_ ? OperationType::SYNC_FOLDER_INVALID : OperationType::DELETE;
            oldEventInfo_.operateType = type;
            PostEvent(oldEventInfo_);
            break;
        }

        // old is in sync folder, new is in sync folder, sync root is the same
        if (oldEventInfo_.syncFolderIndex == eventInfo.syncFolderIndex) {
            oldEventInfo_.operateType = OperationType::MOVE_FROM;
            eventInfo.operateType = OperationType::MOVE_TO;
            PostEvent(oldEventInfo_);
            PostEvent(eventInfo);
            break;
        }

        // old is in sync folder, new is in sync folder, sync root is not the same
        OperationType type = oldIsSyncFolder_ ? OperationType::SYNC_FOLDER_INVALID : OperationType::DELETE;
        oldEventInfo_.operateType = type;
        eventInfo.operateType = OperationType::CREATE;
        PostEvent(oldEventInfo_);
        PostEvent(eventInfo);
    } while (0);
    {
        lock_guard<ffrt::mutex> lck(mutex_);
        oldEventInfo_.Reset();
        oldIsSyncFolder_ = false;
    }
}

void DiskMonitor::HandleCloseWrite(const string &filePath)
{
    auto [syncFolderIndex, _] = GetSyncFolderIndex(filePath);
    if (syncFolderIndex == INVALID_SYNC_FOLDER) {
        return;
    }
    auto eventInfo = EventInfo(userId_, syncFolderIndex, OperationType::CLOSE_WRITE, filePath);
    PostEvent(eventInfo);
}

pair<uint32_t, bool> DiskMonitor::GetSyncFolderIndex(const string &filePath)
{
    if (IsInBlockList(filePath)) {
        return {INVALID_SYNC_FOLDER, false};
    }
    for (auto &it : CloudDiskSyncFolder::GetInstance().GetSyncFolderMap()) {
        if (filePath == it.second.path) {
            return {it.first, true};
        }
        // sync foler is parent dir
        if (filePath.find(it.second.path + "/") == 0) {
            return {it.first, false};
        }
    }
    return {INVALID_SYNC_FOLDER, false};
}

bool DiskMonitor::IsInBlockList(const string &path)
{
    if (path.find(syncFolderPrefix_) != 0) {
        return true;
    }
    for (auto &it : blockList_) {
        if (path.find(it) == 0) {
            return true;
        }
    }
    return false;
}

void DiskMonitor::PostEvent(const EventInfo &eventInfo)
{
    LOGD("opt: %{public}d path: %{public}s name: %{public}s", static_cast<int>(eventInfo.operateType),
         eventInfo.path.c_str(), eventInfo.name.c_str());
    CloudDiskServiceSyncFolder::SetSyncFolderChanges(eventInfo);
}
} // namespace OHOS::FileManagement::CloudDiskService
