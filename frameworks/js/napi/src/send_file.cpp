/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "send_file.h"

#include <fcntl.h>
#include <unistd.h>
#include <tuple>

#include "dfs_filetransfer_callback.h"
#include "event_agent.h"
#include "filetransfer_callback_proxy.h"
#include "i_distributedfile_service.h"
#include "iservice_registry.h"
#include "service_proxy.h"
#include "system_ability_definition.h"
#include "system_ability_manager.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
EventAgent *SendFile::eventAgent_ = nullptr;

int32_t SendFile::RegisterCallback()
{
    sptr<ISystemAbilityManager> systemAbilityMgr =
    SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        LOGE("SendFile::RegisterCallback: Get ISystemAbilityManager failed\n");
        return NAPI_SENDFILE_SA_ERROR;
    }

    sptr<IRemoteObject> remote = systemAbilityMgr->GetSystemAbility(FILEMANAGEMENT_DISTRIBUTED_FILE_SERVICE_SA_ID);
    if (remote == nullptr) {
        LOGE("SendFile::RegisterCallback: SaId = %{public}d fail\n", FILEMANAGEMENT_DISTRIBUTED_FILE_SERVICE_SA_ID);
        return NAPI_SENDFILE_SA_ERROR;
    }

    sptr<IDistributedFileService> distributedFileService = iface_cast<IDistributedFileService>(remote);
    if (distributedFileService == nullptr) {
        LOGE("SendFile::RegisterCallback: nullptr\n");
        return NAPI_SENDFILE_IPC_ERROR;
    }

    sptr<IFileTransferCallback> callback = (std::make_unique<DfsFileTransferCallback>()).release();
    LOGD("SendFile::RegisterCallback: cb[%{public}p]", callback->AsObject().GetRefPtr());
    if (distributedFileService->RegisterNotifyCallback(callback) != IDistributedFileService::DFS_NO_ERROR) {
        LOGD("SendFile::RegisterCallback: remote call failed.\n");
        return NAPI_SENDFILE_IPC_ERROR;
    }
    return NAPI_SENDFILE_NO_ERROR;
}

int32_t SendFile::EmitTransEvent(std::unique_ptr<TransEvent> event, const std::string &cid)
{
    if (cid.empty()) {
        LOGE("SendFile::EmitTransEvent: deviceId is null.\n");
        return NAPI_SENDFILE_PARA_ERROR;
    }

    if (eventAgent_ == nullptr) {
        LOGE("SendFile::EmitTransEvent: event agent null.\n");
        return NAPI_SENDFILE_APP_AGENT_ERROR;
    }

    LOGI("SendFile::EmitTransEvent: %{public}s]", event.get()->GetName().c_str());
    eventAgent_->Emit(std::move(event));
    return NAPI_SENDFILE_NO_ERROR;
}

int32_t SendFile::WriteFile(int32_t fd, const std::string &fileName)
{
    if (fd <= 0) {
        LOGE("NapiWriteFile: file fd error.");
        return NAPI_SENDFILE_FD_ERROR;
    }
    std::string filePath = APP_PATH + fileName;
    unsigned int flags = O_WRONLY;

    if (!Utils::IsFileExist(filePath)) {
        flags |= O_CREAT;
    }

    LOGD("NapiWriteFile: path = [%{public}s]", filePath.c_str());
    char *realPath = realpath(filePath.c_str(), nullptr);
    if (realPath == nullptr) {
        LOGE("NapiWriteFile: null path");
        return NAPI_SENDFILE_FD_ERROR;
    }
    int32_t writeFd = open(realPath, static_cast<int>(flags), (S_IREAD | S_IWRITE) | S_IRGRP | S_IROTH);
    free(realPath);
    if (writeFd <= 0) {
        close(fd);
        LOGE("NapiWriteFile open file failed %{public}d, %{public}s, %{public}d", writeFd, filePath.c_str(), errno);
        return NAPI_SENDFILE_FD_ERROR;
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
                return NAPI_SENDFILE_FD_ERROR;
            }
        }
    } while (actLen > 0);

    close(fd);
    close(writeFd);

    return NAPI_SENDFILE_NO_ERROR;
}

int32_t SendFile::ExecSendFile(const std::string &deviceId, const std::vector<std::string>& srcList,
    const std::vector<std::string>& dstList, uint32_t num)
{
    if (deviceId.empty() || srcList.empty()) {
        LOGE("SendFile::ExecSendFile error: \"Device ID is empty\".\n");
        return NAPI_SENDFILE_PARA_ERROR;
    }

    sptr<ISystemAbilityManager> systemAbilityMgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        LOGE("SendFile::ExecSendFile: Get ISystemAbilityManager failed.\n");
        return NAPI_SENDFILE_SA_ERROR;
    }

    sptr<IRemoteObject> remote = systemAbilityMgr->CheckSystemAbility(FILEMANAGEMENT_DISTRIBUTED_FILE_SERVICE_SA_ID);
    if (remote == nullptr) {
        LOGE("SendFile::ExecSendFile: SaId = %{public}d fail.\n", FILEMANAGEMENT_DISTRIBUTED_FILE_SERVICE_SA_ID);
        return NAPI_SENDFILE_SA_ERROR;
    }

    sptr<IDistributedFileService> distributedFileService = iface_cast<IDistributedFileService>(remote);
    if (distributedFileService == nullptr) {
        LOGE("SendFile::ExecSendFile: get remote service proxy failed.\n");
        return NAPI_SENDFILE_IPC_ERROR;
    }

    char *realPath = realpath(srcList.at(0).c_str(), nullptr);
    if (realPath == nullptr) {
        return NAPI_SENDFILE_FD_ERROR;
    }
    int32_t fd = open(realPath, O_RDONLY);
    free(realPath);
    int32_t result = distributedFileService->OpenFile(fd, srcList.at(0), (S_IREAD | S_IWRITE) | S_IRGRP | S_IROTH);
    if (result != IDistributedFileService::DFS_SUCCESS) {
        LOGE("SendFile::ExecSendFile: error code %{public}d", result);
        return NAPI_SENDFILE_SEND_ERROR;
    }

    result = distributedFileService->SendFile(deviceId, srcList, dstList, num);
    if (result != IDistributedFileService::DFS_SENDFILE_SUCCESS) {
        LOGE("SendFile::ExecSendFile: error code %{public}d", result);
        return NAPI_SENDFILE_SEND_ERROR;
    }

    LOGD("SendFile::ExecSendFile: napi sendfile success.");
    return NAPI_SENDFILE_NO_ERROR;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS