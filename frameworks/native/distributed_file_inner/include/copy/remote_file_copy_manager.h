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

#ifndef DISTRIBUTED_REMOTE_FILE_COPY_MANAGER_H
#define DISTRIBUTED_REMOTE_FILE_COPY_MANAGER_H

#include "copy/file_copy_manager.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

class RemoteFileCopyManager {
public:
    static RemoteFileCopyManager &GetInstance();
    int32_t RemoteCopy(const std::string &srcUri, const std::string &destUri, const sptr<IFileTransListener> &listener,
        const int32_t userId, const std::string &copyPath);
    int32_t RemoteCancel(const std::string &srcUri, const std::string &destUri);

private:
    void RemoveFileInfos(std::shared_ptr<FileInfos> infos);
    // operator of file
    int32_t CreateFileInfos(const std::string &srcUri, const std::string &destUri, std::shared_ptr<FileInfos> &infos,
        const int32_t userId, const std::string &copyPath);
    bool IsMediaUri(const std::string &uriPath);
    void AddFileInfos(std::shared_ptr<FileInfos> infos);
    bool IsFile(const std::string &path);
private:
    std::mutex FileInfosVecMutex_;
    std::vector<std::shared_ptr<FileInfos>> FileInfosVec_;
};

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // DISTRIBUTED_REMOTE_FILE_COPY_MANAGER_H