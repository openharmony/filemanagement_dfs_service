/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "socket_mock.h"

#include "inner_socket.h"

using namespace OHOS::Storage::DistributedFile;
int32_t Socket(SocketInfo info)
{
    return DfsSocket::dfsSocket->Socket(info);
}

int32_t Listen(int32_t socket, const QosTV qos[], uint32_t qosCount, const ISocketListener *listener)
{
    return DfsSocket::dfsSocket->Listen(socket, qos, qosCount, listener);
}

int32_t Bind(int32_t socket, const QosTV qos[], uint32_t qosCount, const ISocketListener *listener)
{
    return DfsSocket::dfsSocket->Bind(socket, qos, qosCount, listener);
}

void Shutdown(int32_t socket)
{
    (void)socket;
}

int SendFile(int32_t socket, const char *sFileList[], const char *dFileList[], uint32_t fileCnt)
{
    return DfsSocket::dfsSocket->SendFile(socket, sFileList, dFileList, fileCnt);
}

int32_t DfsBind(int32_t socket, const ISocketListener *listener)
{
    return DfsSocket::dfsSocket->DfsBind(socket, listener);
}