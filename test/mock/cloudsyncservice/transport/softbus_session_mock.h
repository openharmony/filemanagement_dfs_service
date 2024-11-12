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
#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_MOCK_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_MOCK_H

#include <gmock/gmock.h>
#include "transport/softbus/softbus_session.h"

namespace OHOS::FileManagement::CloudSync {
class ISoftbusSessionMock {
public:
    enum DataType : int32_t {
        TYPE_BYTES,
        TYPE_FILE,
    };
    ISoftbusSessionMock() = default;
    ISoftbusSessionMock(const std::string &peerDeviceId, const std::string &sessionName, DataType type);
    virtual ~ISoftbusSessionMock() = default;

    virtual int32_t Start() = 0;

    virtual int32_t SendData(const void *data, uint32_t dataLen) = 0;
    virtual int32_t SendFile(const std::vector<std::string> &sFileList,
        const std::vector<std::string> &dFileList) = 0;
    virtual int32_t WaitSessionOpened(int sessionId) = 0;
public:
    static inline std::shared_ptr<ISoftbusSessionMock> iSoftbusSessionMock_ = nullptr;
};

class SoftbusSessionMock : public ISoftbusSessionMock {
public:
    MOCK_METHOD0(Start, int32_t());
    MOCK_METHOD0(Stop, int32_t());
    MOCK_METHOD2(SendData, int32_t(const void *data, uint32_t dataLen));
    MOCK_METHOD2(SendFile, int32_t(const std::vector<std::string> &sFileList,
        const std::vector<std::string> &dFileList));
    MOCK_METHOD1(WaitSessionOpened, int32_t(int sessionId));
};
}
#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_SESSION_MOCK_H
