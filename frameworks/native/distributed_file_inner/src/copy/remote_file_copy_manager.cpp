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

#include "copy/remote_file_copy_manager.h"

#include <mutex>
#include <string>
#include <sys/stat.h>

#include "copy/file_size_utils.h"
#include "dfs_error.h"
#include "dfs_radar.h"
#include "datashare_helper.h"
#include "ipc_skeleton.h"
#include "sandbox_helper.h"
#include "utils_log.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD004315
#define LOG_TAG "distributedfile_daemon"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace AppFileService;
using namespace FileManagement;
using namespace std;
static const std::string MEDIA_AUTHORITY = "media";
static const std::string FILE_MANAGER_AUTHORITY = "docs";
static const std::string FILE_SCHEMA = "file://";
static const std::string FILE_SEPARATOR = "/";

static std::string GetBundleName(const std::string &uri)
{
    auto pos = uri.find(FILE_SCHEMA);
    if (pos == std::string::npos) {
        return "";
    }
    auto tmpUri = uri.substr(pos + FILE_SCHEMA.size());
    if (tmpUri.empty()) {
        return "";
    }
    auto bundleNamePos = tmpUri.find(FILE_SEPARATOR);
    if (bundleNamePos == std::string::npos) {
        return "";
    }
    return tmpUri.substr(0, bundleNamePos);
}

static int32_t ChangeOwnerRecursive(const std::string &path, uid_t uid, gid_t gid)
{
    std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(path.c_str()), &closedir);
    if (dir == nullptr) {
        LOGE("Directory is null");
        return EINVAL;
    }

    struct dirent *entry = nullptr;
    while ((entry = readdir(dir.get())) != nullptr) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }
            std::string subPath = path + "/" + entry->d_name;
            if (chown(subPath.c_str(), uid, gid) == -1) {
                LOGE("Change owner recursive failed");
                return EIO;
            }
            return ChangeOwnerRecursive(subPath, uid, gid);
        } else {
            std::string filePath = path + "/" + entry->d_name;
            if (chown(filePath.c_str(), uid, gid) == -1) {
                LOGE("Change owner recursive failed");
                return EIO;
            }
        }
    }
    return E_OK;
}

RemoteFileCopyManager &RemoteFileCopyManager::GetInstance()
{
    static RemoteFileCopyManager instance;
    return instance;
}

bool RemoteFileCopyManager::IsMediaUri(const std::string &uriPath)
{
    Uri uri(uriPath);
    std::string bundleName = uri.GetAuthority();
    return bundleName == MEDIA_AUTHORITY;
}

bool RemoteFileCopyManager::IsFile(const std::string &path)
{
    struct stat buf {};
    int ret = stat(path.c_str(), &buf);
    if (ret == -1) {
        LOGE("stat failed, errno is %{public}d, ", errno);
        RadarParaInfo info = {"IsFile", ReportLevel::INNER, DfxBizStage::HMDFS_COPY,
            "kernel", "", ret, "Stat failed, errno=" + to_string(errno)};
        DfsRadar::GetInstance().ReportFileAccess(info);
        return false;
    }
    return (buf.st_mode & S_IFMT) == S_IFREG;
}

static std::string GetFileName(const std::string &path)
{
    auto pos = path.rfind("/");
    if (pos == std::string::npos) {
        LOGE("invalid path");
        return "";
    }
    return path.substr(pos + 1);
}

void RemoteFileCopyManager::AddFileInfos(std::shared_ptr<FileInfos> infos)
{
    std::lock_guard<std::mutex> lock(FileInfosVecMutex_);
    FileInfosVec_.push_back(infos);
}

int32_t RemoteFileCopyManager::CreateFileInfos(const std::string &srcUri,
    const std::string &destUri, std::shared_ptr<FileInfos> &infos, const int32_t userId, const std::string &copyPath)
{
    infos->srcUri = srcUri;
    infos->destUri = destUri;
    std::string srcPhysicalPath;
    if (SandboxHelper::GetPhysicalPath(srcUri, std::to_string(userId), srcPhysicalPath)) {
        LOGE("Get src path failed, invalid uri");
        RadarParaInfo info = {"CreateFileInfos", ReportLevel::INNER, DfxBizStage::HMDFS_COPY,
            "AFS", "", EINVAL, "Get src path fail"};
        DfsRadar::GetInstance().ReportFileAccess(info);
        return EINVAL;
    }
    std::string dstPhysicalPath;
    if (SandboxHelper::GetPhysicalPath(destUri, std::to_string(userId), dstPhysicalPath)) {
        LOGE("Get dst path failed, invalid uri");
        RadarParaInfo info = {"CreateFileInfos", ReportLevel::INNER, DfxBizStage::HMDFS_COPY,
            "AFS", "", EINVAL, "Get dst path fail"};
        DfsRadar::GetInstance().ReportFileAccess(info);
        return EINVAL;
    }
    Uri uri(destUri);
    auto authority = uri.GetAuthority();
    if (authority != FILE_MANAGER_AUTHORITY && authority != MEDIA_AUTHORITY) {
        std::string bundleName = GetBundleName(destUri);
        std::string fileName = GetFileName(srcPhysicalPath);
        // copy to tmp path
        dstPhysicalPath = "/data/service/el2/" + std::to_string(userId) + "/hmdfs/account/data/" + bundleName +
            "/" + copyPath + "/" + fileName;
    }
    infos->srcPath = srcPhysicalPath;
    infos->destPath = dstPhysicalPath;
    infos->srcUriIsFile = IsMediaUri(infos->srcUri) || IsFile(infos->srcPath);
    infos->callingUid = IPCSkeleton::GetCallingUid();
    AddFileInfos(infos);
    return E_OK;
}

