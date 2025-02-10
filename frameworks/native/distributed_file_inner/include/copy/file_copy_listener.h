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

#ifndef DISTRIBUTED_FILE_COPY_LISTENER_H
#define DISTRIBUTED_FILE_COPY_LISTENER_H

#include <fcntl.h>
#include <filesystem>

#include <map>
#include <memory>
#include <poll.h>
#include <set>
#include <string>
#include <sys/eventfd.h>
#include <sys/inotify.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <thread>
#include <tuple>

namespace OHOS {
namespace Storage {
namespace DistributedFile {

struct ReceiveInfo {
    std::string path;                         // dir name
    std::map<std::string, uint64_t> fileList; // filename, proceededSize
};

using ProcessCallback = std::function<void (uint64_t processSize, uint64_t totalSize)>;
class FileCopyLocalListener {
public:
    FileCopyLocalListener(const FileCopyLocalListener&) = delete;
    FileCopyLocalListener& operator=(const FileCopyLocalListener&) = delete;
    FileCopyLocalListener(const std::string &srcPath,
        bool isFile, const ProcessCallback &processCallback);
    ~FileCopyLocalListener();
    static std::shared_ptr<FileCopyLocalListener> GetLocalListener(const std::string &srcPath,
        bool isFile, const ProcessCallback &processCallback);

public:
    void StartListener();
    void StopListener();
    void AddFile(const std::string &fileName);
    void AddListenerFile(const std::string &destPath, uint32_t mode);
    std::set<std::string> GetFilePath()
    {
        std::lock_guard<std::mutex> lock(filePathsMutex_);
        return filePaths_;
    }
    int32_t GetResult() { return errorCode_; };

private:
    void CloseNotifyFd();
    void GetNotifyEvent();
    void ReadNotifyEvent();
    std::tuple<bool, int, bool> HandleProgress(inotify_event *event);
    bool CheckFileValid(const std::string &filePath);
    int UpdateProgressSize(const std::string &filePath, std::shared_ptr<ReceiveInfo> receivedInfo);

private:
    bool isFile_ = false;
    int32_t notifyFd_ = -1;
    int32_t eventFd_ = -1;
    uint64_t totalSize_ = 0;
    uint64_t progressSize_ = 0;
    std::chrono::steady_clock::time_point notifyTime_;
    std::mutex wdsMutex_;
    std::map<int, std::shared_ptr<ReceiveInfo>> wds_;

    std::thread notifyHandler_;
    std::mutex filePathsMutex_;
    std::set<std::string> filePaths_;
    ProcessCallback processCallback_;
    int32_t errorCode_ = 0;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // DISTRIBUTED_FILE_COPY_LISTENER_H
