/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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
#include "distributedfile_service.h"

#include <exception>
#include <fcntl.h>
#include <unistd.h>

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"
#include "device_manager_agent.h"
#include "dfsu_fd_guard.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "message_parcel.h"
#include "parcel.h"
#include "softbus_agent.h"
#include "system_ability_definition.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
namespace {
    constexpr int32_t FILE_BLOCK_SIZE = 1024;
    constexpr char TEMP_FILE_NAME[] = "/data/system_ce/tmp";
}

REGISTER_SYSTEM_ABILITY_BY_ID(DistributedFileService, FILEMANAGEMENT_DISTRIBUTED_FILE_SERVICE_SA_ID, false);

void DistributedFileService::OnDump()
{
    LOGI("OnDump");
}

void DistributedFileService::OnStart()
{
    PublishSA();
    StartManagers();
}

void DistributedFileService::OnStop()
{
    LOGI("OnStop");
}

void DistributedFileService::PublishSA()
{
    bool ret = SystemAbility::Publish(this);
    if (!ret) {
        LOGE("Leave, publishing DistributedFileService failed!");
        return;
    }
}

void DistributedFileService::StartManagers()
{
    DeviceManagerAgent::GetInstance();
}

int32_t DistributedFileService::CreateSourceResources(const std::vector<std::string> &sourceFileList,
    std::vector<char *> &sFileList)
{
    if (sourceFileList.size() != 1) {
        return DFS_MEM_ERROR;
    }
    for (int index = 0; index < sourceFileList.size(); ++index) {
        LOGI("DistributedFileService::SendFile Source File List %{public}d, %{public}s, %{public}zu",
            index, sourceFileList.at(index).c_str(), sourceFileList.at(index).length());
        if (index == 0) {
            std::string tmpString("/data/system_ce/tmp");
            size_t length = tmpString.length();
            if (length <= 0 || length > SIZE_MAX - 1) {
                return DFS_MEM_ERROR;
            }
            char *sTemp = new char[length + 1];
            if (sTemp == nullptr) {
                return DFS_MEM_ERROR;
            }
            sFileList.push_back(sTemp);
            if (memset_s(sTemp, length + 1, '\0', length + 1) != EOK) {
                return DFS_MEM_ERROR;
            }
            if (memcpy_s(sTemp, length + 1, tmpString.c_str(), length) != EOK) {
                return DFS_MEM_ERROR;
            }
            sTemp[length] = '\0';
        } else {
            size_t length = sourceFileList.at(index).length();
            if (length <= 0 || length > SIZE_MAX - 1) {
                return DFS_MEM_ERROR;
            }
            char *sTemp = new char[length + 1];
            if (sTemp == nullptr) {
                return DFS_MEM_ERROR;
            }
            sFileList.push_back(sTemp);
            if (memset_s(sTemp, length + 1, '\0', length + 1) != EOK) {
                return DFS_MEM_ERROR;
            }
            if (memcpy_s(sTemp, length + 1, sourceFileList.at(index).c_str(), length) != EOK) {
                return DFS_MEM_ERROR;
            }
            sTemp[length] = '\0';
        }
    }
    return DFS_NO_ERROR;
}

int32_t DistributedFileService::CreateDestResources(const std::vector<std::string> &destinationFileList,
    std::vector<char *> &dFileList)
{
    if (destinationFileList.size() > 1) {
        return DFS_MEM_ERROR;
    }

    for (int index = 0; index < destinationFileList.size(); ++index) {
        size_t length = destinationFileList.at(index).length();
        if (length <= 0 || length > SIZE_MAX - 1) {
            return DFS_MEM_ERROR;
        }
        char *sTemp = new char[length + 1];
        if (sTemp == nullptr) {
            return DFS_MEM_ERROR;
        }
        dFileList.push_back(sTemp);
        if (memset_s(sTemp, length + 1, '\0', length + 1) != EOK) {
            return DFS_MEM_ERROR;
        }
        if (memcpy_s(sTemp, length + 1, destinationFileList.at(index).c_str(), length) != EOK) {
            return DFS_MEM_ERROR;
        }
        sTemp[length] = '\0';
    }
    return DFS_NO_ERROR;
}

void DistributedFileService::DestroyFileList(std::vector<char *> &fileList)
{
    if (fileList.empty()) {
        LOGE("Send file list is empty");
        return;
    }

    for (size_t index = 0; index < fileList.size(); ++index) {
        char *temp = fileList.at(index);
        if (temp != nullptr) {
            LOGI("destroy list index [%{public}zu] memory", index);
            delete[] temp;
        }
    }
    fileList.clear();
}

int32_t DistributedFileService::SendFile(const std::string &cid,
                                         const std::vector<std::string> &sourceFileList,
                                         const std::vector<std::string> &destinationFileList,
                                         const uint32_t fileCount)
{
    int32_t result = DFS_NO_ERROR;
    if (cid.empty() || fileCount != 1 || sourceFileList.empty() || sourceFileList.size() != fileCount ||
        destinationFileList.size() > 1) {
        LOGE("SendFile params failed");
        return DFS_PARAM_FILE_COUNT_ERROR;
    }

    std::vector<char *> sFileList;
    std::vector<char *> dFileList;

    result = CreateSourceResources(sourceFileList, sFileList);

    if (!destinationFileList.empty()) {
        result = CreateDestResources(destinationFileList, dFileList);
    }

    if (result != DFS_MEM_ERROR) {
        auto softBusAgent = SoftbusAgent::GetInstance();
        result = softBusAgent->SendFile(cid, (const char **)sFileList.data(),
            (const char **)dFileList.data(), fileCount);
    }

    DestroyFileList(sFileList);
    DestroyFileList(dFileList);
    return result;
}

int32_t DistributedFileService::OpenFile(int32_t fd, const std::string &fileName, int32_t mode)
{
    if (fd <= 0 || fileName.empty()) {
        return DFS_FILE_OP_ERROR;
    }
    DfsuFDGuard readFd(fd);

    const char *tmpFile = TEMP_FILE_NAME;
    struct stat fileStat;
    bool fileExist = (stat(tmpFile, &fileStat) == 0);
    if (fileExist) {
        int32_t result = remove(tmpFile);
        if (result != 0) {
            LOGE("DFS SA remove temp file result %{public}d, %{public}d", result, errno);
            return DFS_FILE_OP_ERROR;
        }
    }

    DfsuFDGuard writeFd(open(tmpFile, O_WRONLY | O_CREAT, (S_IREAD | S_IWRITE) | S_IRGRP | S_IROTH));
    if (!writeFd) {
        LOGE("DFS SA open temp file failed %{public}d", errno);
        return DFS_FILE_OP_ERROR;
    }
    auto buffer = std::make_unique<char[]>(FILE_BLOCK_SIZE);
    ssize_t actLen = 0;
    do {
        actLen = read(readFd.GetFD(), buffer.get(), FILE_BLOCK_SIZE);
        if (actLen > 0) {
            write(writeFd.GetFD(), buffer.get(), actLen);
        } else if (actLen == 0) {
            break;
        } else {
            if (errno == EINTR) {
                actLen = FILE_BLOCK_SIZE;
            } else {
                return DFS_FILE_OP_ERROR;
            }
        }
    } while (actLen > 0);

    return DFS_SUCCESS;
}

int32_t DistributedFileService::RegisterNotifyCallback(sptr<IFileTransferCallback> &callback)
{
    SoftbusAgent::GetInstance()->SetTransCallback(callback);
    return DFS_SUCCESS;
}

int32_t DistributedFileService::UnRegisterNotifyCallback()
{
    SoftbusAgent::GetInstance()->RemoveTransCallbak();
    return DFS_SUCCESS;
}

int32_t DistributedFileService::IsDeviceOnline(const std::string &cid)
{
    std::set<std::string> onlineDevice = DeviceManagerAgent::GetInstance()->getOnlineDevs();
    if (onlineDevice.find(cid) != onlineDevice.end()) {
        return DEVICE_ONLINE;
    }
    return DEVICE_NOT_ONLINE;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS