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

#include "softbus_session_listener_mock.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

int32_t SoftBusSessionListener::QueryActiveUserId()
{
    return 0;
}

std::vector<std::string> SoftBusSessionListener::GetFileName(const std::vector<std::string> &fileList,
                                                             const std::string &path,
                                                             const std::string &dstPath)
{
    if (ISoftBusSessionListenerMock::iSoftBusSessionListenerMock_ == nullptr) {
        return {};
    }
    return ISoftBusSessionListenerMock::iSoftBusSessionListenerMock_->GetFileName(fileList, path, dstPath);
}

void SoftBusSessionListener::OnSessionOpened(int32_t sessionId, PeerSocketInfo info)
{
    return;
}

void SoftBusSessionListener::OnSessionClosed(int32_t sessionId, ShutdownReason reason)
{
    return;
}

std::string SoftBusSessionListener::GetLocalUri(const std::string &uri)
{
    return "";
}

std::string SoftBusSessionListener::GetBundleName(const std::string &uri)
{
    if (ISoftBusSessionListenerMock::iSoftBusSessionListenerMock_ == nullptr) {
        return {};
    }
    return ISoftBusSessionListenerMock::iSoftBusSessionListenerMock_->GetBundleName(uri);
}

std::string SoftBusSessionListener::GetSandboxPath(const std::string &uri)
{
    return "";
}

std::string SoftBusSessionListener::GetRealPath(const std::string &srcUri)
{
    if (ISoftBusSessionListenerMock::iSoftBusSessionListenerMock_ == nullptr) {
        return {};
    }
    return ISoftBusSessionListenerMock::iSoftBusSessionListenerMock_->GetRealPath(srcUri);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS