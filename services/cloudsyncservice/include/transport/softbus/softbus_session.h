/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_FILEMGMT_SOFTBUS_SESSION_H
#define OHOS_FILEMGMT_SOFTBUS_SESSION_H

#include <mutex>
#include <string>
#include <vector>

namespace OHOS::FileManagement::CloudSync {
constexpr int32_t INVALID_SESSION_ID = -1;
class SoftbusSession {
public:
    enum DataType : int32_t {
        TYPE_BYTES,
        TYPE_FILE,
    };
    SoftbusSession(const std::string &peerDeviceId, const std::string &sessionName, DataType type);
    virtual ~SoftbusSession() = default;
    int32_t Start();
    int32_t Stop();
    int32_t SendData(const void *data, uint32_t dataLen);
    int32_t SendFile(const std::vector<std::string> &sFileList, const std::vector<std::string> &dFileList);
    int32_t GetSessionId();
    DataType GetDataType();
    std::string GetPeerDeviceId();
    int32_t WaitSessionOpened(int sessionId);

protected:
    int sessionId_{INVALID_SESSION_ID};

private:
    void CancelReleaseSessionIfNeeded();
    void CancelDelayReleaseSessionTask();
    std::string ToBeAnonymous(const std::string &name);

    std::string peerDeviceId_;
    std::string sessionName_;
    DataType type_{TYPE_BYTES};
    std::mutex sessionMutex_;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_SOFTBUS_SESSION_H