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

#include "copy/file_copy_manager.h"

#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <filesystem>
#include <limits>
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "copy/distributed_file_fd_guard.h"
#include "copy/file_copy_listener.h"
#include "copy/file_size_utils.h"
#include "datashare_helper.h"
#include "dfs_error.h"
#include "distributed_file_daemon_proxy.h"
#include "file_uri.h"
#include "iremote_stub.h"
#include "sandbox_helper.h"
#include "utils_directory.h"
#include "utils_log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD001600
#define LOG_TAG "distributedfile_daemon"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace AppFileService;
using namespace FileManagement;
static const std::string FILE_PREFIX_NAME = "file://";
static const std::string NETWORK_PARA = "?networkid=";
static const std::string MEDIALIBRARY_DATA_URI = "datashare:///media";
static const std::string MEDIA = "media";
static constexpr size_t MAX_SIZE = 1024 * 1024 * 4;
std::shared_ptr<FileCopyManager> FileCopyManager::instance_ = nullptr;

static bool CheckPath(std::shared_ptr<FileInfos> &infos)
{
    std::string destPath = infos->destPath;
    if (infos->srcUriIsFile) {
        auto pos = destPath.rfind("/");
        if (pos == std::string::npos) {
            return false;
        }
        destPath.resize(pos);
    }
    if (!(SandboxHelper::CheckValidPath(infos->srcPath) && SandboxHelper::CheckValidPath(destPath))) {
        return false;
    }
    return true;
}

std::shared_ptr<FileCopyManager> FileCopyManager::GetInstance()
{
    static std::once_flag once;
    std::call_once(once, []() {
        FileCopyManager::instance_ = std::make_shared<FileCopyManager>();
    });

    return instance_;
}

int32_t FileCopyManager::Copy(const std::string &srcUri, const std::string &destUri, ProcessCallback &processCallback)
{
    LOGE("FileCopyManager Copy start ");
    if (srcUri.empty() || destUri.empty()) {
        return E_NOENT;
    }
    if (!Utils::IsFilePathValid(Utils::GetRealUri(srcUri)) || !Utils::IsFilePathValid(Utils::GetRealUri(destUri))) {
        LOGE("path: %{public}s or %{public}s is forbidden",
            GetAnonyString(srcUri).c_str(), GetAnonyString(destUri).c_str());
        return OHOS::FileManagement::E_ILLEGAL_URI;
    }
    auto infos = std::make_shared<FileInfos>();
    auto ret = CreateFileInfos(srcUri, destUri, infos);
    if (ret != E_OK) {
        return ret;
    }

    if (IsRemoteUri(infos->srcUri)) {
        ret = ExecRemote(infos, processCallback);
        RemoveFileInfos(infos);
        return ret;
    }

    if (!CheckPath(infos)) {
        LOGE("invalid srcPath : %{private}s, destPath: %{private}s", GetAnonyString(infos->srcPath).c_str(),
            GetAnonyString(infos->destPath).c_str());
        return E_NOENT;
    }

    infos->localListener = FileCopyLocalListener::GetLocalListener(infos->srcPath,
        infos->srcUriIsFile, processCallback);
    infos->localListener->StartListener();
    auto result = ExecLocal(infos);
    RemoveFileInfos(infos);
    infos->localListener->StopListener();
    
    if (result != E_OK) {
        return result;
    }
    return infos->localListener->GetResult();
}

int32_t FileCopyManager::ExecRemote(std::shared_ptr<FileInfos> infos, ProcessCallback &processCallback)
{
    LOGI("ExecRemote Copy start ");
    sptr<TransListener> transListener (new (std::nothrow) TransListener(infos->destUri, processCallback));
    if (transListener == nullptr) {
        LOGE("new trans listener failed");
        return ENOMEM;
    }
    infos->transListener = transListener;

    auto networkId = transListener->GetNetworkIdFromUri(infos->srcUri);
    auto distributedFileDaemonProxy = DistributedFileDaemonProxy::GetInstance();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    auto ret = distributedFileDaemonProxy->PrepareSession(infos->srcUri, infos->destUri,
        networkId, transListener, transListener->hmdfsInfo_);
    if (ret != E_OK) {
        LOGE("PrepareSession failed, ret = %{public}d.", ret);
        return ret;
    }

    auto copyResult = transListener->WaitForCopyResult();
    if (copyResult == FAILED) {
        return transListener->GetErrCode();
    }
    return transListener->CopyToSandBox(infos->srcUri);
}

