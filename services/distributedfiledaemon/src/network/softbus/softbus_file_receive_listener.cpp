/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "network/softbus/softbus_file_receive_listener.h"

#include <cinttypes>
#include <thread>

#include "dfs_daemon_event_dfx.h"
#include "dfs_error.h"
#include "network/softbus/softbus_handler.h"
#include "sandbox_helper.h"
#include "trans_mananger.h"
#include "utils_directory.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
using namespace FileManagement;
std::string SoftBusFileReceiveListener::path_ = "";
static const int32_t SLEEP_TIME_MS = 10;
void SoftBusFileReceiveListener::OnFile(int32_t socket, FileEvent *event)
{
    if (event == nullptr) {
        LOGE("invalid parameter");
        return;
    }
    switch (event->type) {
        case FILE_EVENT_RECV_START:
            OnReceiveFileStarted(socket, event->fileCnt);
            break;
        case FILE_EVENT_RECV_PROCESS:
            OnReceiveFileProcess(socket, event->bytesProcessed, event->bytesTotal);
            break;
        case FILE_EVENT_RECV_FINISH:
            OnReceiveFileFinished(socket, event->fileCnt);
            break;
        case FILE_EVENT_RECV_ERROR:
            OnFileTransError(socket, event->errorCode);
            break;
        case FILE_EVENT_TRANS_STATUS:
            OnReceiveFileReport(socket, event->statusList, event->errorCode);
            break;
        case FILE_EVENT_RECV_UPDATE_PATH:
            event->UpdateRecvPath = GetRecvPath;
            break;
        default:
            LOGI("Other situations");
            break;
    }
}

const char* SoftBusFileReceiveListener::GetRecvPath()
{
    const char* path = path_.c_str();
    LOGI("path: %s", path);
    return path;
}

void SoftBusFileReceiveListener::SetRecvPath(const std::string &physicalPath)
{
    if (physicalPath.empty()) {
        LOGI("SetRecvPath physicalPath is empty.");
        return;
    }
    LOGI("SetRecvPath physicalPath: %{public}s", GetAnonyString(physicalPath).c_str());
    if (!AppFileService::SandboxHelper::CheckValidPath(physicalPath)) {
        LOGE("invalid path.");
        RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
            RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
            RadarReporter::CREAT_VALID_PATH_ERROR, RadarReporter::PACKAGE_NAME, RadarReporter::appFileService);
        path_ = "";
        return;
    }
    path_ = physicalPath;
}

void SoftBusFileReceiveListener::OnCopyReceiveBind(int32_t socketId, PeerSocketInfo info)
{
    LOGI("OnCopyReceiveBind begin, socketId %{public}d", socketId);
    bindSuccess.store(false);
    SoftBusHandler::OnSinkSessionOpened(socketId, info);
    bindSuccess.store(true);
}

std::string SoftBusFileReceiveListener::GetLocalSessionName(int32_t sessionId)
{
    std::string sessionName = "";
    sessionName = SoftBusHandler::GetSessionName(sessionId);
    return sessionName;
}

void SoftBusFileReceiveListener::OnReceiveFileStarted(int32_t sessionId, int32_t fileCnt)
{
    LOGI("OnReceiveFileStarted, sessionId = %{public}d, fileCnt = %{public}d", sessionId, fileCnt);
}

void SoftBusFileReceiveListener::OnReceiveFileProcess(int32_t sessionId, uint64_t bytesUpload, uint64_t bytesTotal)
{
    LOGI("OnReceiveFileProcess, sessionId = %{public}d, bytesUpload = %{public}" PRIu64 ","
         "bytesTotal = %{public}" PRIu64 "", sessionId, bytesUpload, bytesTotal);
    while (!bindSuccess.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
    }
    std::string sessionName = GetLocalSessionName(sessionId);
    if (sessionName.empty()) {
        LOGE("sessionName is empty");
        return;
    }
    TransManager::GetInstance().NotifyFileProgress(sessionName, bytesTotal, bytesUpload);
}

void SoftBusFileReceiveListener::OnReceiveFileFinished(int32_t sessionId, int32_t fileCnt)
{
    LOGI("OnReceiveFileFinished, sessionId = %{public}d, fileCnt = %{public}d", sessionId, fileCnt);
    std::string sessionName = GetLocalSessionName(sessionId);
    if (sessionName.empty()) {
        LOGE("sessionName is empty");
        return;
    }
    TransManager::GetInstance().NotifyFileFinished(sessionName);
    TransManager::GetInstance().DeleteTransTask(sessionName);
    SoftBusHandler::GetInstance().ChangeOwnerIfNeeded(sessionId, sessionName);
}

void SoftBusFileReceiveListener::OnFileTransError(int32_t sessionId, int32_t errorCode)
{
    LOGE("OnFileTransError");
    RADAR_REPORT(RadarReporter::DFX_SET_DFS, RadarReporter::DFX_SET_BIZ_SCENE, RadarReporter::DFX_FAILED,
        RadarReporter::BIZ_STATE, RadarReporter::DFX_END, RadarReporter::ERROR_CODE,
        RadarReporter::FILE_TRANS_ERROR, RadarReporter::PACKAGE_NAME,
        RadarReporter::dSoftBus + std::to_string(errorCode));
    std::string sessionName = GetLocalSessionName(sessionId);
    if (sessionName.empty()) {
        LOGE("sessionName is empty");
        return;
    }
    TransManager::GetInstance().NotifyFileFailed(sessionName, errorCode);
    TransManager::GetInstance().DeleteTransTask(sessionName);
    SoftBusHandler::GetInstance().CloseSession(sessionId, sessionName);
}

void SoftBusFileReceiveListener::OnReceiveFileReport(int32_t sessionId, FileStatusList statusList, int32_t errorCode)
{
    LOGE("OnReceiveFileReport");
    std::string sessionName = GetLocalSessionName(sessionId);
    if (sessionName.empty()) {
        LOGE("sessionName is empty");
        return;
    }
    TransManager::GetInstance().NotifyFileFailed(sessionName, errorCode);
}

void SoftBusFileReceiveListener::OnReceiveFileShutdown(int32_t sessionId, ShutdownReason reason)
{
    LOGI("OnReceiveFileShutdown, sessionId is %{public}d", sessionId);
    std::string sessionName = GetLocalSessionName(sessionId);
    if (sessionName.empty()) {
        LOGE("sessionName is empty");
        return;
    }
    TransManager::GetInstance().NotifyFileFailed(sessionName, E_SOFTBUS_SESSION_FAILED);
    TransManager::GetInstance().DeleteTransTask(sessionName);
    SoftBusHandler::GetInstance().CloseSession(sessionId, sessionName);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS