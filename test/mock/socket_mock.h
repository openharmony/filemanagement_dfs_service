/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#ifndef OHOS_DFS_SERVICE_SOCKET_MOCK_H
#define OHOS_DFS_SERVICE_SOCKET_MOCK_H

#include <gmock/gmock.h>
#include <memory>

#include "socket.h"

namespace OHOS::Storage::DistributedFile {
class DfsSocket {
public:
    virtual ~DfsSocket() = default;
public:
    virtual int32_t Socket(SocketInfo info) = 0;
    virtual int32_t Listen(int32_t, const QosTV qos[], uint32_t, const ISocketListener *) = 0;
    virtual int32_t Bind(int32_t, const QosTV qos[], uint32_t, const ISocketListener *) = 0;
    virtual int SendFile(int32_t, const char *sFileList[], const char *dFileList[], uint32_t) = 0;
public:
    static inline std::shared_ptr<DfsSocket> dfsSocket = nullptr;
};

class SocketMock : public DfsSocket {
public:
    MOCK_METHOD1(Socket, int32_t(SocketInfo info));
    MOCK_METHOD4(Listen, int32_t(int32_t, const QosTV qos[], uint32_t, const ISocketListener *));
    MOCK_METHOD4(Bind, int32_t(int32_t, const QosTV qos[], uint32_t, const ISocketListener *));
    MOCK_METHOD4(SendFile, int(int32_t, const char *sFileList[], const char *dFileList[], uint32_t));
};
}
#endif