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

#include "copy/file_copy_listener.h"

#include <cinttypes>
#include <unistd.h>

#include "copy/file_size_utils.h"
#include "dfs_error.h"
#include "dfs_radar.h"
#include "utils_log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD00430B
#define LOG_TAG "distributedfile_daemon"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace FileManagement;
using namespace std;
static constexpr int BUF_SIZE = 1024;
static constexpr std::chrono::milliseconds NOTIFY_PROGRESS_DELAY(100);
static constexpr int SLEEP_TIME_US = 100000;
constexpr uint64_t DEFAULT_SPEED_SIZE = 2.8 * 1024 * 1024;

FileCopyLocalListener::FileCopyLocalListener(const std::string &srcPath,
    bool isFile, const ProcessCallback &processCallback) : isFile_(isFile), processCallback_(processCallback)
{
    if (processCallback_ == nullptr) {
        LOGI("processCallback is nullptr");
        return;
    }

    notifyFd_ = inotify_init();
    if (notifyFd_ < 0) {
        LOGE("Failed to init inotify, errno:%{public}d", errno);
        return;
    }
    eventFd_ = eventfd(0, EFD_CLOEXEC);
    if (eventFd_ < 0) {
        LOGE("Failed to init eventFd, errno:%{public}d", errno);
        return;
    }
}

FileCopyLocalListener::~FileCopyLocalListener()
{
    CloseNotifyFdLocked();
    if (notifyHandler_.joinable()) {
        notifyHandler_.join();
    }
}

std::shared_ptr<FileCopyLocalListener> FileCopyLocalListener::GetLocalListener(const std::string &srcPath,
    bool isFile, const ProcessCallback &processCallback)
{
    auto listener = std::make_shared<FileCopyLocalListener>(srcPath, isFile, processCallback);
    return listener;
}

void FileCopyLocalListener::StartListener()
{
    std::lock_guard<std::mutex> lock(processMutex_);
    if (processCallback_ == nullptr || totalSize_ == 0) {
        LOGI("processCallback is nullptr or totalSize is zero, totalSize = %{public}" PRId64 "B", totalSize_);
        return;
    }
    if (notifyHandler_.joinable()) {
        LOGE("notifyHandler has join");
        return;
    }
    if (isMtpPath_) {
        notifyHandler_ = std::thread([this] {
            LOGI("MTP copy StartListener.");
            GetNotifyEvent4Mtp();
    });
    } else {
        notifyHandler_ = std::thread([this] {
            LOGI("StartListener.");
            GetNotifyEvent();
        });
    }
}

void FileCopyLocalListener::GetNotifyEvent4Mtp()
{
    prctl(PR_SET_NAME, "NotifyThread4Mtp");
    while (notifyRun_.load() && (processCallback_ != nullptr)) {
        progressSize_ += DEFAULT_SPEED_SIZE;
        if (progressSize_ >= totalSize_) {
            progressSize_ = totalSize_;
        }
        processCallback_(progressSize_, totalSize_);
        usleep(SLEEP_TIME_US);
    }
}

void FileCopyLocalListener::StopListener()
{
    std::lock_guard<std::mutex> lock(processMutex_);
    LOGI("StopListener start.");
    if (processCallback_ == nullptr) {
        LOGI("processCallback is nullptr");
        return;
    }
    if (isMtpPath_) {
        processCallback_(totalSize_, totalSize_);
    } else {
        processCallback_(progressSize_, totalSize_);
    }
    CloseNotifyFdLocked();
    notifyRun_.store(false);
    {
        std::unique_lock<std::mutex> cvLock(cvLock_);
        notifyCv_.notify_one();
    }
    if (notifyHandler_.joinable()) {
        notifyHandler_.join();
    }
}

void FileCopyLocalListener::AddFile(const std::string &fileName)
{
    std::lock_guard<std::mutex> lock(filePathsMutex_);
    filePaths_.insert(fileName);
}

void FileCopyLocalListener::SubDirAddListener(const std::string &srcPath, const std::string &destPath, uint32_t mode)
{
    LOGI("SubDir add listener");
    std::lock_guard<std::mutex> lock(wdsMutex_);
    int newWd = inotify_add_watch(notifyFd_, destPath.c_str(), mode);
    if (newWd < 0) {
        LOGE("newWd is invalid, newWd = %{public}d, errno = %{public}d", newWd, errno);
        return;
    }
    std::shared_ptr<ReceiveInfo> receiveInfo = std::make_shared<ReceiveInfo>();
    receiveInfo->path = destPath;
    wds_.insert(std::make_pair(newWd, receiveInfo));
}

int32_t FileCopyLocalListener::AddListenerFile(const std::string &srcPath, const std::string &destPath, uint32_t mode)
{
    LOGI("AddListenerFile start");
    if (processCallback_ == nullptr) {
        LOGI("processCallback is nullptr");
        return E_OK;
    }
    std::lock_guard<std::mutex> lock(wdsMutex_);
    int newWd = inotify_add_watch(notifyFd_, destPath.c_str(), mode);
    if (newWd < 0) {
        LOGE("inotify_add_watch, newWd is unvaild, newWd = %{public}d, errno = %{public}d", newWd, errno);
        RadarParaInfo info = {"AddListenerFile", ReportLevel::INNER, DfxBizStage::HMDFS_COPY,
            "kernel", "", newWd, "newWd is invaild, errno=" + to_string(errno)};
        DfsRadar::GetInstance().ReportFileAccess(info);
        return errno;
    }
    std::shared_ptr<ReceiveInfo> receiveInfo = std::make_shared<ReceiveInfo>();
    if (receiveInfo == nullptr) {
        LOGE("Failed to request heap memory.");
        return ENOMEM;
    }
    receiveInfo->path = destPath;
    wds_.insert(std::make_pair(newWd, receiveInfo));
    uint64_t fileSize = 0;
    int32_t err;
    if (!isFile_) {
        err = FileSizeUtils::GetDirSize(srcPath, fileSize);
    } else {
        err = FileSizeUtils::GetFileSize(srcPath, fileSize);
    }
    if (err != E_OK) {
        LOGE("Failed to get src size, isFile=%{public}d, err=%{public}d", isFile_, err);
        RadarParaInfo info = {"AddListenerFile", ReportLevel::INNER, DfxBizStage::HMDFS_COPY,
            "kernel", "", err, "Failed to get src size"};
        DfsRadar::GetInstance().ReportFileAccess(info);
        return err;
    }
    totalSize_ = fileSize;
    notifyTime_ = std::chrono::steady_clock::now() + NOTIFY_PROGRESS_DELAY;
    LOGI("AddListenerFile end");
    return E_OK;
}

void FileCopyLocalListener::GetNotifyEvent()
{
    prctl(PR_SET_NAME, "NotifyThread");
    nfds_t nfds = 2;
    struct pollfd fds[2];
    fds[0].events = 0;
    fds[1].events = POLLIN;
    fds[0].fd = eventFd_;
    fds[1].fd = notifyFd_;
    while (notifyRun_.load() && errorCode_== E_OK && eventFd_ != -1 && notifyFd_ != -1) {
        auto ret = poll(fds, nfds, -1);
        if (ret > 0) {
            if (static_cast<unsigned short>(fds[0].revents) & POLLNVAL) {
                notifyRun_.store(false);
                return;
            }
            if (static_cast<unsigned short>(fds[1].revents) & POLLIN) {
                ReadNotifyEventLocked();
            }
        } else if (ret < 0 && errno == EINTR) {
            continue;
        } else {
            LOGE("poll failed error : %{public}d", errno);
            errorCode_ = errno;
            return;
        }
        {
            std::unique_lock<std::mutex> cvLock(cvLock_);
            notifyCv_.wait_for(cvLock, std::chrono::microseconds(SLEEP_TIME_US), [this]() -> bool {
                return notifyFd_ == -1 || !notifyRun_.load();
            });
        }
    }
}

void FileCopyLocalListener::ReadNotifyEventLocked()
{
    {
        std::lock_guard<std::mutex> lock(readMutex_);
        if (readClosed_) {
            LOGE("read after close");
            return;
        }
        readFlag_ = true;
    }
    ReadNotifyEvent();
    {
        std::lock_guard<std::mutex> lock(readMutex_);
        readFlag_ = false;
        if (readClosed_) {
            LOGE("close after read");
            CloseNotifyFd();
            return;
        }
    }
}

