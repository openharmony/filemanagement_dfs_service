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

#include <unistd.h>

#include "copy/file_size_utils.h"
#include "dfs_error.h"
#include "utils_log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD001600
#define LOG_TAG "distributedfile_daemon"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace FileManagement;
static constexpr int BUF_SIZE = 1024;
static constexpr std::chrono::milliseconds NOTIFY_PROGRESS_DELAY(100);

FileCopyLocalListener::FileCopyLocalListener(const std::string &srcPath,
    bool isFile, const ProcessCallback &processCallback) : isFile_(isFile), processCallback_(processCallback)
{
    if (processCallback_ == nullptr) {
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

    uint64_t fileSize = 0;
    int32_t err;
    if (!isFile_) {
        err = FileSizeUtils::GetDirSize(srcPath, fileSize);
    } else {
        err = FileSizeUtils::GetFileSize(srcPath, fileSize);
    }
    if (err == E_OK) {
        totalSize_ = fileSize;
    }
    notifyTime_ = std::chrono::steady_clock::now() + NOTIFY_PROGRESS_DELAY;
}

FileCopyLocalListener::~FileCopyLocalListener()
{
    CloseNotifyFd();
}

std::shared_ptr<FileCopyLocalListener> FileCopyLocalListener::GetLocalListener(const std::string &srcPath,
    bool isFile, const ProcessCallback &processCallback)
{
    auto listener = std::make_shared<FileCopyLocalListener>(srcPath, isFile, processCallback);
    return listener;
}

void FileCopyLocalListener::StartListener()
{
    if (processCallback_ == nullptr || totalSize_ == 0) {
        return;
    }
    notifyHandler_ = std::thread([this] {
        GetNotifyEvent();
    });
}

void FileCopyLocalListener::StopListener()
{
    processCallback_(progressSize_, totalSize_);
    CloseNotifyFd();
    if (notifyHandler_.joinable()) {
        notifyHandler_.join();
    }
}

void FileCopyLocalListener::AddFile(const std::string &fileName)
{
    std::lock_guard<std::mutex> lock(filePathsMutex_);
    filePaths_.insert(fileName);
}

void FileCopyLocalListener::AddListenerFile(const std::string &destPath, uint32_t mode)
{
    if (processCallback_ == nullptr || totalSize_ == 0) {
        return;
    }

    std::lock_guard<std::mutex> lock(wdsMutex_);
    int newWd = inotify_add_watch(notifyFd_, destPath.c_str(), mode);
    if (newWd < 0) {
        LOGE("inotify_add_watch, newWd is unvaild, newWd = %{public}d", newWd);
        return;
    }
    std::shared_ptr<ReceiveInfo> receiveInfo = std::make_shared<ReceiveInfo>();
    if (receiveInfo == nullptr) {
        LOGE("Failed to request heap memory.");
        return;
    }
    receiveInfo->path = destPath;
    wds_.insert(std::make_pair(newWd, receiveInfo));
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
    while (errorCode_== E_OK && eventFd_ != -1 && notifyFd_ != -1) {
        auto ret = poll(fds, nfds, -1);
        if (ret > 0) {
            if (static_cast<unsigned short>(fds[0].revents) & POLLNVAL) {
                return;
            }
            if (static_cast<unsigned short>(fds[1].revents) & POLLIN) {
                ReadNotifyEvent();
            }
        } else if (ret < 0 && errno == EINTR) {
            continue;
        } else {
            LOGE("poll failed error : %{public}d", errno);
            errorCode_ = errno;
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
    while (index < len) {
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
            return;
        }
        auto currentTime = std::chrono::steady_clock::now();
        if (currentTime >= notifyTime_) {
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

void FileCopyLocalListener::CloseNotifyFd()
{
    if (eventFd_ != -1) {
        close(eventFd_);
        eventFd_ = -1;
    }
    if (notifyFd_ == -1) {
        return;
    }

    std::lock_guard<std::mutex> lock(wdsMutex_);
    for (auto item : wds_) {
        inotify_rm_watch(notifyFd_, item.first);
    }
    close(notifyFd_);
    notifyFd_ = -1;
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
