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

#ifndef OHOS_FILEMGMT_I_SOFTBUS_LISTENER_H
#define OHOS_FILEMGMT_I_SOFTBUS_LISTENER_H

#include <string>

namespace OHOS::FileManagement::CloudSync {
class ISoftbusListener {
public:
    virtual ~ISoftbusListener() = default;
    virtual int OnSessionOpened(int sesssionId, int result) = 0;
    virtual void OnSessionClosed(int sessionId) = 0;
    virtual void OnDataReceived(const std::string &senderNetworkId,
                                int receiverSessionId,
                                const void *data,
                                unsigned int dataLen) = 0;
    virtual void OnFileReceived(const std::string &senderNetworkId, const char *filePath, int result) = 0;
};
} // namespace OHOS::FileManagement::CloudSync

#endif // OHOS_FILEMGMT_I_SOFTBUS_LISTENER_H