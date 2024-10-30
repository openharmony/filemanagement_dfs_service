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
#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_MOCK_H

#include <gmock/gmock.h>
#include "network/softbus/softbus_handler.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class ISoftBusHandlerMock {
public:
    virtual ~ISoftBusHandlerMock() = default;

    virtual int32_t CreateSessionServer(const std::string &packageName, const std::string &sessionName,
                                        DFS_CHANNEL_ROLE role, const std::string physicalPath) = 0;
    virtual int32_t OpenSession(const std::string &mySessionName, const std::string &peerSessionName,
                                const std::string &peerDevId, int32_t &socketId) = 0;
    virtual int32_t CopySendFile(int32_t socketId,
                                 const std::string &sessionName,
                                 const std::string &srcUri,
                                 const std::string &dstUri) = 0;
    virtual std::string GetSessionName(int32_t sessionId) = 0;
    virtual bool IsSameAccount(const std::string &networkId) = 0;

public:
    static inline std::shared_ptr<ISoftBusHandlerMock> iSoftBusHandlerMock_ = nullptr;
};

class SoftBusHandlerMock : public ISoftBusHandlerMock {
public:
    MOCK_METHOD4(CreateSessionServer, int32_t(const std::string &packageName, const std::string &sessionName,
                                              DFS_CHANNEL_ROLE role, const std::string physicalPath));
    MOCK_METHOD4(OpenSession, int32_t(const std::string &mySessionName, const std::string &peerSessionName,
                                      const std::string &peerDevId, int32_t &socketId));
    MOCK_METHOD4(CopySendFile, int32_t(int32_t socketId,
                                       const std::string &sessionName,
                                       const std::string &srcUri,
                                       const std::string &dstUri));
    MOCK_METHOD1(GetSessionName, std::string(int32_t sessionId));
    MOCK_METHOD1(IsSameAccount, bool(const std::string &networkId));
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_MOCK_H
