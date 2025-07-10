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
#ifndef FILEMANAGEMENT_DFS_SERVICE_CHANNEL_MANAGER_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_CHANNEL_MANAGER_MOCK_H

#include "channel_manager.h"
#include <gmock/gmock.h>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class IChannelManagerMock {
public:
    IChannelManagerMock() = default;
    virtual ~IChannelManagerMock() = default;

    virtual int32_t CreateClientChannel(const std::string &networkId) = 0;
    virtual int32_t DestroyClientChannel(const std::string &networkId) = 0;
    virtual bool HasExistChannel(const std::string &networkId) = 0;

    virtual int32_t SendRequest(const std::string &networkId,
                                ControlCmd &request,
                                ControlCmd &response,
                                bool needResponse = false) = 0;
    virtual int32_t SendBytes(const std::string &networkId, const std::string &data) = 0;

public:
    static inline std::shared_ptr<IChannelManagerMock> iChannelManagerMock = nullptr;
};

class ChannelManagerMock : public IChannelManagerMock {
public:
    MOCK_METHOD1(CreateClientChannel, int32_t(const std::string &networkId));
    MOCK_METHOD1(DestroyClientChannel, int32_t(const std::string &networkId));
    MOCK_METHOD1(HasExistChannel, bool(const std::string &networkId));
    MOCK_METHOD4(SendRequest,
                 int32_t(const std::string &networkId, ControlCmd &request, ControlCmd &response, bool needResponse));
    MOCK_METHOD2(SendBytes, int32_t(const std::string &networkId, const std::string &data));
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_CHANNEL_MANAGER_MOCK_H