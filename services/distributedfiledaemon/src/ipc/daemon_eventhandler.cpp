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

#include "daemon_eventhandler.h"

#include "daemon.h"
#include "utils_log.h"

namespace OHOS {
namespace Storage {
namespace DistributedFile {

DaemonEventHandler::DaemonEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
                                       const std::shared_ptr<DaemonExecute> &daemonExecute)
    : AppExecFwk::EventHandler(runner), daemonExecute_(daemonExecute)
{
    LOGI("daemon event handler create.");
}

DaemonEventHandler::~DaemonEventHandler()
{
    LOGI("daemon event handler delete.");
}

void DaemonEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        LOGE("event is nullptr.");
        return;
    }

    std::shared_ptr<DaemonExecute> daemonExecute = daemonExecute_.lock();
    if (daemonExecute == nullptr) {
        LOGE("get DaemonExecute fail.");
        return;
    }
    daemonExecute->ProcessEvent(event);
}

} // namespace DistributedFile
} // namespace Storage
} // namespace OHOS