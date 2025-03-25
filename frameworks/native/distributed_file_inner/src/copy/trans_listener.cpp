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

#include "copy/trans_listener.h"

#include <dirent.h>
#include <filesystem>
#include <random>

#include "dfs_error.h"
#include "daemon_proxy.h"
#include "ipc_skeleton.h"
#include "sandbox_helper.h"
#include "uri.h"
#include "utils_log.h"
#include "distributed_file_daemon_manager_impl.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD001600
#define LOG_TAG "distributedfile_daemon"
 
namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace AppFileService;
using namespace FileManagement;
static const std::string TRANS_NETWORK_PARA = "?networkid=";
static const std::string FILE_MANAGER_AUTHORITY = "docs";
static const std::string MEDIA_AUTHORITY = "media";
static const std::string DISTRIBUTED_PATH = "/data/storage/el2/distributedfiles/";

TransListener::TransListener(const std::string &destUri, ProcessCallback &processCallback)
{
    processCallback_ = processCallback;
    Uri uri(destUri);
    hmdfsInfo_.authority = uri.GetAuthority();
    hmdfsInfo_.sandboxPath = SandboxHelper::Decode(uri.GetPath());
    CreateTmpDir();
}

TransListener::~TransListener()
{
    RmTmpDir();
}

int32_t TransListener::WaitForCopyResult()
{
    LOGI("WaitForCopyResult.");
    std::unique_lock<std::mutex> lock(cvMutex_);
    cv_.wait(lock, [this]() {
        return copyEvent_.copyResult == SUCCESS || copyEvent_.copyResult == FAILED;
    });
    return copyEvent_.copyResult;
}

int32_t TransListener::CreateTmpDir()
{
    std::string tmpDir;
    if (hmdfsInfo_.authority != FILE_MANAGER_AUTHORITY && hmdfsInfo_.authority  != MEDIA_AUTHORITY) {
        tmpDir = CreateDfsCopyPath();
        disSandboxPath_ = DISTRIBUTED_PATH + tmpDir;
        std::error_code errCode;
        if (!std::filesystem::create_directory(disSandboxPath_, errCode)) {
            LOGE("Create dir failed, error code: %{public}d", errCode.value());
            return errCode.value();
        }

        auto pos = hmdfsInfo_.sandboxPath.rfind('/');
        if (pos == std::string::npos) {
            LOGE("invalid file path");
            return EIO;
        }
        auto sandboxDir = hmdfsInfo_.sandboxPath.substr(0, pos);
        if (std::filesystem::exists(sandboxDir, errCode)) {
            hmdfsInfo_.dirExistFlag = true;
        }
    }
    hmdfsInfo_.copyPath = tmpDir;
    return E_OK;
}

void TransListener::RmTmpDir()
{
    if (hmdfsInfo_.authority == FILE_MANAGER_AUTHORITY || hmdfsInfo_.authority == MEDIA_AUTHORITY) {
        LOGI("docs or media file, no need to rm tmp dir.");
        return;
    }
    std::filesystem::path pathName(disSandboxPath_);
    std::error_code errCode;
    if (std::filesystem::exists(pathName, errCode)) {
        std::filesystem::remove_all(pathName, errCode);
        if (errCode.value() != 0) {
            LOGE("Failed to remove directory, error code: %{public}d", errCode.value());
        }
    } else {
        LOGE("pathName is not exists, error code: %{public}d", errCode.value());
    }
}
 
int32_t TransListener::CopyToSandBox(const std::string &srcUri)
{
    if (hmdfsInfo_.authority == FILE_MANAGER_AUTHORITY || hmdfsInfo_.authority == MEDIA_AUTHORITY) {
        LOGI("docs or media path copy success.");
        return E_OK;
    }

    std::error_code errCode;
    if (std::filesystem::exists(hmdfsInfo_.sandboxPath) && std::filesystem::is_directory(hmdfsInfo_.sandboxPath)) {
        LOGI("Copy dir");
        std::filesystem::copy(disSandboxPath_, hmdfsInfo_.sandboxPath,
            std::filesystem::copy_options::recursive | std::filesystem::copy_options::update_existing, errCode);
        if (errCode.value() != 0) {
            LOGE("Copy dir failed: errCode: %{public}d", errCode.value());
            return EIO;
        }
    } else {
        LOGI("Copy file.");
        Uri uri(srcUri);
        auto fileName = GetFileName(uri.GetPath());
        if (fileName.empty()) {
            LOGE("Get filename failed");
            return EIO;
        }
        std::filesystem::copy(disSandboxPath_ + fileName, hmdfsInfo_.sandboxPath,
                              std::filesystem::copy_options::update_existing, errCode);
        if (errCode.value() != 0) {
            LOGE("Copy file failed: errCode: %{public}d", errCode.value());
            return EIO;
        }
    }
    LOGI("CopyToSandBox success.");
    return E_OK;
}

std::string TransListener::CreateDfsCopyPath()
{
    std::random_device rd;
    std::string random = std::to_string(rd());
    while (std::filesystem::exists(DISTRIBUTED_PATH + random)) {
        random = std::to_string(rd());
    }
    return random;
}
 
std::string TransListener::GetFileName(const std::string &path)
{
    auto pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        LOGE("invalid path");
        return "";
    }
    return SandboxHelper::Decode(path.substr(pos));
}
 
std::string TransListener::GetNetworkIdFromUri(const std::string &uri)
{
    return uri.substr(uri.find(TRANS_NETWORK_PARA) + TRANS_NETWORK_PARA.size(), uri.size());
}

int32_t TransListener::Cancel()
{
    auto distributedFileDaemonProxy = DistributedFileDaemonManagerImpl::GetDaemonInterface();
    if (distributedFileDaemonProxy == nullptr) {
        LOGE("proxy is null");
        return E_SA_LOAD_FAILED;
    }
    return distributedFileDaemonProxy->CancelCopyTask(hmdfsInfo_.sessionName);
}
 
int32_t TransListener::OnFileReceive(uint64_t totalBytes, uint64_t processedBytes)
{
    if (processCallback_ == nullptr) {
        LOGI("processCallback is nullptr");
        return ENOMEM;
    }
    processCallback_(processedBytes, totalBytes);
    return E_OK;
}
 
int32_t TransListener::OnFinished(const std::string &sessionName)
{
    LOGI("OnFinished");
    std::lock_guard<std::mutex> lock(cvMutex_);
    copyEvent_.copyResult = SUCCESS;
    cv_.notify_all();
    return E_OK;
}
 
int32_t TransListener::OnFailed(const std::string &sessionName, int32_t errorCode)
{
    LOGI("OnFailed, errorCode is %{public}d", errorCode);
    std::lock_guard<std::mutex> lock(cvMutex_);
    copyEvent_.copyResult = FAILED;
    copyEvent_.errorCode = errorCode;
    cv_.notify_all();
    return E_OK;
}
} // namespace ModuleFileIO
} // namespace FileManagement
} // namespace OHOS