void FileCopyLocalListener::ReadNotifyEvent()
{
    char buf[BUF_SIZE] = { 0x00 };
    struct inotify_event *event = nullptr;
    int len = 0;
    int64_t index = 0;
    while (((len = read(notifyFd_, &buf, sizeof(buf))) < 0) && (errno == EINTR)) {}
    while (notifyRun_.load() && index < len) {
        event = reinterpret_cast<inotify_event *>(buf + index);
        auto [needContinue, errCode, needSend] = HandleProgress(event);
        if (!needContinue) {
            errorCode_ = errCode;
            return;
        }
        if (needContinue && !needSend) {
            index += static_cast<int64_t>(sizeof(struct inotify_event) + event->len);
            continue;
        }
        if (progressSize_ == totalSize_) {
            notifyRun_.store(false);
            return;
        }
        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime >= notifyTime_ && processCallback_ != nullptr) {
            processCallback_(progressSize_, totalSize_);
            notifyTime_ = currentTime + NOTIFY_PROGRESS_DELAY;
        }
        index += static_cast<int64_t>(sizeof(struct inotify_event) + event->len);
    }
}

std::tuple<bool, int, bool> FileCopyLocalListener::HandleProgress(inotify_event *event)
{
    std::shared_ptr<ReceiveInfo> receivedInfo;
    {
        std::lock_guard<std::mutex> lock(wdsMutex_);
        auto iter = wds_.find(event->wd);
        if (iter == wds_.end()) {
            return { true, -1, false };
        }
        receivedInfo = iter->second;
    }
    std::string fileName = receivedInfo->path;
    if (!isFile_) { // files under subdir
        fileName += "/" + std::string(event->name);
        if (!CheckFileValid(fileName)) {
            return { true, -1, false };
        }
        auto err = UpdateProgressSize(fileName, receivedInfo);
        if (err != E_OK) {
            return { false, err, false };
        }
    } else { // file
        uint64_t fileSize = 0;
        auto err = FileSizeUtils::GetFileSize(fileName, fileSize);
        if (err != E_OK) {
            return { false, err, false };
        }
        progressSize_ = fileSize;
    }
    return { true, E_OK, true };
}

bool FileCopyLocalListener::CheckFileValid(const std::string &filePath)
{
    std::lock_guard<std::mutex> lock(filePathsMutex_);
    return filePaths_.count(filePath) != 0;
}

void FileCopyLocalListener::CloseNotifyFdLocked()
{
    std::lock_guard<std::mutex> lock(readMutex_);
    readClosed_ = true;
    if (readFlag_) {
        LOGE("close while reading");
        return;
    }
    CloseNotifyFd();
}

void FileCopyLocalListener::CloseNotifyFd()
{
    readClosed_ = false;
    std::unique_lock<std::mutex> cvLock(cvLock_);
    if (eventFd_ != -1) {
        close(eventFd_);
        eventFd_ = -1;
    }
    if (notifyFd_ == -1) {
        return;
    }

    std::lock_guard<std::mutex> wdsLock(wdsMutex_);
    for (auto item : wds_) {
        inotify_rm_watch(notifyFd_, item.first);
    }
    close(notifyFd_);
    notifyFd_ = -1;
    notifyCv_.notify_one();
}

int FileCopyLocalListener::UpdateProgressSize(const std::string &filePath, std::shared_ptr<ReceiveInfo> receivedInfo)
{
    uint64_t fileSize = 0;
    auto err = FileSizeUtils::GetFileSize(filePath, fileSize);
    if (err != E_OK) {
        LOGE("GetFileSize failed, err: %{public}d.", err);
        return err;
    }
    auto size = fileSize;
    auto iter = receivedInfo->fileList.find(filePath);
    if (iter == receivedInfo->fileList.end()) { // new file
        receivedInfo->fileList.insert({ filePath, size });
        progressSize_ += size;
    } else { // old file
        if (size > iter->second) {
            progressSize_ += (size - iter->second);
            iter->second = size;
        }
    }
    return E_OK;
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
