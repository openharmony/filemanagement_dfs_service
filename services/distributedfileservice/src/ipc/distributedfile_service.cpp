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
    const std::string TEMP_PATH = "/data/system_ce/";
}

REGISTER_SYSTEM_ABILITY_BY_ID(DistributedFileService, STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID, false);

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

int32_t DistributedFileService::SendFile(const std::string &cid,
                                         const std::vector<std::string> &sourceFileList,
                                         const std::vector<std::string> &destinationFileList,
                                         const uint32_t fileCount)
{
    if (cid.empty() || fileCount <= 0 || sourceFileList.empty() || sourceFileList.size() != fileCount) {
        LOGE("SendFile params failed");
        return DFS_PARAM_FILE_COUNT_ERROR;
    }
    LOGI("DistributedFileService::SendFile cid %{public}s, fileCount %{public}d", cid.c_str(), fileCount);
    auto softBusAgent = SoftbusAgent::GetInstance();
    if (softBusAgent->SendFile(cid, sourceFileList, destinationFileList, fileCount) != DFS_SUCCESS) {
        return DFS_SOFTBUS_SEND_ERROR;
    }

    return DFS_SUCCESS;
}

int32_t DistributedFileService::OpenFile(int32_t fd, const std::string &fileName, int32_t mode)
{
    if (fd <= 0 || fileName.empty()) {
        return DFS_FILE_OP_ERROR;
    }

    std::string fullPath = TEMP_PATH + fileName;
    const char *tmpFile = fullPath.c_str();
    struct stat fileStat;
    bool fileExist = (stat(tmpFile, &fileStat) == 0);
    if (fileExist) {
        int32_t result = remove(tmpFile);
        if (result != 0) {
            close(fd);
            LOGE("DFS SA remove temp file result %{public}d, %{public}s, %{public}d", result, strerror(errno), errno);
            return DFS_FILE_OP_ERROR;
        }
    }

    int32_t writeFd = open(tmpFile, O_WRONLY | O_CREAT, (S_IREAD | S_IWRITE) | S_IRGRP | S_IROTH);
    if (writeFd <= 0) {
        close(fd);
        LOGE("DFS SA open temp file failed %{public}d, %{public}s, %{public}d", writeFd, strerror(errno), errno);
        return DFS_FILE_OP_ERROR;
    }
    auto buffer = std::make_unique<char[]>(FILE_BLOCK_SIZE);
    ssize_t actLen = 0;
    do {
        actLen = read(fd, buffer.get(), FILE_BLOCK_SIZE);
        if (actLen > 0) {
            write(writeFd, buffer.get(), actLen);
        } else if (actLen == 0) {
            break;
        } else {
            if (errno == EINTR) {
                actLen = FILE_BLOCK_SIZE;
            } else {
                close(fd);
                close(writeFd);
                return DFS_FILE_OP_ERROR;
            }
        }
    } while (actLen > 0);

    close(fd);
    close(writeFd);

    return DFS_SUCCESS;
}

int32_t DistributedFileService::RegisterNotifyCallback(sptr<IFileTransferCallback> &callback)
{
    SoftbusAgent::GetInstance()->SetTransCallback(callback);
    LOGD("DistributedFileService::RegisterNotifyCallback: cb[%{public}p]", callback->AsObject().GetRefPtr());
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