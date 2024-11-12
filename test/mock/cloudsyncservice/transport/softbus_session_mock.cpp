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

#include "softbus_session_mock.h"

namespace OHOS::FileManagement::CloudSync {

SoftbusSession::SoftbusSession(const std::string &peerDeviceId, const std::string &sessionName, DataType type)
    : peerDeviceId_(peerDeviceId), sessionName_(sessionName), type_(type)
{
}

int32_t SoftbusSession::Start()
{
    if (ISoftbusSessionMock::iSoftbusSessionMock_ == nullptr) {
        return -1;
    }
    return ISoftbusSessionMock::iSoftbusSessionMock_->Start();
}

int32_t SoftbusSession::Stop()
{
    return 0;
}

int32_t SoftbusSession::SendData(const void *data, uint32_t dataLen)
{
    if (ISoftbusSessionMock::iSoftbusSessionMock_ == nullptr) {
        return -1;
    }
    return ISoftbusSessionMock::iSoftbusSessionMock_->SendData(data, dataLen);
}

int32_t SoftbusSession::SendFile(const std::vector<std::string> &sFileList, const std::vector<std::string> &dFileList)
{
    if (ISoftbusSessionMock::iSoftbusSessionMock_ == nullptr) {
        return -1;
    }
    return ISoftbusSessionMock::iSoftbusSessionMock_->SendFile(sFileList, dFileList);
}

int32_t SoftbusSession::WaitSessionOpened(int sessionId)
{
    return 0;
}

SoftbusSession::DataType SoftbusSession::GetDataType()
{
    return type_;
}

std::string SoftbusSession::GetPeerDeviceId()
{
    return peerDeviceId_;
}

int32_t SoftbusSession::GetSessionId()
{
    return 0;
}
}