void RemoteFileCopyManager::RemoveFileInfos(std::shared_ptr<FileInfos> infos)
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

int32_t RemoteFileCopyManager::RemoteCancel(const std::string &srcUri, const std::string &destUri)
{
    LOGI("RemoteCancel");
    std::lock_guard<std::mutex> lock(FileInfosVecMutex_);
    int32_t ret = 0;
    if (!FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(srcUri)) ||
        !FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(destUri))) {
        LOGE("path is forbidden");
        RadarParaInfo info = {"RemoteCancel", ReportLevel::INNER, DfxBizStage::HMDFS_COPY,
            DEFAULT_PKGNAME, "", EINVAL, "path is forbidden"};
        DfsRadar::GetInstance().ReportFileAccess(info);
        return EINVAL;
    }
    for (auto item = FileInfosVec_.begin(); item != FileInfosVec_.end();) {
        if ((*item)->srcUri != srcUri || (*item)->destUri != destUri) {
            ++item;
            continue;
        }
        auto callingUid = IPCSkeleton::GetCallingUid();
        if (callingUid != (*item)->callingUid) {
            LOGE("RemoteCancel failed, calling uid=%{public}d has no permission to cancel copy for uid=%{public}d.",
            callingUid, (*item)->callingUid);
            RadarParaInfo info = {"RemoteCancel", ReportLevel::INNER, DfxBizStage::HMDFS_COPY,
                DEFAULT_PKGNAME, "", EPERM, "RemoteCancel failed"};
            DfsRadar::GetInstance().ReportFileAccess(info);
            return EPERM;
        }
        LOGI("RemoteCancel success");
        (*item)->needCancel.store(true);
        item = FileInfosVec_.erase(item);
        return ret;
    }
    return E_OK;
}

int32_t RemoteFileCopyManager::RemoteCopy(const std::string &srcUri, const std::string &destUri,
    const sptr<IFileTransListener> &listener, const int32_t userId, const std::string &copyPath)
{
    LOGI("RemoteCopy start");
    if (srcUri.empty() || destUri.empty() || listener == nullptr) {
        LOGE("invalid input param");
        return EINVAL;
    }
    if (!FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(srcUri)) ||
        !FileSizeUtils::IsFilePathValid(FileSizeUtils::GetRealUri(destUri))) {
        LOGE("path is forbidden");
        RadarParaInfo info = {"RemoteCopy", ReportLevel::INNER, DfxBizStage::HMDFS_COPY,
            DEFAULT_PKGNAME, "", EINVAL, "path is forbidden"};
        DfsRadar::GetInstance().ReportFileAccess(info);
        return EINVAL;
    }
    auto infos = std::make_shared<FileInfos>();
    auto ret = CreateFileInfos(srcUri, destUri, infos, userId, copyPath);
    if (ret != E_OK) {
        LOGE("CreateFileInfos failed,ret= %{public}d", ret);
        return ret;
    }
    std::function<void(uint64_t processSize, uint64_t totalSize)> processCallback =
        [&listener](uint64_t processSize, uint64_t totalSize) -> void {
        listener->OnFileReceive(totalSize, processSize);
    };
    infos->localListener = FileCopyLocalListener::GetLocalListener(infos->srcPath,
        infos->srcUriIsFile, processCallback);
    auto result = FileCopyManager::GetInstance().ExecLocal(infos);
    if (ChangeOwnerRecursive(infos->destPath, infos->callingUid, infos->callingUid) != 0) {
        LOGE("ChangeOwnerRecursive failed, calling uid= %{public}d", infos->callingUid);
    }
    RemoveFileInfos(infos);
    infos->localListener->StopListener();

    if (result != E_OK) {
        return result;
    }
    result = infos->localListener->GetResult();
    if (result != E_OK) {
        listener->OnFailed("", result);
        return result;
    }
    listener->OnFinished("");
    LOGI("RemoteCopy end");
    return E_OK;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
