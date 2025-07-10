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

#include "channel_manager_mock.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

ChannelManager &ChannelManager::GetInstance()
{
    static ChannelManager instance;
    return instance;
}

int32_t ChannelManager::CreateClientChannel(const std::string &networkId)
{
    return IChannelManagerMock::iChannelManagerMock->CreateClientChannel(networkId);
}

int32_t ChannelManager::DestroyClientChannel(const std::string &networkId)
{
    return IChannelManagerMock::iChannelManagerMock->DestroyClientChannel(networkId);
}

bool ChannelManager::HasExistChannel(const std::string &networkId)
{
    return IChannelManagerMock::iChannelManagerMock->HasExistChannel(networkId);
}

int32_t ChannelManager::SendBytes(const std::string &networkId, const std::string &data)
{
    return IChannelManagerMock::iChannelManagerMock->SendBytes(networkId, data);
}

int32_t ChannelManager::SendRequest(const std::string &networkId,
                                    ControlCmd &request,
                                    ControlCmd &response,
                                    bool needResponse)
{
    return IChannelManagerMock::iChannelManagerMock->SendRequest(networkId, request, response, needResponse);
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS