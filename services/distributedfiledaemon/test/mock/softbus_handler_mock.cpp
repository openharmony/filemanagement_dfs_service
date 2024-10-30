/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "softbus_handler_mock.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
SoftBusHandler::SoftBusHandler() = default;
SoftBusHandler::~SoftBusHandler() = default;

SoftBusHandler &SoftBusHandler::GetInstance()
{
    static SoftBusHandler handle;
    return handle;
}

void SoftBusHandler::OnSinkSessionOpened(int32_t sessionId, PeerSocketInfo info)
{
    return;
}

bool SoftBusHandler::IsSameAccount(const std::string &networkId)
{
    return true;
}

std::string SoftBusHandler::GetSessionName(int32_t sessionId)
{
    return "test";
}

int32_t SoftBusHandler::CreateSessionServer(const std::string &packageName, const std::string &sessionName,
                                            DFS_CHANNEL_ROLE role, const std::string physicalPath)
{
    if (ISoftBusHandlerMock::iSoftBusHandlerMock_ == nullptr) {
        return -1;
    }
    return ISoftBusHandlerMock::iSoftBusHandlerMock_->CreateSessionServer(packageName, sessionName, role, physicalPath);
}

int32_t SoftBusHandler::OpenSession(const std::string &mySessionName, const std::string &peerSessionName,
                                    const std::string &peerDevId, int32_t &socketId)
{
    if (ISoftBusHandlerMock::iSoftBusHandlerMock_ == nullptr) {
        return -1;
    }
    return ISoftBusHandlerMock::iSoftBusHandlerMock_->OpenSession(mySessionName,
        peerSessionName, peerDevId, socketId);
}

bool SoftBusHandler::CreatSocketId(const std::string &mySessionName, const std::string &peerSessionName,
                                   const std::string &peerDevId, int32_t &socketId)
{
    return true;
}

int32_t SoftBusHandler::CopySendFile(int32_t socketId,
                                     const std::string &sessionName,
                                     const std::string &srcUri,
                                     const std::string &dstPath)
{
    if (ISoftBusHandlerMock::iSoftBusHandlerMock_ == nullptr) {
        return -1;
    }
    return ISoftBusHandlerMock::iSoftBusHandlerMock_->CopySendFile(socketId, sessionName, srcUri, dstPath);
}

void SoftBusHandler::ChangeOwnerIfNeeded(int32_t sessionId, const std::string sessionName)
{
    return;
}

void SoftBusHandler::CloseSession(int32_t sessionId, const std::string sessionName)
{
    return;
}

void SoftBusHandler::CloseSessionWithSessionName(const std::string sessionName)
{
    return;
}
void SoftBusHandler::RemoveNetworkId(int32_t socketId)
{
    return;
}

std::vector<int32_t> SoftBusHandler::GetsocketIdFromPeerNetworkId(const std::string &peerNetworkId)
{
    return {0};
}

bool SoftBusHandler::IsService(std::string &sessionName)
{
    return true;
}

void SoftBusHandler::CopyOnStop(const std::string &peerNetworkId)
{
    return;
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS