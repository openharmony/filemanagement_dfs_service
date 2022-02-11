/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef DFS_SOFTBUS_SESSION_DISPATCHER_H
#define DFS_SOFTBUS_SESSION_DISPATCHER_H

#include <map>
#include <memory>
#include <string>

namespace OHOS {
namespace Storage {
namespace DistributedFile {

class SoftbusDispatcher final {
public:
    SoftbusDispatcher() = delete;
    ~SoftbusDispatcher() = delete;
    static void RegisterSessionListener();
    static void UnregisterSessionListener(const std::string busName);

    static int OnSessionOpened(int sessionId, int result);
    static void OnSessionClosed(int sessionId);

    // IFileSendListener
    static int OnSendFileFinished(int sessionId, const char *firstFile);
    static void OnFileTransError(int sessionId);

    // IFileReceiveListener
    static void OnReceiveFileFinished(int sessionId, const char *files, int fileCnt);
};
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS
#endif // DFS_SOFTBUS_SESSION_DISPATCHER_H