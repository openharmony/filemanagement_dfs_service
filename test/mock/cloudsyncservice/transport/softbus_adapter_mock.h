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
#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_ADAPTER_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_ADAPTER_MOCK_H

#include <gmock/gmock.h>
#include "transport/softbus/softbus_adapter.h"

namespace OHOS::FileManagement::CloudSync {
class ISoftbusAdapterMock {
public:
    ISoftbusAdapterMock() = default;
    virtual ~ISoftbusAdapterMock() = default;

    virtual int32_t CreateSessionServer(const char *packageName, const char *sessionName) = 0;
    virtual int32_t OpenSession(char *sessionName, char *peerDeviceId, char *groupId, TransDataType dataType) = 0;
    virtual int OpenSessionByP2P(char *sessionName, char *peerDeviceId, char *groupId, bool isFileType) = 0;

    virtual int SendBytes(int sessionId, const void *data, unsigned int dataLen) = 0;
    virtual int SendFile(int sessionId, const std::vector<std::string> &sFileList,
        const std::vector<std::string> &dFileList) = 0;
public:
    static inline std::shared_ptr<ISoftbusAdapterMock> iSoftbusAdapterMock_ = nullptr;
};

class SoftbusAdapterMock : public ISoftbusAdapterMock {
public:
    MOCK_METHOD2(CreateSessionServer, int32_t(const char *packageName, const char *sessionName));
    MOCK_METHOD4(OpenSession, int32_t(char *sessionName, char *peerDeviceId, char *groupId, TransDataType dataType));
    MOCK_METHOD4(OpenSessionByP2P, int(char *sessionName, char *peerDeviceId, char *groupId, bool isFileType));
    MOCK_METHOD3(SendBytes, int(int sessionId, const void *data, unsigned int dataLen));
    MOCK_METHOD3(SendFile, int(int sessionId, const std::vector<std::string> &sFileList,
        const std::vector<std::string> &dFileList));
};
}
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_ADAPTER_MOCK_H
