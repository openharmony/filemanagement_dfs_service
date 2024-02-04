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

#ifndef FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_H
#define FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_H

#include "session.h"
#include <map>
#include <string>

namespace OHOS {
namespace Storage {
namespace DistributedFile {
class SoftBusHandler {
public:
    SoftBusHandler();
    ~SoftBusHandler();
    static SoftBusHandler &GetInstance();
    int32_t CreateSessionServer(const std::string &packageName, const std::string &sessionName);
    int32_t SetFileReceiveListener(const std::string &packageName,
                                   const std::string &sessionName,
                                   const std::string &dstPath);
    int32_t SetFileSendListener(const std::string &packageName, const std::string &sessionName);
    void ChangeOwnerIfNeeded(int sessionId);
    void CloseSession(int sessionId);

private:
    static inline const std::string SERVICE_NAME{"ohos.storage.distributedfile.daemon"};
    ISessionListener sessionListener_;
    IFileReceiveListener fileReceiveListener_;
    IFileSendListener fileSendListener_;
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS

#endif // FILEMANAGEMENT_DFS_SERVICE_SOFTBUS_HANDLER_H