int32_t FileCopyManager::Cancel()
{
    LOGI("Cancel all Copy");
    std::lock_guard<std::mutex> lock(FileInfosVecMutex_);
    for (auto &item : FileInfosVec_) {
        item->needCancel.store(true);
        if (item->transListener != nullptr) {
            item->transListener->Cancel();
        }
        DeleteResFile(item);
    }
    FileInfosVec_.clear();
    return E_OK;
}

int32_t FileCopyManager::Cancel(const std::string &srcUri, const std::string &destUri)
{
    LOGI("Cancel Copy");
    std::lock_guard<std::mutex> lock(FileInfosVecMutex_);
    int32_t ret = 0;
    if (!Utils::IsFilePathValid(Utils::GetRealUri(srcUri)) || !Utils::IsFilePathValid(Utils::GetRealUri(destUri))) {
        LOGE("path: %{public}s or %{public}s is forbidden",
            GetAnonyString(srcUri).c_str(), GetAnonyString(destUri).c_str());
        return OHOS::FileManagement::E_ILLEGAL_URI;
    }
    for (auto item = FileInfosVec_.begin(); item != FileInfosVec_.end();) {
        if ((*item)->srcUri != srcUri || (*item)->destUri != destUri) {
            ++item;
            continue;
        }
        (*item)->needCancel.store(true);
        if ((*item)->transListener != nullptr) {
            ret = (*item)->transListener->Cancel();
        }
        DeleteResFile(*item);
        item = FileInfosVec_.erase(item);
        return ret;
    }
    return E_OK;
}

void FileCopyManager::DeleteResFile(std::shared_ptr<FileInfos> infos)
{
    std::error_code errCode;
    //delete files in remote cancel
    if (infos->transListener != nullptr) {
        if (std::filesystem::exists(infos->destPath, errCode)) {
            std::filesystem::remove(infos->destPath, errCode);
        }
        return;
    }

    //delete files&dirs in local cancel
    auto filePaths = infos->localListener->GetFilePath();
    for (auto path : filePaths) {
        if (!std::filesystem::exists(path, errCode)) {
            LOGE("Failed to find the file, errcode %{public}d", errCode.value());
            continue;
        }
        std::filesystem::remove(path, errCode);
    }

    std::lock_guard<std::mutex> lock(infos->subDirsMutex);
    for (auto subDir : infos->subDirs) {
        if (!std::filesystem::exists(subDir, errCode)) {
            LOGE("Failed to find the dir, errcode %{public}d", errCode.value());
            continue;
        }
        std::filesystem::remove(subDir, errCode);
    }
}

int32_t FileCopyManager::ExecLocal(std::shared_ptr<FileInfos> infos)
{
    LOGI("start ExecLocal");
    // 文件到文件, 文件到目录的形式由上层改写为文件到文件的形式
    if (infos->srcUriIsFile) {
        if (infos->srcPath == infos->destPath) {
            LOGE("The src and dest is same");
            return E_OK;
        }
        int32_t ret = CheckOrCreatePath(infos->destPath);
        if (ret != E_OK) {
            LOGE("check or create fail, error code is %{public}d", ret);
            return ret;
        }
        infos->localListener->AddListenerFile(infos->destPath, IN_MODIFY);
        return CopyFile(infos->srcPath, infos->destPath, infos);
    }
    
    bool destIsDirectory;
    auto ret = FileSizeUtils::IsDirectory(infos->destUri, false, destIsDirectory);
    if (ret != E_OK) {
        LOGE("destPath: %{public}s not find, error=%{public}d", infos->destPath.c_str(), ret);
        return ret;
    }
    if (destIsDirectory) {
        if (infos->srcPath.back() != '/') {
            infos->srcPath += '/';
        }
        if (infos->destPath.back() != '/') {
            infos->destPath += '/';
        }
        // copyDir
        return CopyDirFunc(infos->srcPath, infos->destPath, infos);
    }
    LOGI("ExecLocal not support this srcUri and destUri");
    return E_NOENT;
}

int32_t FileCopyManager::CopyFile(const std::string &src, const std::string &dest, std::shared_ptr<FileInfos> infos)
{
    LOGI("src = %{private}s, dest = %{private}s", src.c_str(), dest.c_str());
    infos->localListener->AddFile(dest);
    int32_t srcFd = -1;
    int32_t ret = OpenSrcFile(src, infos, srcFd);
    if (srcFd < 0) {
        return ret;
    }
    auto destFd = open(dest.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (destFd < 0) {
        LOGE("Error opening dest file descriptor. errno = %{public}d", errno);
        close(srcFd);
        return errno;
    }
    std::shared_ptr<FDGuard> srcFdg = std::make_shared<FDGuard>(srcFd, true);
    std::shared_ptr<FDGuard> destFdg = std::make_shared<FDGuard>(destFd, true);
    if (srcFdg == nullptr || destFdg == nullptr) {
        LOGE("Failed to request heap memory.");
        close(srcFd);
        close(destFd);
        return -1;
    }
    return SendFileCore(srcFdg, destFdg, infos);
}

void fs_req_cleanup(uv_fs_t* req)
{
    uv_fs_req_cleanup(req);
    if (req) {
        delete req;
        req = nullptr;
    }
}

int32_t FileCopyManager::SendFileCore(std::shared_ptr<FDGuard> srcFdg,
    std::shared_ptr<FDGuard> destFdg, std::shared_ptr<FileInfos> infos)
{
    std::unique_ptr<uv_fs_t, decltype(fs_req_cleanup)*> sendFileReq = {
        new (std::nothrow) uv_fs_t, fs_req_cleanup };
    if (sendFileReq == nullptr) {
        LOGE("Failed to request heap memory.");
        return -1;
    }
    int64_t offset = 0;
    struct stat srcStat{};
    if (fstat(srcFdg->GetFD(), &srcStat) < 0) {
        LOGE("Failed to get stat of file by fd: %{public}d ,errno = %{public}d", srcFdg->GetFD(), errno);
        return errno;
    }
    int32_t ret = 0;
    int64_t size = static_cast<int64_t>(srcStat.st_size);

    while (size >= 0) {
        ret = uv_fs_sendfile(nullptr, sendFileReq.get(), destFdg->GetFD(), srcFdg->GetFD(),
            offset, MAX_SIZE, nullptr);
        if (ret < 0) {
            LOGE("Failed to sendfile by errno : %{public}d", errno);
            return errno;
        }

        if (infos->needCancel.load()) {
            LOGE("need cancel");
            return FileManagement::E_DFS_CANCEL_SUCCESS; // 204
        }
        
        offset += static_cast<int64_t>(ret);
        size -= static_cast<int64_t>(ret);
        if (ret == 0) {
            break;
        }
    }

    if (size != 0) {
        LOGE("The execution of the sendfile task was terminated, remaining file size %{public}" PRIu64, size);
        return E_OK; // EIO
    }
    return E_OK;
}

int32_t FileCopyManager::CopyDirFunc(const std::string &src, const std::string &dest, std::shared_ptr<FileInfos> infos)
{
    LOGI("CopyDirFunc in, src = %{private}s, dest = %{private}s", src.c_str(), dest.c_str());
    size_t found = dest.find(src);
    if (found != std::string::npos && found == 0) {
        LOGE("not support copy src to dest");
        return -1;
    }
    
    // 获取src目录的目录名称
    std::filesystem::path srcPath = std::filesystem::u8path(src);
    std::string dirName;
    if (srcPath.has_parent_path()) {
        dirName = srcPath.parent_path().filename();
    }

    // 构造要拷贝到的路径
    std::string destStr = dest + "/" + dirName;
    return CopySubDir(src, destStr, infos);
}

int32_t FileCopyManager::CopySubDir(const std::string &srcPath,
    const std::string &destPath, std::shared_ptr<FileInfos> infos)
{
    std::error_code errCode;
    if (!std::filesystem::exists(destPath, errCode) && errCode.value() == E_OK) {
        int res = MakeDir(destPath);
        if (res != E_OK) {
            LOGE("Failed to mkdir");
            return res;
        }
    } else if (errCode.value() != E_OK) {
        LOGE("fs exists fail, errcode is %{public}d", errCode.value());
        return errCode.value();
    }
    {
        std::lock_guard<std::mutex> lock(infos->subDirsMutex);
        infos->subDirs.insert(destPath);
    }
    infos->localListener->AddListenerFile(destPath, IN_MODIFY);
    return RecurCopyDir(srcPath, destPath, infos);
}

int32_t FileCopyManager::RecurCopyDir(const std::string &srcPath,
    const std::string &destPath, std::shared_ptr<FileInfos> infos)
{
    auto pNameList = FileSizeUtils::GetDirNameList(srcPath);
    if (pNameList == nullptr) {
        return ENOMEM;
    }
    for (int i = 0; i < pNameList->direntNum; i++) {
        std::string src = srcPath + '/' + std::string((pNameList->namelist[i])->d_name);
        std::string dest = destPath + '/' + std::string((pNameList->namelist[i])->d_name);
        if ((pNameList->namelist[i])->d_type == DT_LNK) {
            continue;
        }
        int ret = E_OK;
        if ((pNameList->namelist[i])->d_type == DT_DIR) {
            ret = CopySubDir(src, dest, infos);
        } else {
            ret = CopyFile(src, dest, infos);
        }
        if (ret != E_OK) {
            return ret;
        }
    }
    return E_OK;
}

void FileCopyManager::AddFileInfos(std::shared_ptr<FileInfos> infos)
{
    std::lock_guard<std::mutex> lock(FileInfosVecMutex_);
    FileInfosVec_.push_back(infos);
}

void FileCopyManager::RemoveFileInfos(std::shared_ptr<FileInfos> infos)
{
    std::lock_guard<std::mutex> lock(FileInfosVecMutex_);
    for (auto it = FileInfosVec_.begin(); it != FileInfosVec_.end();) {
        if ((*it)->srcUri == infos->srcUri && (*it)->destUri == infos->destUri) {
            it = FileInfosVec_.erase(it);
        } else {
            ++it;
        }
    }
}

int32_t FileCopyManager::CreateFileInfos(const std::string &srcUri,
    const std::string &destUri, std::shared_ptr<FileInfos> &infos)
{
    infos->srcUri = srcUri;
    infos->destUri = destUri;
    infos->srcPath = FileSizeUtils::GetPathFromUri(srcUri, true);
    infos->destPath = FileSizeUtils::GetPathFromUri(destUri, false);

    bool isDirectory;
    auto ret = FileSizeUtils::IsDirectory(infos->srcUri, true, isDirectory);
    if (ret != E_OK) {
        LOGE("srcPath: %{public}s not find, err=%{public}d", infos->srcPath.c_str(), ret);
        return ret;
    }
    infos->srcUriIsFile = IsMediaUri(infos->srcUri) || !isDirectory;
    AddFileInfos(infos);
    return E_OK;
}
 
std::string GetModeFromFlags(unsigned int flags)
{
    const std::string readMode = "r";
    const std::string writeMode = "w";
    const std::string appendMode = "a";
    const std::string truncMode = "t";
    std::string mode = readMode;
    mode += (((flags & O_RDWR) == O_RDWR) ? writeMode : "");
    mode = (((flags & O_WRONLY) == O_WRONLY) ? writeMode : mode);
    if (mode != readMode) {
        mode += ((flags & O_TRUNC) ? truncMode : "");
        mode += ((flags & O_APPEND) ? appendMode : "");
    }
    return mode;
}
 
int32_t FileCopyManager::OpenSrcFile(const std::string &srcPth, std::shared_ptr<FileInfos> infos, int32_t &srcFd)
{
    Uri uri(infos->srcUri);
    
    if (uri.GetAuthority() == MEDIA) {
        std::shared_ptr<DataShare::DataShareHelper> dataShareHelper = nullptr;
        sptr<FileIoToken> remote = new (std::nothrow) IRemoteStub<FileIoToken>();
        if (!remote) {
            LOGE("Failed to get remote object");
            return -1;
        }
        dataShareHelper = DataShare::DataShareHelper::Creator(remote->AsObject(), MEDIALIBRARY_DATA_URI);
        if (!dataShareHelper) {
            LOGE("Failed to connect to datashare");
            return -1;
        }
        srcFd = dataShareHelper->OpenFile(uri, GetModeFromFlags(O_RDONLY));
        if (srcFd < 0) {
            LOGE("Open media uri by data share fail. ret = %{public}d", srcFd);
            return -1;
        }
    } else {
        srcFd = open(srcPth.c_str(), O_RDONLY);
        if (srcFd < 0) {
            LOGE("Error opening src file descriptor. errno = %{public}d", errno);
            return errno;
        }
    }
    return 0;
}
 
int FileCopyManager::MakeDir(const std::string &path)
{
    std::filesystem::path destDir(path);
    std::error_code errCode;
    if (!std::filesystem::create_directory(destDir, errCode)) {
        LOGE("Failed to create directory, error code: %{public}d", errCode.value());
        return errCode.value();
    }
    return E_OK;
}
 
bool FileCopyManager::IsRemoteUri(const std::string &uri)
{
    // NETWORK_PARA
    return uri.find(NETWORK_PARA) != uri.npos;
}

bool FileCopyManager::IsMediaUri(const std::string &uriPath)
{
    Uri uri(uriPath);
    std::string bundleName = uri.GetAuthority();
    return bundleName == MEDIA;
}
 
int32_t FileCopyManager::CheckOrCreatePath(const std::string &destPath)
{
    std::error_code errCode;
    if (!std::filesystem::exists(destPath, errCode) && errCode.value() == E_OK) {
        LOGI("destPath not exist");
        auto file = open(destPath.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
        if (file < 0) {
            LOGE("Error opening file descriptor. errno = %{public}d", errno);
            return errno;
        }
        close(file);
    } else if (errCode.value() != 0) {
        return errCode.value();
    }
    return E_OK;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
