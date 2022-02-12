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
    constexpr char TEMP_FILE_NAME[] = "/data/system_ce/tmp";
}

REGISTER_SYSTEM_ABILITY_BY_ID(DistributedFileService, STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID, false);

DistributedFileService::DistributedFileService() : SystemAbility(STORAGE_DISTRIBUTED_FILE_SERVICE_SA_ID, false) {}

DistributedFileService::~DistributedFileService() {}

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
    bool ret = SystemAbility::Publish(DelayedSingleton<DistributedFileService>::GetInstance().get());
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
    if (fileCount < 0) {
        return -1;
    }
    char **sFileList = new char* [fileCount];
    for (int index = 0; index < sourceFileList.size(); ++index) {
        LOGI("DistributedFileService::SendFile Source File List %{public}d, %{public}s, %{public}d",
            index, sourceFileList.at(index).c_str(), sourceFileList.at(index).length());
        if (index == 0) {
            std::string tmpString("/data/system_ce/tmp");
            int32_t length = tmpString.length();
            sFileList[index] = new char[length + 1];
            memset_s(sFileList[index], length + 1, '\0', length + 1);
            int ret = memcpy_s(sFileList[index], length + 1, tmpString.c_str(), length);
            if (ret != EOK) {
                LOGE("memory copy failed");
                return -1;
            }
            sFileList[index][length] = '\0';
        } else {
            int32_t length = sourceFileList.at(index).length();
            sFileList[index] = new char[length + 1];
            memset_s(sFileList[index], length + 1, '\0', length + 1);
            int ret = memcpy_s(sFileList[index], length + 1, sourceFileList.at(index).c_str(), length);
            if (ret != EOK) {
                LOGE("memory copy failed");
                return -1;
            }
            sFileList[index][length] = '\0';
        }
    }

    char **dFileList = new char* [fileCount];
    for (int index = 0; index < destinationFileList.size(); ++index) {
        LOGI("DistributedFileService::SendFile Destination File List %{public}d, %{public}s",
            index, destinationFileList.at(index).c_str());
        int32_t length = destinationFileList.at(index).length();
        dFileList[index] = new char[length + 1];
        memset_s(dFileList[index], length + 1, '\0', length + 1);
        int ret = memcpy_s(dFileList[index], length + 1, destinationFileList.at(index).c_str(), length);
        if (ret != EOK) {
            LOGE("memory copy failed");
            return -1;
        }
        dFileList[index][length] = '\0';
    }

    auto softBusAgent = SoftbusAgent::GetInstance();
    int32_t result = softBusAgent->SendFile(cid, (const char **)sFileList, (const char **)dFileList, fileCount);
    return result;
}

int32_t DistributedFileService::OpenFile(int32_t fd, const std::string &fileName, int32_t mode)
{
    if (fd <= 0) {
        return -1;
    }

    const char *tmpFile = TEMP_FILE_NAME;
    struct stat fileStat;
    bool fileExist = (stat(tmpFile, &fileStat) == 0);
    if (fileExist) {
        int32_t result = remove(tmpFile);
        if (result != 0) {
            LOGE("DFS SA remove temp file result %{public}d, %{public}s, %{public}d", result, strerror(errno), errno);
            return -1;
        }
    }

    int32_t writeFd = open(tmpFile, O_WRONLY | O_CREAT, (S_IREAD & S_IWRITE) | S_IRGRP | S_IROTH);
    if (writeFd <= 0) {
        LOGE("DFS SA open temp file failed %{public}d, %{public}s, %{public}d", writeFd, strerror(errno), errno);
        return -1;
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
                return -1;
            }
        }
    } while (actLen > 0);
    close(writeFd);
    return 0;
}

int32_t DistributedFileService::RegisterNotifyCallback(sptr<IFileTransferCallback> &callback)
{
    SoftbusAgent::GetInstance()->SetTransCallback(callback);
    return 0;
}

int32_t DistributedFileService::UnRegisterNotifyCallback()
{
    SoftbusAgent::GetInstance()->RemoveTransCallbak();
    return 0;
}

int32_t DistributedFileService::IsDeviceOnline(const std::string &cid)
{
    std::set<std::string> onlineDevice = DeviceManagerAgent::GetInstance()->getOnlineDevs();
    if (onlineDevice.find(cid) != onlineDevice.end()) {
        return 1;
    }
    return 0;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
