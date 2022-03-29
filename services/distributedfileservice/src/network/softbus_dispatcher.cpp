/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "softbus_dispatcher.h"
#include "softbus_agent.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {
int SoftbusDispatcher::OnSessionOpened(int sessionId, int result)
{
    SoftbusAgent::GetInstance()->OnSessionOpened(sessionId, result);
    return 0;
}

void SoftbusDispatcher::OnSessionClosed(int sessionId)
{
    SoftbusAgent::GetInstance()->OnSessionClosed(sessionId);
}

int SoftbusDispatcher::OnSendFileFinished(int sessionId, const char *firstFile)
{
    SoftbusAgent::GetInstance()->OnSendFileFinished(sessionId, std::string(firstFile));
    return 0;
}

void SoftbusDispatcher::OnFileTransError(int sessionId)
{
    SoftbusAgent::GetInstance()->OnFileTransError(sessionId);
}

void SoftbusDispatcher::OnReceiveFileFinished(int sessionId, const char *files, int fileCnt)
{
    SoftbusAgent::GetInstance()->OnReceiveFileFinished(sessionId, std::string(files), fileCnt);
}
} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS