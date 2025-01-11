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

#ifndef DISTRIBUTED_FILE_COPY_MANAGER_H
#define DISTRIBUTED_FILE_COPY_MANAGER_H

#include <chrono>
#include <condition_variable>
#include <memory>

#include "copy/distributed_file_fd_guard.h"
#include "copy/file_copy_listener.h"
#include "copy/trans_listener.h"
#include "iremote_broker.h"
#include "refbase.h"
#include "uv.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

class FileIoToken : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.fileio.open");
 
    FileIoToken() = default;
    virtual ~FileIoToken() noexcept = default;
};
 
struct FileInfos {
    std::string srcUri;
    std::string destUri;
    std::string srcPath;
    std::string destPath;
    bool srcUriIsFile = false;
    std::shared_ptr<FileCopyLocalListener> localListener = nullptr;
    sptr<TransListener> transListener = nullptr;
    std::atomic_bool needCancel{ false };

    bool operator==(const FileInfos &infos) const
    {
        return (srcUri == infos.srcUri && destUri == infos.destUri);
    }
    bool operator<(const FileInfos &infos) const
    {
        if (srcUri == infos.srcUri) {
            return destUri < infos.destUri;
        }
        return srcUri < infos.srcUri;
    }
};
 
class FileCopyManager final {
public:
    using ProcessCallback = std::function<void (uint64_t processSize, uint64_t totalSize)>;
    static std::shared_ptr<FileCopyManager> GetInstance();
    int32_t Copy(const std::string &srcUri, const std::string &destUri, ProcessCallback &processCallback);
    int32_t Cancel(const std::string &srcUri, const std::string &destUri);
    int32_t Cancel();
    
private:
    static std::shared_ptr<FileCopyManager> instance_;
    std::mutex FileInfosVecMutex_;
    std::vector<std::shared_ptr<FileInfos>> FileInfosVec_;
 
private:
    // operator of local copy
    int32_t ExecLocal(std::shared_ptr<FileInfos> infos);
    int32_t CopyFile(const std::string &src, const std::string &dest, std::shared_ptr<FileInfos> infos);
    int32_t SendFileCore(std::shared_ptr<FDGuard> srcFdg, std::shared_ptr<FDGuard> destFdg,
        std::shared_ptr<FileInfos> infos);
    int32_t CopyDirFunc(const std::string &src, const std::string &dest, std::shared_ptr<FileInfos> infos);
    int32_t CopySubDir(const std::string &srcPath, const std::string &destPath, std::shared_ptr<FileInfos> infos);
    int32_t RecurCopyDir(const std::string &srcPath, const std::string &destPath, std::shared_ptr<FileInfos> infos);
    int32_t OpenSrcFile(const std::string &srcPth, std::shared_ptr<FileInfos> infos, int32_t &srcFd);

    // operator of remote copy
    int32_t ExecRemote(std::shared_ptr<FileInfos> infos, ProcessCallback &processCallback);

    // operator of file
    int32_t CreateFileInfos(const std::string &srcUri, const std::string &destUri, std::shared_ptr<FileInfos> &infos);
    int32_t CheckOrCreatePath(const std::string &destPath);
    int MakeDir(const std::string &path);
    bool IsRemoteUri(const std::string &uri);
    bool IsMediaUri(const std::string &uriPath);
    void AddFileInfos(std::shared_ptr<FileInfos> infos);
    void RemoveFileInfos(std::shared_ptr<FileInfos> infos);
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
 
#endif // DISTRIBUTED_FILE_COPY_MANAGER_H
